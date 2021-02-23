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

#ifndef CAIROCANVAS_H
#define CAIROCANVAS_H

#include <functional>

#include "canvas.h"
#include "cairo/cairo.h"

namespace IVD
{

cairo_surface_t* createNewCairoSurfaceForPlatform(const Dimens size);

class CairoCanvas : public Canvas
{
    cairo_t* myCai;
    cairo_surface_t* surface;

    void freeContext()
    {
        cairo_surface_destroy(surface);
        cairo_destroy(myCai);

        surface = nullptr;
        myCai = nullptr;
    }

    void clip();

    void drawWith(Color theColor, Color::AlphaType alpha, std::function<void(cairo_t*)> fun);

public:
    CairoCanvas(): myCai(nullptr), surface(nullptr) {}
    ~CairoCanvas() { freeContext(); }

    void setSize(const Dimens size) final;
    Dimens getSize() final;
    bool ready();

    void clear();

    void fillRect(Rect r, Color theColor) final;
    void strokeRect(Rect r, int size, Color theColor, Color::AlphaType alpha) final;
    void drawLine(Coords start, Coords end, int size, Color theColor, Color::AlphaType alpha) final;
    void drawGradient(Rect box,
                      Color toftColor,
                      Color::AlphaType toftAlpha,
                      Color boriColor,
                      Color::AlphaType boriAlpha,
                      Angle theAngle) final;
    void drawDropShadow(Rect box, int size, Color theColor, Color::AlphaType alpha) final;

    virtual void drawBitmapRGBoptionalA(Coords dest,
                                        int stride,
                                        int width,
                                        int height,
                                        int channels,
                                        unsigned char* data) final;

    //Defined in */TextDriver.cpp
    void drawText(Coords origin, const std::string text, DisplayItem* style) final;
    
    void flush() final;

    Bitmap getBitmap() final;
};

}//IVD

#endif // CAIROCANVAS_H
