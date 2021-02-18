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

#ifndef IVD_STATEKEY_H
#define IVD_STATEKEY_H

#include "irisutils/lexcompare.h"
#include "valuekey.h"

namespace IVD
{

class DisplayItem;

struct StateKey
{
    ValueKey identity;
    void* scope;

    StateKey(): scope(nullptr) {}
    StateKey(const ValueKey key):
        identity(key),
        scope(nullptr)
    {}
    
    StateKey(const ValueKey theIdentity, void* theScope):
        identity(theIdentity),
        scope(theScope)
    {}

    IRISUTILS_DEFINE_COMP(StateKey, identity, scope)
};

}//IVD

#endif // IVD_STATEKEY_H
