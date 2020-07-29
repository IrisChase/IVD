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
#include "allmaterials.h"
#include "displayitem.h"
#include "modelitembase.h"
#include "modelcontainer.h"
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

Environment::DisplayItemKey::DisplayItemKey(Element* elem,
                                            ModelItemBase *model):
    element(elem),
    model(model)
{}

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
        assert(!precursor.path);
        assert(baseContext->getModel());
        myStateKey.scope = baseContext->getModel();
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

DisplayItem* Environment::setupNewDisplayItem(Element* elem, ModelItemBase *model)
{
    DisplayItem* item = nullptr;
    {
        std::unique_ptr<DisplayItem> itemUniquePtr =
            std::make_unique<DisplayItem>(this,
                                          model,
                                          elem->getDefaultAttr(),
                                          elem->getPath());

        item = itemUniquePtr.get();

        instances[item] = std::move(itemUniquePtr);
    }

    {
        DisplayItemKey key(elem, model);
        keyToItemMap[key] = item;
        itemToKeyMap[item] = key;
    }

    if(model) model->internalDisplayItems.push_back(item);

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
    }

    for(auto pair : elem->getVariableInitialExpressions())
        item->setVariable(pair.first, pair.second.solve(item));

    setLayout(item); //This is for the default, because it's not always set as an attribute.

    itemsWithChangedAttributeSets.insert(item);
    return item;
}

void Environment::destroyDisplayItem(DisplayItem* item)
{
    DisplayItem*& youKillMyFather = item;



    //Hello, my name is Inigo Montoya,
    youKillMyFather->prepareToDie();



    assert(!triggerMap.count(item));

    {
        const auto key = itemToKeyMap[item];
        keyToItemMap.erase(key);
        itemToKeyMap.erase(item);
    }

    //A word on models.
    //Any model item that references this item is about to be destroyed anyway,
    // if there is a model, it's removal is what triggered this. So don't worry
    // about it's references.

    itemsWithChangedAttributeSets.erase(item);
    myStateManager.removeReferencesToDisplayItem(item);

    instances.erase(item); //Don't you just love unique_ptr?
}

void Environment::destroyModelItem(ModelItemBase* modelItem)
{
    assert(modelItem->internalDisplayItems.size()); //double delete

    for(DisplayItem* item : modelItem->internalDisplayItems)
    {
            markAsBadGeometry(item); //Uh... WHAT IF THIS IS A WINDOW???? TODO FIXME XXX
            destroyDisplayItem(item);
    }
    modelItem->internalDisplayItems.clear();

    myStateManager.removeReferencesToModel(modelItem);

    if(auto* child = modelItem->getChildContainer())
        for(auto* IgotInOneLittleFightAndMyMomGotScared : *child)
            destroyModelItem(IgotInOneLittleFightAndMyMomGotScared);
    //Child container is automatically deleted as it's in a unique ptr

    modelItem->getParentContainer()->safeToDelete(modelItem);
}

