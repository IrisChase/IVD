//Copyright 2021 Iris Chase
//
//Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#pragma once

//THIS FILE MUST REMAIN SAFE TO BE INCLUDED AS A HEADER
// ONLY USER INCLUDE!!!!

#include <sstream>
#include <ostream>
#include <tuple>

#include "../IVD_constants_c.h"

extern "C"
{
#include "../IVD_c.h"
}

namespace IVD
{

//This might all look like pedantic gold-plating, OO-snorting
// idealogical-cargo-culting... But the resulting layout
// code is a helluva lot easier to grok with when these
// concepts are broken up like this. Trust me, I've done both.

enum class Angle
{
    Horizontal = IVD_ANGLE_HORIZONTAL,
    Vertical = IVD_ANGLE_VERTICAL,

    Adjacent = IVD_ANGLE_ADJACENT,
    Opposite = IVD_ANGLE_OPPOSITE,
};


//Greedy simply means that the layout will never take less than
// it is offered.
//Shrinky means that it will take less if it can get away with it.
//But either setting will still ask for more space if not given enough.
enum class FillPrecedence
{
    Greedy = IVD_FILL_PRECEDENCE_GREEDY,
    Shrinky = IVD_FILL_PRECEDENCE_SHRINKY,
};

struct Dimens;

struct Coords
{
    int x;
    int y;

    Coords(): x(0), y(0) {}
    Coords(int x, int y): x(x), y(y) {}
    Coords(const Dimens& theDimens);
    Coords(IVD_Coords* other)
    {
        x = *IVD_coords_x(other);
        y = *IVD_coords_y(other);
    }

    friend Coords operator+(Coords left, const Coords& right)
    {
        left.x += right.x;
        left.y += right.y;

        return left;
    }

    Coords operator+=(const Coords& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    int& get(Angle angle)
    {
        if(angle == Angle::Horizontal) return x;
        return y;

    }
    int get(Angle angle) const
    {
        if(angle == Angle::Horizontal) return x;
        return y;
    }

    Coords operator-(const Coords& other) const
    { return Coords(x - other.x, y - other.y); }

    Coords operator-=(const Coords& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    std::string generatePrintout() const;

    bool operator==(const Coords& other) const
    { return std::tie(x, y) == std::tie(other.x, other.y); }

    bool operator!=(const Coords& other) const
    { return *this != other; }

    bool operator<(const Coords& other) const
    { return std::tie(x, y) < std::tie(other.x, other.y); }

};

struct Dimens
{
    int w;
    int h;

    Dimens(): w(0), h(0) {}
    Dimens(int width, int height): w(width), h(height) {}
    Dimens(const Coords theCoords)
    {
        w = theCoords.x;
        h = theCoords.y;
    }
    Dimens(IVD_Dimens* other)
    {
        w = *IVD_dimens_w(other);
        h = *IVD_dimens_h(other);
    }

    friend Dimens operator+(Dimens left, const Dimens& right)
    {
        left.w += right.w;
        left.h += right.h;

        return left;
    }

    friend Dimens operator+=(Dimens& left, const Dimens& right)
    {
        left.w += right.w;
        left.h += right.h;
        return left; //Back, and to the left.
    }

    friend Dimens operator-(Dimens left, const Dimens& right)
    {
        left.w -= right.w;
        left.h -= right.h;

        return left;
    }

    friend Dimens operator-=(Dimens& left, const Dimens& right)
    {
        left.w -= right.w;
        left.h -= right.h;
        return left;
    }

    friend Coords operator+(Coords left, const Dimens& right)
    {
        left.x += right.w;
        left.y += right.h;

        return left;
    }

    int& get(Angle angle)
    {
        if(angle == Angle::Horizontal)
            return w;
        return h;
    }

    const int& get(Angle angle) const
    {
        if(angle == Angle::Horizontal)
            return w;
        return h;
    }

    std::string generatePrintout() const;

    bool operator==(const Dimens& other) const
    { return std::tie(w, h) == std::tie(other.w, other.h); }

    bool operator!=(const Dimens& other) const
    { return *this != other; }

    bool operator<(const Dimens& other) const
    { return std::tie(w, h) < std::tie(other.w, other.h); }
};

inline Coords::Coords(const Dimens& theDimens)
{
    x = theDimens.w;
    y = theDimens.h;
}

struct Rect
{
    Coords c;
    Dimens d;

    Rect() {}
    Rect(const Coords theCoords, const Dimens theDimens): c(theCoords), d(theDimens) {}
    Rect(IVD_Rect* other)
    {
        c = IVD_rect_get_coords(other);
        d = IVD_rect_get_dimens(other);
    }

    bool checkCollision(const Coords point) const
    {
        if(point.x >= c.x &&
                point.x < c.x + d.w &&
                point.y >= c.y &&
                point.y < c.y + d.h)
        { return true; }

        return false;
    }

    bool checkCollision(const Rect box) const
    {
        return c.x < box.c.x + box.d.w &&
               c.y < box.c.y + box.d.w &&
               box.c.x < c.x + d.w &&
               box.c.y < c.y + d.h;
    }

    const Rect& operator+(const Coords& other);

    std::string generatePrintout() const;

    bool operator==(const Rect& other) const
    { return std::tie(c, d) == std::tie(other.c, other.d); }

    bool operator!=(const Rect& other) const
    { return *this != other; }

    bool operator<(const Rect& other) const
    { return std::tie(c, d) < std::tie(other.c, other.d); }
};


template<typename T>
T getForAngle(T horizontal, T vertical, Angle theAngle)
{
    if(theAngle == Angle::Horizontal)
        return horizontal;
    return vertical;
}

template<typename T>
std::string streamPrintoutHelper(const T& val)
{
    std::stringstream ss;
    ss << val.generatePrintout();
    return ss.str();
}


inline std::ostream& operator<<(std::ostream& theStream, const Dimens theDimens)
{
    theStream << "w " << theDimens.w << ", h " << theDimens.h;
    return theStream;
}

inline std::string Dimens::generatePrintout() const
{ return streamPrintoutHelper(*this); }

inline std::ostream& operator<<(std::ostream& theStream, const Coords theCoords)
{
    theStream << "x " << theCoords.x << ", y " << theCoords.y;
    return theStream;
}

inline std::string Coords::generatePrintout() const
{ return streamPrintoutHelper(*this); }


inline std::ostream& operator<<(std::ostream& theStream, const Rect theRect)
{
    theStream << theRect.d << ", " << theRect.c;
    return theStream;
}

inline std::string Rect::generatePrintout() const
{ return streamPrintoutHelper(*this); }

//Ashamed of this one so it's pretty far down.
inline int zeroGuard(const int i)
{ return (i < 0) ? 0 : i; }


}//IVD
