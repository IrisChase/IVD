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

struct IVD_Runtime;
struct IVD_Model;
struct IVD_Instance;
struct IVD_Material;
struct IVD_GeomtryProposal;

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
IVD_Runtime* IVD_create_environment();
void IVD_destroy_environment(IVD_Runtime*);

IVD_Model* IVD_environment_add_model(IVD_Runtime*, const char* name);
int IVD_environment_load_file(IVD_Runtime*, const char* path);
const char* IVD_environment_get_compiler_errors(IVD_Runtime*);
void IVD_environment_run(IVD_Runtime*);

//-------------------------------------------------------------------------------------------------------Model
IVD_Instance* IVD_model_add_instance(IVD_Model*);
int  IVD_model_instance_count(IVD_Model*);
void IVD_model_erase_later(IVD_Model*, IVD_Instance*);
void IVD_container_swap(IVD_Model*, IVD_Instance*, IVD_Instance*);
IVD_Instance* IVD_model_first(IVD_Model*);

//---------------------------------------------------------------------------------------------------Instance
IVD_Model* IVD_instance_actualize_child_model(IVD_Instance*, const char* name);
IVD_Model* IVD_instance_get_child_model(IVD_Instance*);
void IVD_instance_set_user_data(IVD_Instance*, void*, IVD_user_data_destructor);
void* IVD_instance_get_user_data(IVD_Instance*);

IVD_Instance* IVD_instance_next(IVD_Instance*);

void IVD_instance_set_state(IVD_Instance*, const char*);
void IVD_instance_unset_state(IVD_Instance*, const char*);

void IVD_instance_set_number(IVD_Instance*, const char* key, double val);
void IVD_instance_set_string(IVD_Instance*instance, const char* key, const char* val);

//Instance callback setters
void IVD_instance_set_number_getter(IVD_Instance* instance, IVD_callback_get_number fun);
void IVD_instance_set_string_getter(IVD_Instance* instance, IVD_callback_get_string fun);

void IVD_instance_set_check_number_const(IVD_Instance* instance, IVD_callback_check_const);
void IVD_instance_set_check_string_const(IVD_Instance* instance, IVD_callback_check_const);

void IVD_instance_set_number_setter(IVD_Instance* instance, IVD_callback_set_number fun);
void IVD_instance_set_string_setter(IVD_Instance* instance, IVD_callback_set_string fun);

void IVD_instance_set_trigger_callback(IVD_Instance* instance, IVD_callback_trigger fun);


//---------------------------------------------------------------------------------------------------Material
//-------------------------------------------------------------------------------------------GeometryProposal
IVD_GeomtryProposal* IVD_geoprop_alloc();
void IVD_geoprop_free(IVD_GeomtryProposal* prop);
int IVD_geoprop_expand_horizontal(IVD_GeomtryProposal* prop);
int IVD_geoprop_expand_vertical(IVD_GeomtryProposal* prop);
int IVD_geoprop_shrink_horizontal(IVD_GeomtryProposal* prop);
int IVD_geoprop_shrink_vertical(IVD_GeomtryProposal* prop);
int IVD_geoprop_verify_compliance(IVD_GeomtryProposal* prop, int w, int h);
int IVD_geoprop_round_conflicts_w(IVD_GeomtryProposal* prop, int w);
int IVD_geoprop_round_conflicts_h(IVD_GeomtryProposal* prop, int h);





//That's all, folks! >:3c
#endif //IVD_ALPHA_C_BINDINGS_H
