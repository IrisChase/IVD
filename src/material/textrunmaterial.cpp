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

#include "textrunmaterial.h"

#include "text.h"
#include "canvas.h"
#include "displayitem.h"

namespace IVD
{

void TextRunMaterial::shapeDrawingArea(const GeometryProposal officalProposal)
{
    //This material has no control over the sizing of text, so it's just, whatever, man.
    setDrawingArea(Text::getRunDimensions(myItem, myText));
}

void TextRunMaterial::drawConcrete(Canvas* theCanvas)
{
    drawBasic(theCanvas, getDrawingArea());
    theCanvas->drawText(getDrawingAreaOffset(), myText, myItem);
}

std::string TextRunMaterial::setTextForSpace(const std::string run, const int space)
{
    const auto itemFlowAngle = Default::Filter::getItemFlowAngle(myItem);
    const int revisedSpace = space - myItem->getReservedDimens().get(itemFlowAngle);

    Text::SplitStringPair pair = Text::extractExcess(myItem, run, revisedSpace > 0 ? revisedSpace
                                                                                   : 0);
    myText = pair.remaining;
    return pair.overflowing;
}

}//IVD
