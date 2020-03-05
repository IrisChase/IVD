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

#ifndef EVENT_H
#define EVENT_H

#include <queue>
#include <string>

#include "statekey.h"

namespace IVD
{

class ModelItemBase;
class ModelContainer;

struct ModelEvent
{
    enum class Type
    {
        ModelItemSet,           //key (Field), modelItem
        ModelStateSet,          //modelItem (nullptr if global), key (state name)
        ModelStateUnset,        //modelItem, key (state name)
        TriggerCalled,          //modelItem (nullptr if global), key (triggerName)
        ModelItemAdded,         //container, modelItem
        ModelItemRemoved,       //container, modelItem
        ModelOrderInvalidated,  //container

        ModelItemsSwapped,  //container, modelItem, modelItem2
    };
    Type type;

    ModelContainer* container;
    ModelItemBase* modelItem;
    ModelItemBase* modelItem2;

    ValueKeyPath modelKey;
    StateKey stateKey;
};

class EventQueue
{
    std::queue<ModelEvent> myQueue;

public:
    void pushEvent(ModelEvent e)
    { myQueue.push(e); }

    bool hasEvent()
    { return !myQueue.empty(); }

    ModelEvent popEvent()
    {
        ModelEvent e = myQueue.front();
        myQueue.pop();
        return e;
    }
};

}//IVD

#endif // EVENT_H
