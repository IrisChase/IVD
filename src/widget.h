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
class Environment;


//Layouts are just a specialized type of widget

struct WidgetBlueprints
{
    //Layout/widget uses these
    std::string name;
    bool isWidget;
    IVD_Widget* (*ctor)(IVD_Environment*) = nullptr;
    void (*dtor)(IVD_Widget*) = nullptr;
    int (*getFillPrecedence)(IVD_Widget*, const int) = nullptr;
    void (*shape)(IVD_Widget*, IVD_GeometryProposal*) = nullptr;
    IVD_Dimens* (*getSpace)(IVD_Widget*); //Widgets only know about drawing area
    void (*draw)(IVD_Widget*, IVD_Canvas*) = nullptr; //canbe null
    void (*bubbler)(IVD_Widget*) = nullptr;

    //Widget specific
    int (*detectCollisionPoint)(IVD_Widget*, IVD_Coords*) = nullptr; //canbe null
    void (*triggerHandler)(IVD_Widget*, const char*) = nullptr;
};

class WidgetWrapper
{
    typedef std::unique_ptr<IVD_Widget, void(*)(IVD_Widget*)> SmartWidgetPointer;
    WidgetBlueprints myBlueprints;
    SmartWidgetPointer underlyingWidget;

    bool isSet = false;

public:
    WidgetWrapper(): underlyingWidget(nullptr, nullptr) {}
    WidgetWrapper(Environment* theEnv, const WidgetBlueprints blueprints):
        myBlueprints(blueprints),
        underlyingWidget(blueprints.ctor(reinterpret_cast<IVD_Environment*>(theEnv)), blueprints.dtor),
        isSet(true)
    {}

    void reset(Environment* theEnv, const WidgetBlueprints blueprints)
    {
        underlyingWidget.reset();
        underlyingWidget = SmartWidgetPointer(blueprints.ctor(reinterpret_cast<IVD_Environment*>(theEnv)), blueprints.dtor);
        myBlueprints = blueprints;
    }

    bool checkIsSet()
    { return bool(underlyingWidget); }

    bool isDrawable()
    { return myBlueprints.draw; }

    bool isLayout()
    { return !myBlueprints.isWidget; }

    void destroy()
    { underlyingWidget.reset(); }

    IVD_Widget* get()
    { return underlyingWidget.get(); }

    FillPrecedence getFillPrecedence(Angle theAngel)
    {
        const auto prec = myBlueprints.getFillPrecedence(get(),
                                                       getForAngle(0, 1, theAngel));
        return prec == 0 ? FillPrecedence::Greedy
                         : FillPrecedence::Shrinky;
    }

    Dimens getSpace()
    {
        IVD_Dimens* space = myBlueprints.getSpace(underlyingWidget.get());
        Dimens result = *reinterpret_cast<Dimens*>(space);
        IVD_dimens_free(space);
        return result;
    }

    void shape(GeometryProposal prop)
    { myBlueprints.shape(get(), reinterpret_cast<IVD_GeometryProposal*>(&prop)); }

    void draw(Canvas* canvas)
    { myBlueprints.draw(get(), reinterpret_cast<IVD_Canvas*>(canvas)); }

    void bubble()
    { return myBlueprints.bubbler(get()); }

    bool detectCollisionPoint(Coords point)
    {
        if(!myBlueprints.detectCollisionPoint)
            return false;
        return myBlueprints.detectCollisionPoint(get(), reinterpret_cast<IVD_Coords*>(&point));
    }

    void handleTrigger(const std::string triggerName)
    { myBlueprints.triggerHandler(get(), triggerName.c_str()); }
};


}//IVD
