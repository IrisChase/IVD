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

#pragma once
#include "material.h"

#include <list>
#include "material/textrunmaterial.h"

namespace IVD
{

class InlineLayoutMaterial final : public Material
{
    std::list<TextRunMaterial> myCustomTextRuns;

    struct SingleRow
    {
        int rowAdjacentArea;
        int rowOppositeArea;
        std::vector<Material*> myMaterials;

        SingleRow(const int itemSize, const int rowSize, const std::vector<Material*>& mats):
            rowAdjacentArea(itemSize),
            rowOppositeArea(rowSize),
            myMaterials(mats)
        {}
    };

    std::vector<SingleRow> rows;

public:
    InlineLayoutMaterial(DisplayItem* theItem): Material(theItem) {}

    FillPrecedence getFillPrecedenceForAngle(const Angle theAngle) final
    { return FillPrecedence::Shrinky; };

    void shapeDrawingArea(const GeometryProposal officalProposal) final;

    void setAbsoluteOffset(const Coords offset) final;
    void drawConcrete(Canvas *theCanvas) final;

    //We override because the general version pulls concrete materials out of myItem
    // but we use a lot of proxies.
    bool applyToColliding(const Rect box, std::function<bool (DisplayItem *)> fun) final;
};

}//IVD
