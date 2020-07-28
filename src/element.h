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

#ifndef ELEMENT_H
#define ELEMENT_H

#include <set>
#include <map>

#include "assert.h"

#include "statekey.h"
#include "referenceattributeset.h"
#include "modelcontainer.h"
#include "attributepositionpair.h"
#include "virtualstatekey.h"

namespace IVD
{

class DisplayItem;


//Element probably isn't the best term for this, because
// they're still essentially templates for instances of
// DisplayItems.
class Element
{
    ValueKeyPath path;
    ValueKeyPath modelPath;

    ReferenceAttributeSet defaultAttr;
    
    std::vector<ReferenceAttributeSet> keyedAttributes;
    std::map<ScopedValueKey, int> keyedAttributeMap;

    std::vector<VirtualStateKeyPrecursor> virtualKeys;

    std::map<ValueKey, Expression> variableInitialExpressions;
    
    int obtainPosForKey(const ScopedValueKey key)
    {
        auto it = keyedAttributeMap.find(key);
        
        if(it == keyedAttributeMap.end())
        {
            const int pos = keyedAttributes.size(); 
            keyedAttributes.emplace_back();
            keyedAttributeMap[key] = pos;
            
            return pos;
        }
        
        return it->second;
    }
    
public:
    const ValueKeyPath& getPath() const
    { return path; }

    void setPath(const ValueKeyPath& theName)
    { path = theName; }

    void setModelPath(ValueKeyPath key)
    { modelPath = key; }

    void applyToEachScopedValueKey(std::function<void(ScopedValueKey&)> fun)
    {
        defaultAttr.applyToEachScopedValueKey(fun);
        for(auto& attr : keyedAttributes) attr.applyToEachScopedValueKey(fun);
    }

    void addVirtualStateKey(VirtualStateKeyPrecursor vskey) { virtualKeys.push_back(vskey); }

    void setInitialExpression(const ValueKey key, Expression initial)
    { variableInitialExpressions[key] = initial; }

    auto getVariableInitialExpressions()
    { return variableInitialExpressions; }

    std::vector<VirtualStateKeyPrecursor> getVirtualKeys() { return virtualKeys; }
    ReferenceAttributeSet& getDefaultAttr() { return defaultAttr; }
    std::map<ScopedValueKey, int> getKeyedAttributeMap() { return keyedAttributeMap; }
    ValueKeyPath getModelPath() const { return modelPath; }
    
    ReferenceAttributeSet& getAttributeSetForStateKey(ScopedValueKey statePre)
    {
        if(!statePre.key) return defaultAttr;
        
        const int pos = obtainPosForKey(statePre);
        return keyedAttributes.at(pos);
    }
    
    AttributePositionPair at(const int pos)
    { return AttributePositionPair(pos, &keyedAttributes.at(pos)); }

    void deriveFrom(Element& parentClass)
    {
        defaultAttr.deriveFrom(parentClass.defaultAttr);

        for(const auto& pair : parentClass.keyedAttributeMap)
        {
            const auto key = pair.first;
            const int myPos = obtainPosForKey(key);
            const int otherPos = parentClass.obtainPosForKey(key);

            keyedAttributes.at(myPos).deriveFrom(parentClass.keyedAttributes.at(otherPos));
        }
    }
};

}//IVD

#endif // ELEMENT_H
