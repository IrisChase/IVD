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

#ifndef SIMPLEMATERIAL_H
#define SIMPLEMATERIAL_H

#include "material.h"

namespace IVD
{

//Just a colored rectangle, shrinky precedence. Just goes with whatever is suggested. Spineless...
class SimpleMaterial : public Material
{
public:
    SimpleMaterial(DisplayItem* theItem): Material(theItem) {}

    FillPrecedence computerFillPrecedenceForAngle(const Angle theAngle) final;

    void shapeDrawingArea(const GeometryProposal officialProposal) final
    { simpleShape(officialProposal); }

    void setAbsoluteOffset(const Coords offset) final
    {
        setViewportOffset(offset);
        updateDrawingAreaOffset();
    }

    void drawConcrete(Canvas* theCanvas) final
    { drawBasic(theCanvas, getDrawingArea()); }

    bool applyToColliding(const Rect box, std::function<bool(DisplayItem*)> fun) final
    { return applyToCollidingSingle(box, fun); }

    //Nothin to invalidate, so we just use the default implementation of reset.
    //The reference to myItem is self referential, i.e. that item is what owns this
    // material.
};

}//IVD


#endif // SIMPLEMATERIAL_H
