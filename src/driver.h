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

#ifndef DRIVER_H
#define DRIVER_H

#include "geometry.h"

namespace IVD
{

class DisplayItem;
class StateManager;

class Driver
{
    StateManager* myStateManager;

protected:
    StateManager* getStateManager()
    { return myStateManager; }

public:
    Driver(): myStateManager(nullptr) {}
    virtual ~Driver() {}

    void setStateManager(StateManager* theStateManager)
    { myStateManager = theStateManager; }

    virtual void addDisplayItem(DisplayItem* item) = 0;
    virtual void removeDisplayItem(DisplayItem* item) = 0;
    virtual void processEvents() = 0;

    virtual bool checkHoverInvalidated() = 0;
    virtual DisplayItem* getWindowItemWithMouseFocus() = 0;
    virtual Coords getMousePointRelativeToWindow() = 0;

    virtual void invalidateGeometry(DisplayItem* item) = 0;
    virtual void invalidatePosition(DisplayItem* item) = 0;
    virtual void invalidateCanvas(DisplayItem* item) = 0;
    virtual void invalidateTitleText(DisplayItem* item) = 0;
    virtual void invalidateVisibility(DisplayItem* item) = 0;

    virtual void refresh() = 0;

    virtual bool checkAnythingToDo() = 0;
};

}

#endif // DRIVER_H
