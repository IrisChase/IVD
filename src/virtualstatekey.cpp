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

#include "virtualstatekey.h"
#include "environment.h"
#include "statemanager.h"
#include "keywords.h"
#include "binaryexpressionprinter.h"

namespace IVD
{


VirtualStateKey::Node::Node(const VirtualStateKey::Node& other):
    operation(other.operation),
    negation(other.negation),
    stateKey(other.stateKey)
{
    if(other.left) left   = std::make_unique<VirtualStateKey::Node>(*other.left);
    if(other.right) right = std::make_unique<VirtualStateKey::Node>(*other.right);
}

VirtualStateKey::Node& VirtualStateKey::Node::operator=(const VirtualStateKey::Node& other)
{
    operation = other.operation;
    negation = other.negation;
    stateKey = other.stateKey;

    if(other.left) left   = std::make_unique<VirtualStateKey::Node>(*other.left);
    if(other.right) right = std::make_unique<VirtualStateKey::Node>(*other.right);

    return *this;
}

VirtualStateKeyPrecursor::Node::Node(const VirtualStateKeyPrecursor::Node& other):
    operation(other.operation),
    negation(other.negation),
    stateKeyPrecursor(other.stateKeyPrecursor)
{
    if(other.left)   left = std::make_unique<VirtualStateKeyPrecursor::Node>(*other.left);
    if(other.right) right = std::make_unique<VirtualStateKeyPrecursor::Node>(*other.right);
}

void VirtualStateKey::Node::addYourStateKeys(std::vector<StateKey>* keys)
{
    keys->push_back(stateKey);
    if(left) left->addYourStateKeys(keys);
    if(right) right->addYourStateKeys(keys);
}

bool VirtualStateKey::Node::evaluate(StateManager* stateManager) const
{
    if(!left && !right)
    {
        if(negation) return  !stateManager->checkState(stateKey);
        else         return   stateManager->checkState(stateKey);
    }

    assert(left && right);

    switch(operation)
    {
    case Keyword::Or:  return left->evaluate(stateManager) || right->evaluate(stateManager);
    case Keyword::And: return left->evaluate(stateManager) && right->evaluate(stateManager);
    case Keyword::Xor: return left->evaluate(stateManager) != right->evaluate(stateManager);
    default: assert(false);
    }
}

VirtualStateKeyPrecursor::Node& VirtualStateKeyPrecursor::Node::operator=(const VirtualStateKeyPrecursor::Node& other)
{
    operation = other.operation;
    negation = other.negation;
    stateKeyPrecursor = other.stateKeyPrecursor;

    if(other.left) left   = std::make_unique<VirtualStateKeyPrecursor::Node>(*other.left);
    if(other.right) right = std::make_unique<VirtualStateKeyPrecursor::Node>(*other.right);

    return *this;
}

std::string VirtualStateKeyPrecursor::Node::printoutThySelf() const
{
    std::string literal;
    if(negation) literal += "not ";

    switch(operation)
    {
    case Keyword::ScopedValueKey:
        literal += stateKeyPrecursor.generatePrintout();
        break;
    case Keyword::And:
        literal += "and";
        break;
    case Keyword::Or:
        literal += "or";
        break;
    case Keyword::Xor:
        literal += "xor";
        break;

    default: assert(false);
    }

    return literal;
}

std::vector<StateKey> VirtualStateKey::getAffectedKeys()
{
    std::vector<StateKey> keys;
    root.addYourStateKeys(&keys);
    return keys;
}

void VirtualStateKey::syncProxyState(StateManager* stateManager) const
{
    stateManager->mutateIfObserved(proxyStateKey, root.evaluate(stateManager));
}

VirtualStateKey VirtualStateKeyPrecursor::generateVirtualStateKey(Environment* env)
{
    VirtualStateKey myVirtual;
    myVirtual.proxyStateKey =  env->generateStateKeyFromPrecursor(proxyStateKeyPrecursor, context);
    populateNode(&root, &myVirtual.root, env);
    return myVirtual;
}

std::string VirtualStateKeyPrecursor::generatePrintout()
{
    return generateExpressionPrintout(root);
}

void VirtualStateKeyPrecursor::populateNode(VirtualStateKeyPrecursor::Node* pos,
                                            VirtualStateKey::Node* otherNode,
                                            Environment* env)
{
    otherNode->operation = pos->operation;
    otherNode->negation  = pos->negation;

    //Not every node is a state...
    if(pos->stateKeyPrecursor.key) otherNode->stateKey = env->generateStateKeyFromPrecursor(pos->stateKeyPrecursor, context);

    if(pos->left)
    {
        if(otherNode->left) otherNode->left.reset();
        otherNode->left = std::make_unique<VirtualStateKey::Node>();
        populateNode(pos->left.get(), otherNode->left.get(), env);
    }
    if(pos->right)
    {
        if(otherNode->right) otherNode->right.reset();
        otherNode->right = std::make_unique<VirtualStateKey::Node>();
        populateNode(pos->right.get(), otherNode->right.get(), env);
    }
}


}//IVD
