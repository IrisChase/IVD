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

#ifndef MODELCONTAINERBASE_H
#define MODELCONTAINERBASE_H

#include "modelitembase.h"

namespace IVD
{

//Environment owns root containers, containers own model item instances,
// instances own their child containers (if allocated)
class ModelContainer
{
    friend class Environment;
    ValueKey modelName;
    ModelItemBase* parentItem; //Basically, the only way to do this is to have the user set this
    EventQueue* myEventQueue;

    std::unique_ptr<ModelItemBase> frontItem;
    ModelItemBase* lastItem;
    int itemCount;


    ModelItemBase* getParentItem() const
    {
        if(!parentItem) throw MisconfiguredModelException();
        return parentItem;
    }

    EventQueue* getEventQueue()
    {
        if(myEventQueue) return myEventQueue;
        return getParentItem()->getParentContainer()->getEventQueue();
    }

    void pushAddEvent(ModelItemBase* item)
    {
        ModelEvent e;
        e.type = ModelEvent::Type::ModelItemAdded;
        e.modelItem = item;
        e.container = this;
        getEventQueue()->pushEvent(e);
    }

    void pushSwapEvent(ModelItemBase* item1, ModelItemBase* item2)
    {
        ModelEvent e;
        e.type = ModelEvent::Type::ModelItemsSwapped;
        e.modelItem = item1;
        e.modelItem2 = item2;
        e.container = this;
        getEventQueue()->pushEvent(e);
    }

    //This is for those mythical batch sorts where it's easier for the runtime to just dump the current order
    void pushOrderInvalidationEvent()
    {
        ModelEvent e;
        e.type = ModelEvent::Type::ModelOrderInvalidated;
        e.container = this;
        getEventQueue()->pushEvent(e);
    }

    void pushRemoveEvent(ModelItemBase* item)
    {
        ModelEvent e;
        e.type = ModelEvent::Type::ModelItemRemoved;
        e.modelItem = item;
        e.container = this;
        getEventQueue()->pushEvent(e);
    }

public:
    ModelContainer(ModelItemBase* parent, const std::string& modelName = ""):
        parentItem(parent),
        modelName(modelName),
        myEventQueue(parent->getEventQueue()),
        lastItem(nullptr),
        itemCount(0)
    {}
    ModelContainer(EventQueue* queue, const std::string& modelName = ""):
        parentItem(nullptr),
        modelName(modelName),
        myEventQueue(queue),
        lastItem(nullptr),
        itemCount(0)
    {}

    void setParent(ModelItemBase* parent)
    { parentItem = parent; }

    void setModelName(const ValueKey name)
    { modelName = name; }

    ValueKeyPath getModelPath() const
    {
        if(!parentItem) return {modelName};

        auto path = getParentItem()->getParentContainer()->getModelPath();
        path.push_back(modelName);
        return path;
    }

    //Environment callback, public for testing (oof...)
    void safeToDelete(ModelItemBase* item)
    {
        if(lastItem == item)
        {
            if(item->previousItem) lastItem = item->previousItem;
            else lastItem = nullptr; //Item is root and there are none ahead of it.
        }

        if(frontItem.get() == item)
        {
            if(item->nextItem)
            {
                frontItem = std::move(item->nextItem);
                frontItem->previousItem = nullptr;
            }
            else frontItem.reset();
        }
        else
        {
            if(item->nextItem) item->nextItem->previousItem = item->previousItem;
            item->previousItem->nextItem = std::move(item->nextItem);
        }

        --itemCount;
        //God, linked lists are trivial with smart pointers...
    }

    //STL naming convention from here on out because raisens.
    class iterator
    {
        ModelItemBase* pos;

    public:
        iterator(): pos(nullptr) {}
        iterator(ModelItemBase* item): pos(item) {}

        bool operator==(const iterator& other)
        {
            return pos == other.pos;
        }

        bool operator!=(const iterator& other)
        {
            return pos != other.pos;
        }

        ModelItemBase* operator*()
        { return pos; /*Well that was easy*/ }

        ModelItemBase* operator->()
        { return pos; }

        iterator& operator++()
        {
            if(pos) pos = pos->nextItem.get();
            return *this;
        }

        iterator operator++(int)
        {
            iterator blep = *this;
            this->operator++();
            return blep;
        }
    };

    int size()
    { return itemCount; }

    iterator begin()
    { return iterator(frontItem.get()); }

    iterator end()
    { return iterator(); }

    void erase_later(ModelItemBase* item)
    { pushRemoveEvent(item); }

    void erase_later(iterator it)
    { erase_later(*it); }

    ModelItemBase* push_back_new()
    {
        auto item = std::make_unique<ModelItemBase>(this, myEventQueue);
        auto* savedCopyOfItemPointer = item.get();

        if(lastItem)
        {
            item->previousItem = lastItem;
            lastItem->nextItem = std::move(item);
        }
        else frontItem = std::move(item);

        lastItem = savedCopyOfItemPointer;

        ++itemCount;
        pushAddEvent(savedCopyOfItemPointer);

        return savedCopyOfItemPointer;
    }

    void swap(ModelItemBase *pos1, ModelItemBase *pos2)
    {
        typedef std::unique_ptr<ModelItemBase>& UniquePtrRef;
        std::unique_ptr<ModelItemBase>& pos1Cont = pos1 == frontItem.get() ? frontItem
                                                                           : pos1->previousItem->nextItem;

        std::unique_ptr<ModelItemBase>& pos2Cont = pos2 == frontItem.get() ? frontItem
                                                                           : pos2->previousItem->nextItem;


        auto swapAdjacent = [&](UniquePtrRef left, UniquePtrRef right)
        {
            std::unique_ptr<ModelItemBase> oldLeftTemp = std::move(left);

            left = std::move(right);
            oldLeftTemp->nextItem = std::move(left->nextItem);

            left->previousItem = oldLeftTemp->previousItem;
            oldLeftTemp->previousItem = left.get();
            left->nextItem = std::move(oldLeftTemp);
        };

        if(pos1Cont->nextItem == pos2Cont)
        {
            swapAdjacent(pos1Cont, pos2Cont);
        }
        else if(pos2Cont->nextItem == pos1Cont)
        {
            swapAdjacent(pos2Cont, pos1Cont);
        }
        else
        {
            {
                auto pos1PrevTemp  = pos1->previousItem;
                pos1->previousItem = pos2->previousItem;
                pos2->previousItem = pos1PrevTemp;
            }

            {
                std::unique_ptr<ModelItemBase> pos1NextTemp = std::move(pos1Cont);
                pos1Cont = std::move(pos2Cont);
                pos2Cont = std::move(pos1NextTemp);
            }

            if(!pos1Cont->nextItem) lastItem = pos1Cont.get();
            if(!pos2Cont->nextItem) lastItem = pos2Cont.get();
        }

        pushSwapEvent(pos1, pos2);
    }
};

}//IVD

#endif // MODELCONTAINERBASE_H
