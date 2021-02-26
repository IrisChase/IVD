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

#define IVD_USER_ATTRIBUTE_TYPE_STRING 3001
#define IVD_USER_ATTRIBUTE_TYPE_SCALAR 3002
#define IVD_USER_ATTRIBUTE_TYPE_TOKEN  3003

struct IVD_Environment;

struct IVD_Widget;
//register IVD_Widget as layout to be used with layout attribute

struct IVD_Dimens;
struct IVD_Coords;
struct IVD_Rect;
struct IVD_GeometryProposal;
struct IVD_Style;
struct IVD_Element;
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
                                     IVD_Widget* (*ctor)(IVD_Environment*),
                                     void (*dtor)(IVD_Widget*),
                                     int (*getFillPrecedence)(IVD_Widget*, const int),
                                     void (*shape)(IVD_Widget *, IVD_GeometryProposal *),
                                     void (*draw)(IVD_Widget*, IVD_Canvas*),
                                     IVD_Dimens* (*getSpace)(IVD_Widget *), //canbe null
                                     int (*detectCollisionPoint)(IVD_Widget *, IVD_Coords *), //canbe null
                                     void (*distributeCollisionPoint)(IVD_Widget*),
                                     void (*triggerHandler)(IVD_Widget*, const char*));

//IVD manages widget lifetimes so they can be "deleted later"
IVD_Widget* IVD_environment_widget_create(IVD_Environment*, const char* name, IVD_Widget* parent);
IVD_Element* IVD_environment_create_element_from_class(IVD_Environment*, const char* className, IVD_Widget* parent);
void IVD_environment_widget_destroy(IVD_Environment*, IVD_Widget*);
void IVD_environment_element_destroy(IVD_Environment*, IVD_Widget* parent, IVD_Element*); //Seems like this could do some real damage...


void IVD_environment_register_layout(IVD_Environment* environment,
                                     const char* name,
                                     IVD_Widget* (*ctor)(IVD_Environment*),
                                     void (*dtor)(IVD_Widget*),
                                     int (*getFillPrecedence)(IVD_Widget*, const int),
                                     void (*shape)(IVD_Widget*, IVD_GeometryProposal*),
                                     void (*draw)(IVD_Widget*, IVD_Canvas*),
                                     IVD_Dimens* (*getSpace)(IVD_Widget *),
                                     void (*bubbler)(IVD_Widget*));


void IVD_environment_register_layout_attribute(IVD_Environment*,
                                               const char* layoutName,
                                               const char* attributeKey);


void IVD_environment_register_widget_attribute(IVD_Environment*,
                                               const char* widgetName,
                                               const char* attributeKey);

void IVD_environment_add_layout_attribute_type(IVD_Environment*,
                                               const char* layoutName,
                                               int attributeType);

void IVD_environment_add_widget_attribute_type(IVD_Environment*,
                                               const char* widgetName,
                                               int attributeType);

//----------------------------------------------------------------------------------------------Dust Bindings
IVD_Dimens* IVD_dimens_alloc();
void IVD_dimens_free(IVD_Dimens* space);
int* IVD_dimens_w(IVD_Dimens* space);
int* IVD_dimens_h(IVD_Dimens* space);

IVD_Coords* IVD_coords_alloc();
void   IVD_coords_free(IVD_Coords* point);
int*   IVD_coords_x(IVD_Coords* point);
int*   IVD_coords_y(IVD_Coords* point);

IVD_Rect* IVD_rect_alloc();
void IVD_rect_free(IVD_Rect* rect);
IVD_Dimens* IVD_rect_get_dimens(IVD_Rect* rect); //Still owned by *rect
IVD_Coords* IVD_rect_get_coords(IVD_Rect* rect);
void IVD_rect_set_space(IVD_Rect* rect, IVD_Dimens* space); //Copies values
void IVD_rect_set_point(IVD_Rect* rect, IVD_Coords* point);

//-------------------------------------------------------------------------------------------GeometryProposal
IVD_GeometryProposal* IVD_geoprop_alloc();
IVD_GeometryProposal* IVD_geoprop_alloc_copy(IVD_GeometryProposal*);
void IVD_geoprop_free(IVD_GeometryProposal* prop);
IVD_Dimens* IVD_geoprop_proposed_space(IVD_GeometryProposal* prop);
int* IVD_geoprop_expand_horizontal(IVD_GeometryProposal* prop);
int* IVD_geoprop_expand_vertical(IVD_GeometryProposal* prop);
int* IVD_geoprop_shrink_horizontal(IVD_GeometryProposal* prop);
int* IVD_geoprop_shrink_vertical(IVD_GeometryProposal* prop);
int IVD_geoprop_verify_compliance(IVD_GeometryProposal* prop, IVD_Dimens* space);
void IVD_geoprop_round_conflicts(IVD_GeometryProposal* prop, IVD_Dimens* space);


//----------------------------------------------------------------------------------------------------Element
IVD_Dimens* IVD_element_get_dimens(const IVD_Element*);
int IVD_element_get_fill_precedence(IVD_Element*, int angle); //Angle -> FillPrecedence

void IVD_element_shape(IVD_Element*, IVD_GeometryProposal*);
void IVD_element_set_offset(IVD_Element*, IVD_Coords*);

void IVD_element_draw(IVD_Element*);
void IVD_element_bubble(IVD_Element*);

IVD_Element* IVD_widget_get_underlying_element(IVD_Environment*, IVD_Widget*);

void IVD_widget_get_child_elements(IVD_Environment*, IVD_Widget*, IVD_Element*** result, int* size);
IVD_Element* IVD_widget_get_child_element_for_named_cell(IVD_Environment* environment, IVD_Widget*, const char* name);

//void IVD_draw_X(IVD_Canvas*, ...); //canvas cursor is already set to the correct offset.

void IVD_canvas_draw_image(IVD_Canvas*,
                           int x,
                           int y,
                           int width,
                           int height,
                           int stride,
                           int channels,
                           unsigned char* data);

//------------------------------------------------------------------------------------------------------Style
IVD_Style* IVD_create_style(const IVD_Environment* theEnv, const char* className);
void IVD_destroy_style(const IVD_Environment* theEnv, IVD_Style* style);

void IVD_style_set_state(IVD_Style* style, const char* stateKey, const int state);
//Should we even be able to readback the state?

//That's all, folks! >:3c
#endif //IVD_ALPHA_C_BINDINGS_H
