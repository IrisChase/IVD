#pragma once

#include "expression.h"
#include "color.h"
#include "graph.h"

namespace IVD
{

struct ReferenceAttribute
{
    bool active = false;
    bool clear = false;
    bool stateModifierAttr = false;

    std::optional<int> property;

    std::optional<Expression> starting;
    std::optional<Expression> min;
    std::optional<Expression> max;

    std::optional<Expression> expr;

    std::optional<Color> color;

    std::optional<std::string> literal;
    std::vector<std::string> literalList; //Don't be such a literalist GODDDDDDDD

    std::optional<ScopedValueKey> singleKey;
    std::vector<ScopedValueKey> keys;

    std::optional<int> delay;
    std::optional<Animation::Transition> ease;

    void derive(const ReferenceAttribute& other);
    void applyToEachScopedValueKey(std::function<void (ScopedValueKey &)> fun);
};

}//IVD
