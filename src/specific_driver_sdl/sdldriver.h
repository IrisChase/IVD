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

#ifndef SDLDRIVER_H
#define SDLDRIVER_H

#include "driver.h"
#include "specific_driver_sdl/sdlwindow.h"

#include <map>

namespace IVD
{

class SDLdriver : public Driver
{
    struct ItemWindowPair
    {
        std::unique_ptr<SDLwindow> window;
        bool destroy;

        bool invalidGeometry;
        bool invalidPosition;
        bool invalidCanvas;

        bool invalidTitleText;
        bool invalidResizability;
        bool invalidVisibility;
        //Fullscreen....... too tired do it later

        //Not using std::optional here because for some reason it doesn't
        // copy the structure properly...
        bool previousSizeSet;
        Dimens previousDimens;

        ItemWindowPair():
            destroy(false),
            invalidGeometry(true),
            invalidPosition(true),
            invalidCanvas(true),
            invalidTitleText(true),
            invalidResizability(true),
            invalidVisibility(true),
            previousSizeSet(false)
        {}
    };

    std::map<DisplayItem*, ItemWindowPair> pairs;

    DisplayItem* rootWithMouseFocus;
    //Coords mouseHoverPoint;
    Coords mousePointWindow;
    Coords scrollDist;

    Coords queriedMousePoint;

    bool hoverInvalidated;

    void invalidateHover(const Coords point, const Uint32 windowID);
    void invalidateHoverDesperately();

public:
    SDLdriver();
    ~SDLdriver();
    
    void addDisplayItem(DisplayItem* item);
    void removeDisplayItem(DisplayItem* item);
    void processEvents();

    void invalidateGeometry(DisplayItem* item);
    void invalidatePosition(DisplayItem* item);
    void invalidateCanvas(DisplayItem* item);

    void invalidateTitleText(DisplayItem* item);
    void invalidateVisibility(DisplayItem* item);

    void refresh();

    bool checkAnythingToDo();

    bool checkHoverInvalidated()
    { return hoverInvalidated; }

    DisplayItem* getWindowItemWithMouseFocus()
    { return rootWithMouseFocus; }

    Coords getMousePointRelativeToWindow()
    { return mousePointWindow; }
};

}//IVD

#endif // SDLDRIVER_H
