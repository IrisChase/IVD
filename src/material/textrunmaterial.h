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

#pragma once

#include "material.h"

namespace IVD
{

//This is for a single run of text, set by setTextForSpace. It is an otherwise
// simple object, which only reserves enough space for the text, handles basic collision,
// and drawing
class TextRunMaterial final : public Material
{
    std::string myText;

public:
    TextRunMaterial(DisplayItem* theItem): Material(theItem) {}

    FillPrecedence getFillPrecedenceForAngle(const Angle theAngle) override
    { return FillPrecedence::Shrinky; }

    void shapeDrawingArea(const GeometryProposal officalProposal) override;
    void setAbsoluteOffset(const Coords offset) override
    {
        setViewportOffset(offset);
        updateDrawingAreaOffset();
    }
    void drawConcrete(Canvas *theCanvas) override;
    bool applyToColliding(const Rect box, std::function<bool (DisplayItem *)> fun) override
    { return applyToCollidingSingle(box, fun); }

    //Extended interface
    //INVALIDATES SHAPEEEEEEEEEEEEE
    void setText(const std::string run)
    { myText = run; }
    std::string setTextForSpace(const std::string run, const int space);
};

}//IVD
