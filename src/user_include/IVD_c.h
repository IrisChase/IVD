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

//----------------------------------------------------------------------------------Function pointer typedefs
typedef void(*IVD_instance_User_Data_Destructor)(void*);

typedef double(*IVD_instance_Get_Number_Callback)(const char*, void*);
typedef const char*(*IVD_instance_Get_String_Callback)(const char*, void*);

//0 false 1 true
typedef int(*IVD_Instance_Check_Const_Callback)(const char* key, void*);

typedef void(*IVD_instance_Set_Number_Callback)(const char* key, double, void*);
typedef void(*IVD_instance_Set_String_Callback)(const char* key, const char* val, void*);

typedef void(*IVD_instance_Trigger_Callback)(const char*, void*);

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
void IVD_instance_set_user_data(IVD_Instance*, void*, IVD_instance_User_Data_Destructor);
void* IVD_instance_get_user_data(IVD_Instance*);

IVD_Instance* IVD_instance_next(IVD_Instance*);

void IVD_instance_set_state(IVD_Instance*, const char*);
void IVD_instance_unset_state(IVD_Instance*, const char*);

void IVD_instance_set_number(IVD_Instance*, const char* key, double val);
void IVD_instance_set_string(IVD_Instance*instance, const char* key, const char* val);

//Instance callback setters
void IVD_instance_set_number_getter(IVD_Instance* instance, IVD_instance_Get_Number_Callback fun);
void IVD_instance_set_string_getter(IVD_Instance* instance, IVD_instance_Get_String_Callback fun);

void IVD_instance_set_check_number_const(IVD_Instance* instance, IVD_Instance_Check_Const_Callback);
void IVD_instance_set_check_string_const(IVD_Instance* instance, IVD_Instance_Check_Const_Callback);

void IVD_instance_set_number_setter(IVD_Instance* instance, IVD_instance_Set_Number_Callback fun);
void IVD_instance_set_string_setter(IVD_Instance* instance, IVD_instance_Set_String_Callback fun);

void IVD_instance_set_trigger_callback(IVD_Instance* instance, IVD_instance_Trigger_Callback fun);

//That's all, folks! >:3c
#endif //IVD_ALPHA_C_BINDINGS_H
