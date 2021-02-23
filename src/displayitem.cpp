// Copyright 2021 Iris Chase
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
        return child->computerFillPrecedenceForAngle(theAngle) == FillPrecedence::Greedy;
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

std::optional<std::string> DisplayItem::getCellName()
{
    if(getAttr().checkActive(AttributeKey::PositionWithin))
        return std::optional<std::string>();

    return getAttr().getSingleValueKey(AttributeKey::PositionWithin)->key;
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

    if(myWidget.checkIsSet())
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

    myCellDimens = proposedCellSpace;
    myViewportDimens = officialProposal.roundConflicts(proposedCellSpace);
    compliantGeometry = officialProposal.verifyCompliance(myCellDimens);

    //Cell alignment
    relativeCellOffset = getCellAlignmentOffsetMindingReserved(myCellDimens, myViewportDimens);
}

Dimens DisplayItem::shapeDrawingArea(const GeometryProposal officalProposal)
{
    if(myWidget.checkIsSet())
    {
        myWidget.shape(officalProposal);
        return myWidget.getSpace();
    }
    else return officalProposal.proposedDimensions;
}

void DisplayItem::computerAbsoluteOffsets(const Coords parentViewportOffset)
{
    absoluteViewportOffset = parentViewportOffset + relativeViewportOffset;
    absoluteCellOffset = absoluteViewportOffset + relativeCellOffset;

    for(DisplayItem* child : children)
        child->computerAbsoluteOffsets(absoluteViewportOffset);
}

void DisplayItem::render()
{
    Canvas* theCanvas = myEnv->getCanvas();
    const Rect viewportClip(absoluteViewportOffset, myViewportDimens);
    //Cell offset already takes margins into account
    const Rect cellClip(absoluteCellOffset, myCellDimens);
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
        //--->Draw borders<---

        theCanvas->pushClip(borderLessCell);

        const auto elementColor = Default::Filter::getElementColor(this);
        if(elementColor)
            theCanvas->fillRect(borderLessCell, *elementColor);

        theCanvas->popClip(); //borderlessCell
    }

    if(myWidget.checkIsSet() && myWidget.isDrawable())
    {
        theCanvas->pushClip(contentClip);
        theCanvas->setOffset(contentClip.c);

        myWidget.draw(myWidget.isLayout() ? nullptr : theCanvas);

        theCanvas->resetOffset();
        theCanvas->popClip(); //contentClip
    }

    theCanvas->popClip();//cellClip
    theCanvas->popClip();//viewportClip
    theCanvas->setAlpha(savedAlpha); //restored alpha :)
}

void DisplayItem::updateHover()
{
    const Coords point = myEnv->getMouseOffsetRelativeToWindow();

    if(Rect(absoluteCellOffset, myCellDimens).checkCollision(point))
    {
        if(myWidget.checkIsSet())
        {
            myWidget.bubble();

            //Should this have direct integration or just let
            // the widget do state stuff? I think the latter...
            //So... Why does it have a return value TODO
            const Coords relativePoint = point - absoluteCellOffset + getReservedInnerPaddingDimens();
            myWidget.detectCollisionPoint(relativePoint); //Rename to "handle"?
        }
        //else without a layout/widget we just assume there's no rules for child collision
        // because layouts define all that stuff

        //Now we handle our own
        if(!theStateManager->checkState(StateKey(States::Item::HoverExclusive, this)))
            theStateManager->mutateIfObserved(StateKey(States::Item::HoverExclusive, this), true);

        theStateManager->mutateIfObserved(StateKey(States::Item::HoverInclusive, this), true);
    }
}

std::vector<IVD_Element*> DisplayItem::getChildWidgetInStampOrder()
{
    std::vector<DisplayItem*> sorted;
    for(DisplayItem* child : children)
        sorted.push_back(child);

    auto compareStamp = [&](DisplayItem* left, DisplayItem* right)
    {
        return left->getElementStamp() < right->getElementStamp();
    };
    std::sort(sorted.begin(), sorted.end(), compareStamp);

    //---------------------->stable SORT BY NAMED CELLS<------
    //Just reverse iterate the named cells list, and then sub-iteratorate
    // the result vector, and move the one to the beginning.
    //YES I KNOW IT'S SUPER INNEFICIENT. But it might be better
    // than a clever solution because named cells will probably never be
    // more than like 10, and the children should typically be == to named cells
    // so 10*10 isn't a huge deal
    //It's probably not even worth testing unless this shows up in during
    // profiling.

    if(myAttrs.checkActive(AttributeKey::CellNames))
    {
        auto cells = myAttrs.getLiteralList(AttributeKey::CellNames);

        for(std::vector<std::string>::reverse_iterator rit = cells.rbegin(); rit != cells.rend(); ++rit)
        {
            const std::string& cellName = *rit;

            for(std::vector<DisplayItem*>::iterator it = sorted.begin(); it != sorted.end(); ++it)
            {
                DisplayItem* child = *it;
                if(child->getCellName() == cellName)
                {
                    sorted.erase(it);
                    sorted.insert(sorted.begin(), child);
                    break;
                }
            }
        }
    }


    //Then do a special "first"/"last" reorder if you want that feature
    // back for whatever reason.

    std::vector<IVD_Element*> result;

    for(DisplayItem* child : sorted)
        result.push_back(reinterpret_cast<IVD_Element*>(child));

    return result;
}

IVD_Element* DisplayItem::getChildElementForNamedCell(const std::string name)
{
    for(DisplayItem* child : children)
    {
        auto optionalCellName = getCellName();

        if(!optionalCellName) continue;

        if(*optionalCellName == name)
            return reinterpret_cast<IVD_Element*>(child);
    }

    return nullptr;
}



}//IVD
