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

#include "rowlayoutmaterial.h"

#include "displayitem.h"
#include "shaping/line.h"

namespace IVD
{

void RowLayoutMaterialBase::shapeDrawingArea(const GeometryProposal officialProposal)
{
    const int CellCount = myItem->childCount();

    if(!CellCount) return simpleShape(officialProposal); // :>

    const Dimens rowSpace = Shaping::line(myItem->getChildMaterials(), officialProposal, RowAdjacent);
    setDrawingArea(rowSpace);
}

void RowLayoutMaterialBase::setAbsoluteOffset(const Coords offset)
{
    setViewportOffset(offset);
    updateDrawingAreaOffset();

    const Angle RowOpposite = RowAdjacent == Angle::Horizontal ? Angle::Vertical
                                                               : Angle::Horizontal;

    int adjacentOffset = getDrawingAreaOffset().get(RowAdjacent);
    const int oppositeOffset = getDrawingAreaOffset().get(RowOpposite);

    for(Material* child : myItem->getChildMaterialsInModelOrder())
    {
        Coords childCoords;
        childCoords.get(RowAdjacent) = adjacentOffset;
        childCoords.get(RowOpposite) = oppositeOffset;

        child->setAbsoluteOffset(childCoords);

        adjacentOffset += child->getViewport().d.get(RowAdjacent);
    }
}

void RowLayoutMaterialBase::drawConcrete(Canvas* theCanvas)
{
    drawBasic(theCanvas, getDrawingArea());
    for(Material* child : myItem->getChildMaterials())
        child->draw(theCanvas);
}



}//IVD
