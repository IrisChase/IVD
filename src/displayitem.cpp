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

#include "states.h"

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

GeometryProposal DisplayItem::reviseProposalAsRequired(GeometryProposal prop)
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

FillPrecedence DisplayItem::returnGreedyIFEVENONECHILDBLINKS(const Angle theAngle)
{
    const auto& children = getChildren();

    auto pred = [theAngle](DisplayItem* child)
    {
        return child->getFillPrecedenceForAngle(theAngle) == FillPrecedence::Greedy;
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
                                        const int cellSize,
                                        const int viewportSize,
                                        const int reservedInner,
                                        const int reservedOuter)
{

    const auto optionalProperty = getAlignmentProperty(theAngle);
    const int theAlignmentProperty = optionalProperty && (cellSize < viewportSize) ? *optionalProperty
                                                                               : Default::alignmentProperty;
    switch (theAlignmentProperty)
    {
    case Property::Inner:  return reservedInner;
    case Property::Center: return (viewportSize ? (viewportSize / 2) : 0) - (cellSize ? cellSize / 2 : 0);
    case Property::Outer:  return viewportSize - cellSize - reservedOuter;
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
    GeometryProposal revisedProposal = reviseProposalAsRequired(officialProposal);

    revisedProposal.proposedDimensions -= getReservedDimens();

    if(revisedProposal.proposedDimensions.w < 0)
        revisedProposal.proposedDimensions.w = 0;

    if(revisedProposal.proposedDimensions.h < 0)
        revisedProposal.proposedDimensions.h = 0;

    const Dimens proposedCellSpace = shapeDrawingArea(revisedProposal) + getReservedDimens();

    myCellRect = proposedCellSpace;
    myViewportRect.d = officialProposal.roundConflicts(proposedCellSpace);
    compliantGeometry = officialProposal.verifyCompliance(myCellRect.d);

    //Cell alignment
    myCellRect.c = getCellAlignmentOffsetMindingReserved(myCellRect.d, myViewportRect.d);
}

Dimens DisplayItem::shapeDrawingArea(const GeometryProposal officalProposal)
{
    if(myWidget.isSet())
    {
        myWidget.shape(officalProposal);
        return myWidget.getSpace();
    }
    else return officalProposal.proposedDimensions;
}

void DisplayItem::render(Canvas *theCanvas, const Coords offset)
{
    const Rect viewportClip = [&]()
    {
        Rect clip = myViewportRect;
        clip.c += offset;
        return clip;
    }();

    //Cell offset already takes margins into account
    const Rect cellClip = [&]()
    {
        Rect clip = myCellRect;
        clip.c += offset;
        return clip;
    }();

    const Rect borderLessCell = [&]() -> Rect
    {
        Rect clip = cellClip;
        clip.d -= getReservedBorderDimens();
        clip.c += getReservedInnerBorderDimens();
        return clip;
    }();

    const Rect contentClip = [&]() -> Rect
    {
        Rect clip = borderLessCell;
        clip.d -= getReservedPaddingDimens();
        clip.c += getReservedInnerPaddingDimens();
        return clip;
    }();

    const auto savedAlpha = theCanvas->getAlpha();
    theCanvas->setAlpha(savedAlpha * Default::Filter::getAlpha(this));

    theCanvas->pushClip(viewportClip);
    theCanvas->pushClip(cellClip); //they could overlap


    //Draw boxeee
    {
        //>Draw borders<

        theCanvas->pushClip(borderLessCell);

        const auto elementColor = Default::Filter::getElementColor(this);
        if(elementColor)
            theCanvas->fillRect(borderLessCell, *elementColor);

        theCanvas->popClip(); //borderlessCell
    }

    if(myWidget.isSet() && myWidget.isDrawable())
    {
        theCanvas->pushClip(contentClip);
        theCanvas->setOffset(contentClip.c);

        myWidget.draw(theCanvas);

        theCanvas->resetOffset();
        theCanvas->popClip(); //contentClip
    }

    theCanvas->popClip();//cellClip
    theCanvas->popClip();//viewportClip
    theCanvas->setAlpha(savedAlpha); //restored alpha :)
}

void DisplayItem::updateHoverInclusive(StateManager* theStateManager, const Coords point)
{
    for(DisplayItem* child : children)
        child->updateHoverExclusive(theStateManager, point - myCellRect.c);

    if(myCellRect.checkCollision(point))
        theStateManager->mutateIfObserved(StateKey(States::Item::HoverInclusive, this), true);
}

bool DisplayItem::updateHoverExclusive(StateManager* theStateManager, const Coords point)
{
    for(DisplayItem* child : children)
        if(child->updateHoverExclusive(theStateManager, point - myCellRect.c))
            return true;

    if(myCellRect.checkCollision(point))
    {
        theStateManager->mutateIfObserved(StateKey(States::Item::HoverExclusive, this), true);
        return true;
    }
    return false;
}



}//IVD
