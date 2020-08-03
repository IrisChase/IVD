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

#include "stacklayoutmaterial.h"
#include "displayitem.h"

namespace IVD
{

void StackLayoutMaterial::shapeDrawingArea(const GeometryProposal officalProposal)
{
    std::vector<Dimens> initialSizes;

    for(auto* mat : myItem->getChildMaterials())
    {
        mat->shape(officalProposal);
        initialSizes.emplace_back(mat->getViewport().d);
    }

    Dimens maxed;

    auto maximizeForAngle = [&](const Angle theAngle)
    {
        auto getLargerAnglePredicate = [&](const Angle theAngle)
        {
            return [=](const Dimens& left, const Dimens& right)
            {
                return left.get(theAngle) < right.get(theAngle);
            };
        };

        auto it = std::max_element(initialSizes.begin(),
                                   initialSizes.end(),
                                   getLargerAnglePredicate(theAngle));

        maxed.get(theAngle) = it != initialSizes.end() ? it->get(theAngle)
                                                       : officalProposal.proposedDimensions.get(theAngle);
    };

    maximizeForAngle(Angle::Horizontal);
    maximizeForAngle(Angle::Vertical);

    //Allow shrink/grow all false by default
    GeometryProposal fin;
    fin.proposedDimensions = maxed;

    for(auto* mat : myItem->getChildMaterials()) mat->shape(fin);

    setDrawingArea(maxed);
}

void StackLayoutMaterial::setAbsoluteOffset(const Coords offset)
{
    setViewportOffset(offset);
    updateDrawingAreaOffset();

    for(auto* mat : myItem->getChildMaterials())
        mat->setAbsoluteOffset(offset);
}

void StackLayoutMaterial::drawConcrete(Canvas* theCanvas)
{
    drawBasic(theCanvas, getDrawingArea());

    for(auto* mat : myItem->getChildMaterialsInModelOrder())
        mat->draw(theCanvas);
}

bool StackLayoutMaterial::applyToColliding(const Rect box, std::function<bool (DisplayItem*)> fun)
{
    auto children = myItem->getChildMaterialsInModelOrder(); //basically insertion order?
    for(auto rit = children.rbegin(); rit != children.rend(); ++rit)
    {
        auto* mat = *rit;
        if(mat->applyToColliding(box, fun)) return true;
    }

    applyToCollidingSingle(box, fun); //For pass-through things.
    return false;
}



}//IVD
