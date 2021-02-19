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

#include "displayitem.h"

#include "environment.h"
#include "defaults.h"
#include "canvas.h"

#include "assert.h"

namespace IVD
{

Angle DisplayItem::getRelativeAdjacent()
{
    auto property = getAttr().getProperty(AttributeKey::Orientation);
    return !property ? Default::angle
                     : *property == Property::AdjacentIsHorizontal ? Angle::Horizontal
                                                         : Angle::Vertical;
}

Angle DisplayItem::correctAngle(const Angle theAngle)
{
    //Convert SYMANTIC adjacent into RELATIVE adjacent.
    return theAngle == Angle::Adjacent ? getRelativeAdjacent()
                                       : getRelativeOpposite();
}

int DisplayItem::getReservedForKey(Angle theAngle, const int adjacentKey, const int oppositeKey)
{
    auto key = getAttr().getInt(theAngle == getRelativeAdjacent() ? adjacentKey
                                                                  : oppositeKey);
    return key ? *key
               : 0;
}

void DisplayItem::addAttributeSet(const AttributePositionPair pair)
{
    contributingAttrs[pair.position] = pair.attrs;
    myEnv->markAsChangedAttributes(this);
}

void DisplayItem::removeAttributeSet(const AttributePositionPair pair)
{
    contributingAttrs.erase(pair.position);
    myEnv->markAsChangedAttributes(this);
}


void DisplayItem::recomputeAttributeSet()
{
    myAttrs.beginAttributeSetRecompute();
    myAttrs.mergeIn(defaultState);

    for(auto pair : contributingAttrs)
        myAttrs.mergeIn(*pair.second);

    myAttrs.commitAttributeSetRecompute();
}

void DisplayItem::setParent(DisplayItem* item)
{
    if(item == parent) return;

    deparent();
    parent = item;
    parent->children.insert(this);
}

void DisplayItem::deparent()
{
    if(parent)
    {
        parent->children.erase(this);
        parent = nullptr;
    }
}

void DisplayItem::prepareToDie()
{
    deparent();
    for(DisplayItem* child : children) child->deparent();
}

//The root of the tree is *this* if no parents.
DisplayItem* DisplayItem::getRoot()
{
    DisplayItem* root;
    DisplayItem* candidate = this;

    while(candidate)
    {
        root = candidate;
        candidate = candidate->parent;
    }

    return root;
}

int DisplayItem::getDepth()
{
    if(parent) return parent->getDepth() + 1;
    return 0;
}

GeometryProposal DisplayItem::reviseProposalForDrawingArea(GeometryProposal prop)
{
    auto internal = [&](const Angle theAngle)
    {
        const int attrKey = (theAngle == getRelativeAdjacent()) ? AttributeKey::SizeA
                                                                : AttributeKey::SizeO;

        //Goooooooood the interface to Attribute is so fucked up.
        if(!getAttr().checkActive(attrKey)) return;

        //TODO wanna remove the binding stuff I think
        auto attr = getAttr().getInt(attrKey);

        if(attr && getAttr().isConst(attrKey))
        {
            const int requiredSpace = static_cast<int>(*attr);

            prop.proposedDimensions.get(theAngle) = requiredSpace;
            prop.expandForAngle(theAngle) = false;
            prop.shrinkForAngle(theAngle) = false;
        }
        else if(attr) //non-const case
        {
            prop.proposedDimensions.get(theAngle) = int(*attr);
            prop.proposedDimensions = prop.roundConflicts(prop.proposedDimensions);
        }
    };

    internal(Angle::Horizontal);
    internal(Angle::Vertical);

    return prop;
}

std::optional<int> DisplayItem::getAlignmentProperty(const Angle theAngle)
{
    return getAttr().getProperty(getRelativeAdjacent() == theAngle
                                 ? AttributeKey::AlignAdjacent
                                 : AttributeKey::AlignOpposite);
}

int DisplayItem::getJustificationOffset(const int itemSize, const int cellSize)
{
    const auto optional = Default::Filter::getJustificationProperty(this);

    if(!optional) return 0;

    switch(*optional)
    {
    case Property::Inner: return 0;
    case Property::Center: return (cellSize ? (cellSize / 2) : 0) - (itemSize ? itemSize / 2 : 0);
    case Property::Outer: return cellSize - itemSize;
    default: assert(false);
    }
}



int DisplayItem::getCellAlignmentOffset(const Angle theAngle,
                                        const int itemSize,
                                        const int cellSize,
                                        const int reservedInner,
                                        const int reservedOuter)
{

    const auto optionalProperty = getAlignmentProperty(theAngle);
    const int theAlignmentProperty = optionalProperty && (itemSize < cellSize) ? *optionalProperty
                                                                               : Default::alignmentProperty;
    switch (theAlignmentProperty)
    {
    case Property::Inner:  return reservedInner;
    case Property::Center: return (cellSize ? (cellSize / 2) : 0) - (itemSize ? itemSize / 2 : 0);
    case Property::Outer:  return cellSize - itemSize - reservedOuter;
    default: assert(false);
    }
}

Coords DisplayItem::getTranslationOffset()
{
    Coords trans;

    auto optionalTransA = myAttrs.getInt(AttributeKey::TranslationA);
    auto optionalTransO = myAttrs.getInt(AttributeKey::TranslationO);

    if(optionalTransA) trans.get(getRelativeAdjacent()) = *optionalTransA;
    if(optionalTransO) trans.get(getRelativeOpposite()) = *optionalTransO;

    return trans;
}

int DisplayItem::getSizeForAngle(const Angle theAngle)
{
    auto optionalSize = [&]()
    {
        if(correctAngle(theAngle) == Angle::Adjacent)
            return myAttrs.getInt(AttributeKey::SizeA);
        else
            return myAttrs.getInt(AttributeKey::SizeO);
    }();

    return optionalSize ? *optionalSize
                        : 0;
}

std::optional<FillPrecedence> DisplayItem::filterFillPrecedenceForAngle(const Angle theAngle)
{
    const auto attrKey = correctAngle(theAngle) == Angle::Adjacent ? AttributeKey::OverrideFillPrecedenceAdjacent
                                                                   : AttributeKey::OverrideFillPrecedenceOpposite;
    const auto optionalAttr = myAttrs.getProperty(attrKey);

    if(optionalAttr)
    {
        switch(*optionalAttr)
        {
        case Property::Shrinky: return FillPrecedence::Shrinky;
        case Property::Greedy:  return FillPrecedence::Greedy;
        default: std::terminate(); //Prolly a little drastic... TODO
        }
    }

    return std::optional<FillPrecedence>();
}

FillPrecedence DisplayItem::computerFillPrecedenceForAngle(const Angle theAngle)
{
    const auto optionalOverride = filterFillPrecedenceForAngle(theAngle);

    if(optionalOverride) return *optionalOverride;

    if(myWidget.isSet())
        return myWidget.getFillPrecedence(theAngle);

    return FillPrecedence::Shrinky; //DEFAULT
}

void DisplayItem::shape(const GeometryProposal officialProposal)
{
    GeometryProposal revisedProposal = reviseProposalForDrawingArea(officialProposal);

    revisedProposal.proposedDimensions -= getReservedDimens();

    if(revisedProposal.proposedDimensions.w < 0)
        revisedProposal.proposedDimensions.w = 0;

    if(revisedProposal.proposedDimensions.h < 0)
        revisedProposal.proposedDimensions.h = 0;

    shapeDrawingArea(revisedProposal);

    auto proposedViewport = drawingArea.d + getReservedDimens();

    myViewport.d = officialProposal.roundConflicts(drawingArea.d + getReservedDimens());
}

void DisplayItem::shapeDrawingArea(const GeometryProposal officalProposal)
{
    if(myWidget.isSet())
        myWidget.shape(officalProposal);
    else
        drawingArea.d = officalProposal.proposedDimensions;
}

void DisplayItem::drawConcrete(Canvas *theCanvas)
{
    theCanvas->pushCursor(myViewport.c);
    theCanvas->pushClip(myViewport.d);

    //Draw basic stuffs;
    {
        Color::AlphaType alpha = Default::Filter::getAlpha(this);
        const auto elementColor = Default::Filter::getElementColor(this);

        theCanvas->pushCursor(getReservedInnerMarginDimens());
        //-->>draw borders<<--
        //rounded corner clips

        theCanvas->pushCursor(getReservedInnerBorderDimens());
        //Draw background color (which ignores padding)

        if(elementColor)
        {
            Rect renderingArea = myViewport.d;
            myViewport.d -= getReservedBorderDimens();
            myViewport.d -= getReservedMarginDimens();
            //Rounded corners ughhhhh
            theCanvas->fillRect(renderingArea, *elementColor, alpha);
        }

        //Widget stuffs!
        if(myWidget.isSet())
        {
            theCanvas->pushCursor(getReservedInnerPaddingDimens());
            theCanvas->pushClip(drawingArea.d);

            myWidget.draw(theCanvas);

            theCanvas->popClip();
            theCanvas->popCursor(); //Pad
        }


        theCanvas->popCursor(); //Border
        theCanvas->popCursor(); //Margins
    }

    theCanvas->popClip();
    theCanvas->popCursor(); //Viewport
}



}//IVD
