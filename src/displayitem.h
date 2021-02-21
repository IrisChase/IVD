//Copyright 2021 Iris Chase
//
//Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#pragma once

#include <set>
#include <map>

#include "attributepositionpair.h"
#include "runtimeattributeset.h"
#include "widget.h"

#include <reprodyne.h>

namespace IVD
{

class Environment;
class ModelContainer;
class StateManager;
class Canvas;


class DisplayItem
{
    const int elementStamp;
    Element* myElem;
    Environment* myEnv;
    DisplayItem* parent;

    const ValueKeyPath elementPath;

    const ReferenceAttributeSet defaultState;
    std::map<int, const ReferenceAttributeSet*> contributingAttrs;
    RuntimeAttributeSet myAttrs;

    //Cell is the actual box we want, adjusted for margins?
    //Viewport is the space we have to position in.
    bool compliantGeometry = false;
    Rect myCellRect;
    Rect myViewportRect; //We need the viewport for the clip

    WidgetWrapper myWidget;

    std::set<DisplayItem*> children;

    std::map<ValueKey, double> variableMap;

    Angle getRelativeAdjacent();
    Angle getRelativeOpposite()
    { return getRelativeAdjacent() == Angle::Horizontal ? Angle::Vertical
                                                        : Angle::Horizontal; }

    Angle correctAngle(const Angle theAngle);

    int getReservedForKey(Angle theAngle, const int adjacentKey, const int oppositeKey);

    std::optional<int> getAlignmentProperty(const Angle theAngle);


    //putting this here cause I wanna save it but don't know where to put
    // it yet. Probably gonna be placed beyond the C boundary as a helper
    // eventually.
    FillPrecedence returnGreedyIFEVENONECHILDBLINKS(const Angle theAngle);

public:
    DisplayItem(Element* elem,
                Environment* theEnv,
                const ReferenceAttributeSet& theDefaultState,
                const ValueKeyPath &elemPath,
                const int elementStamp):
        myElem(elem),
        myEnv(theEnv),
        parent(nullptr),
        elementPath(elemPath),
        elementStamp(elementStamp),
        defaultState(theDefaultState),
        myAttrs(this)
    {
        reprodyne_open_scope(this);
    }

    Element* getElement()
    { return myElem; }

    void destroyWidget()
    { myWidget.destroy(); }

    IVD_Widget* setupNewWidget(const WidgetBlueprints blueprints)
    {
        myWidget.reset(blueprints);
        return myWidget.get();
    }

    IVD_Widget* getWidget()
    { return myWidget.get(); }

    void reactToTrigger(const std::string trigg)
    { myWidget.handleTrigger(trigg); }

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

    RuntimeAttributeSet& getAttr()
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

    Dimens getReservedMarginDimens()
    { return getReservedInnerMarginDimens() + getReservedOuterMarginDimens(); }

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

    GeometryProposal reviseProposalAsRequired(GeometryProposal prop);

    bool checkCellAlignmentOffset(const Angle theAngle)
    { return (bool)getAlignmentProperty(theAngle); }

    int getJustificationOffset(const int itemSize, const int cellSize);
    int getCellAlignmentOffset(const Angle theAngle,
                               const int cellSize,
                               const int viewportSize,
                               const int reservedInner,
                               const int reservedOuter);

    int getCellAlignmentOffset(const Angle theAngle, const int cellSize, const int viewportSize)
    {
        return getCellAlignmentOffset(theAngle,
                                      cellSize,
                                      viewportSize,
                                      getReservedInner(theAngle),
                                      getReservedOuter(theAngle));
    }

    Coords getCellAlignmentOffset(const Dimens cellSize,
                                  const Dimens viewportSize,
                                  const Dimens reservedInner,
                                  const Dimens reservedOuter)
    {
        auto get = [&](const Angle theAngle)
        {
            return getCellAlignmentOffset(theAngle,
                                          cellSize.get(theAngle),
                                          viewportSize.get(theAngle),
                                          reservedInner.get(theAngle),
                                          reservedOuter.get(theAngle));
        };

        return Coords(get(Angle::Horizontal), get(Angle::Vertical));
    }

    Coords getCellAlignmentOffsetMindingReserved(const Dimens cellSize,
                                                 const Dimens viewportSize)
    {
        return getCellAlignmentOffset(cellSize,
                                      viewportSize,
                                      getReservedInnerDimens(),
                                      getReservedOuterDimens());
    }

    Coords getTranslationOffset();

    int getSizeForAngle(const Angle theAngle);

    std::optional<FillPrecedence> filterFillPrecedenceForAngle(const Angle theAngle);

    FillPrecedence getFillPrecedenceForAngle(const Angle theAngle);

    Rect getViewport()
    { return myViewportRect; }

    Rect getCell()
    { return myCellRect; }

    FillPrecedence computerFillPrecedenceForAngle(const Angle theAngle);
    void shape(const GeometryProposal officialProposal);
    Dimens shapeDrawingArea(const GeometryProposal officalProposal);

    void setOffset(const Coords offset)
    { myViewportRect.c = offset; }

    void render(Canvas* theCanvas, const Coords offset);

    void updateHover(StateManager* theStateManager, const Coords point);

    //I wish the reference for the container could be const,
    // without that propogating to the pointers...
    const std::set<DisplayItem*>& getChildren()
    { return children; }

    std::vector<IVD_Widget*> getChildWidgetInStampOrder();

    IVD_Widget* getChildWidgetForNamedCell(const std::string name);


    const int childCount()
    { return children.size(); }
};


}//IVD
