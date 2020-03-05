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

#ifndef SDLWINDOW_H
#define SDLWINDOW_H

#include "SDL2/SDL.h"

#include "geometry.h"
#include "specific_driver_sdl/cairocanvas.h"

class SDLwindow
{
    SDL_Window* myWindow;
    SDL_Renderer* myRenderer;

    SDL_PixelFormat* myFormat;

    IVD::CairoCanvas myCanvas;

    void* context;

    void createWindow(const std::string title, const IVD::Rect theRect, const int flags);
    void destroyWindow();

public:
    SDLwindow(IVD::DisplayItem* item);
    ~SDLwindow();

    int getWindowId()
    { return SDL_GetWindowID(myWindow); }

    int getDisplayIndex()
    { return SDL_GetWindowDisplayIndex(myWindow); }

    IVD::Dimens getClientArea();
    IVD::Dimens getWindowArea();
    IVD::Coords getWindowPos();

    void setClientArea(const IVD::Dimens size);
    void setWindowPos(const IVD::Coords pos);

    void setResizable(const bool flag);
    void setVisible(const bool flag);
    void setTitleText(const std::string title);

    IVD::Canvas* getCanvas()
    { return &myCanvas; }

    void present();
};

#endif // SDLWINDOW_H
