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

#include "statemanager.h"

namespace IVD
{

bool State::mutate(const bool setTo, const uint64_t theStamp)
{
    if(active == setTo) return false;
    active = setTo;
    stamp = theStamp;
    for(auto pair : observers) updateSingleObserver(pair.first, pair.second);
    syncAffectedVirtualStateKeys();
    return true;
}

void State::syncAffectedVirtualStateKeys()
{
    for(auto vskey : affectedVirtualKeys)
        vskey.syncProxyState(myStateManager);
}

void State::insertVirtualStateKey(VirtualStateKey vskey)
{
    myVirtualStateKey = vskey;
    myVirtualStateKey->syncProxyState(myStateManager);
}

State& StateManager::initState(const StateKey key)
{
    if(key.scope) volatileStateMap[key.scope].push_back(key.identity);
    State& theState = states[key.identity][key.scope];
    theState.ughSetStateManager(this);
    return theState;
}

State* StateManager::findState(const StateKey key)
{
    auto rangeIt = states.find(key.identity);
    if(rangeIt == states.end()) return nullptr;

    StateRange& range = rangeIt->second;
    auto scopeIt = range.find(key.scope);
    if(scopeIt == range.end()) return nullptr;

    return &scopeIt->second;
}

void StateManager::eraseState(const StateKey key)
{
    states[key.identity].erase(key.scope);
}

void StateManager::eraseStateIfOrphaned(const StateKey key)
{
    auto rangeIterator = states.find(key.identity);
    if(rangeIterator == states.end()) return;

    auto stateIt = rangeIterator->second.find(key.scope);
    if(stateIt == rangeIterator->second.end()) return;

    State* state = &stateIt->second;
    if(!state->checkObserved())
    {
        if(stateIt->second.getVirtualStateKey())
        {
            //Honest to god I want to know if this condition is ever met.
            auto vskey = *stateIt->second.getVirtualStateKey();
            for(StateKey affectedStateKey : vskey.getAffectedKeys())
            {
                State* affectedState = findState(affectedStateKey);
                if(!affectedState) continue;

                affectedState->removeAffectedVirtualStateKey(vskey);
                eraseStateIfOrphaned(affectedStateKey);
            }
        }

        rangeIterator->second.erase(stateIt);

        //Erase range if orphaned
        if(rangeIterator->second.empty())
            states.erase(rangeIterator);
    }
}

void StateManager::deallocateScope(void* scope)
{
    for(ValueKey key : volatileStateMap[scope])
        eraseStateIfOrphaned(StateKey(key, scope));

    volatileStateMap.erase(scope);
}

void StateManager::attemptMutate(State* theState, const bool flag)
{
    uint64_t tryStamp = lastStamp + 1;

    if(theState->mutate(flag, tryStamp))
    {
        lastStamp = tryStamp; //Commit
    }
}

void StateManager::removeReferencesToDisplayItem(DisplayItem* item)
{
    //Remove any states on the item.
    deallocateScope(item);

    //Remove item from any states it observes.
    for(StateKey key : displayItemObserverMap[item])
    {
        State* state = findState(key);
        findState(key)->removeObserver(item);

        eraseStateIfOrphaned(key);
    }

    displayItemObserverMap.erase(item);
}

void StateManager::insertVirtualState(VirtualStateKey vskey)
{
    State* proxyState = findState(vskey.proxyStateKey);
    assert(proxyState); //I mean it could be a no-op otherwise, but why?

    proxyState->insertVirtualStateKey(vskey);

    for(StateKey key : vskey.getAffectedKeys())
    {
        State* affectedState = findState(key);

        //If the affected state doesn't exist, it means that this hasn't been run
        // and or the state is not observed directly by a DisplayItem. But it could
        // still be mutated and virtual keys count as observers.
        if(!affectedState) affectedState = &initState(key);

        affectedState->registerAffectedVirtualStateKey(vskey);
    }
}

bool StateManager::checkAny(const StateKey key)
{
    for(auto range : states.at(key.identity))
    {
        if(range.second.check())
            return true;
    }
    return false;
}

bool StateManager::mutateIfObserved(const StateKey key, const bool active)
{
    auto state = findState(key);
    if(!state) return false;

    assert(state->checkObserved());

    attemptMutate(state, active);
    return true;
}

void StateManager::mutateAll(const StateKey key, const bool active)
{
    for(auto& pair : states[key.identity])
    {
        void* scope = pair.first;
        State& state = pair.second;

        //Ignore the global, it's semantically independent and inherently singular
        if(!scope) continue;

        attemptMutate(&state, active);
    }
}

}//IVD
