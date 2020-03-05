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

#ifndef VALUEKEY_H
#define VALUEKEY_H

#include "irisutils/lexcompare.h"

#include <tuple>
#include <string>
#include <vector>
#include <ostream>
#include <optional>
#include <memory>
#include <functional>

#include "codeposition.h"

namespace IVD
{

typedef std::string ValueKey;
typedef std::string ModelKey;
typedef std::vector<ValueKey> ValueKeyPath;

class Expression;

struct ScopedValueKey
{
    enum class Scope
    {
        Global,
        Element,
        Model,
        Material,

        RemorialClass, //This should only show up in the compiler //Is this still true?
    };
    std::optional<Scope> myScope;
    std::optional<ValueKeyPath> path;
    std::optional<ValueKey> key;
    std::unique_ptr<Expression> parameter;

    //I dare you to rename this.
    CodePosition definedAt;

    ScopedValueKey();
    ScopedValueKey(Scope theScope);
    ~ScopedValueKey();

    ScopedValueKey(const ScopedValueKey& other);

    ScopedValueKey& operator=(const ScopedValueKey& other);

    void setScopeIfUnset(const Scope theScope)
    { if(!myScope) myScope = theScope; }

    bool empty()
    { return !myScope && !path && !key && !parameter; }

    void addToPath(const ValueKey pathpiece)
    {
        if(path) path->push_back(pathpiece);
        else path = {pathpiece};
    }

    void setKey(const ValueKey theKey)
    { key = theKey; }

    void apply(std::function<void(ScopedValueKey&)> fun);

    IRISUTILS_DEFINE_COMP(ScopedValueKey, myScope, path, key)

    //Why does this function exist?
    void merge(const ScopedValueKey& right);

    std::string generatePrintout() const;
};

inline std::ostream& operator<<(std::ostream& theStream, ValueKeyPath path)
{
    for(auto it = path.begin(); it != path.end(); ++it)
    {
        theStream << *it;
        if(it + 1 != path.end()) theStream << "::";
    }

    return theStream;
}

}//IVD

#endif // VALUEKEY_H
