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
#include "widget.h"

#include "OpenImageIO/imagecache.h"

namespace IVD
{

class AttributeSet;
class Driver;


class Environment
{
    std::unique_ptr<Driver> managedDriver;
    //Oh
    Driver* myDriver;

    Compiler myComp;

    StateManager myStateManager;
    std::unique_ptr<OIIO::ImageCache, std::function<void(OIIO::ImageCache*)>> myImageCache;

    std::list<Element> elems;

    std::vector<VirtualStateKeyPrecursor> deferredVirtualStateKeys;

    std::map<DisplayItem*, std::unique_ptr<DisplayItem>> instances;

    std::map<IVD_Widget*, DisplayItem*> userOwnedWidgets;

    //This is for tracking items that need recomputing.
    std::set<DisplayItem*> itemsWithChangedAttributeSets;

    //These are for tracking attributes that have been recomputingted.
    std::set<AnimatableAttribute*> attributeWantsAnimationTick;
    std::set<AnimatableAttribute*> attributesThatHaveChanged; //No really we're changed let us out
    std::set<AnimatableAttribute*> attributesThatMutateTheDrawTree; //Not convinced this is necessary...


    std::map<DisplayItem*, std::vector<ScopedValueKey>> triggerMap;
    std::map<ValueKeyPath, Element*> elementLookupByPath;
    std::map<std::string, Element*> elementModelLookup;

    std::map<Element*, std::set<DisplayItem*>> elementToDisplayItems;

    std::map<std::string, WidgetBlueprints> widgetBlueprints;
    std::map<std::string, WidgetBlueprints> layoutBlueprints;


    void initOthers();
    void processDeferredVirtualStates();

    DisplayItem* setupNewDisplayItem(Element* elem);
    void destroyDisplayItem(DisplayItem* item);
    void positionDisplayItemInDrawTree(DisplayItem* item, IVD_Widget *parentWidget);
    void setWidget(DisplayItem* item);

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

    int loadFromIVDFile(const char* path);

    void registerWidgetBlueprints(const std::string name, const WidgetBlueprints blueprints)
    { widgetBlueprints[name] = blueprints; }

    void registerLayoutBlueprints(const std::string name, const WidgetBlueprints blueprints)
    { layoutBlueprints[name] = blueprints; }

    IVD_Widget* createWidget(const std::string name, IVD_Widget* parent);

    void destroyWidget(IVD_Widget* widget);

    void drawWidget(IVD_Widget* widget);
    void distributeCollisionPointOnWidget(IVD_Widget* widget, const Coords coords);

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
