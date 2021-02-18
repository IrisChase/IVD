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

#ifndef STATEMANAGER_H
#define STATEMANAGER_H

#include <set>

#include "assert.h"

#include "statekey.h"
#include "virtualstatekey.h"
#include "displayitem.h"
#include "element.h"

namespace IVD
{

class StateManager;

class State
{
    //The only important thing a state does is associate a
    // set of attributes to the current system state.
    //So why not store the attribute set along with the
    // observer of this state, and tell it when
    // they've changed.
    bool active;
    uint64_t stamp;
    std::map<DisplayItem*, AttributePositionPair> observers;
    StateManager* myStateManager;

    std::optional<VirtualStateKey> myVirtualStateKey;
    std::set<VirtualStateKey> affectedVirtualKeys;

    void updateSingleObserver(DisplayItem* item, AttributePositionPair pair)
    {
        if(active) item->addAttributeSet(pair);
        else item->removeAttributeSet(pair);
    }

public:
    State(): active(false), stamp(0) {}

    void ughSetStateManager(StateManager* statemanager)
    { myStateManager = statemanager; }

    void reigsterObserver(DisplayItem* item, AttributePositionPair pair)
    {
        observers[item] = pair;
        updateSingleObserver(item, pair);
    }

    int checkObserved() { return observers.size() || affectedVirtualKeys.size(); }
    void removeObserver(DisplayItem* item) { observers.erase(item); }

    bool mutate(const bool setTo, const uint64_t theStamp);

    void syncAffectedVirtualStateKeys();
    void insertVirtualStateKey(VirtualStateKey vskey);

    void registerAffectedVirtualStateKey(VirtualStateKey key)
    { affectedVirtualKeys.insert(key); }

    void removeAffectedVirtualStateKey(VirtualStateKey vskey)
    { affectedVirtualKeys.erase(vskey); }

    std::optional<VirtualStateKey> getVirtualStateKey()
    { return myVirtualStateKey; }

    bool check() { return active; }
    uint64_t getStamp() { return stamp; }
};

class StateManager
{
    //First stamp is always 1. Unstamped states are == 0.
    uint64_t lastStamp;
    //What about compound states...
    typedef std::map<void*, State> StateRange;
    std::map<ValueKey, StateRange> states;

    std::vector<StateKey> triggerStates;
    std::map<void*, std::vector<ValueKey>> volatileStateMap;
    std::map<DisplayItem*, std::vector<StateKey>> displayItemObserverMap;

    State& initState(const StateKey key);
    State* findState(const StateKey key);
    void eraseState(const StateKey key);

    void eraseStateIfOrphaned(const StateKey key);
    void deallocateScope(void* scope);
    void attemptMutate(State* theState, const bool flag);

public:
    StateManager(): lastStamp(0) {}

    void removeReferencesToDisplayItem(DisplayItem* item);
    //void removeReferencesToModel(ModelItemBase* scope)
    //{ deallocateScope(scope); }

    void registerStateObserver(const StateKey key,
                               DisplayItem* observer,
                               AttributePositionPair pair)
    {
        displayItemObserverMap[observer].push_back(key);
        initState(key).reigsterObserver(observer, pair);
    }

    void insertVirtualState(VirtualStateKey vskey);

    bool checkState(const StateKey key)
    {
        State* state = findState(key);
        if(!state) return false;
        return state->check();
    }

    uint64_t getStamp(const StateKey key)
    {
        State* state = findState(key);
        if(!state) return 0;
        return state->getStamp();
    }

    uint64_t getLastStamp()
    { return lastStamp; }

    //False just means it's not observed, not that it mutated, specifically.
    //Kinda shoddy... The only thing that needs this is setTriggerIfObserved...
    bool mutateIfObserved(const StateKey key, const bool active);

    void mutateAll(const StateKey key, const bool active);
    
    void setTriggerIfObserved(const StateKey key)
    {
        if(mutateIfObserved(key, true))
            triggerStates.push_back(key);
    }

    void resetTriggerStates()
    {
        for(StateKey key : triggerStates)
            attemptMutate(findState(key), false);

        triggerStates.clear();
    }

    bool hasTriggerStates()
    { return triggerStates.size(); }
};

}//IVD

#endif // STATEMANAGER_H
