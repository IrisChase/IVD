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

#ifndef DISPLAYITEM_H
#define DISPLAYITEM_H

#include <set>
#include <map>

#include "attributepositionpair.h"
#include "runtimeattributeset.h"
#include "material.h"

#include <reprodyne.h>

namespace IVD
{

class Environment;
class ModelContainer;
class ModelItemBase;
class Canvas;


class DisplayItem
{
    const int elementStamp;
    Environment* myEnv;
    DisplayItem* parent;
    ModelItemBase* myModel; //These are forever bound to eachother

    const ValueKeyPath elementPath;

    const ReferenceAttributeSet defaultState;
    std::map<int, const ReferenceAttributeSet*> contributingAttrs;
    RuntimeAttributeSet myAttrs;

    Coords shapePos;
    Dimens totalCellDimens;
    
    std::unique_ptr<Material> myMaterial;

    std::set<DisplayItem*> children;

    std::map<ValueKey, double> variableMap;

    Angle getRelativeAdjacent();
    Angle getRelativeOpposite()
    { return getRelativeAdjacent() == Angle::Horizontal ? Angle::Vertical
                                                        : Angle::Horizontal; }

    Angle correctAngle(const Angle theAngle);

    int getReservedForKey(Angle theAngle, const int adjacentKey, const int oppositeKey);

    std::optional<int> getAlignmentProperty(const Angle theAngle);


public:
    DisplayItem(Environment* theEnv,
                ModelItemBase* theModel,
                const ReferenceAttributeSet& theDefaultState,
                const ValueKeyPath &elemPath,
                const int elementStamp):
        myEnv(theEnv),
        parent(nullptr),
        myModel(theModel),
        elementPath(elemPath),
        elementStamp(elementStamp),
        defaultState(theDefaultState),
        myAttrs(this)
    {
        reprodyne_open_scope(this);
    }

    int getElementStamp() { return elementStamp; }

    //TODO: warn if the variable isn't found. Don't take that shit from noone.
    void setVariable(const ValueKey key, const double val)
    { variableMap[key] = val; }

    double getVariable(const ValueKey key)
    { return variableMap[key]; }

    Environment* getEnv()
    { return myEnv; }

    void addAttributeSet(const AttributePositionPair pair);
    void removeAttributeSet(const AttributePositionPair pair);
    void recomputeAttributeSet();

    RuntimeAttributeSet getAttr()
    { return myAttrs; }

    void setParent(DisplayItem* item);
    void deparent();
    DisplayItem* getParent()
    { return parent; }

    void prepareToDie();

    bool hasParent()
    { return parent; }
    DisplayItem* getRoot();

    ValueKeyPath getElementPath()
    { return elementPath; }

    int getDepth();

    //TODO: Borderssssssssssssssssssss

    int getInnerMargin(Angle theAngle)
    { return getReservedForKey(theAngle, AttributeKey::MarginAdjIn, AttributeKey::MarginOppIn); }

    int getOuterMargin(Angle theAngle)
    { return getReservedForKey(theAngle, AttributeKey::MarginAdjOut, AttributeKey::MarginOppOut); }

    int getInnerPadding(Angle theAngle)
    { return getReservedForKey(theAngle, AttributeKey::PaddingAdjIn, AttributeKey::PaddingOppIn); }

    int getOuterPadding(Angle theAngle)
    { return getReservedForKey(theAngle, AttributeKey::PaddingAdjOut, AttributeKey::PaddingOppOut); }

    int getReservedMargin(Angle theAngle)
    { return getInnerMargin(theAngle) + getOuterMargin(theAngle); }

    int getReservedPadding(Angle theAngle)
    { return getInnerPadding(theAngle) + getOuterPadding(theAngle); }

    int getReservedInner(Angle theAngle)
    { return getInnerMargin(theAngle) + getInnerPadding(theAngle); }

    int getReservedOuter(Angle theAngle)
    { return getOuterMargin(theAngle) + getOuterPadding(theAngle); }

