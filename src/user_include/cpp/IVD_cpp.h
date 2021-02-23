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

#pragma once

#include <map>
#include <string>
#include <memory>
#include <iostream>
#include <functional>

#include "IVD_geometry_proposal.h"

namespace IVD
{

class UserWidget;
class UserLayout;

namespace Internals //reference at your own peril
{

inline IVD_Widget* cast(UserLayout* widget)
{ return reinterpret_cast<IVD_Widget*>(widget); }

inline IVD_Widget* cast(UserWidget* widget)
{ return reinterpret_cast<IVD_Widget*>(widget); }

inline UserWidget* cast(IVD_Widget* widget)
{ return reinterpret_cast<UserWidget*>(widget); }


}//Internals

class Canvas;
class Element
{
    IVD_Element* elem;

public:
    Element(IVD_Element* elem): elem(elem) {}

    IVD_Element* getUnderlyinggggg()
    { return elem; }

    void render()
    { IVD_element_draw(elem); }

    void set_offset(const Coords offset)
    {
        std::unique_ptr<IVD_Coords, void(*)(IVD_Coords*)>
                managedCoords(IVD_coords_alloc(), IVD_coords_free);
        *IVD_coords_x(managedCoords.get()) = offset.x;
        *IVD_coords_y(managedCoords.get()) = offset.y;
        IVD_element_set_offset(elem, managedCoords.get());
    }

    void bubble()
    { IVD_element_bubble(elem); }

    FillPrecedence get_fill_precedence(const Angle theAngle)
    {
        return static_cast<FillPrecedence>
                (IVD_element_get_fill_precedence(elem, static_cast<int>(theAngle)));
    }

    Dimens get_dimens()
    { return IVD_element_get_dimens(elem); }

    void shape(GeometryProposal prop)
    {
        auto smartProp = prop.createSmartPointerProp();
        IVD_element_shape(elem, smartProp.get());
    }
};

class UserLayout
{
    friend class Environment;
    IVD_Environment* myEnv = nullptr;

protected:
    Dimens myDimens;

    void destroy_child_element(Element elem)
    {
        IVD_environment_element_destroy(myEnv,
                                        Internals::cast(this),
                                        elem.getUnderlyinggggg());
    }

    void render_children_unordered()
    {
        applyToChildren([&](Element child)
        { child.render(); });
    }

    void bubble_children_unordered()
    {
        applyToChildren([&](Element child)
        { child.bubble(); });
    }

    FillPrecedence any_greedy_children_for_angle(const Angle theAngle)
    {
        auto result = FillPrecedence::Shrinky;
        applyToChildren([&](Element child)
        {
            if(child.get_fill_precedence(theAngle) == FillPrecedence::Greedy)
                result = FillPrecedence::Greedy;
        });

        return result;
    }

    void applyToChildren(std::function<void(Element)> fun)
    {
        IVD_Element** childArray = nullptr;
        int childArraySize = 0;

        IVD_widget_get_child_elements(myEnv,
                                      Internals::cast(this),
                                      &childArray,
                                      &childArraySize);

        for(int i = 0; i != childArraySize; ++i)
            fun(Element(childArray[i]));
    }

    UserLayout* getChildForNamedCell(const std::string name)
    {
        return reinterpret_cast<UserLayout*>
                (IVD_widget_get_child_element_for_named_cell(myEnv,
                                                     Internals::cast(this),
                                                     name.c_str()));
    }

public:
    virtual ~UserLayout() {}
    Dimens get_content_dimens() { return myDimens; }

    virtual FillPrecedence get_fill_precedence(const Angle angle) = 0;
    virtual void shape(const GeometryProposal officialProposal) = 0;

    //canvas will be null for layouts;
    virtual void draw(Canvas theCanvas) = 0;

    //This just defines collision order
    //Call "process_collision_point_for_child
    // for all children
    virtual void bubble_children() {}
};

class UserWidget : public UserLayout
{
public:
    virtual ~UserWidget() {}

    virtual bool detect_collision_point(const Coords point) { return false; }
    virtual void trigger(const std::string trig) {}
};

template<typename T>
class ManagedUserWidget
{
    friend class Environment; //So we can have a protected constructor
    std::function<void()> destroyLaterBinding;

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

class Canvas
{
    //Just a thin wrapper, it's owned deep down inside the core of
    // IVD
    IVD_Canvas* internalCanvas;

public:
    Canvas(IVD_Canvas* internalCanvas):
        internalCanvas(internalCanvas)
    {}


