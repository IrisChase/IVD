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


namespace IVD
{

class Attribute;
typedef std::optional<std::reference_wrapper<Attribute>> OptionalAttributeReference;

class Attribute
{
    friend class Compiler;
    friend class AttributeSet;
    friend void ::printOutAttributes(IVD::Element);

    int myAttributeKey;

    DisplayItem* theContext;

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

    std::function<void(Attribute*)> requestAnimationTicker;
    std::function<void(Attribute*)> cancelAnimationTicker;
    std::function<void(Attribute*)> signalChangedAttribute;
    std::function<void(Attribute*)> changeAcceptor;


    struct AnimationData
    {
        std::optional<std::chrono::time_point<std::chrono::steady_clock>> animationStart;
        IrisUtils::DeepCopyableUnique<Attribute> pendingAttribute;
        IrisUtils::DeepCopyableUnique<Attribute> outgoingAttribute;

        double lastRatio;

        std::optional<Expression> originExpr;

        //lastRatio == 1 means that getValue won't try to animate
        AnimationData(): lastRatio(1) {}
    };

    AnimationData myAnimationData;

    bool transitionCreeper(const double ratio);


    void internalMerge(const Attribute& other, const bool modeDerive);


public:
    Attribute(): theContext(nullptr), active(false), clear(false) {}

    void initializeTransitionSystem(const int key)
    {
        myAttributeKey = key;
        myAnimationData.pendingAttribute.makeCopy(Attribute());
        myAnimationData.outgoingAttribute.makeCopy(Attribute());
    }

    DisplayItem* revealContext()
    { return theContext; }

    void setAnimationTickRequester(std::function<void(Attribute*)> fun)
    { requestAnimationTicker = fun; } //I mean, it *is* a lot of fun!
     void setCancelAnimationTicker(std::function<void(Attribute*)> fun)
    { cancelAnimationTicker = fun; }
     void setSignalChangedAttribute(std::function<void(Attribute*)> fun)
    { signalChangedAttribute = fun; }
     void setChangeAcceptor(std::function<void(Attribute*)> fun)
    { changeAcceptor = fun; }

    std::optional<double> getValue();
    void setValue(const double proposed);

    void derive(const Attribute& other) { internalMerge(other, true); }
    void merge(const Attribute& other)  { internalMerge(other, false); }
    void updateToReferenceAttribute(const Attribute& other);

    void signalChange()
    {
        assert(signalChangedAttribute);
        assert(requestAnimationTicker);

        if(ease || delay) requestAnimationTicker(this);
        else signalChangedAttribute(this);
    }

    void executeChangeAcceptor()
    {
        assert(changeAcceptor); //Because assert gives line numbers and a bad function call exception does not.
        changeAcceptor(this);
    }

    void animationTick();

    void quitAnimation()
    {
        cancelAnimationTicker(this);
        myAnimationData.animationStart.reset();
    }


    void applyToEachScopedValueKey(std::function<void(ScopedValueKey&)> fun);

    auto getScopedValueKeys()
    { return keys; }

    bool checkSameInPractice(const Attribute& other);


    IRISUTILS_DEFINE_COMP(Attribute,
                          theContext,
                          active, clear,
                          property,
                          starting, min, max, expr,
                          color,
                          literal, literalList,
                          singleKey, keys,
                          delay,
                          ease)
};


}//IVD

#endif // ATTRIBUTE_H
