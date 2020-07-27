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

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <list>
#include <map>

#include <memory>
#include <functional>

#include "statekey.h" //For ScopedValueKey
#include "keywords.h"
#include "codeposition.h"

namespace IVD { class Element; }
void printOutAttributes(IVD::Element);

namespace IVD
{

class DisplayItem;
class AttributeSet;

struct ExpressionNode
{
    //basic integer to work with values from keywords.h
    //Accepts: Scalar, UnitPercent, UnitStandard, OperatorPlus, OperatorMinus, OperatorTimes, OperatorDivide
    // and lest we forgets, ScopedValueKey
    int operation;
    
    bool weakBranch;
    bool weakTerm; //Only dependency can be weak;
    
    double val;
    ScopedValueKey extVal;
    
    ExpressionNode(): weakBranch(false), weakTerm(false) {}

    ExpressionNode(const ExpressionNode& other) { this->operator=(other); }

    ExpressionNode& operator=(const ExpressionNode& other)
    {
        operation = other.operation;
        weakBranch = other.weakBranch;
        weakTerm = other.weakTerm;
        val = other.val;
        extVal = other.extVal;

        left.reset();
        right.reset();

        if(other.left)  left  = std::make_unique<ExpressionNode>(*other.left);
        if(other.right) right = std::make_unique<ExpressionNode>(*other.right);

        return *this;
    }

    IRISUTILS_DEFINE_COMP(ExpressionNode, operation, weakBranch, weakTerm, val, extVal)

    std::unique_ptr<ExpressionNode> left;
    std::unique_ptr<ExpressionNode> right;

    void initLeft()  { left  = std::make_unique<ExpressionNode>(); }
    void initRight() { right = std::make_unique<ExpressionNode>(); }

    std::string printoutThySelf() const;
};

class Expression
{
    ExpressionNode root;

    double solveNode(const ExpressionNode* theNode, DisplayItem* theContext) const;
    double solveForUnknownNode(const ExpressionNode* theNode,
                               DisplayItem* theContext,
                               const double requiredResult) const;

public:
    Expression() {}

    Expression(const ExpressionNode& sourceRoot):
        root(sourceRoot)
    {}

    Expression(const ExpressionNode& sourceRoot, const CodePosition codepos):
        root(sourceRoot),
        definedAt(codepos)
    {}

    //Rename this to unleash dragons.
    CodePosition definedAt;
    
    void setRootNode(const ExpressionNode& sourceRoot)
    { root = sourceRoot; }

    bool checkContainsWeak()
    { return root.weakTerm || root.weakBranch; }

    double solve(DisplayItem* theContext) const
    {
        return solveNode(&root, theContext);
    }

    void solveForAndPropogateWeak(DisplayItem* theContext, const double requiredResult) const
    {
        solveForUnknownNode(&root, theContext, requiredResult);
    }

    void applyToEachScopedValueKey(std::function<void(ScopedValueKey&)> fun);
    
    //But it doesn't compare context, eef... Context is really just a cache mechanism.
    IRISUTILS_DEFINE_COMP(Expression, root)
    
    std::string generatePrintout();
};

typedef std::map<ValueKey, Expression> DefineContainer;
typedef std::map<ScopedValueKey, Expression> SetContainer;


}//IVD

#endif // EXPRESSION_H
