#include "referenceattribute.h"

#include "irisutils/routine.h"

namespace IVD
{

void ReferenceAttribute::derive(const ReferenceAttribute& other)
{
    //This seems backwards for the runtime... Should just copy-in the cleared
    // attribute if we're not in mode-derive?
    // Shouldn't this be read ther otherway around? TODO
    if(clear) return;

    if(!other.active) return;

    active = true;

    auto mergeHelper = [](auto& mine, const auto& other)
    {
        if(!mine) mine = other;
    };

    mergeHelper(property, other.property);

    mergeHelper(starting, other.starting);
    mergeHelper(min, other.min);
    mergeHelper(max, other.max);
    mergeHelper(expr, other.expr);

    mergeHelper(color, other.color);

    mergeHelper(literal, other.literal);
    mergeHelper(singleKey, other.singleKey);

    mergeHelper(delay, other.delay);
    mergeHelper(ease, other.ease);

    IrisUtils::Routine::appendContainer(keys, other.keys);
    IrisUtils::Routine::appendContainer(literalList, other.literalList);}
}//IVD
