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

#include "valuekey.h"
#include "modelitembase.h"
#include "expression.h"

#include <sstream>

#include "irisutils/routine.h"

#include "assert.h"

namespace IVD
{

ScopedValueKey::ScopedValueKey() {}
ScopedValueKey::ScopedValueKey(Scope theScope): myScope(theScope) {}
ScopedValueKey::~ScopedValueKey()
{ /*Just need that complete type for Expression?*/ }

ScopedValueKey::ScopedValueKey(const ScopedValueKey& other):
    myScope(other.myScope),
    path(other.path),
    key(other.key),
    parameter(other.parameter ? std::unique_ptr<Expression>(new Expression(*other.parameter))
                              : nullptr), //Good lord
    definedAt(other.definedAt)
{}

ScopedValueKey& ScopedValueKey::operator=(const ScopedValueKey& other)
{
    myScope = other.myScope;
    path = other.path;
    key = other.key;

    if(other.parameter) parameter = std::unique_ptr<Expression>(new Expression(*other.parameter));

    definedAt = other.definedAt;

    return *this;
}

void ScopedValueKey::apply(std::function<void (ScopedValueKey&)> fun)
{
    fun(*this);
    if(parameter) parameter->applyToEachScopedValueKey(fun);
}

void ScopedValueKey::merge(const ScopedValueKey& right)
{
    if(!path && right.path)
        path = right.path;
    else if(path && right.path)
        IrisUtils::Routine::appendContainer(*path, *right.path);
}

std::string ScopedValueKey::generatePrintout() const
{
    std::stringstream ss;
    if(myScope == IVD::ScopedValueKey::Scope::Element)          ss << "this";
    if(myScope == IVD::ScopedValueKey::Scope::Model)            ss << "model";
    if(myScope == IVD::ScopedValueKey::Scope::RemorialClass)    ss << "@";
    if(myScope == IVD::ScopedValueKey::Scope::Material)         ss << "material";
    if(myScope == IVD::ScopedValueKey::Scope::Global)           ss << "global";

    if(path) ss << "::" << *path;
    if(key) ss << "." << *key;
    if(parameter)
    {
        //"material" is always printed here.
        ss << ", with parameter expression:" << std::endl
           << parameter.get()->generatePrintout();
    }

    return ss.str();
}


}//IVD
