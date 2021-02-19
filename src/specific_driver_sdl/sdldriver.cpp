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

#include "specific_driver_sdl/sdldriver.h"

#include "states.h"
#include "defaults.h"
#include "statemanager.h"

#include <reprodyne.h>


namespace IVD
{

SDLdriver::SDLdriver():
    rootWithMouseFocus(nullptr),
    hoverInvalidated(true)
{
    int code = SDL_Init(SDL_INIT_VIDEO);
    assert(code >= 0);

    reprodyne_open_scope(this);
}

SDLdriver::~SDLdriver()
{
    SDL_Quit();
}

Driver* createDefaultDriver()
{ return new SDLdriver; }

void SDLdriver::invalidateHover(const Coords point, const Uint32 windowID)
{
    hoverInvalidated = true;
    DisplayItem* myRoot = nullptr;

    for(const auto& pair : pairs)
    {
        if(!pair.second.window) continue;
        if(pair.second.window->getWindowId() == windowID)
        {
            myRoot = pair.first;
            break;
        }
    }

    getStateManager()->mutateAll(States::Window::MouseFocus, false);
    if(myRoot)
        getStateManager()->mutateIfObserved(StateKey(States::Window::MouseFocus, myRoot), true);

    rootWithMouseFocus = myRoot;
}

void SDLdriver::invalidateHoverDesperately()
{
    Coords makeShiftMousePosition;
    SDL_GetMouseState(&makeShiftMousePosition.x, &makeShiftMousePosition.y);

    makeShiftMousePosition.x = reprodyne_intercept_double(this, "makeshift-mouse-x", makeShiftMousePosition.x);
    makeShiftMousePosition.y = reprodyne_intercept_double(this, "makeshift-mouse-y", makeShiftMousePosition.y);
    const auto windowId = reprodyne_intercept_double(this,
                                                                   "makeshift-mouse-position-window-id",
                                                                   SDL_GetWindowID(SDL_GetMouseFocus()));

    invalidateHover(makeShiftMousePosition, windowId);
}

void SDLdriver::addDisplayItem(DisplayItem *item)
{
    auto& pair = pairs[item];

    if(!pair.window)
    {
        pair.window = std::make_unique<SDLwindow>(item);
        getStateManager()->setTriggerIfObserved(StateKey(States::Window::Initialized, item));
    }

    pair.destroy = false;
}

void SDLdriver::removeDisplayItem(DisplayItem* item)
{
    auto it = pairs.find(item);
    if(it == pairs.end()) return;

    it->second.destroy = true;
}

void SDLdriver::processEvents()
{
    StateManager* stateManager = getStateManager();
    auto mapSDLscanCodeToStateSymbol = [&](const Uint16 scanCode) -> std::optional<States::Symbol>
    {
        std::optional<States::Symbol> result;

        switch(scanCode)
        {
        case SDL_SCANCODE_AUDIOPLAY: return States::Key::Play;
        case SDL_SCANCODE_AUDIOSTOP: return States::Key::Stop;
        case SDL_SCANCODE_AUDIONEXT: return States::Key::Next;
        case SDL_SCANCODE_AUDIOPREV: return States::Key::Previous;

        case SDL_SCANCODE_LGUI: return States::Key::LeftSuper;
        case SDL_SCANCODE_LSHIFT: return States::Key::LeftShift;
        case SDL_SCANCODE_LCTRL: return States::Key::LeftCtrl;
        case SDL_SCANCODE_LALT: return States::Key::LeftAlt;

        case SDL_SCANCODE_RGUI: return States::Key::RightSuper;
        case SDL_SCANCODE_RSHIFT: return States::Key::RightShift;
        case SDL_SCANCODE_RCTRL: return States::Key::RightCtrl;
        case SDL_SCANCODE_RALT: return States::Key::RightAlt;

        case SDL_SCANCODE_CAPSLOCK: return States::Key::CapsLock;
            //Alt G
        case SDL_SCANCODE_MENU: return States::Key::Menu;

        case SDL_SCANCODE_PRINTSCREEN: return States::Key::PrintScreen;
        case SDL_SCANCODE_SCROLLLOCK: return States::Key::ScrollLock;
        case SDL_SCANCODE_PAUSE: return States::Key::Pause;

        case SDL_SCANCODE_INSERT: return States::Key::Insert;
        case SDL_SCANCODE_DELETE: return States::Key::Delete;
        case SDL_SCANCODE_HOME: return States::Key::Home;
        case SDL_SCANCODE_END: return States::Key::End;
        case SDL_SCANCODE_PAGEUP: return States::Key::PageUp;
        case SDL_SCANCODE_PAGEDOWN: return States::Key::PageDown;

        case SDL_SCANCODE_LEFT: return States::Key::LeftArrow;
        case SDL_SCANCODE_RIGHT: return States::Key::RightArrow;
        case SDL_SCANCODE_UP: return States::Key::UpArrow;
        case SDL_SCANCODE_DOWN: return States::Key::DownArrow;

        case SDL_SCANCODE_NUMLOCKCLEAR: return States::Key::NumLock;

        case SDL_SCANCODE_ESCAPE: return States::Key::Escape; //Must be Italian

        case SDL_SCANCODE_F1: return States::Key::F1;
        case SDL_SCANCODE_F2: return States::Key::F2;
        case SDL_SCANCODE_F3: return States::Key::F3;
        case SDL_SCANCODE_F4: return States::Key::F4;
        case SDL_SCANCODE_F5: return States::Key::F5;
        case SDL_SCANCODE_F6: return States::Key::F6;
        case SDL_SCANCODE_F7: return States::Key::F7;
        case SDL_SCANCODE_F8: return States::Key::F8;
        case SDL_SCANCODE_F9: return States::Key::F9;
        case SDL_SCANCODE_F10: return States::Key::F10;
        case SDL_SCANCODE_F11: return States::Key::F11;
        case SDL_SCANCODE_F12: return States::Key::F12;

        case SDL_SCANCODE_GRAVE: return States::Key::Tilde;
        case SDL_SCANCODE_1: return States::Key::RowNum1;
        case SDL_SCANCODE_2: return States::Key::RowNum2;
        case SDL_SCANCODE_3: return States::Key::RowNum3;
        case SDL_SCANCODE_4: return States::Key::RowNum4;
        case SDL_SCANCODE_5: return States::Key::RowNum5;
        case SDL_SCANCODE_6: return States::Key::RowNum6;
        case SDL_SCANCODE_7: return States::Key::RowNum7;
        case SDL_SCANCODE_8: return States::Key::RowNum8;
        case SDL_SCANCODE_9: return States::Key::RowNum9;
        case SDL_SCANCODE_0: return States::Key::RowNum0;
        case SDL_SCANCODE_MINUS: return States::Key::Minus;
        case SDL_SCANCODE_EQUALS: return States::Key::Equals;
        case SDL_SCANCODE_BACKSPACE: return States::Key::Backspace;

        case SDL_SCANCODE_TAB: return States::Key::Tab;
        case SDL_SCANCODE_Q: return States::Key::Q;
        case SDL_SCANCODE_W: return States::Key::W;
        case SDL_SCANCODE_E: return States::Key::E;
        case SDL_SCANCODE_R: return States::Key::R;
        case SDL_SCANCODE_T: return States::Key::T;
        case SDL_SCANCODE_Y: return States::Key::Y;
        case SDL_SCANCODE_U: return States::Key::U;
        case SDL_SCANCODE_I: return States::Key::I;
        case SDL_SCANCODE_O: return States::Key::O;
        case SDL_SCANCODE_P: return States::Key::P;
        case SDL_SCANCODE_LEFTBRACKET: return States::Key::LeftBracket;
        case SDL_SCANCODE_RIGHTBRACKET: return States::Key::RightBracket;
        case SDL_SCANCODE_BACKSLASH: return States::Key::Backslash;


        case SDL_SCANCODE_A: return States::Key::A;
        case SDL_SCANCODE_S: return States::Key::S;
        case SDL_SCANCODE_D: return States::Key::D;
        case SDL_SCANCODE_F: return States::Key::F;
        case SDL_SCANCODE_G: return States::Key::G;
        case SDL_SCANCODE_H: return States::Key::H;
        case SDL_SCANCODE_J: return States::Key::J;
        case SDL_SCANCODE_K: return States::Key::K;
        case SDL_SCANCODE_L: return States::Key::L;
        case SDL_SCANCODE_SEMICOLON: return States::Key::Semicolon;
        case SDL_SCANCODE_APOSTROPHE: return States::Key::Apostrophe;
        case SDL_SCANCODE_RETURN: return States::Key::Return;

        case SDL_SCANCODE_Z: return States::Key::Z;
        case SDL_SCANCODE_X: return States::Key::X;
        case SDL_SCANCODE_C: return States::Key::C;
        case SDL_SCANCODE_V: return States::Key::V;
        case SDL_SCANCODE_B: return States::Key::B;
        case SDL_SCANCODE_N: return States::Key::N;
        case SDL_SCANCODE_M: return States::Key::M;
        case SDL_SCANCODE_COMMA: return States::Key::Comma;
        case SDL_SCANCODE_PERIOD: return States::Key::Period;
        case SDL_SCANCODE_SLASH: return States::Key::Slash;

        case SDL_SCANCODE_SPACE: return States::Key::Spacebar;

        case SDL_SCANCODE_KP_DIVIDE: return States::Key::Pad::Slash;
        case SDL_SCANCODE_KP_MULTIPLY: return States::Key::Pad::Star;
        case SDL_SCANCODE_KP_MINUS: return States::Key::Pad::Minus;

        case SDL_SCANCODE_KP_7: return States::Key::Pad::Num7;
        case SDL_SCANCODE_KP_8: return States::Key::Pad::Num8;
        case SDL_SCANCODE_KP_9: return States::Key::Pad::Num9;
        case SDL_SCANCODE_KP_PLUS: return States::Key::Pad::Plus;

        case SDL_SCANCODE_KP_4: return States::Key::Pad::Num4;
        case SDL_SCANCODE_KP_5: return States::Key::Pad::Num5;
        case SDL_SCANCODE_KP_6: return States::Key::Pad::Num6;
        case SDL_SCANCODE_KP_TAB: return States::Key::Pad::Tab;

        case SDL_SCANCODE_KP_1: return States::Key::Pad::Num1;
        case SDL_SCANCODE_KP_2: return States::Key::Pad::Num2;
        case SDL_SCANCODE_KP_3: return States::Key::Pad::Num3;

        case SDL_SCANCODE_KP_0: return States::Key::Pad::Num0;
        case SDL_SCANCODE_KP_PERIOD: return States::Key::Pad::Period;
        case SDL_SCANCODE_KP_ENTER: return States::Key::Pad::Return;

        default: return result;
        }
    };

    auto mapSDLmouseButtonToSymbol = [&](const Uint8 button) -> States::Symbol
    {
        switch(button)
        {
            case SDL_BUTTON_LEFT: return States::Mouse::ButtonLeft;
            case SDL_BUTTON_MIDDLE: return States::Mouse::ButtonMiddle;
            case SDL_BUTTON_RIGHT: return States::Mouse::ButtonRight;
            case SDL_BUTTON_X1: return States::Mouse::ButtonFour;
            case SDL_BUTTON_X2: return States::Mouse::ButtonFive;
            default: assert(false);
        }
    };

    //TODO: Would it be faster to do a search over the states and then see if it matches
    // the event, rather than search and see if the state is observed?

    SDL_Event event;
    if(reprodyne_intercept_double(this, "poll-event", SDL_PollEvent(&event)))
    {
        const double eventType = reprodyne_intercept_double(this, "event-type", event.type);

        switch(int(eventType))
        {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            const auto scancode = reprodyne_intercept_double(this, "scancode", event.key.keysym.scancode);

            const auto symbol = mapSDLscanCodeToStateSymbol(scancode);
            if(symbol)
            {
                if(eventType == SDL_KEYDOWN)
                    stateManager->setTriggerIfObserved(States::getButtonPress(*symbol));
                else if(eventType == SDL_KEYUP)
                    stateManager->setTriggerIfObserved(States::getButtonRelease(*symbol));
            }
            break;
        }

        case SDL_MOUSEBUTTONDOWN:
        {
            const auto keysym = reprodyne_intercept_double(this, "mouse-button-down", event.button.button);

            States::Symbol sym = mapSDLmouseButtonToSymbol(keysym);
            stateManager->setTriggerIfObserved(States::getButtonPress(sym));
            break;
        }
        case SDL_MOUSEBUTTONUP: //Bucko
        {
            const auto keysym = reprodyne_intercept_double(this, "mouse-button-up", event.button.button);

            States::Symbol sym = mapSDLmouseButtonToSymbol(keysym);
            stateManager->setTriggerIfObserved(States::getButtonRelease(sym));
            break;
        }
        case SDL_MOUSEMOTION:
        {
            stateManager->setTriggerIfObserved(States::Mouse::Motion);

            mousePointWindow.x = reprodyne_intercept_double(this, "mouse-motion-x", event.motion.x);
            mousePointWindow.y = reprodyne_intercept_double(this, "mouse-motion-y", event.motion.y);

            const auto windowId = reprodyne_intercept_double(this, "motion-window-id", event.motion.windowID);

            invalidateHover(Coords(mousePointWindow.x, mousePointWindow.y), windowId);
            break;
        }
        case SDL_MOUSEWHEEL:
        {
            //Doesn't necessarily invalidate hover
            stateManager->setTriggerIfObserved(States::Mouse::Wheel);

            if(event.wheel.direction == SDL_MOUSEWHEEL_NORMAL)
            {
                scrollDist.x = event.wheel.x;
                scrollDist.y = event.wheel.y;
            }
            else if(event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
            {
                scrollDist.x = event.wheel.x * -1;
                scrollDist.y = event.wheel.y * -1;
            }

            //All that matters as far as the harness is concerned, is what the result is
            // we'll get to these two lines in either mode.
            scrollDist.x = reprodyne_intercept_double(this, "scroll-dist-x", scrollDist.x);
            scrollDist.y = reprodyne_intercept_double(this, "scroll-dist-y", scrollDist.y);
            break;
        }
        case SDL_WINDOWEVENT:
        {
            for(const auto& pair : pairs)
            {
                if(!pair.second.window)
                    continue;

                DisplayItem* item = pair.first;

                const auto currentRootWindowId = pair.second.window->getWindowId();
                const auto eventWindowId = reprodyne_intercept_double(this,
                                                                                    "window-event-window-id",
                                                                                    event.window.windowID);

                if(currentRootWindowId != eventWindowId) continue;

                switch(int(reprodyne_intercept_double(this, "window-event", event.window.event)))
                {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    //If in top down mode
                    if(Default::Filter::getWindowSizeStrategy(item) == Property::TopDown)
                        invalidateGeometry(item);

                    //If the canvas is updated too soon after the window is resized, then the full
                    // area isn't drawn and there will be a black area (if larger than before)
                    // once the window system responds to the resize request. But we still get a
                    // resize event, so we push another redraw.
                    invalidateCanvas(item);
                    break;
                }

                case SDL_WINDOWEVENT_MOVED:
                    invalidatePosition(item);
                    break;

                case SDL_WINDOWEVENT_CLOSE:
                    stateManager->setTriggerIfObserved(StateKey(States::Window::CloseRequest, item));
                    break;

                case SDL_WINDOWEVENT_LEAVE:
                case SDL_WINDOWEVENT_ENTER:
                    invalidateHoverDesperately();
                    break;

                case SDL_WINDOWEVENT_SHOWN:
                    invalidateCanvas(item);
                    stateManager->setTriggerIfObserved(StateKey(States::Window::Shown, item));
                    break;
                }
            }
            break;
        }
        case SDL_QUIT:
        {
            stateManager->setTriggerIfObserved(States::App::CloseApp);
            break;
        }
        }
    }


