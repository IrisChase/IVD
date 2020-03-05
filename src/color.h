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

#ifndef COLOR_H
#define COLOR_H

#include <cstdint>
#include <string>

#include "irisutils/lexcompare.h"

namespace IVD
{


struct Color
{
    //For "DeepColor", probably should just switch channel
    // to floating point so the (0,255, 5), #ffffff, etc
    // stuff translates easily. Could also be rounded off
    // for legacy hardware.
    typedef uint8_t Channel;
    typedef Channel AlphaType;

    Channel red;
    Channel green;
    Channel blue;

    std::string generateHexPrint() const;
    std::string generateDecPrint() const;

    Color() {}
    Color(Channel r, Channel g, Channel b):
        red(r), green(g), blue(b)
    {}
    Color(const std::string& key);

    IRISUTILS_DEFINE_COMP(Color, red, green, blue)
};

struct GradientColorStop
{
    Color myColor;
    Color::AlphaType myAlpha;
    double pos;
};


}//IVD

#endif // COLOR_H
