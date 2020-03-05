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

#include "text.h"

namespace IVD
{
namespace Text
{

SplitStringPair extractExcess(DisplayItem* style, const std::string text, const int space)
{
    SplitStringPair result;
    
    const int stringLength = getMaxStringLengthForSpace(style, text, space);
    
    result.remaining =   text.substr(0, stringLength);
    result.overflowing = text.substr(stringLength);
    
    return result;
}

}//Text
}//IVD
