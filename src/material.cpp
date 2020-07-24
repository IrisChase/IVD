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

#include "material.h"
#include "displayitem.h"
#include "defaults.h"
#include "canvas.h"

namespace IVD
{

bool Material::applyToCollidingSingle(const Rect box, std::function<bool (DisplayItem*)> fun)
{
    if(!getViewport().checkCollision(box)) return false; //But why tho?

    //NEED TO MASK THIS OFF! TODO
    //WHAT? WHY ARE YOU SHOUTING? i DON'T KNOW WHAT YOU NEED
    //AAAHHHHHHHHH

    Rect collidableArea = getDrawingArea();
    collidableArea.c -= Coords(myItem->getReservedInnerPaddingDimens() - myItem->getReservedInnerBorderDimens());
    collidableArea.d += myItem->getReservedPaddingDimens() + myItem->getReservedBorderDimens();

    if(collidableArea.checkCollision(box))
        return fun(myItem);

    return false;
}

bool Material::applyToCollidingMultiFlat(const Rect box, std::function<bool (DisplayItem*)> fun)
{
    for(Material* child : myItem->getChildMaterials())
        if(child->applyToColliding(box, fun))
            return true;

    return applyToCollidingSingle(box, fun);
}

FillPrecedence Material::returnGreedyIfEVENONECHILDBLINKS(const Angle theAngle)
{
    const auto& children = myItem->getChildren();

    auto pred = [theAngle](DisplayItem* child)
    {
        return child->getMaterial()->getFillPrecedenceForAngle(theAngle) == FillPrecedence::Greedy;
    };
    return std::any_of(children.begin(), children.end(), pred) ? FillPrecedence::Greedy
                                                               : FillPrecedence::Shrinky;

    /// ;; IT'S STILL JUST NOT THE SAAAAAAAAME
    ///
    /// (if (some (lambda (x) (eql (elt something-something-something x) fill-precedence-greedy)) children)
    ///     'fill-precedence-greedy
    ///     'fill-precedence-shrinky)
    ///
    /// ;; I wanne use lithp :<
}

void Material::draw(Canvas* theCanvas)
{
    theCanvas->pushClip(getViewport());
    drawConcrete(theCanvas);
    theCanvas->popClip();
}

FillPrecedence Material::getFillPrecedenceForAngle(const Angle theAngle)
{
    const auto optionalOverride = myItem->filterFillPrecedenceForAngle(theAngle);

    if(optionalOverride) return *optionalOverride;

    return computerFillPrecedenceForAngle(theAngle);
}

void Material::shape(const GeometryProposal officialProposal)
{
    assert(myItem);

    GeometryProposal revisedProposal = myItem->reviseProposalForDrawingArea(officialProposal);

    revisedProposal.proposedDimensions -= myItem->getReservedDimens();

    if(revisedProposal.proposedDimensions.w < 0)
        revisedProposal.proposedDimensions.w = 0;

    if(revisedProposal.proposedDimensions.h < 0)
        revisedProposal.proposedDimensions.h = 0;

    shapeDrawingArea(revisedProposal);

    auto proposedViewport = drawingArea.d + myItem->getReservedDimens();

    myViewport.d = officialProposal.roundConflicts(drawingArea.d + myItem->getReservedDimens());
}

void Material::updateDrawingAreaOffset()
{
    Coords relativeOffset = myItem->getCellAlignmentOffsetMindingReserved(drawingArea.d,
                                                                          myViewport.d);

    setRelativeDrawingAreaOffset(relativeOffset);
}


void Material::setViewportOffset(const Coords theOffset)
{
    myViewport.c = theOffset + myItem->getTranslationOffset();
}

void Material::simpleShape(const GeometryProposal officialProposal)
{
    setDrawingArea(officialProposal.proposedDimensions - myItem->getReservedDimens());
}

void Material::drawBasic(Canvas* theCanvas, Rect renderingArea)
{
    volatile auto x = myItem->getReservedPaddingDimens();
    renderingArea = getDrawingArea();
    renderingArea.c -= Coords(myItem->getReservedInnerPaddingDimens());
    renderingArea.d += myItem->getReservedPaddingDimens();


    assert(myItem);

    Color::AlphaType alpha = Default::Filter::getAlpha(myItem);
    const auto elementColor = Default::Filter::getElementColor(myItem);


    if(elementColor)
        theCanvas->fillRect(renderingArea, *elementColor, alpha);
}


}//IVD
