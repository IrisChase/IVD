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
#include "specific_driver_sdl/sdldriver.h"

//I've gone most of my life without using reinterpret_cast, and now look at me.
//Look at what I've become.
//I'm sorry uncle.
//I'm so sorry.


extern "C"
{
#include "user_include/IVD_c.h"


//--------------------------------------------------------------------------------------Environment
static IVD::Environment* castEnv(IVD_Environment* environment)
{ return reinterpret_cast<IVD::Environment*>(environment); }
static IVD::WidgetWrapper* castWidget(IVD_Widget* widget)
{ return reinterpret_cast<IVD::WidgetWrapper*>(widget); }
static IVD_Widget* castWidget(IVD::WidgetWrapper* widget)
{ return reinterpret_cast<IVD_Widget*>(widget); }

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
                                     IVD_Widget* (*ctor)(),
                                     void (*dtor)(IVD_Widget*),
                                     int (*getFillPrecedence)(IVD_Widget*, const int),
                                     void (*shape)(IVD_Widget*, IVD_GeometryProposal*),
                                     void (*draw)(IVD_Widget*, IVD_Canvas*),//canbe null
                                     IVD_Dimens* (*getSpace)(IVD_Widget*),
                                     int (*detectCollisionPoint)(IVD_Widget*, IVD_Coords*), //canbe null
                                     void (*distributeCollisionPoint)(IVD_Widget*, IVD_Coords*),
                                     void (*triggerHandler)(IVD_Widget*, const char*))
{ castEnv(environment)->registerWidgetBlueprints(name, {name, true, ctor, dtor, getFillPrecedence, shape, getSpace, draw, distributeCollisionPoint, detectCollisionPoint, triggerHandler}); }

//IVD manages widget lifetimes so they can be "deleted later"
IVD_Widget* IVD_environment_widget_create(IVD_Environment* environment, const char* name, IVD_Widget* parent)
{ return castEnv(environment)->createWidget(name, parent); }
void IVD_environment_widget_destroy(IVD_Environment* environment, IVD_Widget* widget)
{ castEnv(environment)->destroyWidget(widget); }


void IVD_environment_register_layout(IVD_Environment* environment,
                                     const char* name,
                                     IVD_Widget* (*ctor)(),
                                     void (*dtor)(IVD_Widget*),
                                     int (*getFillPrecedence)(IVD_Widget*, const int),
                                     void (*shape)(IVD_Widget*, IVD_GeometryProposal*),
                                     void (*draw)(IVD_Widget*, IVD_Canvas*),
                                     IVD_Dimens* (*getSpace)(IVD_Widget *),
                                     void (*distributeCollisionPoint)(IVD_Widget*, IVD_Coords*))
{ castEnv(environment)->registerLayoutBlueprints(name, {name, false, ctor, dtor, getFillPrecedence, shape, getSpace, draw, distributeCollisionPoint, nullptr, nullptr}); }


//Register multiple types?
void IVD_environment_register_layout_attribute(IVD_Environment*,
                                               const char* layoutName,
                                               const char* attributeKey,
                                               int attributeType);

void IVD_environment_register_widget_attribute(IVD_Environment*,
                                               const char* widgetName,
                                               const char* attributeKey,
                                               int attributeType);
//----------------------------------------------------------------------------------------------Dust Bindings


static IVD::Dimens* castSpace(IVD_Dimens* space)
{ return reinterpret_cast<IVD::Dimens*>(space); }

static IVD_Dimens* castSpace(IVD::Dimens* space)
{ return reinterpret_cast<IVD_Dimens*>(space); }

IVD_Dimens* IVD_dimens_alloc()
{ return castSpace(new IVD::Dimens()); }

void IVD_dimens_free(IVD_Dimens* space)
{ delete castSpace(space); }

int* IVD_dimens_w(IVD_Dimens* space)
{ return &castSpace(space)->w; }
int* IVD_dimens_h(IVD_Dimens* space)
{ return &castSpace(space)->h; }


static IVD::Coords* castPoint(IVD_Coords* point)
{ return reinterpret_cast<IVD::Coords*>(point); }

static IVD_Coords* castPoint(IVD::Coords* point)
{ return reinterpret_cast<IVD_Coords*>(point); }

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
{ return castSpace(&castRect(rect)->d); }
IVD_Coords* IVD_rect_get_coords(IVD_Rect* rect)
{ return castPoint(&castRect(rect)->c); }
void IVD_rect_set_space(IVD_Rect* rect, IVD_Dimens* space)
{ castRect(rect)->d = *castSpace(space); }
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
{ return castSpace(&castGeoprop(prop)->proposedDimensions); }

int* IVD_geoprop_expand_horizontal(IVD_GeometryProposal* prop)
{ return &castGeoprop(prop)->expandForAngle(IVD::Angle::Horizontal); }

int* IVD_geoprop_expand_vertical(IVD_GeometryProposal* prop)
{ return &castGeoprop(prop)->shrinkForAngle(IVD::Angle::Vertical); }

int* IVD_geoprop_shrink_horizontal(IVD_GeometryProposal* prop)
{ return &castGeoprop(prop)->shrinkForAngle(IVD::Angle::Horizontal); }

int* IVD_geoprop_shrink_vertical(IVD_GeometryProposal* prop)
{ return &castGeoprop(prop)->shrinkForAngle(IVD::Angle::Vertical); }

int IVD_geoprop_verify_compliance(IVD_GeometryProposal* prop, IVD_Dimens* space)
{ return castGeoprop(prop)->verifyCompliance(*castSpace(space)); }

void IVD_geoprop_round_conflicts(IVD_GeometryProposal* prop, IVD_Dimens* space)
{ *castSpace(space) = castGeoprop(prop)->roundConflicts(*castSpace(space)); }

//-----------------------------------------------------------------------------------------------------Widget
void IVD_widget_draw(IVD_Environment* environment, IVD_Widget* widget)
{ castEnv(environment)->drawWidget(widget); }

void IVD_widget_process_collision_point(IVD_Environment* environment,
                                        IVD_Widget* widget,
                                        IVD_Coords* coords)
{ castEnv(environment)->distributeCollisionPointOnWidget(widget, coords); }

//--------------------Accessors

}//extern "C"
