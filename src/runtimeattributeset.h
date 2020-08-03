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

#ifndef ATTRIBUTESET_H
#define ATTRIBUTESET_H

#include <map>
#include <tuple>
#include <vector>
#include <optional>
#include <functional>
#include <array>

#include "irisutils/routine.h"

#include "referenceattributeset.h"
#include "runtimeattribute.h"
#include "statekey.h"
#include "keywords.h"

namespace IVD
{

class DisplayItem;

class RuntimeAttributeSet
{
    friend class Compiler;
    friend void ::printOutAttributes(IVD::Element);

    DisplayItem* myContext;

    std::array<AnimatableAttribute, AttributeKey::AttributeCount> attr;
    
    const ReferenceAttributeSet::DeclareModifierMap* declareModifiers;
    std::vector<const ReferenceAttributeSet::SetModifierMap*> setModifiers;

    void applyToEachAttribute(std::function<void (AnimatableAttribute&)> fun);

public:
    RuntimeAttributeSet(): myContext(nullptr) {}
    RuntimeAttributeSet(DisplayItem* context);

    void beginAttributeSetRecompute();
    void mergeIn(const ReferenceAttributeSet& other);
    void commitAttributeSetRecompute();

    void executeStateChangers();
    
    void fireSets();

    std::optional<double> getInt(ValueKey key);
    void setInteger(const ValueKey key, const double proposed);

    bool checkActive(const int key)
    { return attr[key].checkActive(); }

    std::optional<double> getInt(const int key)
    { return attr[key].getValue(); }

    void setInteger(const int key, const double proposed)
    { attr[key].setValue(proposed); }
    
    bool isConst(const int key)
    { return attr[key].checkExprIsConst(); }

    //These don't necessarily have to filter like this, but it's consistent
    // and it might make sense someday to filter them with special declare types
    // or something.
    std::optional<int> getProperty(const int key)
    { return attr[key].getProperty(); }

    std::vector<std::string> getLiteralList(const int key)
    { return attr[key].getLiteralList(); }

    std::optional<ScopedValueKey> getSingleValueKey(const int key)
    { return attr[key].getSingleValueKey(); }

    std::vector<ScopedValueKey> getValueKeyList(const int key) //Optional would be more consistent...
    { return attr[key].getValueKeyList(); }

    std::optional<std::string> getUserToken(const int key)
    { return attr[key].getUserToken(); }

    std::optional<Color> getColor(const int key)
    { return attr[key].getColor(); }
};

}//Attributes

#endif // ATTRIBUTESET_H
