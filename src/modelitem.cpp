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

#include "modelcontainer.h"

namespace IVD
{

ModelItemBase::ModelItemBase(ModelContainer* parent, EventQueue* queue):
    parentContainer(parent),
    myEventQueue(queue),
    previousItem(nullptr),
    userData(nullptr),
    userDataDtor(nullptr),
    getNumberCallback(nullptr),
    getStringCallback(nullptr),
    checkNumberConstCallback(nullptr),
    checkStringConstCallback(nullptr),
    setNumberCallback(nullptr),
    setStringCallback(nullptr),
    modelTriggerCallback(nullptr)
{}

ModelItemBase::~ModelItemBase()
{ if(userDataDtor) userDataDtor(userData); }

ModelContainer* ModelItemBase::instantiateChildModel(const std::string& name)
{
    if(childContainer) return childContainer.get(); //BAD things happen if we delete this here...
    return (childContainer = std::make_unique<ModelContainer>(this, name)).get();
}

}//IVD