    int getReserved(Angle theAngle)
    { return getReservedInner(theAngle) + getReservedOuter(theAngle); }

    Dimens getReservedInnerMarginDimens()
    { return Dimens(getInnerMargin(getRelativeAdjacent()),
                    getInnerMargin(getRelativeOpposite())); }

    Dimens getReservedInnerPaddingDimens()
    { return Dimens(getInnerPadding(getRelativeAdjacent()),
                    getInnerPadding(getRelativeOpposite())); }

    Dimens getReservedOuterMarginDimens()
    { return Dimens(getOuterMargin(getRelativeAdjacent()),
                    getOuterMargin(getRelativeOpposite())); }

    Dimens getReservedOuterPaddingDimens()
    { return Dimens(getOuterPadding(getRelativeAdjacent()),
                    getOuterPadding(getRelativeOpposite())); }

    Dimens getReservedPaddingDimens()
    { return getReservedInnerPaddingDimens() + getReservedOuterPaddingDimens(); }

    Dimens getReservedInnerDimens()
    { return Dimens(getReservedInner(getRelativeAdjacent()),
                    getReservedInner(getRelativeOpposite())); }

    Dimens getReservedOuterDimens()
    { return Dimens(getReservedOuter(getRelativeAdjacent()),
                    getReservedOuter(getRelativeOpposite())); }

    Dimens getReservedDimens()
    { return getReservedInnerDimens() + getReservedOuterDimens(); }

    Dimens getReservedInnerBorderDimens()
    { return Dimens(); }

    Dimens getReservedOuterBorderDimens()
    { return Dimens(); }

    Dimens getReservedBorderDimens()
    { return getReservedInnerBorderDimens() + getReservedOuterBorderDimens(); }

    GeometryProposal reviseProposalForDrawingArea(GeometryProposal prop);

    bool checkCellAlignmentOffset(const Angle theAngle)
    { return (bool)getAlignmentProperty(theAngle); }

    int getJustificationOffset(const int itemSize, const int cellSize);
    int getCellAlignmentOffset(const Angle theAngle,
                               const int itemSize,
                               const int cellSize,
                               const int reservedInner,
                               const int reservedOuter);

    int getCellAlignmentOffset(const Angle theAngle, const int itemSize, const int cellSize)
    {
        return getCellAlignmentOffset(theAngle,
                                      itemSize,
                                      cellSize,
                                      getReservedInner(theAngle),
                                      getReservedOuter(theAngle));
    }

    Coords getCellAlignmentOffset(const Dimens itemSize,
                                  const Dimens cellSize,
                                  const Dimens reservedInner,
                                  const Dimens reservedOuter)
    {
        auto get = [&](const Angle theAngle)
        {
            return getCellAlignmentOffset(theAngle,
                                          itemSize.get(theAngle),
                                          cellSize.get(theAngle),
                                          reservedInner.get(theAngle),
                                          reservedOuter.get(theAngle));
        };

        return Coords(get(Angle::Horizontal), get(Angle::Vertical));
    }

    Coords getCellAlignmentOffsetMindingReserved(const Dimens itemSize,
                                                 const Dimens cellSize)
    {
        return getCellAlignmentOffset(itemSize,
                                      cellSize,
                                      getReservedInnerDimens(),
                                      getReservedOuterDimens());
    }

    Coords getTranslationOffset();

    int getSizeForAngle(const Angle theAngle);

    std::optional<FillPrecedence> filterFillPrecedenceForAngle(const Angle theAngle);


    //Materials
    Material* getMaterial()
    { return myMaterial.get(); }

    ModelItemBase* getModel() const
    { return myModel; }


    //I wish the reference for the container could be const,
    // without that propogating to the pointers...
    const std::set<DisplayItem*>& getChildren()
    { return children; }

    const int childCount()
    { return children.size(); }

    std::vector<Material*> getChildMaterials();

    std::vector<Material*> getChildMaterialsInModelOrder();

    void setMaterial(Material* theMaterial)
    { myMaterial.reset(theMaterial); }
};


}//IVD

#endif // DISPLAYITEM_H
