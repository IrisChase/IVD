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

#include "attribute.h"

#include "irisutils/routine.h"

#include "attributeset.h"

#include <reprodyne.h>

namespace IVD
{

std::optional<double> Attribute::getValue()
{
    //For simplicity's sake, min/max are used to round off observed values,
    // but are not calculated in back-propogation, and thus there is no
    // back and forth. An attribute simply takes in a suggested value,
    // calculates it, and when you need to actually observe the value
    // (which is done in a seperate function from from set), the bounds
    // are checked, and rounded, and that should be enough. If there are
    // min/max constraints on both attributes, it gets complicated real quick,
    // and I haven't found a good use case for that. min/max is good for
    // making sure a scalar is within range, but I see no reason why
    // the two values must agree if they both have constraints.
    //tl;dl local attribute min/max takes precedence over everything else.

    std::optional<double> value = 0;

    if(!expr) return value;

    if(myAnimationData.lastRatio != 1)
    {
        const double origin = myAnimationData.originExpr->solve(theContext);
        const double dest = expr->solve(theContext);
        value = ease->graph.getInterpolatedScalarForPercentage(origin, dest, myAnimationData.lastRatio);
    }
    else value = expr->solve(theContext);

    if(min)
    {
        const double computedMin = min->solve(theContext);
        if(computedMin > value) value = computedMin;
    }

    if(max)
    {
        const double computedMax = max->solve(theContext);
        if(computedMax < value) value = computedMax;
    }

    return value;
}

void Attribute::setValue(const double proposed)
{
    starting.reset();

    if(expr) expr->solveForAndPropogateWeak(theContext, proposed);
}

void Attribute::updateToReferenceAttribute(const Attribute& other)
{

    if(checkSameInPractice(other)) return;

    cancelAnimationTicker(this);
    const auto saved = *this;

    //See you... On the other side~
    operator=(other);

    if(ease || delay)
    {
        *myAnimationData.outgoingAttribute = saved;
        *myAnimationData.pendingAttribute  = *this;

        myAnimationData.originExpr = myAnimationData.outgoingAttribute->expr;
    }

    signalChange();
}

template<typename T>
bool innerHelper(Attribute* attr, T& location, const T start, const T end, const double ratio)
{
    if(ratio == 0)
    {
        if(location == start) return false;

        location = start;
    }
    else if(ratio == 1)
    {
        if(location == end) return false;

        location = end;
        attr->quitAnimation();
    }
    else return false;
    return true;
}

template<typename T>
bool helper(Attribute* attr, T& location, const T& start, const T& end, const double ratio)
{ return innerHelper(attr, location, start, end, ratio); }

template<typename T>
bool helper(Attribute* attr,
            std::optional<T>& location,
            const std::optional<T>& start,
            const std::optional<T>& end,
            const double ratio)
{
    if(!start)
    {
        std::optional<T> anustart = T();
        auto a = anustart;
        return innerHelper(attr, location, anustart, end, ratio);
    }
    return innerHelper(attr, location, start, end, ratio);
}

bool Attribute::transitionCreeper(const double ratio)
{
    assert(myAnimationData.outgoingAttribute);
    assert(myAnimationData.pendingAttribute);

    const auto& outgoing = myAnimationData.outgoingAttribute;
    const auto& pendingAttr = myAnimationData.pendingAttribute;

    //I could do you one better in lisp but alas I'm writing in a crippled language.
    auto setupStandardDelay = [&](auto& location, const auto& origin, const auto& dest)
    {
        return helper(this, location, origin, dest, ratio);
    };


    if(checkAttributeKeyForExpressionBodyType(myAttributeKey))
    {
        if(ratio == 1) cancelAnimationTicker(this);

        if(myAnimationData.lastRatio <= ratio)
        {
            myAnimationData.lastRatio = ratio; //eeeehh
            return true; //Signal that we changed things.
        }
        return false;

        //We just compute it on the fly in getInteger.
    }
    else if(checkAttributeKeyForPropertyBodyType(myAttributeKey))
        return setupStandardDelay(property, outgoing->property, pendingAttr->property);
    else if(checkAttributeKeyForStringLiteralType(myAttributeKey) || checkAttributeKeyForUserTokenType(myAttributeKey))
    {
        return setupStandardDelay(literal, outgoing->literal, pendingAttr->literal);
    }
    else if(checkAttributeKeyForUserTokenList(myAttributeKey))
        return setupStandardDelay(literalList, outgoing->literalList, pendingAttr->literalList);
    if(checkAttributeKeyForStateKeyListType(myAttributeKey))
    {
        //SPECIAL CASE: Do NOT use the previous value. Otherwise triggers and state changers
        // would continue to execute...
        return setupStandardDelay(keys, std::vector<ScopedValueKey>{}, pendingAttr->keys);
    }
    else if(checkAttributeKeyForSingleScopedValueKeyType(myAttributeKey))
        return setupStandardDelay(singleKey, outgoing->singleKey, pendingAttr->singleKey);
    else if(checkAttributeKeyForColorType(myAttributeKey))
    {
        //TODO: color interpolation.
        return setupStandardDelay(color, outgoing->color, pendingAttr->color);
    }
    assert(false);
}

void Attribute::animationTick()
{
    double changeRatio = 0;

    if(!myAnimationData.animationStart)
    {
        myAnimationData.animationStart = std::chrono::steady_clock::now();
        myAnimationData.lastRatio = 0; //todo
    }
    else
    {
        //Guaranteed to only be ease or delay.
        const auto maxDuration = std::chrono::milliseconds(ease ? ease->miliseconds
                                                                : *delay);

        const auto startTimePoint   = (*myAnimationData.animationStart);
        const auto elapsedTime      = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTimePoint);


        if(elapsedTime >= maxDuration)
        {
            changeRatio = 1;
        }
        else if(ease) //Delay kind of takes care of itself~
        {
            const double e = elapsedTime.count();
            const double m = maxDuration.count();



            changeRatio = e / m;
        }
    }

