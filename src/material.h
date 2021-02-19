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

#ifndef MATERIAL_H
#define MATERIAL_H

#include <map>
#include <functional>
#include "geometryproposal.h" //includes ivd/geometry.h

extern "C"
{
#include "user_include/IVD_c.h"
}


namespace IVD
{

class Canvas;
class DisplayItem;

class Materialdeadmanwalkin
{
    Rect myViewport;
    Rect drawingArea; //Drawing area less padding/border
    
protected:
    DisplayItem* myItem;

    void setDrawingArea(const Dimens theArea)
    { drawingArea = theArea; }
    
    void setRelativeDrawingAreaOffset(const Coords theOffset)
    { drawingArea.c = theOffset; }
    
    void setViewportArea(const Dimens theArea)
    { myViewport.d = theArea; }
    
    void setViewportOffset(const Coords theOffset);



    //blah
    void simpleShape(const GeometryProposal officialProposal);
    void updateDrawingAreaOffset();
    void drawBasic(Canvas* theCanvas, Rect renderingArea);
    bool applyToCollidingSingle(const Rect box, std::function<bool(DisplayItem*)> fun);
    bool applyToCollidingMultiFlat(const Rect box, std::function<bool(DisplayItem*)> fun);

    FillPrecedence returnGreedyIfEVENONECHILDBLINKS(const Angle theAngle);
    FillPrecedence filterFillPrecedence(const FillPrecedence fill, const Angle theAngle);

public:
    Material(DisplayItem* theItem): myItem(theItem) {}
    virtual ~Material() {}

    DisplayItem* getDisplayItem()
    { return myItem; }

    Rect getViewport()
    { return myViewport; }

    Rect getDrawingArea()
    { return Rect(getDrawingAreaOffset(), drawingArea.d); }

    Coords getDrawingAreaOffset()
    { return drawingArea.c + myViewport.c; }

    Coords getRelativeDrawingAreaOffset()
    { return drawingArea.c; }


    //Return value of "fun" is the loop invariant, i.e. true == continue
    //e.g: for(auto it = children.begin(); it != children.end() && fun(*it); ++it);
    //TODO: I don't like that this is DisplayItem*... It's not generic. THINK ABOUT IT
    virtual bool applyToColliding(const Rect box, std::function<bool(DisplayItem*)> fun)
    { return false; }
};


}//IVD

#endif // MATERIAL_H