    //Button states are out of sync with events by definition.
    {
        int length;
        const Uint8* keyStates = SDL_GetKeyboardState(&length);

        for(Uint16 i = 0; i != length; ++i)
        {
            auto symbol = mapSDLscanCodeToStateSymbol(i);
            if(!symbol) continue;

            const StateKey theStateKey(States::getButtonActive(*symbol));
            const bool pressed = reprodyne_intercept_double(this, "key-state", keyStates[i]);

            stateManager->mutateIfObserved(theStateKey, pressed);
        }
    }
    {
        const Uint32 mask = reprodyne_intercept_double(this,
                                                                     "mouse-mask",
                                                                     SDL_GetMouseState(&queriedMousePoint.x,
                                                                                       &queriedMousePoint.y));

        const StateKey left(States::getButtonActive(States::Mouse::ButtonLeft));
        const StateKey middle(States::getButtonActive(States::Mouse::ButtonMiddle));
        const StateKey right(States::getButtonActive(States::Mouse::ButtonRight));
        const StateKey four(States::getButtonActive(States::Mouse::ButtonFour));
        const StateKey five(States::getButtonActive(States::Mouse::ButtonFive));

        auto thingamajigulator = [&](const auto code, const StateKey stateKey)
        {
            if((mask & code) == code)
                stateManager->mutateIfObserved(stateKey, true);
            else
                stateManager->mutateIfObserved(stateKey, false);
        };

        thingamajigulator(SDL_BUTTON_LMASK, left);
        thingamajigulator(SDL_BUTTON_MMASK, middle);
        thingamajigulator(SDL_BUTTON_RMASK, right);
        thingamajigulator(SDL_BUTTON_X1MASK, four);
        thingamajigulator(SDL_BUTTON_X2MASK, five);
    }
}

