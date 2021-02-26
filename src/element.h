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
    int elementStamp = -42;
    ValueKeyPath path;
    ValueKeyPath modelPath;

    ReferenceAttributeSet defaultAttr;
    
    std::vector<ReferenceAttributeSet> keyedAttributes;
    std::map<ScopedValueKey, int> keyedAttributeMap;

    std::vector<VirtualStateKeyPrecursor> virtualKeys;

    std::map<ValueKey, Expression> variableInitialExpressions;
    
    //I have no idea what this is doing anymore
    int obtainPosForKey(const ScopedValueKey key)
    {
        auto it = keyedAttributeMap.find(key);
        
        if(it == keyedAttributeMap.end())
        {
            const int pos = keyedAttributes.size(); 
            keyedAttributes.emplace_back(defaultAttr.size());
            keyedAttributeMap[key] = pos;
            
            return pos;
        }
        
        return it->second;
    }
    
public:
    Element(const int stamp, const int attributeCount):
        elementStamp(stamp),
        defaultAttr(attributeCount)
    {}

    int getElementStamp() { return elementStamp; }

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

        //ALRIGHT SO
        //This is broken TODO
        //I don't remember when I broke it, but it's been broken since I broke IVD off
        // from the original repo.
        //I know it used to work because there is still a validation test that some ancient
        // build produced correctly.
        //I spent a while trying to track this down, thinking I recently broke something and
        // there was some sort of missing ampersand or something, but nope. Been like this since
        // the initial commit.
        //I'm not going to do anything about it right now because obviously things were (mostly)
        // working before I started the refactor I'm at the tail-end of now, and I just wanna get
        // back to that state first. I'm leaving this comment so that I don't waste time in the future.
        //Basically, the below line doesn't work because we want to merge states that have equivalent
        // virtual keys. This is non-trivial, obviously.
        //I vaguely remember deleting the code thinking it was kludgy? I don't remember why. Or if I didn't
        // realize what I was doing. But yeaaaah it doesn't work.
        //for(auto& vk : parentClass.virtualKeys) virtualKeys.push_back(vk);
    }
};

}//IVD

#endif // ELEMENT_H
