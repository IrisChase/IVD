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

#include "runtimeattribute.h"

#include "irisutils/routine.h"

#include "runtimeattributeset.h"

#include <reprodyne.h>

namespace IVD
{

std::optional<double> RuntimeAttribute::getValue(DisplayItem* theContext) const
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

    std::optional<double> value;

    if(!expr) return value;

    value = expr->solve(theContext);

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

void RuntimeAttribute::setValue(const double proposed, DisplayItem* theContext)
{
    starting = nullptr;

    if(expr) expr->solveForAndPropogateWeak(theContext, proposed);
}

template<typename T, typename TO>
static void mergeHelper(T& mine, const TO& other)
{
    if(other) mine = &other;
}

template<typename T, typename TO>
static void mergeHelper(T& mine, const std::optional<TO>& other)
{
    if(other) mine = &*other;
}

template<typename T, typename TO>
static void mergeHelper(T& mine, const std::vector<TO>& other)
{
    if(other.size()) mine = other;
}

void RuntimeAttribute::merge(const ReferenceAttribute& other)
{
    mergeHelper(property, other.property);
    mergeHelper(starting, other.starting);
    mergeHelper(min, other.min);
    mergeHelper(max, other.max);
    mergeHelper(expr, other.expr);
    mergeHelper(color, other.color);
    mergeHelper(literal, other.literal);
    mergeHelper(singleKey, other.singleKey);

    IrisUtils::Routine::appendContainer(keys, other.keys);
    IrisUtils::Routine::appendContainer(literalList, other.literalList);
}

void RuntimeAttribute::reset()
{
    clear = nullptr;
    property = nullptr;
    starting = min = max = expr = nullptr;
    color = nullptr;
    literal = nullptr;
    literalList.clear();
    singleKey = nullptr;
    keys.clear();
}

const RuntimeAttribute& AnimatableAttribute::getCorrectRTA() const
{
    return lastRatio == 1 ? currentRTA
                          : previousRTA;
}

void AnimatableAttribute::animationTick()
{
    double changeRatio = 0;

    if(!animationStart)
    {
        animationStart = std::chrono::steady_clock::now();
        lastRatio = 0; //todo
    }
    else
    {
        //Guaranteed to only be ease or delay.
        const auto maxDuration = std::chrono::milliseconds(ease ? ease->miliseconds
                                                                : *delay);

        const auto startTimePoint   = (*animationStart);
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

    //This block is for Reprodyne stuff
    {
        //Scope to displayitem itself because our pointer should be able to change.
        std::string key;
        key = getLiteralForSymbol(myAttributeKey);
        key += "-:-animation-ratio";

        changeRatio = reprodyne_intercept_double(revealContext(), key.c_str(), changeRatio);
    }

    if(lastRatio < changeRatio)
    {
        lastRatio = changeRatio;

        if(checkAttributeKeyForExpressionBodyType(myAttributeKey))
            signalChangedAttribute(this);
    }

    if(lastRatio == 1) cancelAnimationTicker(this);
}

void AnimatableAttribute::beginAttributeRecompute()
{
    //We store this not only for animation, but for checking if anything
    // changed in commitAttributeRecompute()
    previousRTA = currentRTA;

    ease = nullptr;
    delay = nullptr;
    active = false;

    currentRTA.reset();
}

void AnimatableAttribute::merge(const ReferenceAttribute& ref)
{
    if(currentRTA.checkClear() || !ref.active) return;

    active = true;

    if(ref.ease  || ref.clear)   ease = &*ref.ease;
    if(ref.delay || ref.clear)  delay = &*ref.delay;

    currentRTA.merge(ref);
}

void AnimatableAttribute::commitAttributeRecompute()
{
    lastRatio = ease || delay ? 0
                              : 1;

    if(!lastRatio) cancelAnimationTicker(this); //Why not call "quitAnimation"? TODO

    //TODO: Compare if they're different.
    if(previousRTA != currentRTA)
        signalChange();
}


std::optional<double> AnimatableAttribute::getValue() const
{
    std::optional<double> result;

    if(lastRatio != 1)
    {
        auto optionalOrigin = previousRTA.getValue(theContext);
        auto optionalDest   =  currentRTA.getValue(theContext);

        if(!optionalOrigin || !optionalDest) return result; //wtf condition

        return ease->graph.getInterpolatedScalarForPercentage(*optionalOrigin,
                                                              *optionalDest,
                                                              lastRatio);
    }
    else return currentRTA.getValue(theContext);
}


std::optional<int> AnimatableAttribute::getProperty() const
{
    const int* i = getCorrectRTA().property;
    return i ? *i
             : std::optional<int>();
}

std::optional<ScopedValueKey> AnimatableAttribute::getSingleValueKey() const
{
    const ScopedValueKey* i = getCorrectRTA().singleKey;
    return i ? *i
             : std::optional<ScopedValueKey>();
}

std::vector<ScopedValueKey> AnimatableAttribute::getValueKeyList() const
{
    return getCorrectRTA().keys;
}

std::optional<std::string> AnimatableAttribute::getUserToken() const
{
    const std::string* i = getCorrectRTA().literal;
    return i ? *i
             : std::optional<std::string>();
}

std::optional<Color> AnimatableAttribute::getColor() const
{
    const Color* i = getCorrectRTA().color;
    return i ? *i
             : std::optional<Color>();
}


} //IVD
