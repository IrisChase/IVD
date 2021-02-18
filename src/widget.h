#pragma once

extern "C"
{
#include "user_include/IVD_c.h"
}

#include <memory>
#include "geometry.h"
#include "geometryproposal.h"

namespace IVD
{

class Canvas;



//Layouts are just a specialized type of widget

struct WidgetBlueprints
{
    //Layout/widget uses these
    std::string name;
    bool isWidget;
    IVD_Widget* (*ctor)() = nullptr;
    void (*dtor)(IVD_Widget*) = nullptr;
    int (*getFillPrecedence)(IVD_Widget*, const int) = nullptr;
    void (*shape)(IVD_Widget*, const IVD_GeometryProposal*) = nullptr;

    //Widget specific
    void (*draw)(IVD_Widget*, IVD_Canvas*) = nullptr; //canbe null
    int (*detectCollisionPoint)(IVD_Widget*, const IVD_Point*) = nullptr; //canbe null
    void (*triggerHandler)(IVD_Widget*, const char*) = nullptr;
};

class WidgetWrapper
{
    WidgetBlueprints blueprints;
    IVD_Widget* underlyingWidget = nullptr;

public:
    WidgetWrapper() {}
    WidgetWrapper(const WidgetBlueprints& blueprints, IVD_Widget* underlyingData):
        blueprints(blueprints),
        underlyingWidget(underlyingData)
    {}

    IVD_Widget* get()
    { return underlyingWidget; }

    FillPrecedence getFillPrecedence(Angle theAngel)
    {
        const auto prec = blueprints.getFillPrecedence(underlyingWidget,
                                                       getForAngle(0, 1, theAngel));
        return prec == 0 ? FillPrecedence::Greedy
                         : FillPrecedence::Shrinky;
    }

    void shape(const GeometryProposal prop)
    {
        blueprints.shape(underlyingWidget,
                         reinterpret_cast<const IVD_GeometryProposal*>(&prop));
    }

    void draw(Canvas* canvas)
    {
        blueprints.draw(underlyingWidget,
                        reinterpret_cast<IVD_Canvas*>(canvas));
    }

    bool detectCollisionPoint(const Coords point)
    {
        return blueprints.detectCollisionPoint(underlyingWidget,
                                               reinterpret_cast<const IVD_Point*>(&point));
    }

    void handleTrigger(const std::string triggerName)
    {
        blueprints.triggerHandler(underlyingWidget,
                                  triggerName.c_str());
    }
};


}//IVD
