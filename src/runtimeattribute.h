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

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <optional>
#include <chrono>

#include "irisutils/lexcompare.h"
#include "irisutils/easyuniquepointer.h"

#include "color.h"
#include "expression.h"
#include "graph.h"

#include "referenceattribute.h"


namespace IVD
{


class AnimatableAttribute;


class RuntimeAttribute //1224 bytes before 632 after~ NOW 248... 96... 128 Fack
{
    //Doesn't really need to be a friend it can just be all public because AnimatableAttribute
    // is all that uses it... Or just make it internal, rename to data, save "runtimeAttribute"
    // as the public class name.....
    friend class AnimatableAttribute;
    int myAttributeKey = -1; //TODO: make constant

    //No "active" because if this exists, it's active

    const bool* clear = nullptr;
    const int* property = nullptr;
    const Expression* starting = nullptr;
    const Expression* min = nullptr;
    const Expression* max = nullptr;
    const Expression* expr = nullptr;
    const Color* color = nullptr;
    const std::string* literal = nullptr;
    std::vector<std::string> literalList; //Don't be such a literalist GODDDDDDDD
    const ScopedValueKey* singleKey = nullptr;
    std::vector<ScopedValueKey> keys;

public:

    void initializeTransitionSystem(const int key)
    { myAttributeKey = key; }

    void reset();


    std::optional<double> getValue(DisplayItem* theContext) const;
    void setValue(const double proposed, DisplayItem* theContext);

    void merge(const ReferenceAttribute& other);

    void applyToEachScopedValueKey(std::function<void(ScopedValueKey&)> fun);

    auto getScopedValueKeys()
    { return keys; }

    bool checkClear()
    { return clear; }

    IRISUTILS_DEFINE_COMP(RuntimeAttribute,
                          clear,
                          property,
                          starting, min, max, expr,
                          color,
                          literal,
                          literalList,
                          singleKey,
                          keys)
};

class AnimatableAttribute //440 byte struct replacing a 1224 byte struct, noice
{
    int myAttributeKey = -1;
    DisplayItem* theContext = nullptr;

    bool active = false;

    RuntimeAttribute previousRTA; //Lol RTA
    RuntimeAttribute currentRTA;


    //These store the current values to preserve them in case
    // the recompute is redundant. (TODO: Should recomputes just not do this, period?)
    RuntimeAttribute checkpointRTA;
    double checkpointRatio;

    //These point to the (current) reference attribute
    const int* delay = nullptr; //Yeah it's silly indirection for an integer but it's consistent.
    const Animation::Transition* ease = nullptr;

    std::optional<std::chrono::time_point<std::chrono::steady_clock>> animationStart;
    //lastRatio == 1 means animation is finished.
    double lastRatio = 1;

    std::function<void(AnimatableAttribute*)> signalChangedAttribute;
    std::function<void(AnimatableAttribute*)> changeAcceptor;
    std::function<void(AnimatableAttribute*)> requestAnimationTicker;
    std::function<void(AnimatableAttribute*)> cancelAnimationTicker;

    const RuntimeAttribute& getCorrectRTA() const;

public:

    void init(DisplayItem* thethecontext, const int key)
    {
        theContext = thethecontext;
        myAttributeKey = key;
    }

    bool checkActive()
    { return active; }

    bool thisIsAhackButCheckIfThereIsDelay()
    { return delay; }

    DisplayItem* revealContext()
    { return theContext; }

     void setSignalChangedAttribute(std::function<void(AnimatableAttribute*)> fun)
    { signalChangedAttribute = fun; }
     void setChangeAcceptor(std::function<void(AnimatableAttribute*)> fun)
    { changeAcceptor = fun; }
    void setAnimationTickRequester(std::function<void(AnimatableAttribute*)> fun)
    { requestAnimationTicker = fun; } //I mean, it *is* a lot of fun!
     void setCancelAnimationTicker(std::function<void(AnimatableAttribute*)> fun)
    { cancelAnimationTicker = fun; }

    void animationTick();

    void quitAnimation()
    {
        cancelAnimationTicker(this);
        animationStart.reset();
    }

    void executeChangeAcceptor()
    {
        assert(changeAcceptor); //Because assert gives line numbers and a bad function call exception does not.
        changeAcceptor(this);
    }

    void signalChange()
    {
        assert(signalChangedAttribute);

        signalChangedAttribute(this);
    }

    RuntimeAttribute& getCurrent()
    { return currentRTA; }

    void beginAttributeRecompute();
    void merge(const ReferenceAttribute& ref);
    void commitAttributeRecompute();


    //data interface
    void setValue(const double proposed)
    { currentRTA.setValue(proposed, theContext); }

    std::optional<double> getValue() const;
    bool checkExprIsConst() const
    {
        auto expr = currentRTA.expr;
        return expr ? !expr->checkContainsWeak()
                    : true;
    }
    std::optional<int> getProperty() const;
    std::vector<std::string> getLiteralList() const;
    std::optional<ScopedValueKey> getSingleValueKey() const;
    std::vector<ScopedValueKey> getValueKeyList() const;
    std::optional<std::string> getUserToken() const;
    std::optional<Color> getColor() const;
};


}//IVD

#endif // ATTRIBUTE_H
