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

#ifndef COMPILER_H
#define COMPILER_H

#include <list>

#include "element.h"

#include "graph.h"
#include "codeposition.h"

#include <sstream>

namespace IVD
{

struct ScopedVariables
{
    DefineContainer defines;
    std::map<ValueKey, Animation::Graph> graphs;
};

struct Token
{
    CodePosition codePosition;
    const int symbol;

    std::string userToken;
    std::string userString;
    double userNumber;
    Color userColor;

    Token(const CodePosition codePosition, const int sym):
        codePosition(codePosition),
        symbol(sym)
    {}
};

class UnexpectedEOF : public std::exception {};
class SyntaxError : public std::exception
{
    const CodePosition pos;

    //I understand that std::string can throw, but this error object is
    // only thrown on syntax errors, not exactly a likely time for
    // an std::string to throw... The problem is that I want to construct
    // error messages conditionally and c strings won't work.
    //We could store the state to regenerate them but ehhh...
    std::optional<std::string> errStr;

    struct Expecting
    {
        int expected;
        int got;
    };
    std::optional<Expecting> expecting;
    std::optional<CodePosition> conflictsWith;

public:
    SyntaxError(const Token theTok, const int expected):
        pos(theTok.codePosition),
        expecting({expected, theTok.symbol})
    {}

    SyntaxError(const Token theTok, const std::string errStr):
        pos(theTok.codePosition),
        errStr(errStr)
    {}

    SyntaxError(const CodePosition pos): pos(pos) {}

    SyntaxError(const CodePosition pos, const std::string errStr):
        pos(pos),
        errStr(errStr)
    {}

    SyntaxError(const std::string errStr,
                const CodePosition pos,
                const CodePosition conflictsWith):
        errStr(errStr),
        pos(pos),
        conflictsWith(conflictsWith)
    {}

    std::string printout(const char* context) const;
};

class Compiler
{
    int lastElementStamp;

    std::vector<std::string> errorMessages;
    //Warnings?

    std::map<std::string, CodePosition> elementNamePositions;
    std::list<Element> finalizedElements;

    enum class ElementType
    {
        IsClass,
        IsRemora,
        IsInstance,
    };

    struct UserToken
    {
        CodePosition pos;
        std::string value;

        UserToken& operator=(const Token& tok)
        {
            pos = tok.codePosition;
            value = tok.userToken;
            return *this;
        }
    };

    struct ElementPrecursor
    {
        CodePosition codePosition;
        int stamp;
        int lastVirtualStateStamp;

        Element elem;
        ElementType myType;
        std::string name;
        UserToken hostName;
        ScopedValueKey currentState; //Doesn't need to be here.

        std::vector<UserToken> myClasses;

        ScopedValueKey myModel;
        std::vector<ElementPrecursor> remoras;
        
        ScopedVariables vars;

        ElementPrecursor(const CodePosition codePosition, const int stamp):
            codePosition(codePosition),
            stamp(stamp),
            lastVirtualStateStamp(0),
            currentState(),
            myModel()
        {}

        int getFreshVirtualStateStamp()
        { return lastVirtualStateStamp++; }

        void autoGenerateRandomName()
        {
            std::stringstream ss;
            std::stringstream generatedName;
            generatedName << "anonymous-" << stamp << "-HORRIBLY-MAANGLED";
            name = generatedName.str();
        }
    };

    int getFreshElementStamp()
    { return lastElementStamp++; }

    std::map<std::string, ElementPrecursor> myClasses;
    //The idea is that the classes can persist across
    // multiple source files, but elements cannot.
    //And that elements can be accessed directly by
    // the user, but classes cannot.

    std::vector<SyntaxError> myErrors; //I mean, they're the user's, but...

    void postSyntaxError(const SyntaxError& err) { myErrors.push_back(err); }
    void nameConflict(const std::string err,
                      const CodePosition pos1,
                      const CodePosition pos2)
    { postSyntaxError(SyntaxError(err.c_str(), pos1, pos2)); }

    std::vector<Token> tokenizeInput(const std::string code);
    std::vector<ElementPrecursor> parseTokens(std::vector<Token> tokens);
    void finalizePrecursors(std::vector<ElementPrecursor> precursors);

public:
    Compiler(): lastElementStamp(0) {}

    bool compileFile(const char* path);
    void compile(std::string code);

    const std::list<Element>& getElements()
    { return finalizedElements; }

    std::vector<std::string> getErrorMessages()
    { return errorMessages; }
};

}//IVD

#endif // COMPILER_H
