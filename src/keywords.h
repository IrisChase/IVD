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

#ifndef KEYWORDS_H
#define KEYWORDS_H

#include "assert.h"
#include <vector>

#include <string>
#include <optional>

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

    //Expression types
    FirstScalarAttribute,

    FontSize = FirstScalarAttribute,

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

    LastScalarAttribute = SizeA,

    //Properties
    FirstPropertyAttribute,

    Orientation = FirstPropertyAttribute,

    WindowState,
    Visibility,

    AlignAdjacent,
    AlignOpposite,

    Justify,

    WindowSizeStrategy,

    ImageSizeProperty,

    Borderless,
    Resizable,
    ModelOrder,

    LastPropertyAttribute = ModelOrder,

    FirstColorTypeAttr,
    ElementColor = FirstColorTypeAttr,
    FontColor,
    BorderColor,

    LastColorTypeAttr = BorderColor,


    //Scoped value key lists
    FirstStateKeyAttr,

    Triggers = FirstStateKeyAttr,
    InduceState,
    BindState,
    ToggleState,
    UnsetState,
    TriggerState,
    RadioState,

    LastStateKeyAttr = RadioState,


    Layout,
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
    Vbox = PropertiesStart,
    Hbox,
    Inline,
    FreeLayout,
    StackLayout,
    Standard,
    ImageMaterial,

    AdjacentIsHorizontal,
    AdjacentIsVertical,

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


inline bool checkAttributeKeyForExpressionBodyType(const int key)
{
    switch(key)
    {
    case AttributeKey::Margins:
    case AttributeKey::Padding:
        return true;

    default:
    if(key >= AttributeKey::FirstScalarAttribute && key <= AttributeKey::LastScalarAttribute)
        return true;
    }

    return false;
}

inline bool checkAttributeKeyForPropertyBodyType(const int key)
{
    using namespace AttributeKey;
    if(key >= FirstPropertyAttribute && key <= LastPropertyAttribute) return true;

    switch(key)
    {
    case Font:
    case Layout:
        return true;
    default: return false;
    }
}

inline bool checkAttributeKeyForStringLiteralType(const int key)
{
    using namespace AttributeKey;
    switch(key)
    {
    case TitleText:
    case Text:
    case Font:
    case ImagePath:
        return true;
    default: return false;
    }
}

inline bool checkAttributeKeyForUserTokenType(const int key)
{
    using namespace AttributeKey;
    switch(key)
    {
    case Layout:
        return true;
    default: return false;
    }
}

inline bool checkAttributeKeyForUserTokenList(const int key)
{
    using namespace AttributeKey;
    switch(key)
    {
    case CellNames:
        return true;
    default: return false;
    }
}

inline bool checkAttributeKeyForStateKeyListType(const int key)
{
    using namespace AttributeKey;
    return key >= FirstStateKeyAttr && key <= LastStateKeyAttr;
}

inline bool checkAttributeKeyForSingleScopedValueKeyType(const int key)
{
    using namespace AttributeKey;
    switch(key)
    {
    case TitleText:
    case Text:
        return true;
    default: return false;
    }
}

inline bool checkAttributeKeyForColorType(const int key)
{
    using namespace AttributeKey;
    return key >= FirstColorTypeAttr && key <= LastColorTypeAttr;
}

inline bool checkAttributeKeyForUnnaturalType(const int key)
{
    return key >= AttributeKey::UnnaturalAttributesStart &&
           key <= AttributeKey::LastUnnatturalAttribute;
}

inline std::vector<int> getNaturalKeysForUnnaturalKey(const int key)
{
    using namespace AttributeKey;

    switch(key)
    {
    case Margins:
        return {MarginOppOut, MarginOppIn, MarginAdjIn, MarginAdjOut};
    case Padding:
        return {PaddingOppOut, PaddingOppIn, PaddingAdjIn, PaddingAdjOut};
    }

    assert(false);
}

inline std::vector<int> getValidPropertyValuesForType(const int type)
{
    switch (type)
    {
    case AttributeKey::Font:
        return {Property::FontSans,
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
                Property::FontMonoBoldItalic};

    case AttributeKey::Layout:
        return {Property::Vbox, Property::Hbox, Property::Inline, Property::Standard, Property::ImageMaterial};

    case AttributeKey::Orientation:
        return {Property::AdjacentIsHorizontal, Property::AdjacentIsVertical};

    case AttributeKey::WindowState:
        return {Property::Maximized,
                Property::Minimized,
                Property::Fullscreen,
                Property::FullscreenTrue};

    case AttributeKey::Visibility:
        return {Property::Enable, Property::Disable};

    case AttributeKey::AlignAdjacent:
    case AttributeKey::AlignOpposite:
    case AttributeKey::Justify:
        return {Property::Inner, Property::Center, Property::Outer};

    case AttributeKey::WindowSizeStrategy:
        return {Property::TopDown, Property::BottomUp};

    case AttributeKey::ImageSizeProperty:
        return {Property::OneToUnit, Property::Native, Property::Stretch, Property::BestFit};

    case AttributeKey::Borderless:
    case AttributeKey::Resizable:
    case AttributeKey::ModelOrder:
        return {Property::Enable, Property::Disable};
    }

    assert(false);
}

struct KeywordSymbolValuePair
{
    const char* value;
    int symbol;
};

inline std::vector<KeywordSymbolValuePair> getSymbolValuePairs()
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
                {"cell-names", AttributeKey::CellNames},

                //Unnaturals
                {"margin", AttributeKey::Margins},
                {"padding", AttributeKey::Padding},

                //Property values
                {"vbox", Property::Vbox},
                {"hbox", Property::Hbox},
                {"inline", Property::Inline},
                {"free-layout", Property::FreeLayout},
                {"stack", Property::StackLayout},
                {"standard", Property::Standard},
                {"image-material", Property::ImageMaterial},

                {"adjacent-is-horizontal", Property::AdjacentIsHorizontal},
                {"adjacent-is-vertical", Property::AdjacentIsVertical},

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

inline bool checkIsDelimitingSymbol(const int sym)
{
    //I realize this looks bad, but the idea is that the compiler
    // can tell that this is a constant mapping... That's the idea.
    //(And it's honestly a small enough search that linear complexity ain't bad)
    for(auto pair : getSymbolValuePairs())
        for(auto candidate : {Keyword::SchoolOperator,
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
                              })
            if(sym == candidate)
                return true;

    return false;
}

inline std::optional<int> getSymbolForLiteral(const std::string literal)
{
    for(auto pair : getSymbolValuePairs())
        if(pair.value == literal)
            return pair.symbol;

    return std::optional<int>();
}

//This is just to make the code in the tokenizer prettier by abstraction.
//Feel free to make not ugly.
inline bool checkIsDelimitingSymbol(const char literal)
{
    const auto optional = getSymbolForLiteral(std::string({literal}));

    if(!optional) return false;

    return checkIsDelimitingSymbol(*optional);
}

inline const char* getLiteralForSymbol(const int sym)
{
    for(auto pair : getSymbolValuePairs())
        if(pair.symbol == sym)
            return pair.value;

    assert(false); //No reason in the world to get here.
}


}//IVD

#endif // KEYWORDS_H
