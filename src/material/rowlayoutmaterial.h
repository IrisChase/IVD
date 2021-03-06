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

#ifndef ROWLAYOUTMATERIAL_H
#define ROWLAYOUTMATERIAL_H

#include "material.h"

namespace IVD
{

class RowLayoutMaterialBase : public Material
{
    const Angle RowAdjacent;

protected:
    RowLayoutMaterialBase(DisplayItem* theItem, const Angle theAdjacent):
        Material(theItem),
        RowAdjacent(theAdjacent)
    {}


public:
    FillPrecedence computerFillPrecedenceForAngle(const Angle theAngle) final
    { return returnGreedyIfEVENONECHILDBLINKS(theAngle); }

    void shapeDrawingArea(const GeometryProposal officialProposal) final;
    void setAbsoluteOffset(const Coords offset) final;

    void drawConcrete(Canvas* theCanvas) final;

    bool applyToColliding(const Rect box, std::function<bool(DisplayItem*)> fun) final
    { return applyToCollidingMultiFlat(box, fun); }
};

class HorizontalRowLayoutMaterial : public RowLayoutMaterialBase
{
public:
    HorizontalRowLayoutMaterial(DisplayItem* item):
        RowLayoutMaterialBase(item, Angle::Horizontal)
    {}
};

class VerticalRowLayoutMaterial : public RowLayoutMaterialBase
{
public:
    VerticalRowLayoutMaterial(DisplayItem* item):
        RowLayoutMaterialBase (item, Angle::Vertical)
    {}
};

}//IVD

#endif // ROWLAYOUTMATERIAL_H
