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

#ifndef GEOMETRYPROPOSAL_H
#define GEOMETRYPROPOSAL_H

#include "geometry.h"

namespace IVD
{

struct GeometryProposal
{
private:
    int allowedExpandHorizontal;
    int allowedExpandVertical;
    int allowedShrinkHorizontal;
    int allowedShrinkVertical;

public:
    GeometryProposal():
        allowedExpandHorizontal(false),
        allowedExpandVertical(false),
        allowedShrinkHorizontal(false),
        allowedShrinkVertical(false)
    {}

    Dimens proposedDimensions;

    int& expandForAngle(Angle angel)
    {
        return angel == Angle::Horizontal ? allowedExpandHorizontal
                                          : allowedExpandVertical;
    }

    const bool expandForAngleConst(Angle angel) const
    {
        return angel == Angle::Horizontal ? allowedExpandHorizontal
                                          : allowedExpandVertical;
    }

    int& shrinkForAngle(Angle angel)
    {
        return angel == Angle::Horizontal ? allowedShrinkHorizontal
                                          : allowedShrinkVertical;
    }

    const bool shrinkForAngleConst(Angle angel) const
    {
        return angel == Angle::Horizontal ? allowedShrinkHorizontal
                                          : allowedShrinkVertical;
    }

    bool verifyCompliance(const Dimens& region) const
    {
        //Oh I'm a fucking moron
        if(!allowedExpandHorizontal && region.w > proposedDimensions.w)
            return false;
        if(!allowedExpandVertical && region.h > proposedDimensions.h)
            return false;
        if(!allowedShrinkHorizontal && region.w < proposedDimensions.w)
            return false;
        if(!allowedShrinkVertical && region.h < proposedDimensions.h)
            return false;

        return true;
    }

    //Round cell size against constraints (Really only from an overconstrained condition,
    // whereby the proposal and revised proposal are in conflict)
    Dimens roundConflicts(Dimens usedSpace) const
    {
        const int propCellAdj = proposedDimensions.get(Angle::Horizontal);
        const int propCellOpp = proposedDimensions.get(Angle::Vertical);

        if((!expandForAngleConst(Angle::Horizontal) && usedSpace.get(Angle::Horizontal) > propCellAdj) ||
           (!shrinkForAngleConst(Angle::Horizontal) && usedSpace.get(Angle::Horizontal) < propCellAdj))
        {
            usedSpace.get(Angle::Horizontal) = propCellAdj;
        }

        if((!expandForAngleConst(Angle::Vertical) && usedSpace.get(Angle::Vertical) > propCellOpp) ||
           (!shrinkForAngleConst(Angle::Vertical) && usedSpace.get(Angle::Vertical) < propCellOpp))
        {
            usedSpace.get(Angle::Vertical) = propCellOpp;
        }

        return usedSpace;
    }
};

}

#endif // GEOMETRYPROPOSAL_H
