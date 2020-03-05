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

namespace Internals //Reference this namespace at your own peril.
{

inline void printRuntimeError(const std::string& str)
{
    std::cerr << "IVD Runtime Error: " << str << "." << std::endl;
}

inline void errorCouldNotFindAccessor(const std::string& type, const std::string& key)
{
    printRuntimeError(std::string("Could not find " + type + ", for key: " + key + "."));
    //TODO: This error needs to be accessible programatically.
}

//Save *minutes* of typing with this one weird template!
template<typename MapT>
typename MapT::mapped_type findFunction(MapT map, const char* key)
{
    auto it = map.find(key);
    if(it != map.end()) return it->second;
    return typename MapT::mapped_type{};
}

template<typename proto>
using FunctionMap = std::map<std::string, std::function<proto>>;

struct InstanceData
{
    FunctionMap<double()> numberGetters;
    FunctionMap<std::string()> stringGetters;

    FunctionMap<void(double)> numberSetters;
    FunctionMap<void(const std::string&)> stringSetters;

    FunctionMap<void()> triggers;

    std::string lastStringGetResult; //Cleverrrrrrrrrr

    static InstanceData* fromVoid(void* data)
    { return reinterpret_cast<InstanceData*>(data); }
};

inline void destroyItemData(void* data)
{ delete reinterpret_cast<InstanceData*>(data); }


inline double numberGetterCallbackHook(const char* key, void* data)
{
    auto func = findFunction(InstanceData::fromVoid(data)->numberGetters, key);
    if(func) return func();

    errorCouldNotFindAccessor("Number Getter", key);

    return 0; //I guess... Although there should be a warning for this behavior...
}

inline const char* stringGetterCallbackHook(const char* key, void* data)
{
    auto* inst = InstanceData::fromVoid(data);
    auto func = findFunction(inst->stringGetters, key);
    if(func)
    {
        //Without this, closures would not be able to return copies of their own local strings.
        inst->lastStringGetResult = func();
        return inst->lastStringGetResult.c_str(); //CLEVER GURRRLLLLLLL
    }

    errorCouldNotFindAccessor("String Getter", key);

    return "";
}

inline int numberConstCheckCallbackHook(const char* key, void* data)
{ return !InstanceData::fromVoid(data)->numberSetters.count(key); }

inline int stringConstCheckCallbackHook(const char* key, void* data)
{ return !InstanceData::fromVoid(data)->stringSetters.count(key); }

inline void numberSetterCallbackHook(const char* key, double num, void* data)
{
    auto func = findFunction(InstanceData::fromVoid(data)->numberSetters, key);
    if(func) func(num);
    else errorCouldNotFindAccessor("Number Setter", key);
}

inline void stringSetterCallbackHook(const char* key, const char* value, void* data)
{
    auto func = findFunction(InstanceData::fromVoid(data)->stringSetters, key);
    if(func) func(value);
    else errorCouldNotFindAccessor("String Setter", key);
}


inline void triggerCallbackHook(const char* key, void* data)
{
    auto func = findFunction(InstanceData::fromVoid(data)->triggers, key);
    if(func) func();
    else errorCouldNotFindAccessor("Trigger", key);
}


inline void setupAllCallbacksies(IVD_Instance* instance)
{
    IVD_instance_set_number_getter(instance, &numberGetterCallbackHook);
    IVD_instance_set_string_getter(instance, &stringGetterCallbackHook);

    IVD_instance_set_check_number_const(instance, &numberConstCheckCallbackHook);
    IVD_instance_set_check_string_const(instance, &stringConstCheckCallbackHook);

    IVD_instance_set_number_setter(instance, &numberSetterCallbackHook);
    IVD_instance_set_string_setter(instance, &stringSetterCallbackHook);

    IVD_instance_set_trigger_callback(instance, &triggerCallbackHook);
}

}//Internals

class Model;

class Instance
{
    friend class Model;

protected:
    IVD_Instance* internal;
    Internals::InstanceData* myData;

public:
    Instance(): internal(nullptr), myData(nullptr) {}
    Instance(IVD_Instance* model, Internals::InstanceData* data): internal(model), myData(data) {}
    Instance(IVD_Instance* instance):
        internal(instance),
        myData(reinterpret_cast<Internals::InstanceData*>(IVD_instance_get_user_data(instance)))
    {}

