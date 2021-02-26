//Copyright 2021 Iris Chase
//
//Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef KEYWORDS_H
#define KEYWORDS_H

#include "assert.h"
#include <vector>

#include <string>
#include <optional>

#include <map>
#include <set>

namespace IVD
{

typedef unsigned int KeyType;

namespace AttributeKey
{

enum
{
    FirstAttribute = 0,

    PositionWithin = FirstAttribute,

    TitleText,
    Text,

    Font,
    ImagePath,

    FontSize,

    TranslationO,
    TranslationA,

    MarginOppOut,
    MarginOppIn,
    MarginAdjIn,
    MarginAdjOut,

    PaddingOppOut,
    PaddingOppIn,
    PaddingAdjIn,
    PaddingAdjOut,

    SizeO,
    SizeA,

    //Properties

    Orientation,

    WindowState,
    Visibility,

    AlignAdjacent,
    AlignOpposite,

    OverrideFillPrecedenceAdjacent,
    OverrideFillPrecedenceOpposite,

    Justify,

    WindowSizeStrategy,

    ImageSizeProperty,

    Borderless,
    Resizable,
    ModelOrder,


    ElementColor,
    FontColor,
    BorderColor,



    Triggers,
    InduceState,
    BindState,
    ToggleState,
    UnsetState,
    TriggerState,
    RadioState,

    Layout,
    Widget,
    CellNames,

    AttributeCount,

    UnnaturalAttributesStart = AttributeCount,

    //These are unnatural (compound) Attributes.
    Margins = UnnaturalAttributesStart,
    Padding,

    LastUnnatturalAttribute = Padding,
    LastAttribute = LastUnnatturalAttribute,
    KeywordsStart = LastAttribute,
};
} //AttributeKey

namespace Keyword
{
enum
{
    SchoolOperator = AttributeKey::KeywordsStart,
    Dot,
    Pound,
    Arrow,
    Colon,
    Comma,
    Semicolon,
    OpenBracket,
    CloseBracket,
    OpenSquare,
    CloseSquare,
    OpenParen,
    CloseParen,

    Pass,
    Clear,
    Delay,
    EaseIn,
    EaseOut,
    Model,
    State,
    This,
    Material,

    Start,
    Min,
    Max,

    Or,
    And,
    Xor,
    Not,

    //Not exactly keywords...
    ScalarType,
    FloatType,
    ScopedValueKey,
    AttributeProperty,
    UserToken,
    UserString,
    ColorLiteral,

    EqualSign,
    UnitPercent,
    UnitStandard,
    UnitMilliseconds,
    UnitSeconds,

    OperatorPlus,
    OperatorMinus,
    OperatorTimes,
    OperatorDivide,

    Expression,
    Graph,
    ScalarKeyword,
    Normalize,

    Linear,
    Smooth,

    Declare,
    Set,

    PropertiesStart
};
}//Keywords

namespace Property
{

enum
{
    PropertiesStart = Keyword::PropertiesStart,
    AdjacentIsHorizontal = PropertiesStart,
    AdjacentIsVertical,

    Greedy,
    Shrinky,

    Maximized,
    Minimized,
    Fullscreen,
    FullscreenTrue,

    TopDown,
    BottomUp,

    Inner,
    Center,
    Outer,

    FontSans,
    FontSansBold,
    FontSansItalic,
    FontSansBoldItalic,

    FontSerif,
    FontSerifBold,
    FontSerifItalic,
    FontSerifBoldItalic,

    FontMono,
    FontMonoBold,
    FontMonoItalic,
    FontMonoBoldItalic,

    OneToUnit,
    Native,
    Stretch,
    BestFit,

    Enable,
    Disable,

    PropertiesEnd,

