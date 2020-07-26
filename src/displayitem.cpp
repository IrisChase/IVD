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

#include <unordered_set>

#include "displayitem.h"
#include "modelitembase.h"

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
    AttributeSet fresh(this);
    fresh.mergeIn(defaultState);
    for(auto pair : contributingAttrs)
        fresh.mergeIn(*pair.second);

    myAttrs.updateToReferenceSet(fresh);
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

std::vector<Material*> DisplayItem::getChildMaterials()
{
    std::vector<Material*> childMaterials;
    for(DisplayItem* child : children)
        childMaterials.push_back(child->getMaterial());

    return childMaterials;
}

std::vector<Material*> DisplayItem::getChildMaterialsInModelOrder()
{
    if(!Default::Filter::checkModelOrder(this) || children.empty()) return getChildMaterials();

    std::vector<Material*> sorted;

    ModelContainer* container = myModel ? myModel->getChildContainer()
                                        : nullptr;

    if(!container)
    {
        //I am ashame.
        ModelItemBase* oneChildModel = nullptr;
        for(auto it = children.begin(); !oneChildModel && it != children.end(); ++it)
            oneChildModel = (*it)->myModel;

        if(!oneChildModel) return getChildMaterials(); //(╯°□°)╯︵ ┻━┻

        container = oneChildModel->getParentContainer();
        assert(container); //THIS CANNOT BE NULL MOTHERFUCKERRRR
    }


    std::vector<Material*> nonModelChildren;
    std::unordered_set<Material*> modelMaterials;

    for(ModelItemBase* mitem : *container)
    {
        //M'Item~
        for(DisplayItem* childItem : mitem->internalDisplayItems)
        {
            if(childItem->getParent() != this) continue;

            sorted.push_back(childItem->getMaterial());
            modelMaterials.insert(childItem->getMaterial());
        }
    }


    //This, as far as I know, can only happen in the sibling model positioning within
    // a model with children (not necessarily children positioned within it).
    for(DisplayItem* child : children)
    {
        if(modelMaterials.count(child->getMaterial())) continue;
        sorted.push_back(child->getMaterial());
    }

    //TODO
    //I would like to issue a warning whenever someone positions an element within a sibling element
    // that has child elements poisitioned within it already. (The warning is because the positioning
    // is undefined... Unless it's named cells?)

    //But it's supposed to be standard to position enumerated elements alongside "regular" ones, right?
    //Consider a static window. Should we be enforcing enumeration here? Either way, it might be legal
    // but the actual layout order is undefined (Unless it's named-cells???)


    return sorted;
}


}//IVD
