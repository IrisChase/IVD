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

#include "inlinelayoutmaterial.h"
#include "defaults.h"
#include "displayitem.h"

#include "shaping/line.h"

namespace IVD
{

void InlineLayoutMaterial::shapeDrawingArea(const GeometryProposal officalProposal)
{
    myCustomTextRuns.clear();
    rows.clear();
    Dimens result;
    auto itemFlowAngle = Default::Filter::getItemFlowAngle(myItem);
    auto rowFlowAngle  = Default::Filter::getRowFlowAngle(myItem);
    
    //Default strategy is to try and honor the itemFlowAngle
    //But spill over into the rowflow regardless.

    if(!officalProposal.expandForAngleConst(itemFlowAngle))
    {
        //So, we either take a reference to the original, or create a proxy
        // to insert into the row... God help us all on reference invalidation.
        std::vector<Material*> currentRow;
        const int maxRowSpace = officalProposal.proposedDimensions.get(itemFlowAngle);
        int rowAdjacentArea = 0;
        int rowOppositeArea = 0;


        auto incrementRow = [&](Material* proxyChild)
        {
            rowAdjacentArea += proxyChild->getViewport().d.get(itemFlowAngle);

            const int lastHeight = proxyChild->getViewport().d.get(rowFlowAngle);
            if(lastHeight > rowOppositeArea)
                rowOppositeArea = lastHeight;

            currentRow.push_back(proxyChild);
        };

        auto nextRow = [&]()
        {
            if(currentRow.empty()) return;
            rows.emplace_back(rowAdjacentArea, rowOppositeArea, currentRow);
            currentRow.clear();

            result.get(rowFlowAngle) += rowOppositeArea;
            rowOppositeArea = 0;

            if(rowAdjacentArea > result.get(itemFlowAngle))
                result.get(itemFlowAngle) = rowAdjacentArea;
            rowAdjacentArea = 0;
        };

        auto getAvailableSpaceForRow = [&]() -> int
        { return maxRowSpace - rowAdjacentArea; };

        for(Material* concreteChild : myItem->getChildMaterialsInModelOrder())
        {
            Material* proxyChild = concreteChild;
            DisplayItem* currentItem = concreteChild->getDisplayItem();
            const std::string text = Default::Filter::getText(currentItem);
            if(text.size())
            {
                std::string remainingText = text;
                while(remainingText.size())
                {
                    TextRunMaterial run(currentItem);

                    std::string remainingForThisRow = run.setTextForSpace(remainingText,
                                                                          getAvailableSpaceForRow());

                    if(remainingForThisRow == remainingText)
                    {
                        //Okay so two things could have gone wrong.
                        // 1) we're out of space for this row and we need to call nextRow
                        //	  (very common).
                        // 2) There was not enough space at all, not even on a fresh row.
                        //    This is a failure mode.

                        if(!rowAdjacentArea)
                        {
                            break; //Give the up.
                            //But shouldn't we just concede the space to the item's viewport?
                            // Are we to assume that there is no space AT ALL EVER?
                            //It's a failure mode either way, but... TODO
                        }

                        nextRow();
                        //Try ONE MORE TIME
                        remainingForThisRow = run.setTextForSpace(remainingText,
                                                                  getAvailableSpaceForRow());

                        if(remainingForThisRow == remainingText) goto failure; // I'm so tired of living
                    }

                    remainingText = remainingForThisRow;

                    {
                        GeometryProposal blankProp;
                        blankProp.expandForAngle(Angle::Horizontal) = true;
                        blankProp.expandForAngle(Angle::Vertical)	= true;
                        run.shape(blankProp);
                    }

                    myCustomTextRuns.push_back(run);
                    proxyChild = &myCustomTextRuns.back();
                    incrementRow(proxyChild);
                }
                failure:;
            }
            else
            {
                //Do an empty proposal to get a baseline, and then see where it might fit best.
                // always call nextRow if it needs more space, even if there still won't be
                // enough.

                //right here bb.....
                GeometryProposal prop;
                prop.expandForAngle(Angle::Horizontal) = true;
                prop.expandForAngle(Angle::Vertical)   = true;
                Material* proxyChild;
                proxyChild->shape(prop);

                int childAdjacent = proxyChild->getViewport().d.get(itemFlowAngle);

                if(childAdjacent > getAvailableSpaceForRow() && rowAdjacentArea)
                    nextRow();

                prop = officalProposal;
                prop.expandForAngle(rowFlowAngle) = true;
                prop.proposedDimensions.get(itemFlowAngle) -= rowAdjacentArea;
                proxyChild->shape(prop);

                incrementRow(proxyChild);
            }
        }

        nextRow(); //Accept last row


        for(auto& row : rows)
        {
            //Level opposite for each item in a row :3c
            for(auto material : row.myMaterials)
            {
                GeometryProposal adjustedProp;
                adjustedProp.proposedDimensions = material->getViewport().d;
                adjustedProp.proposedDimensions.get(rowFlowAngle) = row.rowOppositeArea;
                material->shape(adjustedProp);
            }
        }
    }
    else
    {

        //Should we just use Shaping::Line? It's a little more complex, but...
        // We don't really need proxies here. I think the rules might end up being a
        // *little* different... Especially since we don't ordinarily consider
        // greedy material as distinct from shrinky ones...
        //GOD it'd be simpler...
        //Either way we don't need to think about sizing the text ourselves,
        // the default textmaterial will do the trick for a straight line.

        //Honestly... Who would be dumb enough to put a greedy material in here anyway...

        //GIVE 'EM HELL
        std::vector<Material*> theOneRow = myItem->getChildMaterialsInModelOrder();
        result = Shaping::line(theOneRow, officalProposal, itemFlowAngle);
        rows.emplace_back(result.get(itemFlowAngle), result.get(rowFlowAngle), theOneRow);
    }

    setDrawingArea(result);
}

void InlineLayoutMaterial::setAbsoluteOffset(const Coords offset)
{
    setViewportOffset(offset);
    updateDrawingAreaOffset();

    const Angle itemFlowAngle = Default::Filter::getItemFlowAngle(myItem);
    const Angle rowFlowAngle  = Default::Filter::getRowFlowAngle(myItem);
    const int adjacentArea    = getDrawingArea().d.get(itemFlowAngle);

    Coords lastOffset = getDrawingAreaOffset();

    for(auto& materialWorld : rows)
    {
        lastOffset.get(itemFlowAngle) += myItem->getJustificationOffset(materialWorld.rowAdjacentArea,
                                                                        adjacentArea);

        for(Material* girl : materialWorld.myMaterials)
        {
            girl->setAbsoluteOffset(lastOffset);
            lastOffset.get(itemFlowAngle) += girl->getViewport().d.get(itemFlowAngle);
        }

        lastOffset.get(rowFlowAngle) += materialWorld.rowOppositeArea;
        lastOffset.get(itemFlowAngle) = getDrawingAreaOffset().get(itemFlowAngle);//Reset
    }
}

void InlineLayoutMaterial::drawConcrete(Canvas* theCanvas)
{
    drawBasic(theCanvas, getDrawingArea());

    for(auto& materialWorld : rows)
        for(Material* girl : materialWorld.myMaterials)
            girl->draw(theCanvas);
}

bool InlineLayoutMaterial::applyToColliding(const Rect box, std::function<bool (DisplayItem*)> fun)
{
    for(auto& materialWorld : rows)
        for(Material* girl : materialWorld.myMaterials)
            if(girl->applyToColliding(box, fun))
                return true;

    return applyToCollidingSingle(box, fun);
}


}//IVD
