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

#include "OpenImageIO/imagecache.h"

namespace IVD
{

class ImageMaterial : public Material
{
    OIIO::ImageCache* cache;

    std::string getPath(DisplayItem* item);

public:
    ImageMaterial(DisplayItem* theItem, OIIO::ImageCache* cache):
        Material(theItem),
        cache(cache)
    {}

    FillPrecedence computerFillPrecedenceForAngle(const Angle theAngle) final
    { return FillPrecedence::Shrinky; }

    void setAbsoluteOffset(const Coords offset) final
    {
        setViewportOffset(offset);
        updateDrawingAreaOffset();
    }

    void shapeDrawingArea(const GeometryProposal officalProposal) final;
    void drawConcrete(Canvas *theCanvas) final;

    bool applyToColliding(const Rect box, std::function<bool (DisplayItem *)> fun) final
    { return applyToCollidingSingle(box, fun); }
};

}//IVD
