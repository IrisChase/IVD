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

#ifndef TEXT_H
#define TEXT_H

#include <string>

#include "defaults.h"
#include "geometry.h"

namespace IVD
{

class Canvas;

namespace Text
{

//The following are defined in the corresponding textdriver source file.
Dimens getRunDimensions(DisplayItem* item, const std::string text);
int getMaxStringLengthForSpace(DisplayItem* style, const std::string text, const int space);

//These are defined in text.cpp, as they rely on the generics above.
struct SplitStringPair
{
    std::string remaining;
    std::string overflowing;
};

SplitStringPair extractExcess(DisplayItem* style, const std::string text, const int space);

}//Text
}//IVD

#endif // TEXT_H
