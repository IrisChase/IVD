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

#include <unordered_set>

#include "runtimeattributeset.h"
#include "environment.h"

namespace IVD
{


RuntimeAttributeSet::RuntimeAttributeSet(DisplayItem* context, const ReferenceAttributeSet& initializerSet):
    myContext(context),
    attrs(initializerSet.size(), AnimatableAttribute())
{
    //We only want the initializer set to initialize the state changing attribute set.
    //Otherwise we just need the attr size
    assert(myContext);
    for(int key = 0; key != initializerSet.size(); ++key)
    {
        attrs.at(key).init(context, key);
        context->getEnv()->setupEnvironmentCallbacksOnAttributeForKey(&attrs.at(key), key);

        if(initializerSet.attr[key].stateModifierAttr)
            stateChangingAttributes.push_back(&attrs.at(key));
    }
}

void RuntimeAttributeSet::applyToEachAttribute(std::function<void (AnimatableAttribute&)> fun)
{
    for(int key = 0; key != AttributeKey::AttributeCount; ++key)
        fun(attrs[key]);
}

void RuntimeAttributeSet::beginAttributeSetRecompute()
{
    declareModifiers = nullptr;
    setModifiers.clear();

    applyToEachAttribute([&](AnimatableAttribute& a)
    { a.beginAttributeRecompute(); });
}

void RuntimeAttributeSet::mergeIn(const ReferenceAttributeSet& other)
{
    //We just pretend that the data is clean and that only the first
    // reference set has declare modifiers...
    if(!declareModifiers) declareModifiers = &other.declareModifiers;

    setModifiers.push_back(&other.setModifiers);

    for(int key = 0; key != AttributeKey::AttributeCount; ++key)
        attrs[key].merge(other.attr[key]);
}

void RuntimeAttributeSet::commitAttributeSetRecompute()
{
    applyToEachAttribute([&](AnimatableAttribute& a)
    { a.commitAttributeRecompute(); });
}

void RuntimeAttributeSet::executeStateChangers()
{
    //We only update state modifiers every time the attribute set is updated. Not triggers.
    //TODO unclear code...
    for(auto attr : stateChangingAttributes)
    {
        //Not considered for quick, logical things
        //TODOOOOOOOO
        if(attr->thisIsAhackButCheckIfThereIsDelay()) continue;

        attr->executeChangeAcceptor();
    }
}

void RuntimeAttributeSet::fireSets()
{
    std::unordered_set<const Expression*> firedExpressions;

    //This is still O(N), the inner loop is just a sub-range don't lose your shit.
    for(auto& innerSet : setModifiers)
    {
        for(auto& pair : *innerSet)
        {
            const Expression* expr = &pair.second;

            if(firedExpressions.count(expr)) continue;

            myContext->getEnv()->setInteger(myContext, pair.first, expr->solve(myContext));

            firedExpressions.insert(expr);
        }
    }
}

std::optional<double> RuntimeAttributeSet::getDeclaredInt(ValueKey key)
{
    if(!declareModifiers->count(key)) return std::optional<double>();
    return declareModifiers->at(key).solve(myContext);
}

void RuntimeAttributeSet::setDeclaredInt(const ValueKey key, const double proposed)
{
    assert(declareModifiers->count(key));

    const Expression& expr = declareModifiers->at(key);

    assert(expr.checkContainsWeak());

    expr.solveForAndPropogateWeak(myContext, proposed);
}


}//IVD
