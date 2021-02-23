// Copyright 2020 Iris Chase
//
// Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include "environment.h"
#include "displayitem.h"
#include "states.h"
#include "defaults.h"
#include "driver.h"

#include <reprodyne.h>

#include "user_include/IVD_status.h"

#include <iostream>
#include <chrono>
#include <thread>

namespace IVD
{


Driver* createDefaultDriver();

Environment::Environment():
    managedDriver(createDefaultDriver()),
    myDriver(managedDriver.get())
{
    initOthers();
}

void Environment::initOthers()
{
    myImageCache = typeof (myImageCache){OIIO::ImageCache::create(),
                                         [](OIIO::ImageCache* cache) { OIIO::ImageCache::destroy(cache); }};
    managedDriver->setStateManager(&myStateManager);
}

StateKey Environment::generateStateKeyFromPrecursor(ScopedValueKey precursor, DisplayItem* baseContext)
{
    assert(precursor.key);

    StateKey myStateKey;
    myStateKey.identity = *precursor.key;

    if(precursor.myScope == ScopedValueKey::Scope::Element)
        myStateKey.scope = baseContext;
    else if(precursor.myScope == ScopedValueKey::Scope::Model)
    {
        assert(false);
    }
    //Global is default, scope is nullptr already.

    if(precursor.path)
    {
        auto optionalTarget = deduceTarget(baseContext, *precursor.path);
        assert(optionalTarget);
        myStateKey.scope = *optionalTarget;
    }

    return myStateKey;
}

void Environment::processDeferredVirtualStates()
{
    for(auto vskeyp : deferredVirtualStateKeys)
    {
        VirtualStateKey vskey = vskeyp.generateVirtualStateKey(this);
        myStateManager.insertVirtualState(vskey);
    }

    deferredVirtualStateKeys.clear();
}

void Environment::processDeferredPositioning()
{
    for(auto pair : deferredPositioning)
        positionDisplayItemInDrawTree(pair.item, pair.parent);
    processDeferredVirtualStates();
}

DisplayItem* Environment::setupNewDisplayItem(Element* elem)
{
    DisplayItem* item = nullptr;
    {
        std::unique_ptr<DisplayItem> itemUniquePtr =
            std::make_unique<DisplayItem>(elem,
                                          this,
                                          &myStateManager,
                                          elem->getDefaultAttr(),
                                          elem->getPath(),
                                          elem->getElementStamp());

        item = itemUniquePtr.get();

        instances[item] = std::move(itemUniquePtr);
    }

    elementToDisplayItems[elem].insert(item);

    for(auto val : elem->getKeyedAttributeMap())
    {
        const auto preKey = val.first;
        const auto pos = val.second;
        const auto pair = elem->at(pos);

        myStateManager.registerStateObserver(generateStateKeyFromPrecursor(preKey, item), item, pair);
    }

    for(VirtualStateKeyPrecursor vskeyp : elem->getVirtualKeys())
    {
        vskeyp.context = item;
        deferredVirtualStateKeys.push_back(vskeyp);

        //These are deferred, because they might reference other states that haven't
        // been registered yet, and they'll miss the hook otherwise. Items are instantiated
        // in batches in lots of cases, the batches having the necessary states.
        //We can't even create the virtualstatekey, because it may reference states
        // that we haven't *inserted* yet.
        //Is this still true after the model rewrite? ~~ feb 2021
    }

    for(auto pair : elem->getVariableInitialExpressions())
        item->setVariable(pair.first, pair.second.solve(item));

    itemsWithChangedAttributeSets.insert(item);
    return item;
}

void Environment::destroyDisplayItem(DisplayItem* item)
{
    DisplayItem*& youKillMyFather = item;



    //Hello, my name is Inigo Montoya,
    youKillMyFather->prepareToDie();


    assert(!triggerMap.count(item));


    //A word on widgets.
    //Any widget item that references this item is about to be destroyed anyway,
    // if there is a widget, it's removal is what triggered this. So don't worry
    // about it's references.


    itemsWithChangedAttributeSets.erase(item);
    myStateManager.removeReferencesToDisplayItem(item);

    elementToDisplayItems.at(item->getElement()).erase(item);
    instances.erase(item); //Don't you just love unique_ptr?
}

void Environment::positionDisplayItemInDrawTree(DisplayItem* item, IVD_Widget* parentWidget = nullptr)
{
    if(!parentWidget && userOwnedWidgets.count(item->getWidget()))
        return; //permanent custody

    const auto posPath = Default::Filter::getPositionWithin(item);
    
    if(posPath && posPath->size() && posPath->at(0) == "Environment")
    {
        if(item->getParent())
        {
            myDriver->invalidateGeometry(item->getParent());
            item->deparent();
        }

        myDriver->addDisplayItem(item);
        myDriver->invalidateGeometry(item);
        return;
    }
    
    myDriver->removeDisplayItem(item);
    
    if(parentWidget)
    {
        DisplayItem* parentItem = userOwnedWidgets.at(parentWidget);
        item->setParent(parentItem);
        markAsBadGeometry(parentItem);
    }
    else if(posPath)
    {
        ValueKeyPath path = *posPath;
        auto target = deduceTarget(item, path);

        if(!target)
        {
            std::cerr << "IVD Runtime: Could not position [" << item->getElementPath() << "] ";

            if(userOwnedWidgets.count(item->getWidget()))
                std::cerr << "-> [Widget] ";

            std::cerr << "within [" << path << "] "
                      << "-> [Widget/Static], could not deduce parent."
                      << std::endl;
            return;
        }

        if(item->getParent())
            myDriver->invalidateGeometry(item->getParent());

        item->setParent(*target);
        myDriver->invalidateGeometry(*target);
    }
    else
    {
        myDriver->invalidateGeometry(item->getParent());
        item->deparent();
    }
}

void Environment::setWidget(DisplayItem *item)
{
    if(userOwnedWidgets.count(item->getWidget()))
        return; //it be bound by BLOOD

    item->destroyWidget();

    WidgetBlueprints blueprints;

    if(auto optionalLayoutName = item->getAttr().getUserToken(AttributeKey::Layout))
    {
        blueprints = layoutBlueprints.at(*optionalLayoutName);
    }
    else if(auto optionalWidgetName = item->getAttr().getUserToken(AttributeKey::Widget))
    {
        blueprints = widgetBlueprints.at(*optionalWidgetName);
    }
    else return; //Otherwise, we leave it blank, because it's not actually needed ^^

    item->setupNewWidget(blueprints);
}

std::optional<DisplayItem*> Environment::deduceTarget(DisplayItem *context, const ValueKeyPath key)
{
    //This used to be a lot more sophisticated which is why it's kinda weird looking now
    // but leaving it here because I'm still unsure of the future direction.

    std::optional<DisplayItem*> result;

    Element* targetElem = elementLookupByPath[key];
    const auto count = elementToDisplayItems.at(targetElem).size();

    if(count > 1)
        std::cout << "IVD Runtime Warning: Target Ambiguous: " << key << std::endl;

    //Grab the first
    for(DisplayItem* target : elementToDisplayItems.at(targetElem))
        return target;

    return result;
}

double Environment::commonExternalAccessor(DisplayItem* context,
                                           const ScopedValueKey key,
                                           std::optional<double> value)
{
    if(key.myScope == ScopedValueKey::Scope::Model)
    {
        std::cerr << "Warning dead code path XXX" << std::endl;
        return 42;
    }


    //And NOW it can be a material function... But only on context
    assert(key.myScope == ScopedValueKey::Scope::Element);


    assert(key.key);

    DisplayItem* other = nullptr;

    if(!key.path) other = context; //Self referential.
    else
    {
        auto result = deduceTarget(context, *key.path);

        assert(result);

        other = *result;
    }

    if(auto optional = getSymbolForLiteral(*key.key))
    {
        if(value) other->getAttr().setInteger(*key.key, *value);
        else      return *context->getAttr().getInt(*key.key);
    }
    else //User variable
    {
        if(value) context->setVariable(*key.key, *value);
        else      return context->getVariable(*key.key);
    }

    //If we change anything... Yeah.
    markAsBadGeometry(context);

    return 0;
}

void Environment::markAsBadGeometry(DisplayItem *item)
{
    myDriver->invalidateGeometry(item);
}

void Environment::markAsBadCanvas(DisplayItem *item)
{
    myDriver->invalidateCanvas(item);
}

void Environment::updateHover()
{
    myStateManager.mutateAll(States::Item::HoverExclusive, false);
    myStateManager.mutateAll(States::Item::HoverInclusive, false);

    DisplayItem* root = myDriver->getWindowItemWithMouseFocus();
    if(!root) return;

    root->updateHover();
}

Coords Environment::getMouseOffsetRelativeToWindow()
{ return myDriver->getMousePointRelativeToWindow(); }

void Environment::run()
{
    while(true)
    {
        reprodyne_mark_frame();

        processDeferredPositioning();

        const uint64_t lastStateStampBeforeLoop = myStateManager.getLastStamp();

        {
            auto mySet = itemsWithChangedAttributeSets;
            itemsWithChangedAttributeSets.clear();

            for(DisplayItem* item : mySet)
            {
                item->recomputeAttributeSet();
                item->getAttr().executeStateChangers();
                item->getAttr().fireSets();
            }
        }


        for(AnimatableAttribute* a : attributeWantsAnimationTick)
        {
            //There's a bug in GCC's std::set that I haven't been able to reliably reproduce to report.
            // (Aggressive compiler optimizations probably makes isolation difficult...)
            //But without this conditional, it crashes -somtimes-.
            if(!attributeWantsAnimationTick.size()) break;
            a->animationTick();
        }
        //HAS to come after recomputeAttributeSet, which accepts changes
        myStateManager.resetTriggerStates();

        //------------------------------------------------------------Draw tree frozen beyond this point

        //These take precedence because the others will reference windows that otherwise
        // don't exist...

        {
            auto myDrawTreeMutatingAttributes = attributesThatMutateTheDrawTree;
            attributesThatMutateTheDrawTree.clear();
            for(AnimatableAttribute* attr : myDrawTreeMutatingAttributes)
            {
                attr->executeChangeAcceptor();
            }

            //Do the do
            auto myAttributesChanged = attributesThatHaveChanged;
            attributesThatHaveChanged.clear();
            for(AnimatableAttribute* attr : myAttributesChanged)
            {
                attr->executeChangeAcceptor();
            }
        }

        {
            bool die = false;
            for(auto it : triggerMap)
            {
                DisplayItem* item = it.first;
                for(auto triggerKey : it.second) //Usually only one, but w/e
                {
                    if(triggerKey.myScope == ScopedValueKey::Scope::Model)
                        item->reactToTrigger(*triggerKey.key);

                    //Elemental scope means nothing... But it's the default if nothing else is specified.
                    //Soo... We just assume anything that's not model is global. It's hacky, but eh.
                    //It works for now without reworking part of the compiler...
                    else
                    {
                        if(triggerKey.key == Default::quitTrigger)
                            die = true; //Guarantee that all triggers get called. Safe shutdown, etc...
                        //else trigger unrecognized
                    }
                }
            }
            triggerMap.clear();//Mother FUCKER
            if(die) break;
        }

        myDriver->processEvents();
        if(myDriver->checkHoverInvalidated()) updateHover();
        myDriver->refresh(); //<--- This is expected to handle reprodyne video frame validation

        if(!attributeWantsAnimationTick.size() &&
           !myDriver->checkAnythingToDo() &&
           lastStateStampBeforeLoop == myStateManager.getLastStamp())
        {
            //Sleepy pea.
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}

int Environment::loadFromIVDFile(const char* path)
{
    if(!myComp.compileFile(path)) return IVD_STATUS_FILE_NOT_FOUND;
    if(myComp.getErrorMessages().size()) return IVD_STATUS_COMPILE_ERROR;

    for(Element& elem : myComp.getElements())
    {
        elementLookupByPath[elem.getPath()] = &elem;

        if(elem.getModelPath().size())
        {
            //DIRTY HAXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
            //FIX THE COMPILER
            elementModelLookup[elem.getModelPath()[0]] = &elem;

            //Defer actual instantiation until the main loop, as it's a continuous process.
            continue;
        }

        //Spin-up static elements
        setupNewDisplayItem(&elem);
    }

    processDeferredVirtualStates();
    return IVD_STATUS_SUCCESS;
}

IVD_Widget* Environment::createWidget(const std::string name, IVD_Widget* parent)
{
    DisplayItem* item = setupNewDisplayItem(elementModelLookup[name]);
    IVD_Widget* widget = item->setupNewWidget(widgetBlueprints.at(name));

    userOwnedWidgets[widget] = item;

    //We have to defer parenting because otherwise
    // the children potentially created in the constructor
    // of widget will be trying to position in a parent not
    // yet registered in userOwnedWidgets.
    deferredPositioning.push_back({item, parent});
}

IVD_Element* Environment::createIVDelementFromClass(const std::string className, IVD_Widget* parent)
{
    Element* classElement = myComp.getElementForClass(className);
    if(!classElement || !parent)
    {
        //todo runtime error
        return nullptr;
    }

    DisplayItem* item = setupNewDisplayItem(classElement);
    deferredPositioning.push_back({item, parent});

    return reinterpret_cast<IVD_Element*>(item);
}

void Environment::destroyWidget(IVD_Widget* widget)
{
    DisplayItem* item = userOwnedWidgets.at(widget);
    userOwnedWidgets.erase(widget);

    if(widgetOwnedDisplayItems.count(widget))
    {
        for(DisplayItem* item : widgetOwnedDisplayItems.at(widget))
            destroyDisplayItem(item);
    }

    markAsBadGeometry(item); //okayyyy is this safe AT ALL?? With models there was a comment about root windows XXX
    destroyDisplayItem(item); //Calls the dtor in the blueprints.
    //Unlike construction, child destruction should be straight forward.
}

void Environment::destroyIVDelement(IVD_Widget* parent, IVD_Element* elem)
{
    if(!parent)
    {
        //todo
        std::terminate(); //HOLD IT RIGHT THERE
    }

    DisplayItem* item = reinterpret_cast<DisplayItem*>(elem);
    widgetOwnedDisplayItems.at(parent).erase(item);

    markAsBadGeometry(item);
    destroyDisplayItem(item);
}


double Environment::getInteger(DisplayItem* context, const ScopedValueKey key)
{
    if(key.myScope == ScopedValueKey::Scope::Global &&
       key.path &&
       key.path->size() == 1 &&
       key.path->front() == Default::MousePath)
    {
        assert(key.key);

        //Should this be relative to the item or the window?
        // by default it's relative to window... TODO
        if(key.key == getLiteralForSymbol(AttributeKey::TranslationO))
            return myDriver->getMousePointRelativeToWindow().x;
        if(key.key == getLiteralForSymbol(AttributeKey::TranslationA))
            return myDriver->getMousePointRelativeToWindow().y;
        //TODO needs scroll distance and there is no keyword for it.
        //Not a mouse
    }

    return commonExternalAccessor(context, key, std::optional<double>());
}

std::string Environment::getString(DisplayItem* context, const ScopedValueKey key)
{
    return "Dead code path";
}

void Environment::setupEnvironmentCallbacksOnAttributeForKey(AnimatableAttribute* attr, const int key)
{
    //Maybe check if even active?

    //Bleh, these should all be stored lambdas, and just send a reference to the attributes.......
    using namespace AttributeKey;
    attr->setAnimationTickRequester([&](AnimatableAttribute* attr)
    { attributeWantsAnimationTick.insert(attr); });

    attr->setCancelAnimationTicker([&](AnimatableAttribute* attr)
    { attributeWantsAnimationTick.erase(attr); });

    if(key == PositionWithin)
    {
        attr->setSignalChangedAttribute([&](AnimatableAttribute* attr)
        { attributesThatMutateTheDrawTree.insert(attr); });
    }
    else
    {
        attr->setSignalChangedAttribute([&](AnimatableAttribute* attr)
        { attributesThatHaveChanged.insert(attr); });
    }

    //Just a little helper for the state changing attributes~
    auto applyToStates = [&](AnimatableAttribute* attr, std::function<void(const StateKey key)> fun)
    {
        auto states = attr->getValueKeyList();
        if(!states.size()) return; //TODO Should this... EVER be possible? Assert here fails.

        for(const ScopedValueKey pre : states)
        {
            const StateKey key = generateStateKeyFromPrecursor(pre, attr->revealContext());
            fun(key);
        }
    };


    switch(key)
    {
    case PositionWithin:
        attr->setChangeAcceptor([&](AnimatableAttribute* attr)
        {
            positionDisplayItemInDrawTree(attr->revealContext());
        });
        break;

    case TitleText:
        attr->setChangeAcceptor([&](AnimatableAttribute* attr)
        {
            myDriver->invalidateTitleText(attr->revealContext());
        });
        break;

    case Text:
    case ImagePath:
        attr->setChangeAcceptor([&](AnimatableAttribute* attr)
        {
            markAsBadGeometry(attr->revealContext());
        });
        break;

    case Font:
    case FontSize:
    case TranslationO:
    case TranslationA:
    case MarginOppOut:
    case MarginOppIn:
    case MarginAdjIn:
    case MarginAdjOut:
    case PaddingOppOut:
    case PaddingOppIn:
    case PaddingAdjIn:
    case PaddingAdjOut:
    case SizeO:
    case SizeA:
    case AttributeKey::Orientation:
    case WindowSizeStrategy:
    case CellNames:
    case Justify:
    case AlignAdjacent:
    case AlignOpposite:
    case OverrideFillPrecedenceAdjacent:
    case OverrideFillPrecedenceOpposite:
        attr->setChangeAcceptor([&](AnimatableAttribute* attr)
        {
            markAsBadGeometry(attr->revealContext());
        });
        break;

    case Visibility:
        attr->setChangeAcceptor([&](AnimatableAttribute* attr)
        {
            myDriver->invalidateVisibility(attr->revealContext());
        });
        break;

    case Borderless:
    case Resizable:
        attr->setChangeAcceptor([&](AnimatableAttribute* attr)
        {
            assert(false);
        });
        break;

    case ElementColor:
    case FontColor:
    case BorderColor:
        attr->setChangeAcceptor([&](AnimatableAttribute* attr)
        {
            markAsBadCanvas(attr->revealContext());
        });
        break;

    case Layout:
    case Widget:
        attr->setChangeAcceptor([&](AnimatableAttribute* attr)
        {
            setWidget(attr->revealContext());
        });
        break;

    case WindowState:
        attr->setChangeAcceptor([&](AnimatableAttribute* attr)
        {
            //assert(false); //I dunno
        });
        break;

    case InduceState:
        attr->setChangeAcceptor([&, applyToStates](AnimatableAttribute* attr)
        {
            applyToStates(attr, [&](const StateKey key)
            {
                myStateManager.mutateIfObserved(key, true);
            });
        });
        break;

    case BindState:
        attr->setChangeAcceptor([&](AnimatableAttribute* attr)
        {
            //silent failure... Feck.
        });
        break;

    case ToggleState:
        attr->setChangeAcceptor([&, applyToStates](AnimatableAttribute* attr)
        {
            applyToStates(attr, [&](const StateKey key)
            {
                if(myStateManager.checkState(key))
                    myStateManager.mutateIfObserved(key, false);
                else
                    myStateManager.mutateIfObserved(key, true);
            });
        });
        break;

    case UnsetState:
        attr->setChangeAcceptor([&, applyToStates](AnimatableAttribute* attr)
        {
            applyToStates(attr, [&](const StateKey key)
            {
                myStateManager.mutateIfObserved(key, false);
            });
        });
        break;

    case TriggerState:
        attr->setChangeAcceptor([&, applyToStates](AnimatableAttribute* attr)
        {
            applyToStates(attr, [&](const StateKey key)
            {
                myStateManager.setTriggerIfObserved(key);
            });
        });
        break;

    case RadioState:
        attr->setChangeAcceptor([&, applyToStates](AnimatableAttribute* attr)
        {
            StateKey topKey;
            uint64_t lastStamp = 0;
            int count = 0;

            applyToStates(attr, [&](const StateKey key)
            {
                if(!myStateManager.checkState(key)) return;

                const uint64_t myStamp = myStateManager.getStamp(key);

                if(myStamp > lastStamp)
                {
                    lastStamp = myStamp;
                    topKey = key;
                }

                ++count;
            });

            if(count > 1)
            {
                applyToStates(attr, [&](const StateKey key)
                {
                    if(key == topKey) return;

                    myStateManager.mutateIfObserved(key, false);
                });
            }
        });
        break;

    case Triggers:
        attr->setChangeAcceptor([&](AnimatableAttribute* attr)
        {
            DisplayItem* item = attr->revealContext();
            auto triggers = item->getAttr().getValueKeyList(AttributeKey::Triggers);
            if(triggers.size()) triggerMap[item] = triggers;
        });
        break;
    }

}

}//IVD
