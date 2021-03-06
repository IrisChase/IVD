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

template<typename F>
void setupCallback(IVD_Instance* instance, F callback)
{ reinterpret_cast<IVD::ModelItemBase*>(instance)->setCallback(callback); }

extern "C"
{
#include "user_include/IVD_c.h"


//--------------------------------------------------------------------------------------Environment
IVD_Runtime* IVD_create_environment()
{ return reinterpret_cast<IVD_Runtime*>(new IVD::Environment()); }
void IVD_destroy_environment(IVD_Runtime* environment)
{ delete reinterpret_cast<IVD::Environment*>(environment); }

//--------------------Accessors
IVD_Model* IVD_environment_add_model(IVD_Runtime* environment, const char* name)
{
    auto* properEnv       = reinterpret_cast<IVD::Environment*>(environment);
    return reinterpret_cast<IVD_Model*>(properEnv->instantiateModel(name));
}
int IVD_environment_load_file(IVD_Runtime* environment, const char* path)
{
    auto* properEnv = reinterpret_cast<IVD::Environment*>(environment);
    return properEnv->loadFromIVDFile(path);
}
const char* IVD_environment_get_compiler_errors(IVD_Runtime* environment)
{
   auto* properEnv = reinterpret_cast<IVD::Environment*>(environment);
   return properEnv->getCompilerErrors();
}
void IVD_environment_run(IVD_Runtime* environment)
{
    auto* properEnv = reinterpret_cast<IVD::Environment*>(environment);
    
    //And this is where we take control! 🎊
    properEnv->run();
}

//--------------------------------------------------------------------------------------Model
IVD_Instance* IVD_model_add_instance(IVD_Model* model)
{
    return reinterpret_cast<IVD_Instance*>
            (reinterpret_cast<IVD::ModelContainer*>(model)->push_back_new());
}

int IVD_model_instance_count(IVD_Model* container)
{ return reinterpret_cast<IVD::ModelContainer*>(container)->size(); }

void IVD_model_erase_later(IVD_Model* container, IVD_Instance* item)
{ reinterpret_cast<IVD::ModelContainer*>(container)->erase_later(reinterpret_cast<IVD::ModelItemBase*>(item)); }

void IVD_container_swap(IVD_Model* container, IVD_Instance* item1, IVD_Instance* item2)
{
    reinterpret_cast<IVD::ModelContainer*>(container)->swap(reinterpret_cast<IVD::ModelItemBase*>(item1),
                                                            reinterpret_cast<IVD::ModelItemBase*>(item2));
}

IVD_Instance* IVD_model_first(IVD_Model* model)
{
    auto it = reinterpret_cast<IVD::ModelContainer*>(model)->begin();
    //Can dereference the end iterator because it just points to a nullptr,
    // which it then evaluates to.
    return reinterpret_cast<IVD_Instance*>(*it);
}
//--------------------------------------------------------------------------------------Instance
IVD_Model* IVD_instance_actualize_child_model(IVD_Instance* instance, const char* name)
{
    return reinterpret_cast<IVD_Model*>
            (reinterpret_cast<IVD::ModelItemBase*>(instance)->instantiateChildModel(name));
}

IVD_Model* IVD_instance_get_child_model(IVD_Instance* instance)
{
    return reinterpret_cast<IVD_Model*>
            (reinterpret_cast<IVD::ModelItemBase*>(instance)->getChildContainer());
}

void IVD_instance_set_user_data(IVD_Instance* instance, void* data, IVD_instance_User_Data_Destructor dtor)
{
    reinterpret_cast<IVD::ModelItemBase*>(instance)->setUserData(data, dtor); //D A T A    D E T O U R
}

void* IVD_instance_get_user_data(IVD_Instance* instance)
{ return reinterpret_cast<IVD::ModelItemBase*>(instance)->getUserData(); }

IVD_Instance* IVD_instance_next(IVD_Instance* item)
{
    auto* properItem = reinterpret_cast<IVD::ModelItemBase*>(item);
    auto it = IVD::ModelContainer::iterator(properItem);
    return reinterpret_cast<IVD_Instance*>(*(++it));
}

void IVD_instance_set_state(IVD_Instance* instance, const char* state_key)
{ reinterpret_cast<IVD::ModelItemBase*>(instance)->setState(state_key); }

void IVD_instance_unset_state(IVD_Instance* instance, const char* state_key)
{ reinterpret_cast<IVD::ModelItemBase*>(instance)->unsetState(state_key); }

void IVD_instance_set_number(IVD_Instance* instance, const char* key, double val)
{ reinterpret_cast<IVD::ModelItemBase*>(instance)->setNumber(key, val); }

void IVD_instance_set_string(IVD_Instance* instance, const char* key, const char* val)
{ reinterpret_cast<IVD::ModelItemBase*>(instance)->setString(key, val); }


void IVD_instance_set_number_getter(IVD_Instance* instance, IVD_instance_Get_Number_Callback fun)
{ setupCallback(instance, fun); }
void IVD_instance_set_string_getter(IVD_Instance* instance, IVD_instance_Get_String_Callback fun)
{ setupCallback(instance, fun); }
//Well aren't these two fucking special...
void IVD_instance_set_check_number_const(IVD_Instance* instance, IVD_Instance_Check_Const_Callback fun)
{  reinterpret_cast<IVD::ModelItemBase*>(instance)->setCallbackCheckNumber(fun); }
void IVD_instance_set_check_string_const(IVD_Instance* instance, IVD_Instance_Check_Const_Callback fun)
{ reinterpret_cast<IVD::ModelItemBase*>(instance)->setCallbackCheckString(fun); }
void IVD_instance_set_number_setter(IVD_Instance* instance, IVD_instance_Set_Number_Callback fun)
{ setupCallback(instance, fun); }
void IVD_instance_set_string_setter(IVD_Instance* instance, IVD_instance_Set_String_Callback fun)
{ setupCallback(instance, fun); }
void IVD_instance_set_trigger_callback(IVD_Instance* instance, IVD_instance_Trigger_Callback fun)
{ setupCallback(instance, fun); }



//--------------------Accessors

}//extern "C"
