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

#include "harfbuzz/hb.h"
#include "harfbuzz/hb-ft.h"

#include "specific_driver_sdl/cairocanvas.h"
#include "cairo/cairo.h"
#include "cairo/cairo-ft.h"

#include <map>

namespace IVD
{

namespace Text
{

class RunWrapper
{
    Dimens myDimens;
    Coords bearings;
    const Angle itemFlowAngle;

    static FT_Library myFtLib;
    hb_buffer_t* harfBuzzBuffer;
    unsigned int glyphCount;
    hb_glyph_info_t* glyphInfo;
    hb_glyph_position_t* glyphPos;

    struct LoadedFont
    {
        hb_font_t* myHarfBuzzFont;
        FT_Face freeTypeFace;
        cairo_font_face_t* cairoFace;

        bool ready()
        { return myHarfBuzzFont && freeTypeFace && cairoFace; }

        LoadedFont(): myHarfBuzzFont(nullptr), freeTypeFace(nullptr), cairoFace(nullptr) {}
    };

    static std::map<Default::Filter::FontData, LoadedFont> fontCache;

    LoadedFont myLoadedFont;

    LoadedFont loadFace(DisplayItem* style);


    bool validState;



public:
    RunWrapper(DisplayItem* style, const std::string text);
    ~RunWrapper();

    bool checkValidState()
    { return validState; }

    Dimens getDimens() const
    { return myDimens; }

    Coords getBearings() const
    { return bearings; }

    //API inversion?
    int getItemFlowDimension() const
    { return getDimens().get(itemFlowAngle); }

    std::vector<cairo_glyph_t> getCairoGlyphs() const;

