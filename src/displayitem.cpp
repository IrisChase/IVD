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
    //Precedence is Named Cells > Declaration Order (stamp) > Model Order
    std::vector<DisplayItem*> sorted(children.begin(), children.end());

    if(Default::Filter::checkModelOrder(this))
    {
        auto compareModelContainer = [&](DisplayItem* left, DisplayItem* right)
        {
            return (left->getModel() ? left->getModel()->getParentContainer()
                                     : nullptr)
                    <
                    (right->getModel() ? right->getModel()->getParentContainer()
                                       : nullptr);
        };
        std::sort(sorted.begin(), sorted.end(), compareModelContainer);


        auto sortModelRange = [&](auto begin, auto end)
        {
            //Alright so the predicate is the position in the parent
            // container itself. Soooo... Just overwrite the range.
            //I hate this.

            //So... We have a range of unsorted items, and the only way to get the sorting
            // data is to read from the parent container. So... Might as well just overwrite
            // the range.

            if(begin == end) return;

            assert((*begin)->getModel());

            //Keeping in mind, of course, that that not all internalDisplayItems
            // are children of this item... Because an item may become a child of
            // it's sibling, but not all siblings are automatically children in that case
            // so you can't just remove the this pointer.

            //This all looks bad/inefficient, but it doesn't currently show up on any profiling
            // passes so fuck it.

            //Sorry if this code is shit, I'm having a bad day and I just want this done.

            std::vector<DisplayItem*> modelsDisplayItems;

            //Iterating through the parent container is in model order (Thank god at least something
            // here is intuitive). Items bound to the model are going to end up resorted in stamp order
            // in the next pass, preserving the relative model order.

            for(ModelItemBase* modelItem : *(*begin)->getModel()->getParentContainer())
                for(auto* replacement : modelItem->internalDisplayItems)
                    modelsDisplayItems.push_back(replacement);

            const auto childModelItemsEnd =
                    std::remove_if(modelsDisplayItems.begin(), modelsDisplayItems.end(), [&](DisplayItem* item)
            {
                return !children.count(item);
            });

            auto destIt = begin;
            for(auto it = modelsDisplayItems.begin(); it != childModelItemsEnd; ++it)
            {
                assert(destIt != end);
                (*destIt++) = *it;
            }
        };

        for(auto it = sorted.begin(); it != sorted.end();)
        {
            //Find the ranges. Wasn't happy with any algorithms I found in the
            // standard library...

            if(!(*it)->getModel())
            {
                ++it;
                continue;
            }

            const ModelContainer* startModel = (*it)->getModel()->getParentContainer();

            auto begin = it;

            for(; it != sorted.end() &&
                (*it)->getModel() &&
                startModel == (*it)->getModel()->getParentContainer()
                ; ++it);

            sortModelRange(begin, it);
        }
    }

    auto compareStamp = [&](DisplayItem* left, DisplayItem* right)
    {
        return left->getElementStamp() < right->getElementStamp();
    };
    std::stable_sort(sorted.begin(), sorted.end(), compareStamp);


    if(getAttr().checkActive(AttributeKey::CellNames))
    {
        auto cells = getAttr().getLiteralList(AttributeKey::CellNames);

        cells.insert(cells.begin(), "front");
        cells.insert(cells.end(), "back");

        auto pred = [&](DisplayItem* left, DisplayItem* right)
        {
            //Missing piece of the pie

            auto calc = [&](DisplayItem* item) -> int
            {
                assert(item->getAttr().checkActive(AttributeKey::PositionWithin));
                auto optionalKey = item->getAttr().getSingleValueKey(AttributeKey::PositionWithin)->key;
                assert(optionalKey);
                const auto key = *optionalKey;

                for(int val = 0; val != cells.size(); ++val)
                    if(cells[val] == key) return val;

                std::cerr << "IVD Runtime Warning: Unrecognized cell name: " << key
                          << ", in: " << elementPath << ", "
                          << (getModel() ? "[model]"
                                         : "[static]") << std::endl;

                return cells.size() + 1;
            };

            return calc(left) < calc(right);
        };
        std::stable_sort(sorted.begin(), sorted.end(), pred);
    }

    assert(children.size() == sorted.size());

    //Ehhhhhhhhhhhhhhhck there's GOT to be an easier way!
    // But alas, I'm lazy today!
    std::vector<Material*> sortedMaterials;
    for(auto* dip : sorted) sortedMaterials.push_back(dip->getMaterial());
    return sortedMaterials;}


}//IVD
