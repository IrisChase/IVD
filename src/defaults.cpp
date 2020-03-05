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

#include "defaults.h"
#include "displayitem.h"
#include "environment.h"

#include "corefonts.h"

#include <fstream>
#include <iostream>

namespace IVD
{

namespace Default
{

namespace Filter
{

int getFontSize(DisplayItem* item)
{
    const auto optional = item->getAttr().getInt(AttributeKey::FontSize);
    return optional ? *optional
                    : Default::FontSize;
}

FontData loadCustomFont(const std::string path)
{
    //TODO thread-safety? Just lock it if we ever need that. No point in storing
    // two copies of the same font, defeating the purpose of the cache, and no
    // problem waiting on the other thread to load it, when that's all we'd do anyway.

    //Memory mapping would be better.
    static std::map<std::string, std::vector<unsigned char>> cachedFonts;

    if(cachedFonts.count(path)) return FontData(&cachedFonts[path][0], cachedFonts[path].size());

    //Load the fugger...
    std::ifstream myFile;
    myFile.open(path.c_str(), std::ios::binary);

    if(!myFile)
    {
        std::cerr << "IVD Runtime: Could not open custom font: " << path << std::endl
                  << "Falling back on \"sans\"..." << std::endl;

        return FontData(corefontsans, corefontsansSize);
    }

    std::vector<unsigned char> data((std::istreambuf_iterator<char>(myFile)),
                                     std::istreambuf_iterator<char>());

    cachedFonts[path] = data;

    return FontData(&cachedFonts[path][0], cachedFonts[path].size());
}

FontData getCoreFont(const int fontProperty)
{
    switch(fontProperty)
    {
    case Property::FontSans:           return FontData(corefontsans, corefontsansSize);
    case Property::FontSansBold:       return FontData(corefontsansbold, corefontsansboldSize);
    case Property::FontSansItalic:     return FontData(corefontsansitalic, corefontsansitalicSize);
    case Property::FontSansBoldItalic: return FontData(corefontsansbolditalic, corefontsansbolditalicSize);

    case Property::FontSerif:           return FontData(corefontserif, corefontserifSize);
    case Property::FontSerifBold:       return FontData(corefontserifbold, corefontserifboldSize);
    case Property::FontSerifItalic:     return FontData(corefontserifitalic, corefontserifitalicSize);
    case Property::FontSerifBoldItalic: return FontData(corefontserifbolditalic, corefontserifbolditalicSize);

    case Property::FontMono:           return FontData(corefontmono, corefontmonoSize);
    case Property::FontMonoBold:       return FontData(corefontmonobold, corefontmonoboldSize);
    case Property::FontMonoItalic:     return FontData(corefontmonoitalic, corefontmonoitalicSize);
    case Property::FontMonoBoldItalic: return FontData(corefontmonobolditalic, corefontmonobolditalicSize);

    default: assert(false);
    }
}

FontData getFontFace(DisplayItem* item)
{
    const auto optionalCustomFont = item->getAttr().getUserToken(AttributeKey::Font);

    if(optionalCustomFont) return loadCustomFont(*optionalCustomFont);


    const auto optionalFontProperty = item->getAttr().getProperty(AttributeKey::Font);

    return getCoreFont(optionalFontProperty ? *optionalFontProperty
                                              : FontFace);

}

static std::string simpleStringFilter(DisplayItem* item, const int key, const std::string def)
{
    auto tt = item->getAttr().getUserToken(key);
    if(tt) return *tt;

    auto vk = item->getAttr().getSingleValueKey(key);
    if(vk) return item->getEnv()->getString(item, *vk);

    return def;
}

std::string getTitleText(DisplayItem* item)
{
    return simpleStringFilter(item, AttributeKey::TitleText, TitleText);
}

std::string getText(DisplayItem* item)
{
    return simpleStringFilter(item, AttributeKey::Text, "");
}

Color getFontColor(DisplayItem* item)
{
    const auto optional = item->getAttr().getColor(AttributeKey::FontColor);

    return optional ? *optional
                    : FontColor;
}

Color::AlphaType getAlpha(DisplayItem* item)
{
    return DefaultAlpha;
}

std::optional<Color> getElementColor(DisplayItem* item)
{
    //Whether it's set or not is important. No color is different from
    // alpha.
    return item->getAttr().getColor(AttributeKey::ElementColor);
}

bool getVisibility(DisplayItem* item)
{
    auto vis = item->getAttr().getProperty(AttributeKey::Visibility);
    if(vis) return *vis == Property::Enable;
    return visible;
}

bool getFullscreen(DisplayItem* item)
{
    return fullscreen;
}

bool checkResizable(DisplayItem* item)
{
    //TODO: Also not resizable if fullscreen... But that should be handled by the driver?
    auto transAdj = getSizeAdjacent(item);
    auto transOpp = getSizeOpposite(item);

    if(transAdj || transOpp) return false;

    auto flag = item->getAttr().getProperty(AttributeKey::Resizable);

    if(!flag)
    {
        const int sizeStrategy = getWindowSizeStrategy(item);

        if(sizeStrategy == Property::BottomUp)
            return false;
        else return true;
    }

    return *flag == Property::Enable;
}

bool checkModelOrder(DisplayItem* item)
{
    auto optional = item->getAttr().getProperty(AttributeKey::ModelOrder);
    if(!optional) return modelOrder;
    return *optional;
}

std::optional<int> getSizeAdjacent(DisplayItem* item)
{
    return item->getAttr().getInt(AttributeKey::SizeA);
}

std::optional<int> getSizeOpposite(DisplayItem* item)
{
    return item->getAttr().getInt(AttributeKey::SizeO);
}

std::optional<ValueKeyPath> getPositionWithin(DisplayItem* item)
{
    if(item->getAttr().getSingleValueKey(AttributeKey::PositionWithin))
    {
        auto posWith = item->getAttr().getSingleValueKey(AttributeKey::PositionWithin);


        auto optionalPath = posWith->path;
        auto optionalKey = posWith->key;

            if(optionalPath)
                return *optionalPath;

            return ValueKeyPath({*posWith->key});
    }

    return std::optional<ValueKeyPath>();
}

std::optional<int> getAdjacentAlignmentProperty(DisplayItem* item)
{
    return item->getAttr().getProperty(AttributeKey::AlignAdjacent);
}

std::optional<int> getOppositeAlignmentProperty(DisplayItem* item)
{
    return item->getAttr().getProperty(AttributeKey::AlignOpposite);
}

Angle getItemFlowAngle(DisplayItem* item)
{
    return Angle::Horizontal; //Ehem...
}

Angle getRowFlowAngle(DisplayItem* item)
{
    return Angle::Vertical; //EHEM...
}

int getWindowSizeStrategy(DisplayItem* item)
{
    //Also top-down if fullscreen... TODO
    const auto primary = item->getAttr().getProperty(AttributeKey::WindowSizeStrategy);
    if(primary) return *primary;

    return WindowSizeStrategy;
}

std::optional<int> getHorizontalWindowAlignmentProperty(DisplayItem* item)
{
    assert(false);
    //return item->getAttr().getProperty(AttributeKey::InitialWindowAlignAdj);
}

std::optional<int> getVerticalWindowAlignmentProperty(DisplayItem* item)
{
    assert(false);
    //return item->getAttr().getProperty(AttributeKey::InitialWindowAlignOpp);
}

std::optional<int> getJustificationProperty(DisplayItem* item)
{
    return item->getAttr().getProperty(AttributeKey::Justify);
}




}//Filter
}//Default
}//IVD
