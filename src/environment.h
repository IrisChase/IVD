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

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <string>
#include <vector>
#include <list>

#include "compiler.h"
#include "statemanager.h"
#include "displayitem.h"
#include "element.h"
#include "statekey.h"
#include "virtualstatekey.h"
#include "event.h"

#include "OpenImageIO/imagecache.h"

namespace IVD
{

class AttributeSet;
class Driver;


class Environment
{
    friend EventQueue& getEventQueue(const Environment&);
    //Oh
    std::unique_ptr<Driver> managedDriver;
    Driver* myDriver;
    
    struct DisplayItemKey
    {
        Element* element;
        ModelItemBase* model;

        DisplayItemKey(): element(nullptr), model(nullptr) {}
        DisplayItemKey(Element* elem, ModelItemBase* model);

        friend bool operator<(const DisplayItemKey& left, const DisplayItemKey& right)
        { return std::tie(left.element, left.model) < std::tie(right.element, right.model); }
    };

    Compiler myComp;


    EventQueue mainEventQueue;
    StateManager myStateManager;
    std::unique_ptr<OIIO::ImageCache, std::function<void(OIIO::ImageCache*)>> myImageCache;

    std::list<Element> elems;

    std::vector<VirtualStateKeyPrecursor> deferredVirtualStateKeys;

    std::map<DisplayItem*, std::unique_ptr<DisplayItem>> instances;

    std::vector<std::unique_ptr<ModelContainer>> rootContainers;

    std::map<DisplayItemKey, DisplayItem*> keyToItemMap;
    std::map<DisplayItem*, DisplayItemKey> itemToKeyMap;

    //This is for tracking items that need recomputing.
    std::set<DisplayItem*> itemsWithChangedAttributeSets;

    //These are for tracking attributes that have been recomputingted.
    std::set<AnimatableAttribute*> attributeWantsAnimationTick;
    std::set<AnimatableAttribute*> attributesThatHaveChanged; //No really we're changed let us out
    std::set<AnimatableAttribute*> attributesThatMutateTheDrawTree; //Not convinced this is necessary...


    std::map<DisplayItem*, std::vector<ScopedValueKey>> triggerMap;
    std::map<ValueKeyPath, Element*> elementLookupByPath;
    std::map<ValueKeyPath, std::vector<Element*>> elementModelLookup;

    void initOthers();
    void processDeferredVirtualStates();

    DisplayItem* setupNewDisplayItem(Element* elem, ModelItemBase* model);
    void destroyDisplayItem(DisplayItem* item);
    void destroyModelItem(ModelItemBase* item);
    void positionDisplayItemInDrawTree(DisplayItem* item);
    void setLayout(DisplayItem* item);

    std::optional<DisplayItem *> deduceTarget(DisplayItem* context, const ValueKeyPath key);

    double commonExternalAccessor(DisplayItem* context,
                                  const ScopedValueKey key,
                                  std::optional<double> value);

    void markAsBadGeometry(DisplayItem* item);
    void markAsBadCanvas(DisplayItem* item);
    void updateHover();

public:
    Environment();

    //The big one
    void run();

    ModelContainer* instantiateModel(const std::string& name)
    {
        rootContainers.emplace_back(std::make_unique<ModelContainer>(&mainEventQueue, name));
        return rootContainers.back().get();
    }

    int loadFromIVDFile(const char* path);

    const char* getCompilerErrors()
    { return myComp.getErrorMessageDigest().c_str(); }

    double getInteger(DisplayItem* context, const ScopedValueKey key);

    void setInteger(DisplayItem* context, const ScopedValueKey key, const double val)
    { commonExternalAccessor(context, key, val); }

    std::string getString(DisplayItem* context, const ScopedValueKey key);

    void markAsChangedAttributes(DisplayItem* item)
    { itemsWithChangedAttributeSets.insert(item); }

    void setupEnvironmentCallbacksOnAttributeForKey(AnimatableAttribute* attr, const int key);

    StateKey generateStateKeyFromPrecursor(ScopedValueKey precursor, DisplayItem* baseContext);
};

}//IVD


#endif // ENVIRONMENT_H
