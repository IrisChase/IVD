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

#include <string>
#include <optional>
#include <memory>
#include <functional>
#include <sstream>
#include <assert.h>

namespace IVD
{

template<typename Node>
std::string generateExpressionPrintout(const Node& root)
{
    //Form the tree
    //*___
    //    |
    // ___/___
    //|       |
    //3    ___*____
    //    |        |
    //    4   [other.key]

    const int minPadding = 3;
    const int minWidth = 7;
    const int barPadding = 3;

    struct ShadowNode
    {
        std::string literal;

        int width;
        int centerOffset;
        int intraNodePadding;
        int barWidth;

        std::unique_ptr<ShadowNode> left;
        std::unique_ptr<ShadowNode> right;

        bool isLeaf() const
        { return !left && !right; }
    };

    std::function<ShadowNode(const Node&)> createShadowTree = [&](const Node& node) -> ShadowNode
    {
        ShadowNode shadowNode;

        shadowNode.literal = node.printoutThySelf();

        if(node.left)  shadowNode.left  = std::make_unique<ShadowNode>(createShadowTree(*node.left));
        if(node.right) shadowNode.right = std::make_unique<ShadowNode>(createShadowTree(*node.right));

        assert(node.left && node.right || !node.left && !node.right);

        if(shadowNode.isLeaf())
        {
            if(shadowNode.literal.size() <= minWidth) shadowNode.width = minWidth;
            else shadowNode.width = shadowNode.literal.size(); //TODO Make unicode safe (ICU)

            shadowNode.intraNodePadding = 0;
            shadowNode.centerOffset = shadowNode.width / 2;
        }
        else
        {
            const int leftWidth  = shadowNode.left->width;
            const int leftCenter = shadowNode.left->centerOffset;

            const int rightWidth  = shadowNode.right->width;
            const int rightCenter = shadowNode.right->centerOffset;

            const int naturalWidth = leftWidth + rightWidth + minPadding;

            if(naturalWidth < minWidth)
            {
                shadowNode.width = minWidth;
                shadowNode.intraNodePadding = minPadding - (leftWidth + rightWidth);
            }
            else
            {
                shadowNode.width = naturalWidth;
                shadowNode.intraNodePadding = minPadding;
            }

            shadowNode.barWidth = leftWidth - leftCenter + shadowNode.intraNodePadding + rightCenter;

            //Make a quick adjustment for people that think it's cool to have an operator
            // longer than 1 character.
            const int minBarWidth = barPadding * 2 + shadowNode.literal.size();
            if(shadowNode.barWidth < minBarWidth)
            {
                const int difference = minBarWidth - shadowNode.barWidth;

                shadowNode.width += difference;
                shadowNode.intraNodePadding += difference;
                shadowNode.barWidth = minBarWidth;
            }

            shadowNode.centerOffset = shadowNode.barWidth / 2 + leftCenter;
        }

        return shadowNode;
    };

    const ShadowNode shadowTree = createShadowTree(root);

    struct Matrix
    {
        int x;
        int y;
        std::vector<std::string> lines;
    };
    std::vector<Matrix> myMatrices;

    std::function<void(const ShadowNode&, const int, const int)> developMatrices =
    [&](const ShadowNode& myNode, const int xoffset, const int yoffset) -> void
    {
        Matrix myMatrix;
        myMatrix.x = xoffset;
        myMatrix.y = yoffset;

        const int nodeYoffset = yoffset + 2;

        myMatrix.lines.emplace_back();
        std::string& firstLine = myMatrix.lines.back();

        if(!myNode.isLeaf())
        {
            std::string secondLine;

            firstLine.append(myNode.centerOffset, ' ');
            firstLine.push_back('|');

            const int childLeftOffset = myNode.left->centerOffset;

            secondLine.append(childLeftOffset, ' ');
            secondLine.append(myNode.barWidth + 1, '.'); //+ 1 because truncation

            const int opPosition = myNode.centerOffset - myNode.literal.size() / 2;
            secondLine.replace(opPosition, myNode.literal.size(), myNode.literal);

            myMatrix.lines.push_back(secondLine);
        }
        else
        {
            firstLine.append(myNode.centerOffset, ' ');
            firstLine.push_back('|');
            myMatrix.lines.push_back(myNode.literal);
        }

        myMatrices.push_back(myMatrix);

        if(!myNode.isLeaf())
        {
            developMatrices(*myNode.left, xoffset, nodeYoffset);
            developMatrices(*myNode.right,
                            xoffset + myNode.left->width + myNode.intraNodePadding,
                            nodeYoffset);
        }
    };

    developMatrices(shadowTree, 0, 0);

    std::vector<std::string> lineBuffer;

    //Composite matrices
    for(Matrix myMatrix : myMatrices)
    {
        while(myMatrix.lines.size() + myMatrix.y > lineBuffer.size())
            lineBuffer.emplace_back();

        for(int i = 0; i != myMatrix.lines.size(); ++i)
        {
            const int yoffset = myMatrix.y + i;
            std::string& outputLine = lineBuffer[yoffset];
            std::string& lineSection = myMatrix.lines[i];
            const int xoffset = myMatrix.x;

            if(lineSection.size() + xoffset > outputLine.size())
                outputLine.append(lineSection.size() + xoffset - outputLine.size(), ' ');

            outputLine.replace(xoffset, lineSection.size(), lineSection);
        }
    }

    //Finally. :)
    std::stringstream printout;
    for(const std::string line : lineBuffer) printout << line << std::endl;

    return printout.str();
};

}//IVD
