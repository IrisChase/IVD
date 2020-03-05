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

#include "attributeset.h"
#include "modelitembase.h"
#include "environment.h"

namespace IVD
{

void AttributeSet::mergeModifiers(const AttributeSet& other)
{
    //declares are always on the default state, but since this method
    // is used to merge classes as well as states, we just merge declares
    // here, and rely on the compiler to prevent declares from being added
    // to attribute sets that refer to a different state.

    for(auto pair : other.declareModifiers)
        declareModifiers[pair.first] = pair.second;

    for(auto pair : other.setModifiers)
        setModifiers[pair.first] = pair.second;
}

AttributeSet::AttributeSet(DisplayItem* context):
    myContext(context)
{
    assert(myContext);
    for(int key = 0; key != AttributeKey::AttributeCount; ++key)
    {
        attr.at(key).initializeTransitionSystem(key);
        attr.at(key).theContext = context;
        context->getEnv()->setupEnvironmentCallbacksOnAttributeForKey(&attr.at(key), key);
    }
}

void AttributeSet::applyToEachScopedValueKey(std::function<void (ScopedValueKey &)> fun)
{
    for(auto& it : declareModifiers)
        it.second.applyToEachScopedValueKey(fun);
    for(auto& it : setModifiers)
        it.second.applyToEachScopedValueKey(fun);

    for(int i = 0; i != AttributeKey::AttributeCount; ++i)
        attr[i].applyToEachScopedValueKey(fun);
}

void AttributeSet::deriveFrom(const AttributeSet& other)
{
    mergeModifiers(other);
    
    for(int key = 0; key != AttributeKey::AttributeCount; ++key)
        attr[key].derive(other.attr[key]);
}

void AttributeSet::mergeIn(const AttributeSet& other)
{
    mergeModifiers(other);

    for(int key = 0; key != AttributeKey::AttributeCount; ++key)
        attr[key].merge(other.attr[key]);
}

void AttributeSet::updateToReferenceSet(const AttributeSet& other)
{
    mergeModifiers(other);

    for(KeyType key = 0; key != AttributeKey::AttributeCount; ++key)
        attr.at(key).updateToReferenceAttribute(other.attr.at(key));
}

void AttributeSet::executeStateChangers()
{
    //We only update state modifiers every time the attribute set is updated. Not triggers.
    //TODO unclear code...
    for(KeyType key = AttributeKey::InduceState; key != AttributeKey::LastStateKeyAttr + 1; ++key)
    { attr.at(key).executeChangeAcceptor(); }
}

void AttributeSet::fireSets()
{
    for(auto& pair : setModifiers)
        myContext->getEnv()->setInteger(myContext, pair.first, pair.second.solve(myContext));
}

std::optional<double> AttributeSet::getInt(ValueKey key)
{
    const auto optionalSymbol = getSymbolForLiteral(key.c_str());
    if(optionalSymbol) return getInt(*optionalSymbol);

    if(!declareModifiers.count(key)) return std::optional<double>();
    return declareModifiers[key].solve(myContext);
}

void AttributeSet::setInteger(const ValueKey key, const double proposed)
{
    const auto optionalSymbol = getSymbolForLiteral(key.c_str());
    if(optionalSymbol)
    {
        setInteger(*optionalSymbol, proposed);
        return;
    }

    assert(declareModifiers.count(key));

    Expression& expr = declareModifiers[key];

    assert(expr.checkContainsWeak());

    expr.solveForAndPropogateWeak(myContext, proposed);
}


}//IVD
