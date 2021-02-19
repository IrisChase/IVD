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

#ifndef IVD_ALPHA_C_BINDINGS_H
#define IVD_ALPHA_C_BINDINGS_H

#ifdef __cpluplus
#error IVD_c.h is a C header only, wrap your include in an extern block or use IVD_cpp.h instead.
#endif

#include "IVD_status.h"

#define IVD_FILL_PRECEDENCE_GREEDY	0
#define IVD_FILL_PRECEDENCE_SHRINKY	1

struct IVD_Environment;

struct IVD_Widget;
//register IVD_Widget as layout to be used with layout attribute

struct IVD_Space;
struct IVD_Point;
struct IVD_Rect;
struct IVD_GeometryProposal;
struct IVD_Style;
//struct IVD_Element; // superceded by Widget
struct IVD_Canvas;

//----------------------------------------------------------------------------------Function pointer typedefs
typedef void(*IVD_user_data_destructor)(void*);

typedef double(*IVD_callback_get_number)(const char*, void*);
typedef const char*(*IVD_callback_get_string)(const char*, void*);

//0 false 1 true
typedef int(*IVD_callback_check_const)(const char* key, void*);

typedef void(*IVD_callback_set_number)(const char* key, double, void*);
typedef void(*IVD_callback_set_string)(const char* key, const char* val, void*);

typedef void(*IVD_callback_trigger)(const char*, void*);

//------------------------------------------------------------------------------------------------Environment
IVD_Environment* IVD_create_environment();
void IVD_destroy_environment(IVD_Environment*);

int IVD_environment_load_file(IVD_Environment*, const char* path);
const char* IVD_environment_get_compiler_errors(IVD_Environment*);
void IVD_environment_run(IVD_Environment*);

void IVD_environment_register_widget(IVD_Environment *environment, const char* name,
                                     IVD_Widget* (*ctor)(),
                                     void (*dtor)(IVD_Widget*),
                                     int (*getFillPrecedence)(IVD_Widget*, const int),
                                     void (*shape)(IVD_Widget*, const IVD_GeometryProposal*),
                                     void (*draw)(IVD_Widget*, IVD_Canvas*),
                                     IVD_Space* (*getSpace)(IVD_Widget *), //canbe null
                                     int (*detectCollisionPoint)(IVD_Widget*, const IVD_Point*), //canbe null
                                     void (*triggerHandler)(IVD_Widget*, const char*));

//IVD manages widget lifetimes so they can be "deleted later"
IVD_Widget* IVD_environment_widget_create(IVD_Environment*, const char* name, IVD_Widget* parent);
void IVD_environment_widget_destroy(IVD_Environment*, IVD_Widget*);


void IVD_environment_register_layout(IVD_Environment*,
                                     const char* name,
                                     IVD_Widget* (*ctor)(),
                                     void (*dtor)(IVD_Widget*),
                                     int (*getFillPrecedence)(IVD_Widget*, const int),
                                     void (*shape)(IVD_Widget*, const IVD_GeometryProposal*),
                                     IVD_Space* (*getSpace)(IVD_Widget *));


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
IVD_Space* IVD_space_alloc();
void IVD_space_free(IVD_Space* space);
int* IVD_space_w(IVD_Space* space);
int* IVD_space_h(IVD_Space* space);

IVD_Point* IVD_coords_alloc();
void   IVD_point_free(IVD_Point* point);
int*   IVD_point_x(IVD_Point* point);
int*   IVD_point_y(IVD_Point* point);

IVD_Rect* IVD_rect_alloc();
void IVD_rect_free(IVD_Rect* rect);
IVD_Space* IVD_rect_get_space(IVD_Rect* rect); //Still owned by *rect
IVD_Point* IVD_rect_get_point(IVD_Rect* rect);
void IVD_rect_set_space(IVD_Rect* rect, IVD_Space* space); //Copies values
void IVD_rect_set_point(IVD_Rect* rect, IVD_Point* point);

//-------------------------------------------------------------------------------------------GeometryProposal
IVD_GeometryProposal* IVD_geoprop_alloc();
IVD_GeometryProposal* IVD_geoprop_alloc_copy(IVD_GeometryProposal*);
void IVD_geoprop_free(IVD_GeometryProposal* prop);
IVD_Space* IVD_geoprop_proposed_space(IVD_GeometryProposal* prop);
int* IVD_geoprop_expand_horizontal(IVD_GeometryProposal* prop);
int* IVD_geoprop_expand_vertical(IVD_GeometryProposal* prop);
int* IVD_geoprop_shrink_horizontal(IVD_GeometryProposal* prop);
int* IVD_geoprop_shrink_vertical(IVD_GeometryProposal* prop);
int IVD_geoprop_verify_compliance(IVD_GeometryProposal* prop, IVD_Space* space);
void IVD_geoprop_round_conflicts(IVD_GeometryProposal* prop, IVD_Space* space);


//-----------------------------------------------------------------------------------------------------Widget
IVD_Space* IVD_widget_get_space(const IVD_Widget*);
int IVD_get_fill_precedence(const IVD_Widget*, int); //Angle -> FillPrecedence

void IVD_widget_shape(IVD_Widget*, const IVD_GeometryProposal*);
void IVD_widget_set_offset(IVD_Widget*, const IVD_Point*);


//void IVD_draw_X(IVD_Canvas*, ...); //canvas cursor is already set to the correct offset.

//------------------------------------------------------------------------------------------------------Style
IVD_Style* IVD_create_style(const IVD_Environment* theEnv, const char* className);
void IVD_destroy_style(const IVD_Environment* theEnv, IVD_Style* style);

void IVD_style_set_state(IVD_Style* style, const char* stateKey, const int state);
//Should we even be able to readback the state?

//That's all, folks! >:3c
#endif //IVD_ALPHA_C_BINDINGS_H
