#pragma once

#include "expression.h"
#include "color.h"
#include "graph.h"

namespace IVD
{

struct ReferenceAttribute
{
    bool active;
    bool clear;

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

    void applyToEachScopedValueKey(std::function<void (ScopedValueKey &)> fun)
    {
        auto guard = [&](std::optional<Expression> optExpr)
        {
            if(optExpr) optExpr->applyToEachScopedValueKey(fun);
        };

        guard(starting);
        guard(min);
        guard(max);
        guard(expr);

        for(ScopedValueKey& key : keys) fun(key);
        if(singleKey) fun(*singleKey);
    }
};

}//IVD
