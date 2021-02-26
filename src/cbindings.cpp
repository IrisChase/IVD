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

#include "environment.h"
#include "displayitem.h"
#include "specific_driver_sdl/sdldriver.h"

//I've gone most of my life without using reinterpret_cast, and now look at me.
//Look at what I've become.
//I'm sorry uncle.
//I'm so sorry.


static IVD::Environment* castEnv(IVD_Environment* environment)
{ return reinterpret_cast<IVD::Environment*>(environment); }
static IVD::WidgetWrapper* castWidget(IVD_Widget* widget)
{ return reinterpret_cast<IVD::WidgetWrapper*>(widget); }
static IVD_Widget* castWidget(IVD::WidgetWrapper* widget)
{ return reinterpret_cast<IVD_Widget*>(widget); }
static IVD::Dimens* castDimens(IVD_Dimens* space)
{ return reinterpret_cast<IVD::Dimens*>(space); }

static IVD_Dimens* castDimens(IVD::Dimens* space)
{ return reinterpret_cast<IVD_Dimens*>(space); }
static IVD::Coords* castPoint(IVD_Coords* point)
{ return reinterpret_cast<IVD::Coords*>(point); }

static IVD_Coords* castPoint(IVD::Coords* point)
{ return reinterpret_cast<IVD_Coords*>(point); }

static IVD::DisplayItem* cast(IVD_Element* elem)
{ return reinterpret_cast<IVD::DisplayItem*>(elem); }

static const IVD::DisplayItem* cast(const IVD_Element* elem)
{ return reinterpret_cast<const IVD::DisplayItem*>(elem); }