    void prepareCairoContext(cairo_t* theCai, DisplayItem* theItem);
};

FT_Library RunWrapper::myFtLib = nullptr;
std::map<Default::Filter::FontData, RunWrapper::LoadedFont> RunWrapper::fontCache =
        std::map<Default::Filter::FontData, RunWrapper::LoadedFont>();


RunWrapper::RunWrapper(DisplayItem* style, const std::string text):
    harfBuzzBuffer(nullptr),
    glyphInfo(nullptr),
    glyphPos(nullptr),
    validState(false),
    itemFlowAngle(Default::Filter::getItemFlowAngle(style))
{
    if(!myFtLib)
    {
        auto error = FT_Init_FreeType(&myFtLib);
        if(error) return;
    }

    myLoadedFont = loadFace(style);
    {
        const int sizeInPt = Default::Filter::getFontSize(style);

        //Something something size in pt so "dpi" is 72 by definition? Seems to be the
        // same as what cairo does.
        const int hdpi = 72;
        const int vdpi = 72;

        //Alright, so we have to set the size here, because cairo apparently caches
        // the size on the font? Because if we don't do this, the next call to
        // hb_shape with the same font but different size results in really weird spacing.
        //The only difference being that the "cairo_set_font_size" has been called with
        // the previous font, but not the current size. It *MUST* be caching it here
        //Which, incidentally, means that this is exactly where we should be updating it
        // anyway~
        FT_Set_Char_Size(myLoadedFont.freeTypeFace, 0, sizeInPt * 64, hdpi, vdpi);
    }

    if(!myLoadedFont.ready()) return;

    harfBuzzBuffer = hb_buffer_create();
    hb_buffer_add_utf8(harfBuzzBuffer, text.c_str(), -1, 0, -1);

    hb_buffer_set_direction(harfBuzzBuffer, HB_DIRECTION_LTR);
    hb_buffer_set_script(harfBuzzBuffer, HB_SCRIPT_LATIN);
    hb_buffer_set_language(harfBuzzBuffer, hb_language_from_string(text.c_str(), -1));

    hb_shape(myLoadedFont.myHarfBuzzFont, harfBuzzBuffer, nullptr, 0);

    glyphInfo = hb_buffer_get_glyph_infos(harfBuzzBuffer, &glyphCount);
    glyphPos = hb_buffer_get_glyph_positions(harfBuzzBuffer, &glyphCount);



    {
        //absolutely arbitrary size... We just need a valid surface to test against... TODO
        cairo_surface_t* surf = createNewCairoSurfaceForPlatform(Dimens(100,100));
        cairo_t* myCai = cairo_create(surf);

        prepareCairoContext(myCai, style);

        auto glyphs = getCairoGlyphs();
        cairo_text_extents_t extents;

        cairo_glyph_extents(myCai, &glyphs[0], glyphs.size(), &extents);

        bearings.x = extents.x_bearing;
        bearings.y = extents.y_bearing;

        const Angle rowFlowAngle = Default::Filter::getRowFlowAngle(style);

        myDimens.get(rowFlowAngle) = rowFlowAngle == Angle::Horizontal ? extents.width
                                                                       : extents.height;

        //Gotta include that dank as whitespace.
        myDimens.get(itemFlowAngle) = itemFlowAngle == Angle::Horizontal ? extents.x_advance
                                                                         : extents.y_advance;

        cairo_destroy(myCai);
        cairo_surface_destroy(surf);
    }

    validState = true;
}

RunWrapper::LoadedFont RunWrapper::loadFace(DisplayItem *style)
{
    Default::Filter::FontData fontData = Default::Filter::getFontFace(style);

    auto pos = fontCache.find(fontData);

    if(pos != fontCache.end())
        return pos->second;

    LoadedFont myLoaded;

    Default::Filter::FontData myFont = Default::Filter::getFontFace(style);
    auto error = FT_New_Memory_Face(myFtLib, myFont.data, myFont.size, 0, &myLoaded.freeTypeFace);

    if(error == FT_Err_Unknown_File_Format) return LoadedFont();
    else if(error) return LoadedFont();

    myLoaded.myHarfBuzzFont = hb_ft_font_create_referenced(myLoaded.freeTypeFace);

    if(error) return LoadedFont();

    myLoaded.cairoFace = cairo_ft_font_face_create_for_ft_face(myLoaded.freeTypeFace, 0);

    fontCache[fontData] = myLoaded;
    return myLoaded;
}

RunWrapper::~RunWrapper()
{
    //Alright we'll just never fucking free anything, have it your way, cairo.
    hb_buffer_destroy(harfBuzzBuffer);
}

std::vector<cairo_glyph_t> RunWrapper::getCairoGlyphs() const
{
    std::vector<cairo_glyph_t> cairoGlyphs;

    int xAdvance = 0;
    int yAdvance = 0;

    for(int i = 0; i != glyphCount; ++i)
    {
        cairo_glyph_t oneCairoGlyph;

        //According to the harfbuzz docs, hb_glyph_info_t::codepoint refers to a unicode codepoint
        // before shaping, and to the glyph index after, which is what cairo needs here.
        oneCairoGlyph.index = glyphInfo[i].codepoint;
        hb_glyph_position_t hbGlyphPos = glyphPos[i];

        oneCairoGlyph.x = xAdvance;
        oneCairoGlyph.y = yAdvance;

        xAdvance += hbGlyphPos.x_advance / 64;
        yAdvance += hbGlyphPos.y_advance / 64;

        cairoGlyphs.push_back(oneCairoGlyph);
    }
    
    return cairoGlyphs;
}

void RunWrapper::prepareCairoContext(cairo_t* theCai, DisplayItem* theItem)
{
    //TODO: Would "font height" be a good attribute? It's more controllable, I guess, and certainly
    // more predictable. Apparently that's what font-size is in CSS (sum of ascent and descent)
    cairo_set_font_face(theCai, myLoadedFont.cairoFace);
    cairo_set_font_size(theCai, Default::Filter::getFontSize(theItem));
    assert(!cairo_status(theCai));
}


Dimens getRunDimensions(DisplayItem* item, const std::string text)
{
    return RunWrapper(item, text).getDimens();
}

int getMaxStringLengthForSpace(DisplayItem* style, const std::string text, const int space)
{
    int lastGoodSize = 0; //'member yer last goooooooooood size? Aww yeh...

    for(; lastGoodSize != text.size(); ++lastGoodSize)
    {
        Text::RunWrapper myRun(style, text.substr(0, lastGoodSize));
        if(myRun.getItemFlowDimension() > space)
            return lastGoodSize ? --lastGoodSize
                                : lastGoodSize;
    }
    return lastGoodSize;
}

}//Text


void CairoCanvas::drawText(Coords origin, const std::string text, DisplayItem *style)
{
	Text::RunWrapper myRun = Text::RunWrapper(style, text);
    auto cairoGlyphs = myRun.getCairoGlyphs();
    const Coords textPos = origin - myRun.getBearings();

    drawWith(Default::Filter::getFontColor(style), Default::Filter::getAlpha(style), [&](cairo_t* cai)
    {
        myRun.prepareCairoContext(cai, style);
        cairo_translate(cai, textPos.x, textPos.y);
        //cairo_scale(cai, 1, 1);
        cairo_show_glyphs(cai, &cairoGlyphs[0], cairoGlyphs.size());

        auto i = cairo_status(cai);
        assert(i == 0);
    });
}

}//IVD
