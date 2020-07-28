#include "referenceattributeset.h"

namespace IVD
{

void ReferenceAttributeSet::mergeModifiers(const ReferenceAttributeSet& other)
{
    //declares are always on the default state, but since this method
    // is used to merge classes as well as states, we just merge declares
    // here, and rely on the compiler to prevent declares from being added
    // to attribute sets that refer to a different state.

    for(auto pair : other.declareModifiers)
        declareModifiers[pair.first] = pair.second;

    for(auto pair : other.setModifiers)
        setModifiers[pair.first] = pair.second;
}

void ReferenceAttributeSet::deriveFrom(const ReferenceAttributeSet& other)
{
    mergeModifiers(other);

    for(int key = 0; key != AttributeKey::AttributeCount; ++key)
        attr[key].derive(other.attr[key]);
}

void ReferenceAttributeSet::applyToEachScopedValueKey(std::function<void (ScopedValueKey &)> fun)
{
    for(auto& it : declareModifiers)
        it.second.applyToEachScopedValueKey(fun);
    for(auto& it : setModifiers)
        it.second.applyToEachScopedValueKey(fun);

    for(int i = 0; i != AttributeKey::AttributeCount; ++i)
        attr[i].applyToEachScopedValueKey(fun);
}

}//IVD