    {
        //Scope to displayitem itself because our pointer should be able to change.
        std::string key;
        key = getLiteralForSymbol(myAttributeKey);
        key += "-:-animation-ratio";

        changeRatio = reprodyne_intercept_double(revealContext(), key.c_str(), changeRatio);
    }


    //TODO: Busy atm but I just realized that there is absolutely no reason for non-ease type
    // attributes to call transition every damn time. Only on changes, people. FIX IT

    if(transitionCreeper(changeRatio)) signalChangedAttribute(this);
}

void Attribute::internalMerge(const Attribute& other, const bool modeDerive)
{
    //This seems backwards for the runtime... Should just copy-in the cleared
    // attribute if we're not in mode-derive?
    // Shouldn't this be read ther otherway around? TODO
    if(clear) return;

    if(!other.active) return;

    active = true;

    auto mergeHelper = [modeDerive](auto& mine, const auto& other)
    {
        if(modeDerive  && !mine) mine = other;
        if(!modeDerive && other) mine = other;
    };

    mergeHelper(property, other.property);

    mergeHelper(starting, other.starting);
    mergeHelper(min, other.min);
    mergeHelper(max, other.max);
    mergeHelper(expr, other.expr);

    mergeHelper(color, other.color);

    mergeHelper(literal, other.literal);
    mergeHelper(singleKey, other.singleKey);

    mergeHelper(delay, other.delay);
    mergeHelper(ease, other.ease);

    IrisUtils::Routine::appendContainer(keys, other.keys);
    IrisUtils::Routine::appendContainer(literalList, other.literalList);
}




void Attribute::applyToEachScopedValueKey(std::function<void (ScopedValueKey &)> fun)
{
    auto guard = [&](std::optional<Expression>& optExpr)
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

bool Attribute::checkSameInPractice(const Attribute& other)
{
    auto tie = [](const Attribute& theAttr)
    {
        return std::tie(theAttr.property,
                        theAttr.starting,
                        theAttr.min,
                        theAttr.max,
                        theAttr.expr,
                        theAttr.color,
                        theAttr.literal,
                        theAttr.literalList,
                        theAttr.singleKey,
                        theAttr.keys,
                        theAttr.delay,
                        theAttr.ease);
    };

    return tie(*this) == tie(other);
}


} //IVD
