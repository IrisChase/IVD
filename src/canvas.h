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

#ifndef CANVAS_H
#define CANVAS_H

#include <vector>
#include <memory>
#include "geometry.h"
#include "color.h"

namespace IVD
{

struct Bitmap
{
    int stride; //Is stride really necessary at this layer of abstraction?
    int width;
    int height;
    int channels;
    unsigned char* data;
};

class DisplayItem;

class Canvas
{
    Coords offset;


protected:
    //Okay so I promise this is for a real reason and not because I'm too lazy
    // to add clips this to all the drawing functions, which theoretically
    // would be "faster".
    //The reason is that clips are cumulative, and need to be unwound, and
    // children shouldn't know about parent clips.
    std::vector<Rect> clips;

    double alpha = 1;

public:
    void pushClip(const Rect clip)
    { clips.push_back(clip); }

    void popClip()
    { clips.pop_back(); }

    void setOffset(const Coords theOffset)
    { offset = theOffset; }

    Coords getOffset()
    { return offset; }

    void setAlpha(const double theAlpha)
    { alpha = theAlpha; }

    double getAlpha()
    { return alpha; }


    virtual void setSize(const Dimens size) = 0;
    virtual Dimens getSize() = 0;
    virtual bool ready() = 0;

    virtual void clear() = 0;
    
    virtual void fillRect(Rect r, Color theColor) = 0;
    virtual void strokeRect(Rect r, int size, Color theColor, Color::AlphaType alpha) = 0;
    virtual void drawLine(Coords start, Coords end, int size, Color theColor, Color::AlphaType alpha) = 0;
    virtual void drawGradient(Rect box,
                      Color toftColor,
                      Color::AlphaType toftAlpha,
                      Color boriColor,
                      Color::AlphaType boriAlpha,
                      Angle theAngle) = 0;
    virtual void drawDropShadow(Rect box, int size, Color theColor, Color::AlphaType alpha) = 0;

    virtual void drawBitmapRGBoptionalA(Rect dest, Bitmap image) = 0;

    virtual void drawText(Coords origin, const std::string text, DisplayItem* style) = 0;
    
    virtual void flush() = 0;
    virtual Bitmap getBitmap() = 0;
};

}//IVD


#endif // CANVAS_H
