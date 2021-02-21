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

#ifndef DEFAULTS_H
#define DEFAULTS_H

#include "color.h"
#include "geometry.h"
#include "valuekey.h"

#include "irisutils/lexcompare.h"

namespace IVD
{

class DisplayItem;

namespace Default
{
//Prefer Default::Filter::get*(DisplayItem*)
// to the literals.

const int FontFace = Property::FontSans;

const int FontSize = 12;
const Color FontColor = Color(0, 0, 0); //Paint it black
const Color ElementColor = Color(255, 255, 255);
const Color::AlphaType DefaultAlpha = 255;

const std::string TitleText = "IVD Window";

const ValueKey MousePath = "IVD-Mouse";

const bool visible = true;
const bool fullscreen = false;
const bool resizable = true;
const bool modelOrder = true;

const std::string quitTrigger = "IVD-Core-Quit";

const Angle angle = Angle::Adjacent;
const int alignmentProperty = Property::Inner;

const int WindowSizeStrategy = Property::TopDown;

namespace Filter
{

int getFontSize(DisplayItem* item);

struct FontData
{
    const unsigned char* data;
    const int size;


    FontData(const unsigned char* data, const int size):
        data(data),
        size(size)
    {}

    IRISUTILS_DEFINE_COMP(FontData, data, size);
};
FontData getFontFace(DisplayItem* item);

std::string getTitleText(DisplayItem* item);
std::string getText(DisplayItem* item);
//Alignment getAlignment(DisplayItem* item, const int attrkey);
Color getFontColor(DisplayItem* item);
Color::AlphaType getAlpha(DisplayItem* item);
std::optional<Color> getElementColor(DisplayItem* item);

bool getVisibility(DisplayItem* item);
bool getFullscreen(DisplayItem* item);

bool checkResizable(DisplayItem* item);

bool checkModelOrder(DisplayItem* item);

Angle getItemFlowAngle(DisplayItem* item);
Angle getRowFlowAngle(DisplayItem* item);

std::optional<int> getSizeAdjacent(DisplayItem* item);
std::optional<int> getSizeOpposite(DisplayItem* item);

std::optional<ValueKeyPath> getPositionWithin(DisplayItem* item);

//Why are these optional??
std::optional<int> getAdjacentAlignmentProperty(DisplayItem* item);
std::optional<int> getOppositeAlignmentProperty(DisplayItem* item);

std::optional<int> getJustificationProperty(DisplayItem* item);

std::optional<int> getHorizontalWindowAlignmentProperty(DisplayItem* item);
std::optional<int> getVerticalWindowAlignmentProperty(DisplayItem* item);

int getWindowSizeStrategy(DisplayItem* item);

}//Filter
}//Default
}//IVD

#endif // DEFAULTS_H
