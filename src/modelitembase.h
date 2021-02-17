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

#ifndef MODELITEMBASE_H
#define MODELITEMBASE_H

extern "C"
{
#include "user_include/IVD_c.h"
}

#include <assert.h>
#include <vector>
#include <map>
#include <set>

#include "event.h"
#include "statekey.h"


namespace IVD
{

class ModelContainer;
class DisplayItem;
class Material;

struct ModelBlueprints
{
    IVD_Model* (*ctor)();
    void (*dtor)(IVD_Model*);

    const char* (*overrideMaterial)(IVD_Model*);

    std::map<std::string, void (*)(IVD_Model*)> triggers;
};


class Material
{

};

class Model
{
    const ModelBlueprints& myBlueprints;
    std::unique_ptr<IVD_Model, void (*)(IVD_Model*)> underlyingModel;

    Model* parent = nullptr;
    std::set<Model*> childModels;
    std::set<DisplayItem*> childDisplayItems;

public:
    Model(const ModelBlueprints& prints):
        myBlueprints(prints),
        underlyingModel(prints.ctor(), prints.dtor)
    {}

    void executeTrigger(const std::string trig)
    { myBlueprints.triggers.at(trig)(underlyingModel.get()); }

    void setChildModel(Model* model)
    {
        model->parent = this;
        childModels.insert(model);
    }

    void removeChild(Model* model)
    {
        assert(model->parent == this);
        model->parent = nullptr;
        childModels.erase(model);
    }

    Material* overrideMaterial()
    {

    }
};


class MisconfiguredModelException : public std::exception {};

class ModelItemBase
{
    friend class ModelContainer;
    friend class ModelContainer; //Booooo!
    friend class Environment;
    friend class DisplayItem;

    ModelContainer* parentContainer;
    EventQueue* myEventQueue;

    std::vector<DisplayItem*> internalDisplayItems;

    //Linked List stuff, managed by StandardModelContainer (parent).
    ModelItemBase* previousItem;
    std::unique_ptr<ModelItemBase> nextItem;

    //
    std::unique_ptr<ModelContainer> childContainer;

    //This is for C interop
    void* userData;
    IVD_user_data_destructor userDataDtor;

    IVD_callback_get_number getNumberCallback;
    IVD_callback_get_string getStringCallback;
    IVD_callback_check_const checkNumberConstCallback;
    IVD_callback_check_const checkStringConstCallback;
    IVD_callback_set_number setNumberCallback;
    IVD_callback_set_string setStringCallback;

    IVD_callback_trigger modelTriggerCallback;


    ModelContainer* getParentContainer()
    {
        if(!parentContainer) throw MisconfiguredModelException();
        return parentContainer;
    }

    EventQueue* getEventQueue()
    {
        if(myEventQueue) return myEventQueue;
        throw MisconfiguredModelException();
    }

    void setParent(ModelContainer* parent, EventQueue* even)
    {
        parentContainer = parent;
        myEventQueue = even;
    }

    StateKey getStateKey(const ValueKey ident)
    { return StateKey(ident, this); }

    //Expected to always be called when setNumber or setString
    // mutate data.
    void pushSetEvent()
    {
        ModelEvent e;
        e.type = ModelEvent::Type::ModelItemSet;
        e.modelItem = this;
        e.container = parentContainer;
        getEventQueue()->pushEvent(e);
    }
    
public:
    ModelItemBase(ModelContainer* parent, EventQueue* queue);
    ~ModelItemBase();

    void setUserData(void* suppliedUserData, IVD_user_data_destructor dtor)
    {
        userData = suppliedUserData;
        userDataDtor = dtor;
    }

    void* getUserData()
    { return userData; }

    void setState(ValueKey key)
    {
        ModelEvent e;
        e.type = ModelEvent::Type::ModelStateSet;
        e.modelItem = this;
        e.stateKey = getStateKey(key);
        getEventQueue()->pushEvent(e);
    }

    void unsetState(ValueKey key)
    {
        ModelEvent e;
        e.type = ModelEvent::Type::ModelStateUnset;
        e.modelItem = this;
        e.stateKey = getStateKey(key);
        getEventQueue()->pushEvent(e);
    }

    Material* overrideMaterial();

    void setCallback(IVD_callback_get_number fun)
    {  getNumberCallback = fun; /*There is nothing = fun about writing all these*/ }
    void setCallback(IVD_callback_get_string fun)
    { getStringCallback = fun; }
    void setCallback(IVD_callback_set_number fun)
    { setNumberCallback = fun; }
    void setCallback(IVD_callback_set_string fun)
    { setStringCallback = fun; }
    void setCallback(IVD_callback_trigger fun)
    { modelTriggerCallback = fun; }

    void setCallbackCheckNumber(IVD_callback_check_const fun)
    { checkNumberConstCallback = fun; }
    void setCallbackCheckString(IVD_callback_check_const fun)
    { checkStringConstCallback = fun; }

    //Maybe... Return a status code if the function pointer is null? Probably. TODO

    double getNumber(const ModelKey key)
    { return getNumberCallback(key.c_str(), userData); }
    
    std::string getString(const ModelKey key)
    { return std::string(getStringCallback(key.c_str(), userData)); }

    bool isNumberConst(const ModelKey key)
    { return checkNumberConstCallback(key.c_str(), userData); }
    bool isStringConst(const ModelKey key)
    { return checkStringConstCallback(key.c_str(), userData); }

    void setNumber(const ModelKey key, double num)
    {
        setNumberCallback(key.c_str(), num, userData);
        pushSetEvent();
    }
    
    void setString(const ModelKey key, const std::string str)
    {
        setStringCallback(key.c_str(), str.c_str(), userData);
        pushSetEvent();
    }

    void reactToTrigger(const std::string key)
    { modelTriggerCallback(key.c_str(), userData); }

    ModelContainer* instantiateChildModel(const std::string& name);

    ModelContainer* getChildContainer()
    { return childContainer.get(); }
};




}//IVD


#endif // MODELITEMBASE_H
