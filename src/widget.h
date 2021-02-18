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
    typedef std::unique_ptr<IVD_Widget, void(*)(IVD_Widget*)> SmartWidgetPointer;
    WidgetBlueprints blueprints;
    SmartWidgetPointer underlyingWidget;

public:
    WidgetWrapper(): underlyingWidget(nullptr, nullptr) {}
    WidgetWrapper(const WidgetBlueprints blueprints):
        blueprints(blueprints),
        underlyingWidget(blueprints.ctor(), blueprints.dtor)
    {}

    void reset(const WidgetBlueprints blueprints)
    {
        underlyingWidget.reset();
        underlyingWidget = SmartWidgetPointer(blueprints.ctor(), blueprints.dtor);
    }

    void destroy()
    { underlyingWidget.reset(); }

    IVD_Widget* get()
    { return underlyingWidget.get(); }

    FillPrecedence getFillPrecedence(Angle theAngel)
    {
        const auto prec = blueprints.getFillPrecedence(get(),
                                                       getForAngle(0, 1, theAngel));
        return prec == 0 ? FillPrecedence::Greedy
                         : FillPrecedence::Shrinky;
    }

    void shape(const GeometryProposal prop)
    {
        blueprints.shape(get(),
                         reinterpret_cast<const IVD_GeometryProposal*>(&prop));
    }

    void draw(Canvas* canvas)
    {
        blueprints.draw(get(),
                        reinterpret_cast<IVD_Canvas*>(canvas));
    }

    bool detectCollisionPoint(const Coords point)
    {
        return blueprints.detectCollisionPoint(get(),
                                               reinterpret_cast<const IVD_Point*>(&point));
    }

    void handleTrigger(const std::string triggerName)
    {
        blueprints.triggerHandler(get(),
                                  triggerName.c_str());
    }
};


}//IVD