    Custom = Keyword::UserToken, //Ah, cute.
};

}//Property

namespace Spec
{

enum
{
    spec = Property::PropertiesEnd,
    bleeding,
};

}//Spec

inline std::set<int> getAttributeExpressionTypeSet()
{
    using namespace AttributeKey;
    return {Margins,
                Padding,

                FontSize,

                TranslationO,
                TranslationA,

                MarginOppOut,
                MarginOppIn,
                MarginAdjIn,
                MarginAdjOut,

                PaddingOppOut,
                PaddingOppIn,
                PaddingAdjIn,
                PaddingAdjOut,

                SizeO,
                SizeA,
    };
}

inline std::set<int> getAttributePropertyTypeSet()
{
    using namespace AttributeKey;
    return {
        Orientation,

                WindowState,
                Visibility,

                AlignAdjacent,
                AlignOpposite,

                OverrideFillPrecedenceAdjacent,
                OverrideFillPrecedenceOpposite,

                Justify,

                WindowSizeStrategy,

                ImageSizeProperty,

                Borderless,
                Resizable,
                ModelOrder,
                Font,
    };
}

inline std::set<int> getAttributeStringLiteralTypeSet()
{
    using namespace AttributeKey;
    return {
        TitleText,
                Text,
                Font,
                ImagePath,
    };
}

inline std::set<int> getAttributeUserTokenTypeSet()
{
    using namespace AttributeKey;
    return {Layout, Widget};
}

inline std::set<int> getAttributeUserTokenListTypeSet()
{
    using namespace AttributeKey;
    return {CellNames};
}

//state key list type
inline std::set<KeyType> getAttributeStateKeyListTypeSet()
{
    using namespace AttributeKey;
    return {
    Triggers,
    InduceState,
    BindState,
    ToggleState,
    UnsetState,
    TriggerState,
    RadioState,
    };
}

inline std::set<int> getAttributeSingleScopedValueKeyType()
{
    using namespace AttributeKey;
    return {TitleText, Text};
}

inline std::set<int> getAttributeColorTypeSet()
{
    using namespace AttributeKey;
    return {
    ElementColor,
    FontColor,
    BorderColor,
    };
}

inline std::set<int> getAttributeUnnaturalTypeSet()
{
    using namespace AttributeKey;
    return {
        Margins,
        Padding,
    };
}

//Vector because the order is important
inline std::map<int, std::vector<int>> getNaturalKeysToUnnaturalKeyMap()
{
    using namespace AttributeKey;
    return {
        {Margins, {MarginOppOut, MarginOppIn, MarginAdjIn, MarginAdjOut}},
        {Padding, {PaddingOppOut, PaddingOppIn, PaddingAdjIn, PaddingAdjOut}},
    };
}

inline std::map<int, std::set<int>>  getAttributeKeyToValidPropertyList()
{
    const std::initializer_list<int> fillprecedenceSet = {Property::Greedy, Property::Shrinky};
    const std::initializer_list<int> booleanSet = {Property::Enable, Property::Disable};
    const std::initializer_list<int> alignSet = {Property::Inner, Property::Center, Property::Outer};

    return {
        {AttributeKey::Font,
            {Property::FontSans,
             Property::FontSansBold,
             Property::FontSansItalic,
             Property::FontSansBoldItalic,
             Property::FontSerif,
             Property::FontSerifBold,
             Property::FontSerifItalic,
             Property::FontSerifBoldItalic,
             Property::FontMono,
             Property::FontMonoBold,
             Property::FontMonoItalic,
             Property::FontMonoBoldItalic}},
        {AttributeKey::Orientation,
            {Property::AdjacentIsHorizontal,
             Property::AdjacentIsVertical}},
        {AttributeKey::OverrideFillPrecedenceAdjacent,
                    fillprecedenceSet},
        {AttributeKey::OverrideFillPrecedenceOpposite,
                    fillprecedenceSet},
        {AttributeKey::WindowState,
            {Property::Maximized,
             Property::Minimized,
             Property::Fullscreen,
             Property::FullscreenTrue}},
        {AttributeKey::Visibility,
            {Property::Enable,
             Property::Disable}},
        {AttributeKey::AlignAdjacent,
                    alignSet},
        {AttributeKey::AlignOpposite,
                    alignSet},
        {AttributeKey::Justify,
                    alignSet},
        {AttributeKey::WindowSizeStrategy,
            {Property::TopDown,
             Property::BottomUp}},
        {AttributeKey::ImageSizeProperty,
            {Property::OneToUnit,
             Property::Native,
             Property::Stretch,
             Property::BestFit}},
        {AttributeKey::Borderless,
                    booleanSet},
        {AttributeKey::Resizable,
                    booleanSet},
        {AttributeKey::ModelOrder,
                    booleanSet},
    };
}

inline std::map<std::string, int> getTokenToSymbolMap()
{
    return     {{"@", Keyword::SchoolOperator},
                {".", Keyword::Dot},
                {"#", Keyword::Pound},
                {"->", Keyword::Arrow},
                {":", Keyword::Colon},
                {",", Keyword::Comma},
                {";", Keyword::Semicolon},
                {"{", Keyword::OpenBracket},
                {"}", Keyword::CloseBracket},
                {"[", Keyword::OpenSquare},
                {"]", Keyword::CloseSquare},
                {"(", Keyword::OpenParen},
                {")", Keyword::CloseParen},

                {"=", Keyword::EqualSign},
                {"%", Keyword::UnitPercent},
                {"u", Keyword::UnitStandard},
                {"ms", Keyword::UnitMilliseconds},
                {"milliseconds", Keyword::UnitMilliseconds},
                {"sec", Keyword::UnitSeconds},
                {"seconds", Keyword::UnitSeconds},

                {"+", Keyword::OperatorPlus},
                {"-", Keyword::OperatorMinus},
                {"*", Keyword::OperatorTimes},
                {"/", Keyword::OperatorDivide},

                {"expression", Keyword::Expression},
                {"graph", Keyword::Graph},
                {"scalar", Keyword::ScalarKeyword},

                {"linear", Keyword::Linear},
                {"smooth", Keyword::Smooth},

                {"declare", Keyword::Declare},
                {"set", Keyword::Set},

                {"normalize", Keyword::Normalize},

                {"pass", Keyword::Pass},
                {"clear", Keyword::Clear},
                {"delay", Keyword::Delay},
                {"ease-in", Keyword::EaseIn},
                {"ease-out", Keyword::EaseOut},
                {"model", Keyword::Model},
                {"state", Keyword::State},
                {"this", Keyword::This},
                {"material", Keyword::Material},

                {"start", Keyword::Start},
                {"min", Keyword::Min},
                {"max", Keyword::Max},


                //Bitwise gamgee
                {"or", Keyword::Or},
                {"|", Keyword::Or},
                {"and", Keyword::And},
                {"&", Keyword::And},
                {"xor", Keyword::Xor},
                {"!=", Keyword::Xor},
                {"not", Keyword::Not},
                {"!", Keyword::Not},


                {"position-within", AttributeKey::PositionWithin},

                {"title-text", AttributeKey::TitleText},
                {"text", AttributeKey::Text},

                {"font", AttributeKey::Font},

                {"image-path", AttributeKey::ImagePath},

                {"font-size", AttributeKey::FontSize},

                {"orientation", AttributeKey::Orientation},

                {"trans-o", AttributeKey::TranslationO},
                {"trans-a", AttributeKey::TranslationA},
                {"trans-y", AttributeKey::TranslationO},
                {"trans-x", AttributeKey::TranslationA},

                {"size-o", AttributeKey::SizeO},
                {"size-a", AttributeKey::SizeA},
                {"size-y", AttributeKey::SizeO},
                {"size-x", AttributeKey::SizeA},
                {"height", AttributeKey::SizeO},
                {"width", AttributeKey::SizeA},

                {"margin-o-out", AttributeKey::MarginOppOut},
                {"margin-o-in", AttributeKey::MarginOppIn},
                {"margin-a-in", AttributeKey::MarginAdjIn},
                {"margin-a-out", AttributeKey::MarginAdjOut},

                {"margin-bottom", AttributeKey::MarginOppOut},
                {"margin-top", AttributeKey::MarginOppIn},
                {"margin-left", AttributeKey::MarginAdjIn},
                {"margin-right", AttributeKey::MarginAdjOut},

                {"padding-o-out", AttributeKey::PaddingOppOut},
                {"padding-o-in", AttributeKey::PaddingOppIn},
                {"padding-a-in", AttributeKey::PaddingAdjIn},
                {"padding-a-out", AttributeKey::PaddingAdjOut},

                {"padding-bottom", AttributeKey::PaddingOppOut},
                {"padding-top", AttributeKey::PaddingOppIn},
                {"padding-left", AttributeKey::PaddingAdjIn},
                {"padding-right", AttributeKey::PaddingAdjOut},

                {"window-state", AttributeKey::WindowState},
                {"visibility", AttributeKey::Visibility},

                {"align-a", AttributeKey::AlignAdjacent},
                {"align-o", AttributeKey::AlignOpposite},

                {"align-x", AttributeKey::AlignAdjacent},
                {"align-y", AttributeKey::AlignOpposite},

                {"fill-precedence-adjacent", AttributeKey::OverrideFillPrecedenceAdjacent},
                {"fill-precedence-opposite", AttributeKey::OverrideFillPrecedenceOpposite},

                {"fill-precedence-a", AttributeKey::OverrideFillPrecedenceAdjacent},
                {"fill-precedence-o", AttributeKey::OverrideFillPrecedenceOpposite},

                {"fill-precedence-x", AttributeKey::OverrideFillPrecedenceAdjacent},
                {"fill-precedence-y", AttributeKey::OverrideFillPrecedenceOpposite},

                {"justify", AttributeKey::Justify},

                {"window-size-strategy", AttributeKey::WindowSizeStrategy},

                {"borderless", AttributeKey::Borderless},
                {"resizable", AttributeKey::Resizable},
                {"model-order", AttributeKey::ModelOrder},

                {"color", AttributeKey::ElementColor},
                {"font-color", AttributeKey::FontColor},
                {"border-color", AttributeKey::BorderColor},

                {"induce-state", AttributeKey::InduceState},
                {"bind-state", AttributeKey::BindState},
                {"toggle-state", AttributeKey::ToggleState},
                {"unset-state", AttributeKey::UnsetState},
                {"trigger-state", AttributeKey::TriggerState},
                {"radio-state", AttributeKey::RadioState},

                {"trigger", AttributeKey::Triggers},

                {"layout", AttributeKey::Layout},
                {"widget", AttributeKey::Widget},
                {"cell-names", AttributeKey::CellNames},

                //Unnaturals
                {"margin", AttributeKey::Margins},
                {"padding", AttributeKey::Padding},

                //Property values

                {"adjacent-is-horizontal", Property::AdjacentIsHorizontal},
                {"adjacent-is-vertical", Property::AdjacentIsVertical},

                {"greedy", Property::Greedy},
                {"shrinky", Property::Shrinky},

                {"maximize", Property::Maximized},
                {"minimize", Property::Minimized},
                {"fullscreen", Property::Fullscreen},
                {"fullscreen-true", Property::FullscreenTrue},

                {"top-down", Property::TopDown},
                {"bottom-up", Property::BottomUp},

                {"align-inner", Property::Inner},
                {"align-center", Property::Center},
                {"align-outer", Property::Outer},
                {"align-left", Property::Inner},
                {"align-right", Property::Outer},
                {"align-top", Property::Inner},
                {"align-bottom", Property::Outer},

                {"sans",             Property::FontSans},
                {"sans-bold",        Property::FontSansBold},
                {"sans-italic",      Property::FontSansItalic},
                {"sans-bold-italic", Property::FontSansBoldItalic},
                {"sans-italic-bold", Property::FontSansBoldItalic},

                {"serif",             Property::FontSerif},
                {"serif-bold",        Property::FontSerifBold},
                {"serif-italic",      Property::FontSerifItalic},
                {"serif-bold-italic", Property::FontSerifBoldItalic},
                {"serif-italic-bold", Property::FontSerifBoldItalic},

                {"mono",             Property::FontMono},
                {"mono-bold",        Property::FontMonoBold},
                {"mono-italic",      Property::FontMonoItalic},
                {"mono-bold-italic", Property::FontMonoBoldItalic},
                {"mono-italic-bold", Property::FontMonoBoldItalic},

                {"one-to-unit",      Property::OneToUnit},
                {"native",           Property::Native},
                {"stretch",          Property::Stretch},
                {"best-fit",         Property::BestFit},

                {"enable", Property::Enable},
                {"disable", Property::Disable},






        {"spec", Spec::spec},
        {"bleeding", Spec::bleeding},

                };
}

inline std::set<int> getDelimitingSymbolSet()
{
    return {Keyword::SchoolOperator,
            Keyword::Dot,
            Keyword::Pound,
            Keyword::Arrow,
            Keyword::Colon,
            Keyword::Comma,
            Keyword::Semicolon,
            Keyword::OpenBracket,
            Keyword::CloseBracket,
            Keyword::OpenSquare,
            Keyword::CloseSquare,
            Keyword::OpenParen,
            Keyword::CloseParen,
            Keyword::Not,

                };
}

//------------------------------------Helpers below, all syntax defined above
inline std::map<int, std::string> getSymbolToTokenMap()
{
    std::map<int, std::string> result;
    auto aye = getTokenToSymbolMap();

    //This will overwrite aliases :/
    for(auto pair : aye)
        result[pair.second] = pair.first;

    return result;
}

}//IVD

#endif // KEYWORDS_H
