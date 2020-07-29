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

#include "compiler.h"
#include "statekey.h" //For the << path thing

#include <iostream>
#include <string>
#include <sstream>

using IVD::operator<<; //Never done that before.


void printOutAttributes(IVD::Element elem)
{
    std::cout << "=================================Element========================================" << std::endl;
    std::cout << "Path: " << elem.getPath() << std::endl;

    if(elem.getModelPath().size())
        std::cout << "Model " << elem.getModelPath() << std::endl;

    //Now the fun part...
    std::cout << "------------------------------------------Body" << std::endl;

    auto printAttrs = [&](const IVD::ReferenceAttributeSet& attrSet, std::ostream& cout)
    {
        if(attrSet.declareModifiers.size())
        {
            for(auto pair : attrSet.declareModifiers)
                cout << "--Declared Variable: " << pair.first << std::endl
                          << pair.second.generatePrintout() << std::endl;
        }

        if(attrSet.setModifiers.size())
        {
            for(auto pair : attrSet.setModifiers)
                cout << "--Set Target: " << pair.first.generatePrintout() << std::endl
                          << pair.second.generatePrintout() << std::endl;
        }

        for(int i = 0; i != IVD::AttributeKey::AttributeCount; ++i)
        {
            auto attr = attrSet.attr[i];
            if(!attr.active) continue;

            cout << "------------Attr Key: " << IVD::getLiteralForSymbol(i) << std::endl;

            if(attr.delay)
                cout << "Delay: " << *attr.delay << "ms" << std::endl;

            if(attr.ease)
                cout << "Ease-In: " << attr.ease->generatePrintout() << std::endl;

            if(attr.clear)
                cout << "Clear Inherit" << std::endl;

            if(attr.property)
                cout << "Property: " << IVD::getLiteralForSymbol(*attr.property) << std::endl;

            
            if(attr.starting)
                cout << "Starting Expression" << std::endl
                          << attr.starting->generatePrintout() << std::endl;
            
            if(attr.min)
                cout << "Min Constraint" << std::endl
                          << attr.min->generatePrintout() << std::endl;
            
            if(attr.max)
                cout << "Max Constraint" << std::endl
                          << attr.max->generatePrintout() << std::endl;
            
            if(attr.expr)
                cout << "Main Constraint" << std::endl
                          << attr.expr->generatePrintout() << std::endl;


            if(attr.color)
                cout << "Color: " << attr.color->generateHexPrint() << " ("
                          << attr.color->generateDecPrint() << ")" << std::endl;

            
            if(attr.literal)
                cout << "Literal: \"" << *attr.literal  << "\"" << std::endl;

            if(attr.singleKey)
                cout << "Single Key: " << attr.singleKey->generatePrintout() << std::endl;

            if(attr.keys.size())
            {
                cout << "--Key List" << std::endl;
                for(auto key : attr.keys)
                    cout << key.generatePrintout() << std::endl;
            }
            
            if(attr.literalList.size())
            {
                cout << "--Literal List" << std::endl;
                for(auto literal : attr.literalList)
                    cout << literal << std::endl;
            }
        }
    };

    std::map<IVD::ScopedValueKey, IVD::VirtualStateKeyPrecursor> virtualStates;

    for(auto vskp : elem.getVirtualKeys())
        virtualStates[vskp.proxyStateKeyPrecursor] = vskp;

    std::cout << "-----------------------------State default" << std::endl;
    printAttrs(elem.getDefaultAttr(), std::cout);
    
    std::vector<std::string> orderedStatePrintouts;
    orderedStatePrintouts.insert(orderedStatePrintouts.end(), elem.getKeyedAttributeMap().size(), "");

    for(auto pair : elem.getKeyedAttributeMap())
    {
        auto vskpi = virtualStates.find(pair.first);
        const int veryImportantOrdinalPosition = pair.second;

        std::stringstream cout;

        cout << "-----------------------------";
        if(vskpi == virtualStates.end())
            cout << "State Key: " << pair.first.generatePrintout() << std::endl;
        else
            cout << "State Expression: " << std::endl
                 << vskpi->second.generatePrintout() << std::endl; // >:3c

        printAttrs(*elem.at(pair.second).attrs, cout);
        orderedStatePrintouts.at(veryImportantOrdinalPosition) = cout.str();
    }

    for(const std::string& printout : orderedStatePrintouts) std::cout << printout;
}

int main(int argc, char** argv)
{
    //Get args
    if(argc == 1)
    {
        std::cout << "Usage: ivdserialcompiler sourcefile.ivd" << std::endl;
        return 0;
    }
    else if(argc != 2)
    {
        std::cout << "Invalid number of arguments" << std::endl;
        return 0;
    }

    IVD::Compiler comp;
    comp.compileFile(argv[1]);

    if(comp.getErrorMessages().size())
    {
        std::cout << "The IVD compiler has encountered the following "
                  << (comp.getErrorMessages().size() == 1 ? "error:" : "errors")
                  << std::endl;
        for(auto msg : comp.getErrorMessages()) std::cout << msg << std::endl;

        std::cout << "---------------------------------------------------" << std::endl;
    }

    for(IVD::Element elem : comp.getElements()) printOutAttributes(elem);

    return 0;
}
