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

#include "cairocanvas.h"

#include <functional>
#include <iostream>

namespace IVD
{

cairo_surface_t* createNewCairoSurfaceForPlatform(const Dimens size)
{ return cairo_image_surface_create(CAIRO_FORMAT_RGB24, size.w, size.h); }

void CairoCanvas::clip()
{
    for(Rect c : clips)
    {
        cairo_rectangle(myCai, c.c.x, c.c.y, c.d.w, c.d.h);
        cairo_clip(myCai);
    }
}

void CairoCanvas::drawWith(Color theColor,
                           Color::AlphaType alpha,
                           std::function<void(cairo_t*)> fun)
{
    cairo_save(myCai);
    clip();
    cairo_set_source_rgba(myCai,
                          (theColor.red + 1) / 256.0,
                          (theColor.green + 1) / 256.0,
                          (theColor.blue  + 1) / 256.0,
                          (alpha + 1) / 256.0);
    fun(myCai);
    cairo_restore(myCai);
}

void CairoCanvas::setSize(const Dimens size)
{
    if(myCai) freeContext();

    surface = createNewCairoSurfaceForPlatform(size);
    myCai = cairo_create(surface);
}

Dimens CairoCanvas::getSize()
{
    Dimens size;
    if(!myCai) return size;

    size.w = cairo_image_surface_get_width(surface);
    size.h = cairo_image_surface_get_height(surface);

    return size;
}


void CairoCanvas::clear()
{
    //Magic numbers because it's just fucking white, people.
    drawWith(Color(255,255,255), 255, [&](cairo_t* cai)
    {
        cairo_paint(cai);
    });
}

void CairoCanvas::fillRect(Rect r, Color theColor)
{
    drawWith(theColor, alpha, [&](cairo_t* cai)
    {
        cairo_rectangle(cai, r.c.x, r.c.y, r.d.w, r.d.h); //Alphabet soup
        cairo_fill(cai);
    });
}

void CairoCanvas::fillAdvancedRect(AdvancedRect r, Color theColor, Color::AlphaType alpha)
{
    fillRect(r.r, theColor); //shhhhhhhhhhhhhhhhhhh
}

void CairoCanvas::strokeRect(Rect r, int size, Color theColor, Color::AlphaType alpha)
{
    drawWith(theColor, alpha, [&](cairo_t* cai)
    {
        cairo_set_line_width(cai, size);
        cairo_rectangle(cai, r.c.x - .5, r.c.y - .5, r.d.w, r.d.h);
        cairo_stroke(cai);
    });
}

void CairoCanvas::drawLine(Coords start, Coords end, int size, Color theColor, Color::AlphaType alpha)
{
}

void CairoCanvas::drawGradient(Rect box,
                          Color toftColor, Color::AlphaType toftAlpha,
                          Color boriColor, Color::AlphaType boriAlpha, Angle theAngle)
{
assert(false);
}

void CairoCanvas::drawDropShadow(Rect box, int size, Color theColor, Color::AlphaType )
{
assert(false);
}

void CairoCanvas::drawBitmapRGBoptionalA(Rect dest, Bitmap image)
{
    //Goddddddddd we have to rearrange the pixel data ughh...
    const auto format = CAIRO_FORMAT_ARGB32;

#ifdef BIG_ENDIAN_SYSTEM
    const int destinationAlphaOffset = 0;
    const int destinationRedOffset   = 1;
    const int destinationGreenOffset = 2;
    const int destinationBlueOffset  = 3;
#elif LITTLE_ENDIAN_SYSTEM
    const int destinationAlphaOffset = 3;
    const int destinationRedOffset   = 2;
    const int destinationGreenOffset = 1;
    const int destinationBlueOffset  = 0;

#endif

    const int stride = cairo_format_stride_for_width(format, image.width);

    const int dstPixelWidth = 4;
    const int srcPixelWidth = image.channels;

    std::vector<unsigned char> theTrueNameOfBaal(stride * image.height * image.channels);

    for(int row = 0; row != image.height; ++row)
    {
        unsigned char* src = image.data + image.width * image.channels * row;
        unsigned char* dst = &theTrueNameOfBaal[0] + stride * row;

        int di = 0;
        int si = 0;

        while(si != image.width * image.channels)
        {
            //Pixel, *not* subpixel!
            unsigned char alpha = dst[di + destinationAlphaOffset] = image.channels == 4 ? src[si + 3]
                                                                                         : 0xff;
            dst[di + destinationRedOffset]   = src[si + 0] * -alpha;
            dst[di + destinationGreenOffset] = src[si + 1] * -alpha;
            dst[di + destinationBlueOffset]  = src[si + 2] * -alpha;
            //I guess that's how you do premultiplied alpha...

            di += dstPixelWidth;
            si += srcPixelWidth;
        }
    }

    cairo_surface_t* mySurf = cairo_image_surface_create_for_data(&theTrueNameOfBaal[0],
                                                                  format,
                                                                  image.width,
                                                                  image.height,
                                                                  stride);

    cairo_save(myCai);
    clip();

    //Does this need to be offset by .5?
    cairo_rectangle(myCai, dest.c.x, dest.c.y, dest.d.w, dest.d.h);
    cairo_set_source_surface(myCai, mySurf, dest.c.x, dest.c.y);

    //Cram the little fucker in there.
    cairo_scale(myCai,
                dest.d.w  / image.width,
                dest.d.h  / image.height);


    cairo_fill(myCai);

    cairo_restore(myCai);
    cairo_surface_destroy(mySurf);
}

void CairoCanvas::flush()
{
    cairo_surface_flush(surface);
}

bool CairoCanvas::ready()
{
    if(myCai) return true;

    assert(!surface);
    return false;
}

Bitmap CairoCanvas::getBitmap()
{
    Bitmap bitmap;

    bitmap.stride = cairo_image_surface_get_stride(surface);
    bitmap.width = cairo_image_surface_get_width(surface);
    bitmap.height = cairo_image_surface_get_height(surface);
    bitmap.data = cairo_image_surface_get_data(surface);
    bitmap.channels = 3; //rgb24 see above

    return bitmap;
}



}//IVD
