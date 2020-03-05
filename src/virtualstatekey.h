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

#include "assert.h"
#include <memory>
#include <functional>
#include <vector>

#include "statekey.h"

namespace IVD
{

class Environment;
class StateManager;

struct VirtualStateKey
{
    StateKey proxyStateKey;

    struct Node
    {
        int operation;
        bool negation;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;

        StateKey stateKey;


        Node() {}
        Node(const Node& other);
        Node& operator=(const Node& other);

        void addYourStateKeys(std::vector<StateKey>* keys);
        bool evaluate(StateManager* stateManager) const;

    };

    Node root;

    std::vector<StateKey> getAffectedKeys();
    void syncProxyState(StateManager* stateManager) const;

    bool operator<(const VirtualStateKey& other) const
    { return proxyStateKey < other.proxyStateKey; }

    bool operator==(const VirtualStateKey& other) const
    { return !(*this < other) && !(other < *this); }

    bool operator!=(const VirtualStateKey& other) const
    { return !(*this == other); }
};

struct VirtualStateKeyPrecursor
{
    //Must have element scope and no path.
    ScopedValueKey proxyStateKeyPrecursor;
    DisplayItem* context;

    struct Node
    {
        int operation;
        bool negation;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;

        ScopedValueKey stateKeyPrecursor;

        Node(): negation(false) {}
        Node(const Node& other);
        Node& operator=(const Node& other);

        std::string printoutThySelf() const;
    };

    Node root;

    bool checkIsVirtual()
    { return !root.left && !root.right && !root.negation; }

    VirtualStateKey generateVirtualStateKey(Environment* env);

    std::string generatePrintout();

private:
    void populateNode(VirtualStateKeyPrecursor::Node* pos,
                      VirtualStateKey::Node* otherNode, Environment* env);
};


}//IVD