void SDLdriver::invalidateGeometry(DisplayItem *item)
{
    if(pairs.count(item->getRoot()))
        pairs.at(item->getRoot()).invalidGeometry = true;
}

void SDLdriver::invalidatePosition(DisplayItem* item)
{
    auto root = item->getRoot();
    if(root == item)
    {
        pairs.at(root).invalidPosition = true;
    }
    else invalidateGeometry(item);
}

void SDLdriver::invalidateCanvas(DisplayItem *item)
{
    if(pairs.count(item->getRoot()))
        pairs.at(item->getRoot()).invalidCanvas = true;
}

void SDLdriver::invalidateTitleText(DisplayItem *item)
{
    auto it = pairs.find(item);
    if(it == pairs.end()) return;

    it->second.invalidTitleText = true;
}

void SDLdriver::invalidateVisibility(DisplayItem *item)
{
    if(item->getRoot() == item)
        pairs[item].invalidVisibility = true;
    else
        invalidateCanvas(item->getRoot());
}

void SDLdriver::refresh()
{
    hoverInvalidated = false;
    //That kinda thing.

    for(auto it = pairs.begin(); it != pairs.end(); /*nope*/)
    {
        DisplayItem* item = it->first;
        auto& pairData = it->second;

        if(pairData.destroy)
            pairData.window.reset();

        if(!pairData.window)
        {
            it = pairs.erase(it);
            continue;
        }

        SDLwindow* window = pairData.window.get();

        const int displayIndex = reprodyne_intercept_double(this, "display-index", window->getDisplayIndex());

        SDL_Rect displayUsable;
        SDL_Rect displayFull;

        SDL_GetDisplayBounds(displayIndex, &displayFull);
        SDL_GetDisplayUsableBounds(displayIndex, &displayUsable);

        displayUsable.w = reprodyne_intercept_double(this, "display-usable-w", displayUsable.w);
        displayUsable.h = reprodyne_intercept_double(this, "display-usable-h", displayUsable.h);
        displayUsable.x = reprodyne_intercept_double(this, "display-usable-x", displayUsable.x);
        displayUsable.y = reprodyne_intercept_double(this, "display-usable-y", displayUsable.y);

        displayFull.w = reprodyne_intercept_double(this, "display-full-w", displayFull.w);
        displayFull.h = reprodyne_intercept_double(this, "display-full-h", displayFull.h);
        displayFull.x = reprodyne_intercept_double(this, "display-full-x", displayFull.x);
        displayFull.y = reprodyne_intercept_double(this, "display-full-y", displayFull.y);



        if(pairData.invalidGeometry)
        {
            //We just assume any time geometry is invalidated, resizability is as well.
            pairData.window->setResizable(Default::Filter::checkResizable(item));


            const bool fullscreen = Default::Filter::getFullscreen(item);

            GeometryProposal prop;

            //This isn't a great way to handle this. How do we set fullscreen?
            //How do we know what the true bounds are? (There is a "fake" fullscreen mode)
            if(fullscreen)
            {
                prop.proposedDimensions.get(Angle::Horizontal) = displayFull.w;
                prop.proposedDimensions.get(Angle::Vertical)   = displayFull.h;

                //The default is all false for the expand/shrink flags
            }
            else
            {
                //if in top-down mode, then get the window size and lock both
                // dimensions. Else, in order, use window-size-hint, then
                // starting size. Normal size expressions will lock the
                // viewport in the material, we don't need them here.
                prop.proposedDimensions.get(Angle::Adjacent) = displayUsable.w;
                prop.proposedDimensions.get(Angle::Opposite) = displayUsable.h;

                const int sizeStrategy = Default::Filter::getWindowSizeStrategy(item);

                if(sizeStrategy == Property::BottomUp)
                {
                    prop.shrinkForAngle(Angle::Vertical)   = true;
                    prop.shrinkForAngle(Angle::Horizontal) = true;
                }
                else
                {
                    //Use the current window size if initial not set. Relying on the
                    // user to make sure there are initial sizes on first pass...

                    const Dimens currentSize = window->getClientArea();

                    prop.proposedDimensions.get(Angle::Adjacent) = currentSize.get(Angle::Adjacent);
                    prop.proposedDimensions.get(Angle::Opposite) = currentSize.get(Angle::Opposite);
                }
            }

            item->shape(prop);
            item->setOffset(Coords());

            invalidateHoverDesperately();

            const Dimens drawableDimens = item->getViewport().d;

            if(!pairData.previousSizeSet || pairData.previousDimens != drawableDimens)
            {
                window->setClientArea(drawableDimens);
                pairData.previousDimens = drawableDimens;
                pairData.previousSizeSet = true;

                //Rely on window size change event invalidating the canvas again, otherwise the update
                // is too soon and the window system hasn't responded to the resize request yet.
            }
            else pairData.invalidCanvas = true;

            //Does this make it so that the surface doesn't "jiggle" by one pixel on resizes?
            pairData.invalidCanvas = true;

            pairData.invalidPosition = true;
            pairData.invalidGeometry = false;
        }

        if(pairData.invalidPosition)
        {
            //We don't use SDL_WINDOWPOS_CENTERED because we can't compare
            // it to the current position of the window for flag invalidation.
            //Looking at the SDL source code, it doesn't make any special calls or
            // anything, it just does a basic centering equation.
            //Also, it uses DisplayBounds and not UsableDisplayBound like I would
            // like. So...
            //ALSO: Nothing fancy for different screens, etc. TODO




            //

            //This doesn't really work, because if it's still grabbed, then the
            // resize doesn't happen, and an event is not generated. And nothing is
            // generated once the mouse is released... Shit. It only really works
            // with constant polling.

            /*
            if(window->getWindowPos() == theNewPosition)
            {
                invalidateHoverDesperately(); //AFTER we move the window (shouldn't matter tho?)
                pairData.invalidPosition = false;
            }
            */
            //else, we couldn't set the position. Probably because the window is still grabbed.
            // In this case we just spin until the user lets go. Hopefully no other position puts us
            // in this state, such as attempting to set the position whilst fullscreen.
            //(still doesn't work TODO)
        }

        if(pairData.invalidCanvas)
        {
            if(window->getCanvas()->ready())
            {
                auto canvas = window->getCanvas();
                canvas->clear();

                item->render(canvas, Coords());
                window->present();
            }

            pairData.invalidCanvas = false;
        }

        if(pairData.invalidTitleText)
        {
            window->setTitleText(Default::Filter::getTitleText(item));
            pairData.invalidTitleText = false;
        }

        if(pairData.invalidResizability)
        {
            window->setResizable(Default::Filter::checkResizable(item));
            pairData.invalidResizability = false;
        }

        if(pairData.invalidVisibility)
        {
            window->setVisible(Default::Filter::getVisibility(item));
            pairData.invalidVisibility = false;
        }

        {
            const Bitmap bipbap = window->getCanvas()->getBitmap();
            reprodyne_validate_bitmap_hash(item, "bitmap",
                                           bipbap.width * bipbap.channels,
                                           bipbap.height,
                                           bipbap.stride,
                                           bipbap.data);
        }

        ++it;
    }
}

bool SDLdriver::checkAnythingToDo()
{
    if(hoverInvalidated)
        return true;

    if(reprodyne_intercept_double(this, "has-event", SDL_HasEvents(SDL_FIRSTEVENT, SDL_LASTEVENT)))
        return true;


    for(auto& pair : pairs)
    {
        auto& data = pair.second;

        //Is data.destroy ever true here? This should get run right after refresh which handles that...
        // Of course it handles everything else, too... Except invalidPosition, sometimes.
        if(data.destroy || data.invalidCanvas || data.invalidGeometry || data.invalidPosition ||
                data.invalidTitleText || data.invalidVisibility || data.invalidResizability)
            return true;
    }

    return false;
}

}//IVD
