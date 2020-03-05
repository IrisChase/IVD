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

#include "material/fallbackparagraphmaterial.h"
#include "canvas.h"
#include "defaults.h"
#include "displayitem.h"
#include "text.h"

namespace IVD
{

class FallbackParagraphMaterial;

void FallbackParagraphMaterial::shape(const GeometryProposal officialProposal)
{
    //Revise proposal according to DisplayItem, but do not engage.
    // I mean, do not muck with margins. Each textrun has it's own margins,
    // and our drawing area is their viewport.
    auto prop = myItem->reviseProposalForDrawingArea(officialProposal);
    shapeDrawingArea(prop);
    setViewportArea(officialProposal.roundConflicts(getDrawingArea().d));
}

void FallbackParagraphMaterial::shapeDrawingArea(const GeometryProposal officialProposal)
{
    myRunMaterials.clear();
    std::string remainingText = Default::Filter::getText(myItem);
    
    if(remainingText.empty())
    {
        simpleShape(officialProposal);
        return;
    }
    
    Dimens result;

    const Angle rowFlowAngle = Default::Filter::getRowFlowAngle(myItem);
    const Angle itemFlowAngle = Default::Filter::getItemFlowAngle(myItem);

    if(!officialProposal.expandForAngleConst(itemFlowAngle))
    {
        while(remainingText.size())
        {
            const int space = officialProposal.proposedDimensions.get(itemFlowAngle);
            TextRunMaterial run(myItem);
            const std::string remainingForThisRun = run.setTextForSpace(remainingText, space);

            if(remainingText.size() == remainingForThisRun.size()) break; //Big bad failure daddy.
            remainingText = remainingForThisRun;

            //Ehhh... We can't do much about the opposite, can we...
            GeometryProposal runProposal;
            runProposal.expandForAngle(Angle::Horizontal) = true;
            runProposal.expandForAngle(Angle::Vertical)	  = true;
            run.shape(runProposal);

            myRunMaterials.push_back(run);

            const Dimens runViewport = run.getViewport().d;

            result.get(rowFlowAngle) += runViewport.get(rowFlowAngle);
            
            if(result.get(itemFlowAngle) < runViewport.get(itemFlowAngle))
                result.get(itemFlowAngle) += runViewport.get(itemFlowAngle);
        }
    }
    else
    {
        TextRunMaterial run(myItem);
        run.setText(remainingText);
        run.shape(officialProposal);
        result = run.getViewport().d;
        myRunMaterials.push_back(run);
    }
    
    
    //this means that the text will overflow across the line angle, worst case.
    //I mean. Not sure how it should even be defined...
    // It won't blow up, and that makes me hap.
    setDrawingArea(result);
}

void FallbackParagraphMaterial::setAbsoluteOffset(const Coords offset)
{
    const Angle itemFlowAngle = Default::Filter::getItemFlowAngle(myItem);
    const Angle rowFlowAngle  = Default::Filter::getRowFlowAngle(myItem);
    const int adjacentArea    = getViewport().d.get(itemFlowAngle);

    setViewportOffset(offset);

    setRelativeDrawingAreaOffset(myItem->getCellAlignmentOffset(getDrawingArea().d,
                                                                getViewport().d,
                                                                Dimens(),
                                                                Dimens()));

    Coords lastOffset = getDrawingAreaOffset();

    for(auto& run : myRunMaterials)
    {
        const int rowAdjacentArea = run.getViewport().d.get(itemFlowAngle);
        lastOffset.get(itemFlowAngle) += myItem->getJustificationOffset(rowAdjacentArea,
                                                                        adjacentArea);


        run.setAbsoluteOffset(lastOffset);

        lastOffset.get(rowFlowAngle) += run.getViewport().c.get(rowFlowAngle) +
                                        run.getViewport().d.get(rowFlowAngle);

        lastOffset.get(itemFlowAngle) = getDrawingAreaOffset().get(itemFlowAngle);//Reset
    }
}

bool FallbackParagraphMaterial::applyToColliding(const Rect box, std::function<bool (DisplayItem*)> fun)
{
    for(auto& run : myRunMaterials)
        if(run.applyToColliding(box, fun))
            return true;

    return false;
}



}//IVD