    void set_state(const std::string state) const
    { IVD_instance_set_state(internal, state.c_str()); }

    void unset_state(const std::string& state) const
    { IVD_instance_unset_state(internal, state.c_str()); }

    Model actualize_child_model(const std::string& name) const;
    Model get_child_model() const;

    double get_number(const std::string& key) const
    { return myData->numberGetters[key](); }

    std::string get_string(const std::string& key) const
    { return myData->stringGetters[key](); }

    void bind_accessors(const std::string key, double& number)
    {
        set_number_getter(key, [&]() { return number; });
        set_number_setter(key, [&](double newNum) { number = newNum; });
    }

    void bind_accessors(const std::string key, const double& number)
    {
        set_number_getter(key, [&]() { return number; });
    }

    void bind_accessors(const std::string key, std::string& str)
    {
        set_string_getter(key, [&]() { return str; });
        set_string_setter(key, [&](std::string newNum) { str = newNum; });
    }

    void bind_accessors(const std::string key, const std::string& str)
    {
        set_string_getter(key, [&] { return str; });
    }


    void set_number(const std::string key, double num) const
    { IVD_instance_set_number(internal, key.c_str(), num); }

    void set_string(const std::string key, const std::string value) const
    { IVD_instance_set_string(internal, key.c_str(), value.c_str()); }


    void set_number_getter(const std::string& key, std::function<double()> fun) const
    { myData->numberGetters[key] = fun; }

    void set_string_getter(const std::string& key, std::function<std::string()> fun) const
    { myData->stringGetters[key] = fun; }


    void set_number_setter(const std::string& key, std::function<void(double)> fun) const
    { myData->numberSetters[key] = fun; }

    void set_string_setter(const std::string& key, std::function<void(const std::string&)> fun) const
    { myData->stringSetters[key] = fun; }


    void set_trigger(const std::string& key, std::function<void()> trigger) const
    { myData->triggers[key] = trigger; }

    bool operator==(const Instance& other)
    { return other.internal == internal && other.myData == myData; }
};

class Model
{
    friend class Environment;
    friend class Instance;
    IVD_Model* internal;

public:
    Model(): internal(nullptr) {}
    Model(IVD_Model* model): internal(model) {}

    class iterator
    {
        IVD_Instance* pos;

    public:
        iterator(): pos(nullptr) {}
        iterator(IVD_Instance* pos): pos(pos) {}

        bool operator==(const iterator& other)
        { return pos == other.pos; }

        bool operator!=(const iterator& other)
        { return pos != other.pos; }

        Instance operator*()
        { return Instance(pos); }

        Instance operator->()
        { return Instance(pos); }

        iterator& operator++()
        {
            if(pos) pos = IVD_instance_next(pos);
            return *this;
        }

        iterator operator++(int)
        {
            iterator temp = *this;
            this->operator++();
            return temp;
        }
    };

    iterator begin()
    { return iterator(IVD_model_first(internal)); }

    iterator end()
    { return iterator(); }

    int size() const { return IVD_model_instance_count(internal); }

    void erase_later(const Instance& item) const
    { IVD_model_erase_later(internal, item.internal); }

    Instance add_instance() const
    {
        IVD_Instance* internalInstance = IVD_model_add_instance(internal);

        auto* itemData = new Internals::InstanceData;
        IVD_instance_set_user_data(internalInstance, itemData, &Internals::destroyItemData);

        Internals::setupAllCallbacksies(internalInstance);

        return Instance(internalInstance, itemData);
    }

    void swap_instances(const Instance& a, const Instance& b) const
    { IVD_container_swap(internal, a.internal, b.internal); }
};

inline Model Instance::actualize_child_model(const std::string& name) const
{ return Model(IVD_instance_actualize_child_model(internal, name.c_str())); }

inline Model Instance::get_child_model() const
{ return Model(IVD_instance_get_child_model(internal)); }


class Runtime
{
    std::unique_ptr<IVD_Runtime, void(*)(IVD_Runtime*)> internal;

public:
    Runtime(): internal(IVD_create_environment(), &IVD_destroy_environment) {}

    Model add_model(const std::string& name)
    {  return Model(IVD_environment_add_model(internal.get(), name.c_str())); }

    int load_IVD_from_file(const std::string& path)
    { return IVD_environment_load_file(internal.get(), path.c_str()); }

    void run()
    { IVD_environment_run(internal.get()); }
};



}//IVD
