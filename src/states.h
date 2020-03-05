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

#ifndef STATES_H
#define STATES_H

#include <string>

namespace IVD
{

namespace States
{

typedef std::string Symbol;

enum class StateType
{
    Press,
    Release,
    Active,
};

enum class KeyType
{
    Positional,
    Literal,
};

//Just for consistency, to pretend that the below
// are just generic constants. WHICH YOU SHOULD!!!
inline Symbol getButton(const Symbol button)
{ return button; }

inline Symbol getButtonActive(const Symbol button)
{ return (std::string(button) + "-Active").c_str(); }

inline Symbol getButtonPress(const Symbol button)
{ return (std::string(button) + "-Press").c_str(); }

inline Symbol getButtonRelease(const Symbol button)
{ return (std::string(button) + "-Release").c_str(); }


namespace Item
{

const Symbol Hover = "IVD-Item-Hover";

//const Symbol GeometryChanged = "IVD-Item-Geometry-Changed";

const Symbol ModelChanged = "Model-Changed";

}//Item

namespace Window
{
/** \brief */

//These are only set on the display item.
/** \brief State: Set while the window is visible. Item specific.*/
const Symbol Visible = "IVD-Window-Visible";

/** \brief State: Set while the window is hidden, Item specific.*/
const Symbol Hidden = "IVD-Window-Hidden";

/** \brief Trigger State: Set when the window is shown. Item specific.*/
const Symbol Shown = "IVD-Window-Shown";

/** \brief Trigger State: This is set after a window is initialized,
 *         and the geometry is set for the first time. Item specific.*/
const Symbol Initialized = "IVD-Window-Initialized";

//Is there a difference between mouse and keyboard focus?
//(mouse focus just means the mouse is over it?)

/** \brief State: Set while the window has keyboard focus. Item Specific.*/
const Symbol Focused = "IVD-Window-Focused";

/** \brief State: Set while the window has mouse focus. Item Specific.*/
const Symbol MouseFocus = "IVD-Window-Focus-Mouse";

/** \brief Trigger State: Set when the window is minimized. Item Specific.*/
const Symbol Minimized = "IVD-Window-Minimized";

/** \brief Trigger State: Set when the window is maximized. Item Specific.*/
const Symbol Maximized = "IVD-Window-Maximized";

/** \brief Trigger State: Set when the close button is pressed on a window. Item Specific.*/
const Symbol CloseRequest = "IVD-Window-Close";

}//Window

namespace App
{
/** \brief Trigger State: Set when the application receives a close event,
 *         in addition to IVD-Window-Close for the last window.*/
const Symbol CloseApp = "IVD-Close";
}//App

namespace Key
{
//Keyboards are more standard than you'd expect,
// but perhaps not as standard as they should you'd hope be wise.

//Which one is ze any key?
const Symbol Any = "IVD-Any-Key";

const Symbol Play = "IVD-Key-Media-Play";
const Symbol Stop = "IVD-Key-Media-Stop";
const Symbol Previous = "IVD-Key-Media-Previous";
const Symbol Next = "IVD-Key-Media-Next";

const Symbol LeftSuper = "IVD-Key-Left-Super";
const Symbol LeftShift = "IVD-Key-Left-Shift";
const Symbol LeftCtrl = "IVD-Key-Left-Ctrl";
const Symbol LeftAlt = "IVD-Key-Left-Alt";

const Symbol RightSuper = "IVD-Key-Right-Super";
const Symbol RightShift = "IVD-Key-Right-Shift";
const Symbol RightCtrl = "IVD-Key-Right-Ctrl";
const Symbol RightAlt = "IVD-Key-Right-Alt";

const Symbol CapsLock = "IVD-Key-Caps-Lock";
const Symbol AltGr = "IVD-Key-Alt-Gr";
const Symbol Menu = "IVD-Key-Menu";

const Symbol PrintScreen = "IVD-Key-Print-Screen";
const Symbol ScrollLock = "IVD-Key-Scroll-Lock";
const Symbol Pause = "IVD-Key-Pause";

const Symbol Insert = "IVD-Key-Insert";
const Symbol Delete = "IVD-Key-Delete";
const Symbol Home = "IVD-Key-Home";
const Symbol End = "IVD-Key-End";
const Symbol PageUp = "IVD-Key-Page-Up";
const Symbol PageDown = "IVD-Key-Page-Down";

const Symbol LeftArrow = "IVD-Key-Arrow-Left";
const Symbol RightArrow = "IVD-Key-Arrow-Right";
const Symbol UpArrow = "IVD-Key-Arrow-Up";
const Symbol DownArrow = "IVD-Key-Arrow-Down";

const Symbol NumLock = "IVD-Key-Num-Lock";

const Symbol Escape = "IVD-Key-Escape";


const Symbol F1 = "IVD-Key-F1";
const Symbol F2 = "IVD-Key-F2";
const Symbol F3 = "IVD-Key-F3";
const Symbol F4 = "IVD-Key-F4";
const Symbol F5 = "IVD-Key-F5";
const Symbol F6 = "IVD-Key-F6";
const Symbol F7 = "IVD-Key-F7";
const Symbol F8 = "IVD-Key-F8";
const Symbol F9 = "IVD-Key-F9";
const Symbol F10 = "IVD-Key-F10";
const Symbol F11 = "IVD-Key-F11";
const Symbol F12 = "IVD-Key-F12";

//Row 0
const Symbol Tilde = "IVD-Scan-Tilde";
const Symbol RowNum1 = "IVD-Scan-Row-Num-1";
const Symbol RowNum2 = "IVD-Scan-Row-Num-2";
const Symbol RowNum3 = "IVD-Scan-Row-Num-3";
const Symbol RowNum4 = "IVD-Scan-Row-Num-4";
const Symbol RowNum5 = "IVD-Scan-Row-Num-5";
const Symbol RowNum6 = "IVD-Scan-Row-Num-6";
const Symbol RowNum7 = "IVD-Scan-Row-Num-7";
const Symbol RowNum8 = "IVD-Scan-Row-Num-8";
const Symbol RowNum9 = "IVD-Scan-Row-Num-9";
const Symbol RowNum0 = "IVD-Scan-Row-Num-0";
const Symbol Minus = "IVD-Scan-Minus";
const Symbol Equals = "IVD-Scan-Equals";
const Symbol Backspace = "IVD-Scan-Backspace";

//Row 1
const Symbol Tab = "IVD-Scan-Tab";
const Symbol Q = "IVD-Scan-Q";
const Symbol W = "IVD-Scan-W";
const Symbol E = "IVD-Scan-E";
const Symbol R = "IVD-Scan-R";
const Symbol T = "IVD-Scan-T";
const Symbol Y = "IVD-Scan-Y";
const Symbol U = "IVD-Scan-U";
const Symbol I = "IVD-Scan-I";
const Symbol O = "IVD-Scan-O";
const Symbol P = "IVD-Scan-P";
const Symbol LeftBracket = "IVD-Scan-Left-Bracket";
const Symbol RightBracket = "IVD-Scan-Right-Bracket";
const Symbol Backslash = "IVD-Scan-Backslash";

//Row 2
const Symbol A = "IVD-Scan-A";
const Symbol S = "IVD-Scan-S";
const Symbol D = "IVD-Scan-D";
const Symbol F = "IVD-Scan-F";
const Symbol G = "IVD-Scan-G";
const Symbol H = "IVD-Scan-H";
const Symbol J = "IVD-Scan-J";
const Symbol K = "IVD-Scan-K";
const Symbol L = "IVD-Scan-L";
const Symbol Semicolon = "IVD-Scan-Semicolon";
const Symbol Apostrophe = "IVD-Scan-Apostrophe";
const Symbol Return = "IVD-Scan-Return";

//Row 3
const Symbol Z = "IVD-Scan-Z";
const Symbol X = "IVD-Scan-X";
const Symbol C = "IVD-Scan-C";
const Symbol V = "IVD-Scan-V";
const Symbol B = "IVD-Scan-B";
const Symbol N = "IVD-Scan-N";
const Symbol M = "IVD-Scan-M";
const Symbol Comma = "IVD-Scan-Comma";
const Symbol Period = "IVD-Scan-Period";
const Symbol Slash = "IVD-Scan-Slash";

//Row 4
const Symbol Spacebar = "IVD-Scan-Spacebar";

namespace Pad
{

//Just going to start with basics
const Symbol Slash = "IVD-Scan-Pad-Slash";
const Symbol Star = "IVD-Scan-Pad-Star";
const Symbol Minus = "IVD-Scan-Pad-Minus";

const Symbol Num7 = "IVD-Scan-Pad-7";
const Symbol Num8 = "IVD-Scan-Pad-8";
const Symbol Num9 = "IVD-Scan-Pad-9";
const Symbol Plus = "IVD-Scan-Pad-Plus";

const Symbol Num4 = "IVD-Scan-Pad-4";
const Symbol Num5 = "IVD-Scan-Pad-5";
const Symbol Num6 = "IVD-Scan-Pad-6";
const Symbol Tab = "IVD-Scan-Pad-Tab";

const Symbol Num1 = "IVD-Scan-Pad-1";
const Symbol Num2 = "IVD-Scan-Pad-2";
const Symbol Num3 = "IVD-Scan-Pad-3";

const Symbol Num0 = "IVD-Scan-Pad-0";
const Symbol Period = "IVD-Scan-Pad-Period";

const Symbol Return = "IVD-Scan-Pad-Return";

}//Pad
}//Key


namespace Mouse
{

const Symbol ButtonLeft = "IVD-Mouse-Left";
const Symbol ButtonMiddle = "IVD-Mouse-Middle";
const Symbol ButtonRight = "IVD-Mouse-Right";
const Symbol ButtonFour = "IVD-Mouse-Four";
const Symbol ButtonFive = "IVD-Mouse-Five";

const Symbol Motion = "IVD-Mouse-Motion";
const Symbol Wheel = "IVD-Mouse-Wheel";

}//Mouse


}//States
}//IVD

#endif // STATES_H
