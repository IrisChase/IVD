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

#include "sdlwindow.h"

#include "defaults.h"

static IVD::Dimens getDrawableAreaForRenderer(SDL_Renderer* theRenderer)
{
    IVD::Dimens drawable;
    SDL_GetRendererOutputSize(theRenderer, &drawable.w, &drawable.h);

    return drawable;
}

void SDLwindow::createWindow(const std::string title, const IVD::Rect theRect, const int flags)
{
    myWindow = SDL_CreateWindow(title.c_str(), theRect.c.x, theRect.c.y, theRect.d.w, theRect.d.h, flags);

    assert(myWindow);

    myRenderer = SDL_CreateRenderer(myWindow, -1, SDL_RENDERER_ACCELERATED |
                                                  SDL_RENDERER_PRESENTVSYNC);

    assert(myRenderer);
}

void SDLwindow::destroyWindow()
{
    SDL_DestroyRenderer(myRenderer);
    SDL_DestroyWindow(myWindow);

    myRenderer = nullptr;
    myWindow = nullptr;
}

SDLwindow::SDLwindow(IVD::DisplayItem* item):
    myFormat(SDL_AllocFormat(SDL_PIXELFORMAT_RGB24))
{
    int flags = SDL_WINDOW_HIDDEN;

    if(IVD::Default::Filter::checkResizable(item))
        flags |= SDL_WINDOW_RESIZABLE;

    createWindow("", IVD::Rect(), flags);
}

SDLwindow::~SDLwindow()
{
    SDL_FreeFormat(myFormat);
    destroyWindow();
    //bye bye :<
}

IVD::Dimens SDLwindow::getClientArea()
{
    IVD::Dimens myDimens;
    SDL_GetWindowSize(myWindow, &myDimens.w, &myDimens.h);

    return myDimens;
}

IVD::Dimens SDLwindow::getWindowArea()
{
    int top;
    int left;
    int bottom;
    int right;

    SDL_GetWindowBordersSize(myWindow, &top, &left, &bottom, &right);

    IVD::Dimens myDimens = getClientArea();
    myDimens.h += top + bottom;
    myDimens.w += left + right;

    return myDimens;
}

IVD::Coords SDLwindow::getWindowPos()
{
    IVD::Coords myCoords;
    SDL_GetWindowPosition(myWindow, &myCoords.x, &myCoords.y);
    return myCoords;
}

void SDLwindow::setClientArea(const IVD::Dimens size)
{
    {
        Uint32 flags = SDL_GetWindowFlags(myWindow);

        if((flags & SDL_WINDOW_FULLSCREEN) != SDL_WINDOW_FULLSCREEN &&
                (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP)
        {
            SDL_SetWindowSize(myWindow, size.w, size.h);
        }
    }

    myCanvas.setSize(size);
}

void SDLwindow::setWindowPos(const IVD::Coords pos)
{
    //uh.. What about decorations?
    //Well. According to https://bugzilla.libsdl.org/show_bug.cgi?id=3845
    //These SDL_*WindowPosition functions return the position of the viewport, and
    // not the window decorations. This is a bug in SDL. Ideally, this
    // next call would correct for the decoration offset. TODO
    //coordinateOffset = pos + SDL_GetWindowBordersSize() values.
    SDL_SetWindowPosition(myWindow, pos.x, pos.y);
}

void SDLwindow::setResizable(const bool flag)
{
    int flags = SDL_GetWindowFlags(myWindow);

    if(flag  != ((flags & SDL_WINDOW_RESIZABLE) == SDL_WINDOW_RESIZABLE))
    {
        if(!flag) flags &= ~SDL_WINDOW_RESIZABLE;
        else 	  flags |= SDL_WINDOW_RESIZABLE;

        assert(flag  == ((flags & SDL_WINDOW_RESIZABLE) == SDL_WINDOW_RESIZABLE));

        IVD::Rect windowRect;
        windowRect.d = getClientArea();
        windowRect.c = getWindowPos();
        const std::string title = SDL_GetWindowTitle(myWindow);

        destroyWindow();
        createWindow(title, windowRect, flags);

        setClientArea(windowRect.d);
    }
}

void SDLwindow::setVisible(const bool flag)
{
    if(flag)
        SDL_ShowWindow(myWindow);
    else
        SDL_HideWindow(myWindow);
}

void SDLwindow::setTitleText(const std::string title)
{
    SDL_SetWindowTitle(myWindow, title.c_str());
}

void SDLwindow::present()
{
    IVD::Dimens myDimens = getDrawableAreaForRenderer(myRenderer);

    //Crashes if less than 2 because... Pixel offsets? Sorry too busy to figure out.
    if(myDimens.w < 2 || myDimens.h < 2) return;

    const int height = myDimens.h;
    const int widthInPixels = myDimens.w;

    void* destData;
    int destPitch;

    SDL_Texture* myTarget = SDL_CreateTexture(myRenderer,
                                              myFormat->format,
                                              SDL_TEXTUREACCESS_STREAMING,
                                              myDimens.w,
                                              myDimens.h);
    SDL_LockTexture(myTarget, nullptr, &destData, &destPitch);

    myCanvas.flush();

    IVD::Bitmap mySurface = myCanvas.getBitmap();

    unsigned char* sourceData = mySurface.data;
    const int sourcePitch = mySurface.stride;

    auto mapMaskToOffset = [](const Uint32 mask)
    {
             if(mask == 0x000000ff) return 0;
        else if(mask == 0x0000ff00) return 1;
        else if(mask == 0x00ff0000) return 2;
        else if(mask == 0xff000000) return 3;

        assert(false);
    };

    const int destRedOffset = mapMaskToOffset(myFormat->Rmask);
    const int destGreenOffset = mapMaskToOffset(myFormat->Gmask);
    const int destBlueOffset = mapMaskToOffset(myFormat->Bmask);
    const int pixelWidthSDL = myFormat->BytesPerPixel;

    //As per the documentation for CAIRO_FORMAT_RGB24
    const int sourceRedOffset = 2;
    const int sourceGreenOffset = 1;
    const int sourceBlueOffset = 0;
    const int pixelWidthCairo = 4;

    const int destDataWidth = widthInPixels * pixelWidthSDL;
    const int sourceDataWidth = widthInPixels * pixelWidthCairo;

    for(int row = 0; row != height; ++row)
    {
        unsigned char* dest = (unsigned char*)destData + (destPitch * row);
        unsigned char* source = sourceData + (sourcePitch * row);

        int di = 0;
        int si = 0;

        while(di != destDataWidth && si != sourceDataWidth)
        {
            dest[di + destRedOffset]   = source[si + sourceRedOffset];
            dest[di + destGreenOffset] = source[si + sourceGreenOffset];
            dest[di + destBlueOffset]  = source[si + sourceBlueOffset];

            di += pixelWidthSDL;
            si += pixelWidthCairo;
        }
    }

    SDL_UnlockTexture(myTarget);
    SDL_RenderCopy(myRenderer, myTarget, nullptr, nullptr);
    SDL_DestroyTexture(myTarget);
    SDL_RenderPresent(myRenderer);
}