extern "C"
{
#include "user_include/IVD_c.h"


//--------------------------------------------------------------------------------------Environment

IVD_Environment* IVD_create_environment()
{ return reinterpret_cast<IVD_Environment*>(new IVD::Environment()); }
void IVD_destroy_environment(IVD_Environment* environment)
{ delete castEnv(environment); }

int IVD_environment_load_file(IVD_Environment* environment, const char* path)
{
    auto* properEnv = reinterpret_cast<IVD::Environment*>(environment);
    return properEnv->loadFromIVDFile(path);
}
const char* IVD_environment_get_compiler_errors(IVD_Environment* environment)
{
   auto* properEnv = reinterpret_cast<IVD::Environment*>(environment);
   return properEnv->getCompilerErrors();
}
void IVD_environment_run(IVD_Environment* environment)
{
    auto* properEnv = reinterpret_cast<IVD::Environment*>(environment);

    //And this is where we take control! 🎊
    properEnv->run();
}

void IVD_environment_register_widget(IVD_Environment* environment,
                                     const char* name,
                                     IVD_Widget* (*ctor)(IVD_Environment*),
                                     void (*dtor)(IVD_Widget*),
                                     int (*getFillPrecedence)(IVD_Widget*, const int),
                                     void (*shape)(IVD_Widget*, IVD_GeometryProposal*),
                                     void (*draw)(IVD_Widget*, IVD_Canvas*),//canbe null
                                     IVD_Dimens* (*getSpace)(IVD_Widget*),
                                     int (*detectCollisionPoint)(IVD_Widget*, IVD_Coords*), //canbe null
                                     void (*bubbler)(IVD_Widget*),
                                     void (*triggerHandler)(IVD_Widget*, const char*))
{ castEnv(environment)->registerWidgetBlueprints(name, {name, true, ctor, dtor, getFillPrecedence, shape, getSpace, draw, bubbler, detectCollisionPoint, triggerHandler}); }

//IVD manages widget lifetimes so they can be "deleted later"
IVD_Widget* IVD_environment_widget_create(IVD_Environment* environment, const char* name, IVD_Widget* parent)
{ return castEnv(environment)->createWidget(name, parent); }

IVD_Element* IVD_environment_create_element_from_class(IVD_Environment* environment, const char* className, IVD_Widget* parent)
{ return castEnv(environment)->createIVDelementFromClass(className, parent); }

void IVD_environment_widget_destroy(IVD_Environment* environment, IVD_Widget* widget)
{ castEnv(environment)->destroyWidget(widget); }

void IVD_environment_element_destroy(IVD_Environment* environment, IVD_Widget* parent, IVD_Element* elem)
{ castEnv(environment)->destroyIVDelement(parent, elem); }


void IVD_environment_register_layout(IVD_Environment* environment,
                                     const char* name,
                                     IVD_Widget* (*ctor)(IVD_Environment*),
                                     void (*dtor)(IVD_Widget*),
                                     int (*getFillPrecedence)(IVD_Widget*, const int),
                                     void (*shape)(IVD_Widget*, IVD_GeometryProposal*),
                                     void (*draw)(IVD_Widget*, IVD_Canvas*),
                                     IVD_Dimens* (*getSpace)(IVD_Widget*),
                                     void (*bubbler)(IVD_Widget*))
{ castEnv(environment)->registerLayoutBlueprints(name, {name, false, ctor, dtor, getFillPrecedence, shape, getSpace, draw, bubbler, nullptr, nullptr}); }


//----------------------------------------------------------------------------------------------Dust Bindings



IVD_Dimens* IVD_dimens_alloc()
{ return castDimens(new IVD::Dimens()); }

void IVD_dimens_free(IVD_Dimens* space)
{ delete castDimens(space); }

int* IVD_dimens_w(IVD_Dimens* space)
{ return &castDimens(space)->w; }
int* IVD_dimens_h(IVD_Dimens* space)
{ return &castDimens(space)->h; }



IVD_Coords* IVD_coords_alloc()
{ return castPoint(new IVD::Coords()); }

void IVD_coords_free(IVD_Coords* point)
{ delete castPoint(point); }

int* IVD_coords_x(IVD_Coords* point)
{ return &castPoint(point)->x; }
int* IVD_coords_y(IVD_Coords* point)
{ return &castPoint(point)->y; }


static IVD::Rect* castRect(IVD_Rect* rect)
{ return reinterpret_cast<IVD::Rect*>(rect); }

IVD_Rect* IVD_rect_alloc()
{ return reinterpret_cast<IVD_Rect*>(new IVD::Rect()); }

void IVD_rect_free(IVD_Rect* rect)
{ delete castRect(rect); }

IVD_Dimens* IVD_rect_get_dimens(IVD_Rect* rect)
{ return castDimens(&castRect(rect)->d); }
IVD_Coords* IVD_rect_get_coords(IVD_Rect* rect)
{ return castPoint(&castRect(rect)->c); }
void IVD_rect_set_space(IVD_Rect* rect, IVD_Dimens* space)
{ castRect(rect)->d = *castDimens(space); }
void IVD_rect_set_point(IVD_Rect* rect, IVD_Coords* point)
{ castRect(rect)->c = *castPoint(point); }


//-------------------------------------------------------------------------------------------GeometryProposal
static IVD::GeometryProposal* castGeoprop(IVD_GeometryProposal* prop)
{ return reinterpret_cast<IVD::GeometryProposal*>(prop); }

IVD_GeometryProposal* IVD_geoprop_alloc()
{ return reinterpret_cast<IVD_GeometryProposal*>(new IVD::GeometryProposal()); }

void IVD_geoprop_free(IVD_GeometryProposal* prop)
{ delete castGeoprop(prop); }

IVD_Dimens* IVD_geoprop_proposed_space(IVD_GeometryProposal* prop)
{ return castDimens(&castGeoprop(prop)->proposedDimensions); }

int* IVD_geoprop_expand_horizontal(IVD_GeometryProposal* prop)
{ return &castGeoprop(prop)->expandForAngle(IVD::Angle::Horizontal); }

int* IVD_geoprop_expand_vertical(IVD_GeometryProposal* prop)
{ return &castGeoprop(prop)->shrinkForAngle(IVD::Angle::Vertical); }

int* IVD_geoprop_shrink_horizontal(IVD_GeometryProposal* prop)
{ return &castGeoprop(prop)->shrinkForAngle(IVD::Angle::Horizontal); }

int* IVD_geoprop_shrink_vertical(IVD_GeometryProposal* prop)
{ return &castGeoprop(prop)->shrinkForAngle(IVD::Angle::Vertical); }

int IVD_geoprop_verify_compliance(IVD_GeometryProposal* prop, IVD_Dimens* space)
{ return castGeoprop(prop)->verifyCompliance(*castDimens(space)); }

void IVD_geoprop_round_conflicts(IVD_GeometryProposal* prop, IVD_Dimens* space)
{ *castDimens(space) = castGeoprop(prop)->roundConflicts(*castDimens(space)); }

//-----------------------------------------------------------------------------------------------------Widget
IVD_Dimens* IVD_element_get_dimens(const IVD_Element* elem)
{
    thread_local static IVD::Dimens dimens;
    dimens = cast(elem)->getViewportDimens();
    return castDimens(&dimens);
}

int IVD_element_get_fill_precedence(IVD_Element* elem, int angle)
{
    const auto fillPrec =
            cast(elem)->computerFillPrecedenceForAngle(static_cast<IVD::Angle>(angle));
    return static_cast<int>(fillPrec);
}

void IVD_element_shape(IVD_Element* elem, IVD_GeometryProposal* prop)
{ cast(elem)->shape(prop); }

void IVD_element_set_offset(IVD_Element* elem, IVD_Coords* coords)
{ cast(elem)->setOffset(coords); }

void IVD_element_draw(IVD_Element* elem)
{ cast(elem)->render(); }

void IVD_element_bubble(IVD_Element* elem)
{ cast(elem)->updateHover(); }

IVD_Element* IVD_widget_get_underlying_element(IVD_Environment* environment, IVD_Widget* widget)
{ return reinterpret_cast<IVD_Element*>(castEnv(environment)->getUnderlyingDisplayItemForWidget(widget)); }

//On requiring widget instead of parent element here.
// ehhhh it doesn't make a whole lot of difference.
//But this does kind of prevent you from going deeper into the
// tree than one level (you can't recursively get child elements)
// which I kind of like.
void IVD_widget_get_child_elements(IVD_Environment* environment, IVD_Widget* widget, IVD_Element*** result, int* size)
{
    thread_local static std::vector<IVD_Element*> resultContainer;
    IVD::DisplayItem* item = castEnv(environment)->getUnderlyingDisplayItemForWidget(widget);

    resultContainer = item->getChildWidgetInStampOrder();

    *size = resultContainer.size();
    *result = &resultContainer[0];
}

IVD_Element* IVD_widget_get_child_element_for_named_cell(IVD_Environment* environment, IVD_Widget* parent, const char* name)
{
    IVD::DisplayItem* item = castEnv(environment)->getUnderlyingDisplayItemForWidget(parent);
    return item->getChildElementForNamedCell(name);
}
//--------------------Accessors

void IVD_canvas_draw_image(IVD_Canvas* canvas,
                           int x,
                           int y,
                           int width,
                           int height,
                           int stride,
                           int channels,
                           unsigned char* data)
{
    IVD::Canvas* myCanvas = reinterpret_cast<IVD::Canvas*>(canvas);

    //absolute drawing offset is already set by the
    // owning DisplayItem, the x,y here are relative
    myCanvas->drawBitmapRGBoptionalA(IVD::Coords(x,y),
                                     stride,
                                     width,
                                     height,
                                     channels,
                                     data);
}

}//extern "C"
