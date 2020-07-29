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

#include "expression.h"

#include <iostream>

#include "environment.h"
#include "binaryexpressionprinter.h"


namespace IVD
{

std::string ExpressionNode::printoutThySelf() const
{
    std::string literal; //not to be confused with a standard string literal

    switch(operation)
    {
    case Keyword::ScalarType:
        literal = std::to_string(val);
        literal.push_back('i');
        break;
    case Keyword::UnitPercent:
        literal = std::to_string(val);
        literal.push_back('%');
        break;
    case Keyword::UnitStandard:
        literal = std::to_string(val);
        literal.push_back('u');
        break;
    case IVD::Keyword::ScopedValueKey:
        if(weakTerm) literal = "[";
        literal += extVal.generatePrintout();
        if(weakTerm) literal += "]";
        break;
    case IVD::Keyword::OperatorPlus:
        literal = "(+)";
        break;
    case IVD::Keyword::OperatorMinus:
        literal = "(-)";
        break;
    case IVD::Keyword::OperatorTimes:
        literal = "(*)";
        break;
    case IVD::Keyword::OperatorDivide:
        literal = "(/)";
        break;
    default: assert(false);
    }

    return literal;
}

double Expression::solveNode(const ExpressionNode* theNode, DisplayItem* theContext) const
{
    if(!theNode) return 0;

    const double left =  solveNode(theNode->left.get(), theContext);
    const double right = solveNode(theNode->right.get(), theContext);

    switch(theNode->operation)
    {
    case Keyword::UnitPercent: //Should probably do something with this...
    case Keyword::UnitStandard:
    case Keyword::ScalarType: return theNode->val;
    case Keyword::ScopedValueKey:
        assert(theContext);
        return theContext->getEnv()->getInteger(theContext, theNode->extVal);
    case Keyword::OperatorPlus:
        return left + right;
    case Keyword::OperatorMinus:
        return left - right;
    case Keyword::OperatorTimes:
        return left * right;
    case Keyword::OperatorDivide:
        return left / right;
    default: throw std::logic_error("Unsupported operator in solveNode. This can't happen.");
    }
}

double Expression::solveForUnknownNode(const ExpressionNode* theNode,
                                       DisplayItem* theContext,
                                       const double requiredResult) const
{
    if(theNode->weakTerm)
    {
        assert(theContext);
        theContext->getEnv()->setInteger(theContext, theNode->extVal, requiredResult);
        return requiredResult;
    }

    if(!theNode->weakBranch)
    {
        std::cerr << "IVD Runtime: Attempting to solve for unknown on constant expression." << std::endl;
        return 0;
    }

    //The compiler should reject any code that leads to this condition, so
    // no real error handling.
    assert(!(theNode->left.get()->weakBranch && theNode->right.get()->weakBranch));

    const bool leftUnknown = theNode->left.get()->weakBranch;

    const double knownValue = [&]
    {
        if(leftUnknown)
            return solveNode(theNode->right.get(), theContext);
        else
            return solveNode(theNode->left.get(), theContext);
    }();

    ExpressionNode* weakNode = [&]
    {
        if(leftUnknown)
            return theNode->left.get();
        else
            return theNode->right.get();
    }();

    //wheee
    return solveForUnknownNode(weakNode, theContext, [&]() -> double
    {
        switch(theNode->operation)
        {
        case Keyword::OperatorPlus:  return requiredResult - knownValue;
        case Keyword::OperatorTimes: return requiredResult / knownValue;

        case Keyword::OperatorMinus:
            if(leftUnknown) return knownValue + requiredResult;
            else            return knownValue - requiredResult;

        case Keyword::OperatorDivide:
            if(leftUnknown) return knownValue * requiredResult;
            else            return knownValue / requiredResult;
        default: throw std::logic_error("Unsupported operator in solveUnknownNode. This can't happen.");
        }
    }());
}

void Expression::applyToEachScopedValueKey(std::function<void (ScopedValueKey &)> fun)
{
    std::function<void(ExpressionNode*)> applicator = [&](ExpressionNode* node)
    {
        if(node->operation == Keyword::ScopedValueKey)
            node->extVal.apply(fun);

        if(node->left)  applicator(node->left.get());
        if(node->right) applicator(node->right.get());
    };

    applicator(&root);
}

std::string Expression::generatePrintout()
{
    return generateExpressionPrintout(root);
}


}//IVD
