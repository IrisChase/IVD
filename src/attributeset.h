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

#include "attribute.h"
#include "statekey.h"
#include "keywords.h"

namespace IVD
{

class DisplayItem;

class AttributeSet
{
    friend class Compiler;
    friend void ::printOutAttributes(IVD::Element);

    DisplayItem* myContext;
    

    std::array<Attribute, AttributeKey::AttributeCount> attr;
    
    //Set can modify but not observe.
    //Declare can observe, and provide a filter for modification,
    std::map<ValueKey, Expression> declareModifiers;
    std::map<ScopedValueKey, Expression> setModifiers;

    void mergeModifiers(const AttributeSet& other);


public:
    AttributeSet(): myContext(nullptr) {}
    AttributeSet(DisplayItem* context);
    AttributeSet(DisplayItem* context, const AttributeSet& other):
        myContext(context),
        attr(other.attr)
    {}


    Attribute getAttribute(const int key) const
    { return attr[key]; }

    void insertAttribute(Attribute theAttribute, const int key)
    { attr[key] = theAttribute; }

    void insertDeclareModifier(ValueKey target, Expression expr)
    { declareModifiers[target] = expr; }

    void insertSetModifier(ScopedValueKey target, Expression expr)
    { setModifiers[target] = expr; }

    void applyToEachScopedValueKey(std::function<void(ScopedValueKey&)> fun);

    void deriveFrom(const AttributeSet& other);
    void mergeIn(const AttributeSet& other);
    void updateToReferenceSet(const AttributeSet& other);

    void executeStateChangers();
    
    void fireSets();

    std::optional<double> getInt(ValueKey key);
    void setInteger(const ValueKey key, const double proposed);

    bool checkActive(const int key)
    { return attr[key].active; }

    std::optional<double> getInt(const int key)
    { return attr[key].getValue(); }



    void setInteger(const int key, const double proposed)
    { attr[key].setValue(proposed); }
    
    bool isConst(const int key)
    { return !attr[key].expr->checkContainsWeak(); }

    std::optional<int> getProperty(const int key)
    { return attr[key].property; }

    std::optional<ScopedValueKey> getSingleValueKey(const int key)
    { return attr[key].singleKey; }

    std::vector<ScopedValueKey> getValueKeyList(const int key) //Optional would be more consistent...
    { return attr[key].keys; }

    std::optional<std::string> getUserToken(const int key)
    { return attr[key].literal; }

    std::optional<Color> getColor(const int key)
    { return attr[key].color; }
};

}//Attributes

#endif // ATTRIBUTESET_H
