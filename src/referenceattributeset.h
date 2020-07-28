#pragma once

#include <array>
#include "keywords.h"
#include "referenceattribute.h"

namespace IVD
{

struct ReferenceAttributeSet
{
    typedef std::map<ValueKey, Expression> DeclareModifierMap;
    typedef std::map<ScopedValueKey, Expression> SetModifierMap;


    std::array<ReferenceAttribute, AttributeKey::AttributeCount> attr;

    //Set can modify but not observe.
    //Declare can observe, and provide a filter for modification,
    DeclareModifierMap declareModifiers;
    SetModifierMap setModifiers;

    ReferenceAttribute getAttribute(const int key) const
    { return attr[key]; }

    void insertAttribute(ReferenceAttribute theAttribute, const int key)
    { attr[key] = theAttribute; }

    void insertDeclareModifier(ValueKey target, Expression expr)
    { declareModifiers[target] = expr; }

    void insertSetModifier(ScopedValueKey target, Expression expr)
    { setModifiers[target] = expr; }

    void mergeModifiers(const ReferenceAttributeSet& other);

    void deriveFrom(const ReferenceAttributeSet& other);
    void applyToEachScopedValueKey(std::function<void(ScopedValueKey&)> fun);
};

}//IVD
