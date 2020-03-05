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

#pragma once

#include <memory>

namespace IrisUtils
{

//I know it's frowned upon to inherit from std classes but fucking byte me.
//This is just for the trivial case where I want a deep copy and nothing fancy.
template<typename T>
class DeepCopyableUnique : public std::unique_ptr<T>
{
public:
    typedef std::unique_ptr<T> Parent;
    DeepCopyableUnique() noexcept {}
    DeepCopyableUnique(const DeepCopyableUnique& other) noexcept:
        Parent(other.get() ? std::make_unique<T>(*other.get())
                           : nullptr)
    {}

    DeepCopyableUnique(const Parent&& other) noexcept:
        Parent(other)
    {}

    DeepCopyableUnique& operator=(const DeepCopyableUnique& other) noexcept
    {
        if(other.get())
            Parent::operator=(std::make_unique<T>(*other.get()));
        else Parent::reset();
        return *this;
    }

    DeepCopyableUnique& makeCopy(const T& other)
    {
        Parent::operator=(std::make_unique<T>(other));
        return *this;
    }
};

}//IrisUtils
