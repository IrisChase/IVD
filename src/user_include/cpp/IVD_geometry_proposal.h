#pragma once

#include "IVD_geometry.h"

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
    GeometryProposal(IVD_GeometryProposal* other)
    {
        allowedExpandHorizontal = *IVD_geoprop_expand_horizontal(other);
        allowedExpandVertical   = *IVD_geoprop_expand_vertical(other);
        allowedShrinkHorizontal = *IVD_geoprop_shrink_horizontal(other);
        allowedShrinkVertical   = *IVD_geoprop_shrink_vertical(other);

        proposedDimensions = IVD_geoprop_proposed_space(other);
    }

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
