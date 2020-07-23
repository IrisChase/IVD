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

#ifndef LINE_H
#define LINE_H

#include "material.h"

namespace IVD
{
namespace Shaping
{

template<typename Cont>
Dimens line(Cont theMaterials, const GeometryProposal proposal, const Angle Adjacent)
{
    const int CellCount = theMaterials.size();
    const Angle Opposite = (Adjacent == Angle::Horizontal) ? Angle::Vertical
                                                           : Angle::Horizontal;

    const int AvailableAdjacentSpace = proposal.proposedDimensions.get(Adjacent);
    const int AvailableOppositeSpace = proposal.proposedDimensions.get(Opposite);

    std::vector<Material*> greedy;
    std::vector<Material*> shrinky;

    for(Material* material : theMaterials)
    {
        const FillPrecedence Pref = material->getFillPrecedenceForAngle(Adjacent);
        if(Pref == FillPrecedence::Greedy)
            greedy.push_back(material);
        if(Pref == FillPrecedence::Shrinky)
            shrinky.push_back(material);
    }

    int usedAdjacentSpace;
    int usedOppositeSpace;

    auto resetWorkingAdjacent = [&]
    { usedAdjacentSpace = 0; };

    auto resetWorkingOpposite = [&]
    { usedOppositeSpace = 0; };

    auto resetWorkingDimensions = [&]
    {
        resetWorkingAdjacent();
        resetWorkingOpposite();
    };

    resetWorkingDimensions();

    bool invalidOpposite = false;

    auto applyAdjacentFormulaToChild = [&](Material* material,
                                           GeometryProposal childProposal,
                                           std::function<int(Material*)> adjacentFormula)
    {
        //Please leave this here I'm sick of adding it back for debugging
        const auto proposedAdjacentSize = adjacentFormula(material);

        //----------Adjacent
        childProposal.proposedDimensions.get(Adjacent) = proposedAdjacentSize;

        material->shape(childProposal);
        const Dimens childDimens = material->getViewport().d;

        usedAdjacentSpace += childDimens.get(Adjacent);

        //----------Opposite
        const int UsedOppositeThisRound = childDimens.get(Opposite);

        if(UsedOppositeThisRound > usedOppositeSpace)
            usedOppositeSpace = UsedOppositeThisRound;

        if(AvailableOppositeSpace != UsedOppositeThisRound)
            invalidOpposite = true;

        //----------
        assert(childProposal.verifyCompliance(childDimens));
    };

    auto applyToSet = [&](const std::vector<Material*>& items,
                          GeometryProposal childProposal,
                          std::function<int(Material*)> formula)
    {
        for(Material* material : items)
            applyAdjacentFormulaToChild(material, childProposal, formula);
    };

    auto adjustOpposite = [&]()
    {
        if(!invalidOpposite) return;

        //The problem here is that one widget might have actually used all of the opposite
        // correctly, while a different one shrank. So really we need a better test to see
        // if the widgets didn't all use the same space!!! : TODO, is this still a thing???
        // I don't understand if it's still a problem, doesn't seem like it???
        resetWorkingAdjacent();

        //We don't discriminate between greedy and shrinky this time, because
        // we're only looking to update the child opposites and perhaps
        // shrink our adjacent.
        for(Material* material : theMaterials)
        {
            //We've already determined the opposite, so lock both opposite shrink and expand.
            //If the opposite is larger, we might recover some adjacent space,
            // so we don't shrink lock that dimension.

            //If the opposite is smaller, do we need to lock adjacent shrinking?
            //I can't think of anything but the most contrived scenario where
            // the adjacent would shrink further. But I also don't see why we
            // *have to* lock it...
            //(Wouldn't it shrink if we increase the opposite in a vertical text flow scenario?)
            //Yeah... Lock it.

            //The computed adjacent is always big enough to handle the computed opposite
            // or larger. We won't be suggesting a smaller opposite, so the adjacent should
            // not expand further.

            GeometryProposal childProposal = proposal;
            childProposal.proposedDimensions = material->getViewport().d;
            childProposal.proposedDimensions.get(Opposite) = usedOppositeSpace;

            childProposal.expandForAngle(Opposite) = false;
            childProposal.shrinkForAngle(Opposite) = false;
            childProposal.expandForAngle(Adjacent) = false;
            childProposal.shrinkForAngle(Adjacent) = false;

            material->shape(childProposal);
            const Dimens childDimens = material->getViewport().d;
            usedAdjacentSpace += childDimens.get(Adjacent);

            assert(childProposal.verifyCompliance(childDimens));
        }

        invalidOpposite = false;
    };

    //Initial pass, get an idea of what the natural sizes are
    {
        //No explicit expand for opposite because it's taking in the
        // higher up suggestion which we much obey anyway.
        //Otherwise it throws off the overall opposite rule.
        //If it's locked it can't legally grow!!!! Or shrink... No shrinky!
        // extra space goes to the child cell anyway.
        GeometryProposal childProposal = proposal;
        childProposal.expandForAngle(Adjacent) = true;
        childProposal.shrinkForAngle(Adjacent) = true;

        auto initalCellSizeFormula = [=](Material*) -> int
        { return zeroGuard((AvailableAdjacentSpace - usedAdjacentSpace) / CellCount); };

        applyToSet(shrinky, childProposal, initalCellSizeFormula);
        applyToSet(greedy, childProposal, initalCellSizeFormula);
        adjustOpposite();
    }


    //If the sizes are bad, then we adjust
    if(usedAdjacentSpace > proposal.proposedDimensions.get(Adjacent))
    {
        if(!proposal.expandForAngleConst(Adjacent))
        {
            //No overflow mode, because if we can't expand, we can't expand.
            //So it must go to the children. The default is for the whole thing
            // to take on the full size, if you want a viewport, put this layout
            // inside an item with a viewport layout.

            const int cutSize = (usedAdjacentSpace - AvailableAdjacentSpace) / CellCount;

            auto adjustingSizeFormula = [&](Material* material) -> int
            { return zeroGuard(material->getViewport().d.get(Adjacent) - cutSize); };

            //(╯°□°）╯︵ ┻━┻
            resetWorkingDimensions();

            GeometryProposal childProposal = proposal;
            childProposal.expandForAngle(Adjacent) = false;

            applyToSet(shrinky, childProposal, adjustingSizeFormula);
            applyToSet(greedy, childProposal, adjustingSizeFormula);
            adjustOpposite();
        }//Else is just whatev's
    }

    if(usedAdjacentSpace < proposal.proposedDimensions.get(Adjacent))
    {
        if(!proposal.shrinkForAngleConst(Adjacent) && CellCount)
        {
            //By default we only expand greedy. But if there is no greedy and we MUST expand...
            //Actually not sure if this is ever greedy, now that I thonk abut it? Can it be? TODO
            auto& theSet = greedy.size() ? greedy
                                         : shrinky;

            auto& opposet = greedy.size() ? shrinky
                                          : greedy;

            const int padSize = (AvailableAdjacentSpace - usedAdjacentSpace) / theSet.size();
            resetWorkingDimensions();

            auto padFormula = [&](Material* material) -> int
            { return material->getViewport().d.get(Adjacent) + padSize; };

            GeometryProposal childProposal = proposal;
            childProposal.shrinkForAngle(Adjacent) = false;

            applyToSet(theSet, childProposal, padFormula);

            //One last problem, we gotta add the opposets to the usedAdjacent and Opposite(?) spaces...
            //(This could maybe be abstracted with some code from applyToSet TODO)
            for(Material* m : opposet)
            {
                const Dimens d = m->getViewport().d;
                if(d.get(Opposite) > usedOppositeSpace)
                {
                    //Does this ever happen????? TODO
                    usedOppositeSpace = d.get(Opposite);
                    invalidOpposite = true;
                }
                usedAdjacentSpace += d.get(Adjacent);
            }

            adjustOpposite();
        }
    }

    Dimens usedSpace;
    usedSpace.get(Adjacent) = usedAdjacentSpace;
    usedSpace.get(Opposite) = usedOppositeSpace;

    return usedSpace;
}

}//Shaping
}//IVD

#endif // LINE_H
