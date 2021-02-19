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

#pragma once

#include <map>
#include <string>
#include <memory>
#include <iostream>
#include <functional>

extern "C"
{
#include "IVD_c.h"
}

namespace IVD
{

enum class FillPrecedence
{
    Greedy,
    Shrinky,
};

#define IVD_ANGLE_HORIZONTAL 0
#define IVD_ANGLE_VERTICAL 1

enum class Angle
{
    Horizontal,
    Vertical,
};

class GeometryProposal
{
public:
    GeometryProposal(IVD_GeometryProposal* internalProp)
    {}
};

class Canvas
{
public:
    Canvas(IVD_Canvas* internalCanvas)
    {}
};

struct Space
{
    int w;
    int h;
};

struct Point
{
    int x;
    int y;
};

class UserLayout
{
public:
    virtual FillPrecedence getFillPrecedence(const Angle angle) = 0;
    virtual void shape(const GeometryProposal prop) = 0;
    virtual Space getSpace() = 0;
    virtual void draw(Canvas theCanvas) = 0;
    virtual bool detectCollisionPoint(const Point point) { return false; }
};

class UserWidget : public UserLayout
{
public:
    virtual ~UserWidget();

    virtual void trigger(const std::string trig) {}
};

namespace Internals
{

IVD_Widget* cast(UserWidget* widget)
{ return reinterpret_cast<IVD_Widget*>(widget); }

UserWidget* cast(IVD_Widget* widget)
{ return reinterpret_cast<UserWidget*>(widget); }

void userWidgetDestructorHook(IVD_Widget* widget)
{ delete cast(widget); }

int userWidgetGetFillPrecedenceHook(IVD_Widget* widget, const int angle)
{
    const Angle inputAngle =
            angle == IVD_ANGLE_VERTICAL ? Angle::Vertical
                                        : Angle::Horizontal;

    const FillPrecedence resultPrec = cast(widget)->getFillPrecedence(inputAngle);

    return resultPrec == FillPrecedence::Greedy ? IVD_FILL_PRECEDENCE_GREEDY
                                                : IVD_FILL_PRECEDENCE_SHRINKY;
}

void userWidgetShapeHook(IVD_Widget* widget, IVD_GeometryProposal* prop)
{ cast(widget)->shape(GeometryProposal(prop)); }

void userWidgetDrawHook(IVD_Widget* widget, IVD_Canvas* canvas)
{ cast(widget)->draw(Canvas(canvas)); }

IVD_Space* userWidgetGetSpaceHook(IVD_Widget* widget)
{
    const Space space = cast(widget)->getSpace();
    //Alloc AFTER in case getSpace throws, although it should be in a try block here anyway...
    // TODO XXX

    IVD_Space* result = IVD_space_alloc();
    *IVD_space_h(result) = space.h;
    *IVD_space_w(result) = space.w;
    return result;
}

int userWidgetDetectCollisionPointHook(IVD_Widget* widget, IVD_Point* point)
{
    const Point p{*IVD_point_x(point), *IVD_point_y(point)};
    return cast(widget)->detectCollisionPoint(p);
}

void userWidgetTriggerHook(IVD_Widget* widget, const char* trig)
{ cast(widget)->trigger(trig); }

}//Internals

template<typename T>
class ManagedUserWidget
{
    friend class Environment; //So we can have a protected constructor
    std::function<void(T*)> destroyLaterBinding;

    T* myWigee;

    ManagedUserWidget(T* theWidget,
                      std::function<void()> dtorBinding):
        myWigee(theWidget),
        destroyLaterBinding(dtorBinding)
    {}

public:
    ~ManagedUserWidget()
    { destroyLaterBinding(); }

    T* get()
    { return myWigee; }
};

class Environment
{
    std::unique_ptr<IVD_Environment, void(*)(IVD_Environment*)> internal;

public:
    Environment(): internal(IVD_create_environment(), &IVD_destroy_environment) {}

    template<typename T>
    void register_widget(const std::string name, T* (*factory)())
    {
        //---->VOODOO<----
        auto castFactory = reinterpret_cast<IVD_Widget* (*)()>(factory);
        //---->VOODOO<----

        IVD_environment_register_widget(internal.get(),
                                        name.c_str(),
                                        castFactory,
                                        Internals::userWidgetDestructorHook,
                                        Internals::userWidgetGetFillPrecedenceHook,
                                        Internals::userWidgetShapeHook,
                                        Internals::userWidgetDrawHook,
                                        Internals::userWidgetGetSpaceHook,
                                        Internals::userWidgetDetectCollisionPointHook,
                                        Internals::userWidgetTriggerHook);
    }

    template<typename T>
    void register_layout(const std::string name, T* (*factory)())
    {
        //---->VOODOO<----
        auto castFactory = reinterpret_cast<IVD_Widget* (*)()>(factory);
        //---->VOODOO<----

        IVD_environment_register_layout(internal.get(),
                                        name.c_str(),
                                        castFactory,
                                        Internals::userWidgetDestructorHook,
                                        Internals::userWidgetGetFillPrecedenceHook,
                                        Internals::userWidgetShapeHook,
                                        Internals::userWidgetDrawHook,
                                        Internals::userWidgetGetSpaceHook,
                                        Internals::userWidgetDetectCollisionPointHook);
    }

    template<typename T>
    T* create_unmanaged_user_widget(const std::string name, T* parent = nullptr)
    {
        IVD_Widget* widget = IVD_environment_widget_create(internal.get(),
                                                           name.c_str(),
                                                           reinterpret_cast<IVD_Widget*>(parent));
        return reinterpret_cast<T*>(widget);
    }


    template<typename T>
    ManagedUserWidget<T> create_managed_user_widget(const std::string name, T* parent = nullptr)
    {
        T* unmanaged = create_unmanaged_user_widget(name, parent);

        std::function<void()> dtor = [&]
        { IVD_environment_widget_destroy(internal.get(), unmanaged); };

        return ManagedUserWidget<T>(unmanaged, dtor);
    }

    int load_IVD_from_file(const std::string& path)
    { return IVD_environment_load_file(internal.get(), path.c_str()); }

    std::string get_compiler_errors()
    { return std::string(IVD_environment_get_compiler_errors(internal.get())); }

    void run()
    { IVD_environment_run(internal.get()); }
};



}//IVD
