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

#ifndef RECT_H
#define RECT_H

#include <sstream>
#include <ostream>
#include "irisutils/lexcompare.h"
#include "keywords.h"

namespace IVD
{

//This might all look like pedantic gold-plating, OO-snorting
// idealogical-cargo-culting... But the resulting layout
// code is a helluva lot easier to grok with when these
// concepts are broken up like this. Trust me, I've done both.

enum class Angle
{
    Horizontal,
    Vertical,

    Adjacent = Horizontal,
    Opposite = Vertical,
};

enum class Alignment
{
    Inner = Property::Inner, //Default
    Center = Property::Center,
    Outer = Property::Outer,
};

//Greedy simply means that the layout will never take less than
// it is offered.
//Shrinky means that it will take less if it can get away with it.
//But either setting will still ask for more space if not given enough.
enum class FillPrecedence
{
    Greedy,
    Shrinky,
};

struct Dimens;

struct Coords
{
    int x;
    int y;

    Coords(): x(0), y(0) {}
    Coords(int x, int y): x(x), y(y) {}
    Coords(const Dimens& theDimens);

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

    IRISUTILS_DEFINE_COMP(Coords, x, y)
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

    IRISUTILS_DEFINE_COMP(Dimens, w, h)
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
    Rect(Dimens theDimens): d(theDimens) {}
    Rect(const Coords theCoords, const Dimens theDimens): c(theCoords), d(theDimens) {}

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

    IRISUTILS_DEFINE_COMP(Rect, c, d)
};

struct AdvancedRect
{
    int borderRadiusTl;
    int borderRadiusTr;
    int borderRadiusBl;
    int borderRadiusBr;
    Rect r;

    AdvancedRect():
        borderRadiusTl(0),
        borderRadiusTr(0),
        borderRadiusBl(0),
        borderRadiusBr(0)
    {}

    AdvancedRect(const Rect& other):
        borderRadiusTl(0),
        borderRadiusTr(0),
        borderRadiusBl(0),
        borderRadiusBr(0),
        r(other)
    {}

    bool checkCollision(const Coords pos) const
    {
        return r.checkCollision(pos);

        //heh, supposed to do border checking...
        //TODO
    }

    IRISUTILS_DEFINE_COMP(AdvancedRect,
                          borderRadiusTl,
                          borderRadiusTr,
                          borderRadiusBl,
                          borderRadiusBr)
};

/*
struct Shape
{
    std::vector<AdvancedRect> rects;

    Dimens getOverallRegion() const
    {
        Dimens theDimens;

        for(const AdvancedRect ar : rects)
        {
            const int arxboundry = ar.r.c.x + ar.r.d.w;
            const int aryboundry = ar.r.c.y + ar.r.d.h;

            if(arxboundry > theDimens.w)
                theDimens.w = arxboundry;
            if(aryboundry > theDimens.h)
                theDimens.h = aryboundry;
        }

        return theDimens;
    }

    bool checkCollision(const Coords pos)
    {
        for(const AdvancedRect ar : rects)
            if(ar.checkCollision(pos)) return true;
        return false;
    }

    Shape(){}
    Shape(const Rect& other):
        rects({other})
    {}

    const Shape& operator=(const Rect& other)
    {
        rects = {other};
        return *this;
    }

    const Shape& operator+(const Shape& other);
    const Shape& operator+(const Rect& other);

    IRISUTILS_DEFINE_COMP(Shape, rects)
};
*/


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

#endif // RECT_H