void Environment::positionDisplayItemInDrawTree(DisplayItem* item)
{
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
    
    if(posPath)
    {
        ValueKeyPath path = *posPath;
        auto target = deduceTarget(item, path);

        if(!target)
        {
            std::cerr << "IVD Runtime: Could not position [" << item->getElementPath() << "] ";

            if(item->getModel())
                std::cerr << "-> [Model] ";

            std::cerr << "within [" << path << "] "
                      << "-> [Model/Static], could not deduce parent."
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

void Environment::setLayout(DisplayItem *item)
{
    if(Default::Filter::getText(item).size())
    {
        item->setMaterial(new FallbackParagraphMaterial(item));
    }
    else if(item->getAttr().getUserToken(AttributeKey::ImagePath))
    {
        item->setMaterial(new ImageMaterial(item, myImageCache.get()));
    }
    else if(auto theType = item->getAttr().getProperty(AttributeKey::Layout))
    {
        if(theType == Property::Vbox)
            item->setMaterial(new VerticalRowLayoutMaterial(item));
        if(theType == Property::Hbox)
            item->setMaterial(new HorizontalRowLayoutMaterial(item));
        if(theType == Property::Inline)
            item->setMaterial(new InlineLayoutMaterial(item));
        if(theType == Property::FreeLayout)
            item->setMaterial(new FreeLayoutMaterial(item));
        if(theType == Property::StackLayout)
            item->setMaterial(new StackLayoutMaterial(item));
        if(theType == Property::Custom)
        {
            assert(false);
            //TODO: Require forward declarations in the compiler to stop
            // this from happening.
        }
    }
    else item->setMaterial(new SimpleMaterial(item));
}

std::optional<DisplayItem*> Environment::deduceTarget(DisplayItem *context, const ValueKeyPath key)
{
    DisplayItemKey superKey;
    superKey.model = context->getModel();

    {
        auto it = elementLookupByPath.find(key);

        if(it == elementLookupByPath.end())
            return std::optional<DisplayItem*>(); //WAit why are we using optional pointers?

        superKey.element = it->second;
    }


    //std::vector<std::string> elementChain; TODO, track the whole chain of parents
    // for debug information on steroids
    findParent:
    auto parentIterator = keyToItemMap.find(superKey);
    if(parentIterator == keyToItemMap.end())
    {
        if(superKey.model)
        {
            //Then search higher up the model tree, and for a static item if
            // that doesn't work out.
            //parentItem is Null in the case of no parent. Static and root items have
            // a DisplayItemKey with a null model member. So it all works out.
            //DON'T USE ModelContainer::getParentItem() !! It throws an exception on
            // null, it's a guarded accessor.
            superKey.model = superKey.model->getParentContainer()->parentItem;
            goto findParent;
        }

        return std::optional<DisplayItem*>();
    }

    return parentIterator->second;
}

double Environment::commonExternalAccessor(DisplayItem* context,
                                           const ScopedValueKey key,
                                           std::optional<double> value)
{
    if(key.myScope == ScopedValueKey::Scope::Model)
    {
        assert(!key.path);
        assert(key.key); //LET's have a key key

        if(value)
        {
            if(context->getModel()->isNumberConst(*key.key))
                 std::cerr << "IVD Runtime warning: Model is const for key: " << *key.key << std::endl;
            else context->getModel()->setNumber(*key.key, *value);
            return 0;
        }
        else return context->getModel()->getNumber(*key.key);
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
    myStateManager.mutateAll(States::Item::Hover, false);

    DisplayItem* root = myDriver->getWindowItemWithMouseFocus();
    if(!root) return;

    Rect mouseRect;
    mouseRect.c = myDriver->getMousePointRelativeToWindow();
    mouseRect.d = Dimens(1,1);

    root->getMaterial()->applyToColliding(mouseRect, [&](DisplayItem* item) -> bool
    {
        myStateManager.mutateIfObserved(StateKey(States::Item::Hover, item), true);

        //Exclusive.
        return true;
    });
}

void Environment::run()
{
    while(true)
    {
        reprodyne_mark_frame();

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
        while(mainEventQueue.hasEvent())
        {
            ModelEvent e = mainEventQueue.popEvent();
    
            if(e.type == ModelEvent::Type::ModelItemAdded)
            {
                for(Element* elem : elementModelLookup[e.container->getModelPath()])
                    setupNewDisplayItem(elem, e.modelItem);

                processDeferredVirtualStates();
            }
            else if(e.type == ModelEvent::Type::ModelOrderInvalidated ||
                    e.type == ModelEvent::Type::ModelItemsSwapped)
            {
                //BUT if we're not using model order... Then it should not be invalidated. Shet.

                //TODO
                //ALMOST guaranteed to be the same root, but it's not a rule.

                //OKAY OKAY I just realized. We can track, somehow, whether
                // this the set is uniform or not. Or a little less optimal,
                // whether it CAN be broken by static analysis, and place a
                // flag on the model or model item denoting it as such.


                //Note from the future: Okay so I read the above a few days ago and wasn't really sure of it's
                // significance. It's referring to the fact marking *all* of the internalDisplayItems as
                // invalid geometry is mostly likely redundant, but not absolutely. And that a neat
                // optimization would be to have a static analysis in the compiler that can guarantee that
                // we only need to invalidate just one's parent and it'll be fine.
                for(DisplayItem* item : e.modelItem->internalDisplayItems)
                {
                    markAsBadGeometry(item);
                    break;
                }

                //I mean, this might be a useful event someday for some optimization, but
                // today is not that day.
                if(e.type == ModelEvent::Type::ModelItemsSwapped)
                {
                    for(DisplayItem* item : e.modelItem2->internalDisplayItems)
                    {
                        markAsBadGeometry(item);
                        break;
                    }
                }
            }
            else if(e.type == ModelEvent::Type::ModelItemRemoved)
            {
                //XXX If you post a "ModelItemRemoved" event for a child and a parent item,
                // you unleash dragons. Events need to be compacted before processing to prevent
                // this.
                destroyModelItem(e.modelItem);
            }
            else if(e.type == ModelEvent::Type::ModelItemSet)
            {
                for(DisplayItem* item : e.modelItem->internalDisplayItems)
                {
                    myStateManager.setTriggerIfObserved(StateKey(States::Item::ModelChanged, item));
                    markAsBadGeometry(item);
                    //e.modelKey should be the field
                    //e.modelItem should be the item.

                    //But... Do we just invalidate the attributeset? How do force re-read?

                    //re: Doesn't setting this trigger state force a recompute of the attribute set,
                    // forcing it to compare changes?

                    //No... That only gets it to see *attribute* changes, we don't have a system
                    // for model changes... That shit ain't even tracked.
                }
            }
            else if(e.type == ModelEvent::Type::ModelStateSet)
            {
                myStateManager.mutateIfObserved(e.stateKey, true);
            }
            else if(e.type == ModelEvent::Type::ModelStateUnset)
            {
                myStateManager.mutateIfObserved(e.stateKey, false);
            }
            else continue;
        }

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
                    if(triggerKey.myScope == ScopedValueKey::Scope::Model && item->getModel())
                        item->getModel()->reactToTrigger(*triggerKey.key); //T R I G G E R E D
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

    elems = myComp.getElements();

    for(Element& elem : elems)
    {
        elementLookupByPath[elem.getPath()] = &elem;

        if(elem.getModelPath().size())
        {
            elementModelLookup[elem.getModelPath()].push_back(&elem);

            //Defer actual instantiation until the main loop, as it's a continuous process.
            continue;
        }

        //Spin-up static elements
        setupNewDisplayItem(&elem, nullptr);
    }

    processDeferredVirtualStates();
    return IVD_STATUS_SUCCESS;
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
    //TODO god change some of these asserts to fucking warnings will ya jesus way to bring this from
    // a 0 to an 11.
    assert(!key.path);
    assert(key.key);
    assert(key.myScope == ScopedValueKey::Scope::Model); //All we're supportin' today, f r i e n d.
    assert(context->getModel());

    return context->getModel()->getString(*key.key);
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
            setLayout(attr->revealContext());
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
        attr->setChangeAcceptor([&](AnimatableAttribute* attr)
        {
            setLayout(attr->revealContext());
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