    virtual void drawBitmapRGBoptionalA(Coords dest,
                                        int width,
                                        int height,
                                        int stride,
                                        int channels,
                                        unsigned char* data)
    {
        IVD_canvas_draw_image(internalCanvas,
                              dest.x,
                              dest.y,
                              width,
                              height,
                              stride,
                              channels,
                              data);
    }

    //Image and font are the only things that aren't handled by
    // the regular thangs
};


namespace Internals
{

inline void userWidgetDestructorHook(IVD_Widget* widget)
{ delete cast(widget); }

inline int userWidgetGetFillPrecedenceHook(IVD_Widget* widget, const int angle)
{
    const auto inputAngle = static_cast<Angle>(angle);
    return static_cast<int>(cast(widget)->get_fill_precedence(inputAngle));
}

inline void userWidgetShapeHook(IVD_Widget* widget, IVD_GeometryProposal* prop)
{ cast(widget)->shape(GeometryProposal(prop)); }

inline void userWidgetDrawHook(IVD_Widget* widget, IVD_Canvas* canvas)
{ cast(widget)->draw(Canvas(canvas)); }

inline IVD_Dimens* userWidgetGetDimensHook(IVD_Widget* widget)
{
    const Dimens dimens = cast(widget)->get_content_dimens();
    //Alloc AFTER in case getDimens throws, although it should be in a try block here anyway...
    // TODO XXX

    IVD_Dimens* result = IVD_dimens_alloc();
    *IVD_dimens_h(result) = dimens.h;
    *IVD_dimens_w(result) = dimens.w;
    return result;
}

inline void userWidgetBubble(IVD_Widget* widget)
{ cast(widget)->bubble_children(); }
inline int userWidgetDetectCollisionCoordsHook(IVD_Widget* widget, IVD_Coords* coords)
{ return cast(widget)->detect_collision_point(Coords(coords)); }

inline void userWidgetTriggerHook(IVD_Widget* widget, const char* trig)
{ cast(widget)->trigger(trig); }

template<typename T>
T* generic_factory()
{
    return new T;
}

}//Internals


class Environment
{
    std::unique_ptr<IVD_Environment, void(*)(IVD_Environment*)> internal;

public:
    Environment(): internal(IVD_create_environment(), &IVD_destroy_environment) {}

    template<typename T>
    void register_widget(const std::string name)
    {
        T* (*factory)() = Internals::generic_factory<T>; //WE CAN DO THAT???

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
                                        Internals::userWidgetGetDimensHook,
                                        Internals::userWidgetDetectCollisionCoordsHook,
                                        Internals::userWidgetBubble,
                                        Internals::userWidgetTriggerHook);
    }

    template<typename T>
    void register_layout(const std::string name)
    {
        T* (*factory)() = Internals::generic_factory<T>;
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
                                        Internals::userWidgetGetDimensHook,
                                        Internals::userWidgetBubble);
    }

    template<typename T>
    T* create_unmanaged_user_widget(const std::string name, UserLayout* parent = nullptr)
    {
        IVD_Widget* widget = IVD_environment_widget_create(internal.get(),
                                                           name.c_str(),
                                                           reinterpret_cast<IVD_Widget*>(parent));
        auto result = reinterpret_cast<T*>(widget);
        result->myEnv = internal.get();
        return result;
    }


    template<typename T>
    ManagedUserWidget<T> create_managed_user_widget(const std::string name, UserLayout* parent = nullptr)
    {
        T* unmanaged = create_unmanaged_user_widget<T>(name, parent);

        std::function<void()> dtor = [&]
        { IVD_environment_widget_destroy(internal.get(), unmanaged); };

        return ManagedUserWidget<T>(unmanaged, dtor);
    }

    Element create_unmanaged_widget_from_class(const std::string className, UserLayout* parent = nullptr)
    {
        return IVD_environment_create_element_from_class(internal.get(),
                                                        className.c_str(),
                                                        reinterpret_cast<IVD_Widget*>(parent));
    }

    int load_IVD_from_file(const std::string& path)
    { return IVD_environment_load_file(internal.get(), path.c_str()); }

    std::string get_compiler_errors()
    { return std::string(IVD_environment_get_compiler_errors(internal.get())); }

    void run()
    { IVD_environment_run(internal.get()); }
};



}//IVD
