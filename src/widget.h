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
    IVD_Widget* (*ctor)();
    void (*dtor)(IVD_Widget*);
    int (*getFillPrecedence)(IVD_Widget*, const int);
    void (*shape)(IVD_Widget*, const IVD_GeometryProposal*);

    //Widget specific
    void (*draw)(IVD_Widget*, IVD_Canvas*); //canbe null
    int (*detectCollisionPoint)(IVD_Widget*, const IVD_Point*); //canbe null
    void (*triggerHandler)(IVD_Widget*, const char*);
};

class Widget
{
    const WidgetBlueprints& blueprints;
    std::unique_ptr<IVD_Widget, void(*)(IVD_Widget*)> underlyingWidget;

public:
    Widget(const WidgetBlueprints& blueprints):
        blueprints(blueprints),
        underlyingWidget(blueprints.ctor(), blueprints.dtor)
    {}

    FillPrecedence getFillPrecedence(Angle theAngel)
    {
        const auto prec = blueprints.getFillPrecedence(underlyingWidget.get(),
                                                       getForAngle(0, 1, theAngel));
        return prec == 0 ? FillPrecedence::Greedy
                         : FillPrecedence::Shrinky;
    }

    void shape(const GeometryProposal prop)
    {
        blueprints.shape(underlyingWidget.get(),
                         reinterpret_cast<const IVD_GeometryProposal*>(&prop));
    }

    void draw(Canvas* canvas)
    {
        blueprints.draw(underlyingWidget.get(),
                        reinterpret_cast<IVD_Canvas*>(canvas));
    }

    bool detectCollisionPoint(const Coords point)
    {
        return blueprints.detectCollisionPoint(underlyingWidget.get(),
                                               reinterpret_cast<const IVD_Point*>(&point));
    }

    void handleTrigger(const std::string triggerName)
    {
        blueprints.triggerHandler(underlyingWidget.get(),
                                  triggerName.c_str());
    }
};


}//IVD
