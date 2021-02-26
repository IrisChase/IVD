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

#include <iostream>
#include <fstream>
#include <functional>

#include "irisutils/routine.h"

#include "keywords.h"
#include "codeposition.h"


namespace IVD
{

std::string SyntaxError::printout(const char* context) const
{
    const int terminalWidth = 80;
    const int maxArrowLineWidth = terminalWidth / 2;

    auto getPrettyPositionReadout = [&](const CodePosition pos)
    {
        const char* p = context;

        const int columnSize = pos.column + 1;
        const int frontTruncation = columnSize > maxArrowLineWidth ? columnSize - maxArrowLineWidth
                                                                   : 0;

        const int arrowPos = frontTruncation ? maxArrowLineWidth - 1
                                             : pos.column;

        for(int lineCount = 0; *p && lineCount != pos.line; ++p) if(*p == '\n') ++lineCount;

        std::string codeLine;
        for(; *p && *p != '\n'; ++p) codeLine += *p;

        if(frontTruncation)
        {
            codeLine = std::string(codeLine.begin() + frontTruncation, codeLine.end());
            for(int i = 0; i != 3; ++i) codeLine[i] = '.';
        }

        if(codeLine.size() > terminalWidth)
        {
            codeLine = std::string(codeLine.begin(), codeLine.begin() + terminalWidth - 1);

            int i = 0;
            for(auto rit = codeLine.rbegin(); i != 3; ++i) *rit = '.';
        }

        std::string arrowLine;
        while(arrowLine.size() != arrowPos) arrowLine += ' ';
        arrowLine += '^';

        return codeLine + '\n' + arrowLine + '\n';
    };


    std::stringstream myErrStream;

    //Line numbers are 1 based smfh...
    myErrStream << "Syntax error on line " << pos.line + 1 << ":" << std::endl
                << getPrettyPositionReadout(pos);

    if(expecting)
    {
        auto filterExpecting = [&](const int sym)
        {
            //This is for _any_ type that isn't really a type...
            //no idea what the above comment means ~ Iris feb 2021
            //But this switch does seem incomplete.
            switch(sym)
            {
            case Keyword::UserToken: return "User Token";
            case Keyword::ScalarType:    return "Number";
            case Keyword::FloatType:     return "Float";
            default: return "getLiteralForSymbol(sym) TODO LOLOL";
            }
        };

        myErrStream << "Was expecting: \"" << filterExpecting(expecting->expected)
                    << "\", got: \"" << filterExpecting(expecting->got) << "\"" << std::endl;
    }
    else if(errStr && conflictsWith)
    {
        myErrStream << *errStr << " conflicts with previous definition on line: "
                    << conflictsWith->line + 1 << ":" << std::endl
                    << getPrettyPositionReadout(*conflictsWith);
    }
    else if(errStr)
    {
        myErrStream << *errStr << std::endl;
    }

    return myErrStream.str();
}

std::vector<Token> Compiler::tokenizeInput(const std::string code)
{
    auto pos = code.begin();


    CodePosition currentPos{0, 0};


    auto notEnd = [&]
    { return pos != code.end(); };

    auto reachedEnd = [&]
    { return !notEnd(); };

    auto checkCurrentIsWhitespace = [&]() -> bool
    { return notEnd() && (*pos == '\r' || *pos == '\n' || *pos == '\t' || *pos == '\f' || *pos == '\v' || *pos == ' '); };

    auto nextChar = [&]()
    {
        if(!notEnd()) return false; //If not not end? Why not not?

        if(*pos == '\n')
        {
            while(notEnd() && *pos == '\n')
            {
                ++pos;
                ++currentPos.line;
                currentPos.column = 0;
            }
        }
        else
        {
            ++currentPos.column;
            ++pos;
        }

        return notEnd();
    };

    auto checkNextIsEq = [&](const char myChar) -> bool
    {
        return notEnd() && pos + 1 != code.end() && *(pos + 1) == myChar;
    };

    auto compareStringAtPos = [&](const std::string myString) -> bool
    {
        if(reachedEnd()) return false;

        auto ipos = pos;
        auto spos = myString.begin();
        while(ipos != code.end() && spos != myString.end())
        {
            if(*ipos != *spos) return false;

            ++ipos;
            ++spos;
        }
        return spos == myString.end();
    };

    std::vector<Token> myTokens;


    //TODO it's inconsistent here. Some matchers store the starting code positions
    // and this first one just creates a token off the bat, which gets the current position,
    // which I believe to be more elegant... TODO

    auto matchLiteral = [&]()
    {
        if(reachedEnd()) return false;

        if(*pos == '"')
        {
            Token myToken(currentPos, Keyword::UserString);

            while(nextChar())
            {
                if(*pos == '\\' && checkNextIsEq('"'))
                {
                    nextChar(); //Eat the slash, the loop eats the quote.
                    continue;
                }

                if(*pos == '"')
                {
                    nextChar();
                    break;
                }

                myToken.userString += *pos;
            }

            if(myToken.userString.size()) //We ignore empty strings, apparently.
                myTokens.push_back(myToken);

            return true;
        }
        return false;
    };

    auto matchWhitespace = [&]
    {
        if(!checkCurrentIsWhitespace()) return false;
        while(nextChar() && checkCurrentIsWhitespace());
        return true;
    };

    auto matchComment = [&]()
    {
        if(reachedEnd()) return false;

        if(*pos == '/' && checkNextIsEq('/'))
        {
            while(nextChar() && *pos != '\n');
            return true;
        }
        return false;
    };

    auto matchColor = [&]()
    {
        if(reachedEnd()) return false;

        if(*pos != '#') return false;

        const CodePosition startPos = currentPos;

        std::string hexNumber;
        while(nextChar() && std::isxdigit(*pos))
            hexNumber += *pos;

        if(pos == code.end() || hexNumber.size() != 6) return false;

        Color theColor;

        auto it = hexNumber.begin();

        theColor.red = std::stoi(std::string(it, it + 2), 0, 16);
        it += 2;
        theColor.green = std::stoi(std::string(it, it + 2), 0, 16);
        it += 2;
        theColor.blue = std::stoi(std::string(it, it + 2), 0, 16);

        Token myToken(startPos, Keyword::ColorLiteral);
        myToken.userColor = theColor;

        myTokens.push_back(myToken);

        return true;
    };

    auto matchDelimitingSymbol = [&]()
    {
        if(reachedEnd()) return false;

        std::optional<int> symbol;

        const CodePosition startPos = currentPos;

        if(*pos == '-' && checkNextIsEq('>'))
        {
            symbol = Keyword::Arrow;
            nextChar(); //Eats an extra one
        }
        else symbol = getSymbolForLiteral(std::string({*pos}));

        if(symbol && checkIsDelimitingSymbol(*symbol))
        {
            myTokens.emplace_back(startPos, *symbol);
            nextChar();
            return true;
        }
        return false;
    };

    auto matchNumber = [&]()
    {
        if(reachedEnd()) return false;

        std::string number; //Yeah, that's not confusing at all

        const CodePosition startPos = currentPos;

        auto consumeDigits = [&]
        {
            bool worked = false;
            for(; notEnd() && std::isdigit(*pos); nextChar())
            {
                number += *pos;
                worked = true;
            }
            return worked;
        };

        if(!consumeDigits()) return false;

        if(notEnd() && *pos == '.')
        {
            //Not sure if this is strictly necessary, but I'm worried
            // starting with just the floating point might conflict with
            // locale rules... But thats questionable with "." too? TODO
            if(number.empty()) number += '0';

            number += '.'; //stod needs this.
            nextChar(); //Eat the dot...

            if(!consumeDigits()) return false;

            Token myToken(startPos, Keyword::FloatType);
            myToken.userNumber = std::stod(number);
            myTokens.push_back(myToken);
            return true;
        }
        else if(number.size())
        {
            Token myToken(startPos, Keyword::ScalarType);
            myToken.userNumber = std::stoi(number);
            myTokens.push_back(myToken);
            return true;
        }
        return false;
    };

    auto matchToken = [&]()
    {
        if(reachedEnd()) return false;

        const CodePosition tokenStartPosition = currentPos;

        std::string tokenString;
        for(; notEnd(); nextChar())
        {
            auto optionalLiteral = getSymbolForLiteral({*pos});
            if(optionalLiteral && checkIsDelimitingSymbol(*optionalLiteral))
                break;

            //God how is this so much simpler than my last approach??
            if(*pos == '-' && checkNextIsEq('>')) break;

            //Ehhh... I don't want slashes in character names, they make even less sense than
            // +... But... Consistency... Aghh...
            if(*pos == '/' && checkNextIsEq('/')) break;

            if(checkCurrentIsWhitespace()) break;

            tokenString += *pos;
        }

        if(tokenString.size())
        {
            //Check whether the token is a reserved keyword or not.
            const auto optional = getSymbolForLiteral(tokenString);

            if(optional)
                myTokens.emplace_back(tokenStartPosition, *optional);
            else
            {
                Token myToken(tokenStartPosition, Keyword::UserToken);
                myToken.userToken = tokenString;
                myTokens.push_back(myToken);
            }

            return true;
        }
        return false;
    };


    for(; notEnd();)
    {
        auto filter = [&](auto fun)
        {
            auto savedPos = pos;
            auto savedCodePos = currentPos;
            if(!fun())
            {
                pos = savedPos;
                currentPos = savedCodePos;
                return false;
            }
            return true;
        };

        //Order here is absolutely arbitrary(?), but it was working before so eh TODO
        if(!filter(matchLiteral) &&
           !filter(matchWhitespace) &&
           !filter(matchComment) &&
           !filter(matchColor) &&
           !filter(matchDelimitingSymbol) &&
           !filter(matchNumber) &&
           !filter(matchToken))
        { break; }
    }

    assert(pos == code.end());

    return myTokens;
}

std::vector<Compiler::ElementPrecursor> Compiler::parseTokens(std::vector<Token> tokens)
{
    auto parserPos = tokens.begin();
    std::vector<ElementPrecursor> precursors;

    auto reachedEnd = [&]() { return parserPos == tokens.end(); };

    auto getCurrentToken = [&]
    {
        if(reachedEnd()) throw(UnexpectedEOF());
        return *parserPos;
    };

    auto matchSymbolSomft = [&](const int symbol)
    {
        if(reachedEnd()) return false;
        return parserPos->symbol == symbol;
    };
    
    auto matchSymbolHard = [&](const int symbol)
    {
        if(parserPos->symbol != symbol) throw SyntaxError(getCurrentToken(), symbol);
    };

    auto next = [&]
    {
        if(reachedEnd()) return false;
        ++parserPos;
        return !reachedEnd();
    };

    auto nextHard = [&]()
    {
        if(!next())
            throw UnexpectedEOF();
    };

    auto matchNextAdvance = [&](const int symbol)
    {
        nextHard();
        matchSymbolHard(symbol);
    };

    auto peekSymbolSequence = [&](const std::vector<int> symbols)
    {
        auto iit = symbols.begin();
        auto sit = parserPos;

        while(iit != symbols.end() && sit != tokens.end())
        {
            if(*iit != sit->symbol) return false;
            ++iit;
            ++sit;
        }
        return true;
    };

    auto advanceTo = [&](const std::vector<int> delimiters)
    {
        //TODO definitely print where we currently are, and also maybe say "advancing to line/column" ? :)

        while(next()) for(const int del : delimiters) if(matchSymbolSomft(del)) return;
    };


    //Factory function becauseeeeeee it seems to change all the time
    auto allocateElementPrecursor = [&]()
    {
        return ElementPrecursor(getCurrentToken().codePosition,
                                getFreshElementStamp(),
                                attrKeyToBodyTypeMap.size());
    };



    //-----------------Let the parsing  e X t R a V a G a N z A  BEGIN!-------------------------
    //Free defines are limited in scope to the current translation unit. Should be easy
    // enough to add an "export" function if need be.
    ScopedVariables varsForTranslationUnit;

    std::function<Expression()> parseExpression;

    auto parseValueKeyScopeAndPath = [&]
    {
        //This function. On purpose. Ignores a single token.
        //This is because the rules for how to deal with a single one vary
        //based on what it is for. Expression value keys need the single one
        // to be the key, others need it to be part of the path. The user is expected
        // to handle this case.
        ScopedValueKey result;
        result.definedAt = getCurrentToken().codePosition;

        if(peekSymbolSequence({Keyword::UserToken, Keyword::Dot}) ||
           peekSymbolSequence({Keyword::UserToken, Keyword::Colon, Keyword::Colon}))
        {
            //Where else would it point to?
            result.myScope = ScopedValueKey::Scope::Global;
            result.addToPath(getCurrentToken().userToken);

            //userToken (:: | .) //That's... a lot of nipples...
            nextHard();
        }
        else if(matchSymbolSomft(Keyword::Model))
        {
            result.myScope = ScopedValueKey::Scope::Model;
            nextHard();
        }
        else if(matchSymbolSomft(Keyword::This))
        {
            result.myScope = ScopedValueKey::Scope::Element;
            nextHard();
        }
        else if(matchSymbolSomft(Keyword::SchoolOperator))
        {
            result.myScope = ScopedValueKey::Scope::RemorialClass;
            nextHard();
        }
        else if(matchSymbolSomft(Keyword::Material))
        {
            result.myScope = ScopedValueKey::Scope::Material;
            nextHard();
        }
        else if(peekSymbolSequence({Keyword::Colon, Keyword::Colon}))
        {
            result.myScope = ScopedValueKey::Scope::Global;

            if(!peekSymbolSequence({Keyword::Colon, Keyword::Colon, Keyword::UserToken}))
            {
                next();
                nextHard();
            }
        }

        while(peekSymbolSequence({Keyword::Colon, Keyword::Colon, Keyword::UserToken}))
        {
            next();
            next();
            result.addToPath(getCurrentToken().userToken);
            next();
        }

        return result;
    };

    auto parseScopedValueKeyForExpression = [&]
    {
        ScopedValueKey result = parseValueKeyScopeAndPath();

        if(matchSymbolSomft(Keyword::Material))
        {
            nextHard();
            matchSymbolHard(Keyword::Dot);
            nextHard();
            matchSymbolHard(Keyword::UserToken);

            result.setKey(getCurrentToken().userToken);

            nextHard();

            matchSymbolHard(Keyword::OpenParen);

            result.myScope = ScopedValueKey::Scope::Material;
            result.parameter = std::make_unique<Expression>(parseExpression());

            matchSymbolHard(Keyword::CloseParen);
            next();
            return result;
        }

        if(!result.empty())
        {
            matchSymbolHard(Keyword::Dot);
            nextHard();
        }

        if(matchSymbolSomft(Keyword::UserToken))
        {
            result.setKey(getCurrentToken().userToken);
        }
        else
        {
            const int sym = getCurrentToken().symbol;

            if(!attrKeyToBodyTypeMap.at(sym).expression)
                throw SyntaxError(getCurrentToken(), "Invalid value for key, expected user token or attribute key.");

            result.setKey(getLiteralForSymbol(sym));
        }

        next();

        result.setScopeIfUnset(ScopedValueKey::Scope::Element);
        return result;
    };

    auto parseScopedValueKeyForStateKey = [&]
    {
        ScopedValueKey result = parseValueKeyScopeAndPath();

        if(!result.empty())
        {
            matchSymbolHard(Keyword::Dot);
            nextHard();
        }

        matchSymbolHard(Keyword::UserToken);
        result.setKey(getCurrentToken().userToken);

        next();

        result.setScopeIfUnset(ScopedValueKey::Scope::Element);
        return result;
    };

    auto parseScopedValueKeyForPositionWithin = [&]
    {
        ScopedValueKey result = parseValueKeyScopeAndPath();

        if(result.myScope &&
           result.myScope != ScopedValueKey::Scope::Global &&
           result.myScope != ScopedValueKey::Scope::RemorialClass)
        {
            throw SyntaxError(result.definedAt, "Invalid scope for position-within, must be global or remorial.");
        }

        if(!result.empty() && matchSymbolSomft(Keyword::Dot))
        {
            nextHard();
            if(!matchSymbolSomft(Keyword::UserToken))
                throw SyntaxError(getCurrentToken().codePosition,
                                  "Position-within requires user token as element or cell key.");

            result.setKey(getCurrentToken().userToken);
            next();
        }
        else if(result.empty())
        {
            matchSymbolHard(Keyword::UserToken);
            result.addToPath(getCurrentToken().userToken);
            next();
        }

        if(result.empty())
            throw SyntaxError(result.definedAt, "Was expecting value key path.");

        result.setScopeIfUnset(ScopedValueKey::Scope::Global);
        return result;
    };

    auto parseScopedValueKeyForModelPath = [&]
    {
        ScopedValueKey result = parseValueKeyScopeAndPath();

        if(result.empty())
        {
            matchSymbolHard(Keyword::UserToken);
            result.addToPath(getCurrentToken().userToken);
            next();
        }

        result.setScopeIfUnset(ScopedValueKey::Scope::Global);

        switch(*result.myScope)
        {
        case ScopedValueKey::Scope::Material:
        case ScopedValueKey::Scope::Element:
            throw SyntaxError(result.definedAt, "Invalid scope for model path. Must be remorial, generic model or global.");
        default: break;
        }

        return result;
    };

    std::function<ExpressionNode(const int)> parseExpressionForNode = [&](const int depth)
    {
        auto parseValueNode = [&]
        {
            if(matchSymbolSomft(Keyword::OpenParen))
            {
                nextHard();
                return parseExpressionForNode(depth + 1);
            }

            ExpressionNode unitNode;

            if(matchSymbolSomft(Keyword::OpenSquare))
            {
                unitNode.weakTerm = true;
                nextHard();
            }

            if(matchSymbolSomft(Keyword::ScalarType) || matchSymbolSomft(Keyword::FloatType))
            {
                if(unitNode.weakTerm)
                {
                    throw SyntaxError(getCurrentToken(), "Only non-constant terms may be [weak].");
                }

                unitNode.val = getCurrentToken().userNumber;
                unitNode.operation = Keyword::ScalarType; //Always because yes.

                next();

                if(matchSymbolSomft(Keyword::UnitStandard) ||
                   matchSymbolSomft(Keyword::UnitPercent))
                {
                    unitNode.operation = getCurrentToken().symbol;
                    next();
                }
                else if(matchSymbolSomft(Keyword::UnitSeconds) ||
                        matchSymbolSomft(Keyword::UnitMilliseconds))
                {
                    throw SyntaxError(getCurrentToken(), "Time units are not valid in scalar expression.");
                }
            }
            else
            {
                try
                {
                    unitNode.operation = Keyword::ScopedValueKey;
                    unitNode.extVal = parseScopedValueKeyForExpression();
                }
                catch(SyntaxError&)
                {
                    throw SyntaxError(getCurrentToken(), "Malformed Expression, expected Scalar or Value Key.");
                }
            }

            if(unitNode.weakTerm)
            {
                matchSymbolHard(Keyword::CloseSquare);
                next();
            }

            return unitNode;
        };
        
        std::vector<ExpressionNode> nodes;

        nodes.push_back(parseValueNode());
        
        while(matchSymbolSomft(Keyword::OperatorPlus)  ||
              matchSymbolSomft(Keyword::OperatorMinus) ||
              matchSymbolSomft(Keyword::OperatorTimes) ||
              matchSymbolSomft(Keyword::OperatorDivide))
        {
            ExpressionNode operationNode;
            operationNode.operation = getCurrentToken().symbol;
            nodes.push_back(operationNode);
            
            nextHard();
            
            nodes.push_back(parseValueNode());
        }
        
        if(matchSymbolSomft(Keyword::CloseParen) && depth != 0) next(); //c o n s u m e
        
        auto collapse = [&](const std::vector<int> operators)
        {
            if(!nodes.size()) return;

            auto it = nodes.begin();
            while(true)
            {
                //Entry assumes x (op x)
                //              ^

                auto leftIt = it;

                ++it;

                if(it == nodes.end()) break; //Singlet~

                //Assuming x op x
                //           ^

                auto middleIt = it;

                ++it;
                //Assuming x op x (op x)
                //              ^ //Acceptable re-entry point.

                auto rightIt = it;

                //Note to anyone thinking of hiring me, I'm not necessarily proud of this one liner...
                //Unless you are impressed. Then yeeeeee-haa boy am I proud!
                if([&]{for(const int op : operators) if(middleIt->operation == op) return true; return false; }())
                {
                    middleIt->left  = std::make_unique<ExpressionNode>(*leftIt);
                    middleIt->right = std::make_unique<ExpressionNode>(*rightIt);

                    middleIt = nodes.erase(leftIt);
                    nodes.erase(middleIt + 1);

                    it = middleIt;
                }
            }
        };

        collapse({Keyword::OperatorTimes, Keyword::OperatorDivide});
        collapse({Keyword::OperatorPlus, Keyword::OperatorMinus});

        if(nodes.size() != 1) throw std::logic_error("THIS CAN'T FUCKING HAPPEN");
        
        return nodes.front();
    };

    parseExpression = [&]
    { return Expression(parseExpressionForNode(0), getCurrentToken().codePosition); };

    auto parseGraph = [&](ScopedVariables* scope = nullptr)
    {
        //graph ( linear|smooth , FLOAT @ 0-100 % )
        //nameOfGraph

        //oops TODO
        //normalize-graph( nameOfGraph, ... )

        std::optional<Animation::Graph> optionalResult;

        if(matchSymbolSomft(Keyword::UserToken))
        {
            const ValueKey key = getCurrentToken().userToken;

            auto justBeatIt = [&](auto& container)
            {
                auto it = container.find(key);
                if(it != container.end())
                {
                    optionalResult = it->second;
                    return true;
                }
                return false;
            };

            if(!(scope && justBeatIt(scope->graphs)))
                justBeatIt(varsForTranslationUnit.graphs);

            if(optionalResult) next();

            return optionalResult;
        }

        if(!matchSymbolSomft(Keyword::Graph)) return optionalResult;

        matchNextAdvance(Keyword::OpenParen);
        nextHard();

        int mode;

        if(matchSymbolSomft(Keyword::Linear))      mode = Keyword::Linear;
        else if(matchSymbolSomft(Keyword::Smooth)) mode = Keyword::Smooth;
        else throw SyntaxError(getCurrentToken(), "Graphs require interpolation mode to be explicitly defined.");

        nextHard();

        Animation::Graph result(mode);

        bool gotSample = false;

        while(matchSymbolSomft(Keyword::Comma))
        {
            nextHard();

            auto matchNumber = [&]
            {
                //Must be a Keyword::Float of less than 1, or a scalar of 1 or 0
                // OR a scalar between 0 and 100, followed by a percent sign
                if(!matchSymbolSomft(Keyword::ScalarType) && !matchSymbolSomft(Keyword::FloatType))
                    throw SyntaxError(getCurrentToken(), Keyword::ScalarType);

                const double val = getCurrentToken().userNumber;
                next();

                if(matchSymbolSomft(Keyword::UnitPercent))
                {
                    next();

                    if(val < 0 || val > 100)
                        throw SyntaxError(getCurrentToken(), "Graph samples point percentage out of range.");

                    return val / 100;
                }

                if(val < 0 || val > 1)
                    throw SyntaxError(getCurrentToken(), "Floating point sample for graph out of range.");

                return val;
            };

            const std::string badNumber = "Invalid number range/type";

            const double x = matchNumber();
            matchSymbolHard(Keyword::SchoolOperator);
            nextHard();
            const double y = matchNumber();

            result.addSample(y, x);
            gotSample = true;
        }

        if(!gotSample) throw SyntaxError(getCurrentToken(), "Graphs may not be empty (Why not? idek).");

        matchSymbolHard(Keyword::CloseParen);
        next();

        if(!gotSample) return optionalResult;
        optionalResult = result;
        return optionalResult;
    };

    auto parseSet = [&](SetContainer* sets)
    {
        //Keyword::Set ScopedValueKey Keyword::EqualSign Expression Keyword::Semicolon

        if(!matchSymbolSomft(Keyword::Set)) return false;

        next();

        matchSymbolHard(Keyword::Colon);
        next();

        ScopedValueKey target = parseScopedValueKeyForExpression();

        matchSymbolHard(Keyword::EqualSign);
        next();

        Expression expr = parseExpression();

        matchSymbolHard(Keyword::Semicolon);
        next();

        //Done actually parsing

        auto it = sets->find(target);

        if(it != sets->end())
        {
            nameConflict("Set target", target.definedAt, it->first.definedAt);
            return true;
        }

        //Woof, that was a lot of work...
        sets->operator[](target) = expr;
        return true;
    };

    auto parseDeclare = [&](ScopedVariables* vars, ElementPrecursor* precursor = nullptr)
    {
        if(!matchSymbolSomft(Keyword::Declare)) return false;

        nextHard();

        const int symbol = getCurrentToken().symbol;
        const CodePosition typePos = getCurrentToken().codePosition;

        nextHard();
        matchSymbolHard(Keyword::Colon);
        nextHard();
        matchSymbolHard(Keyword::UserToken);

        const std::string declaredName = getCurrentToken().userToken;

        nextHard();
        matchSymbolHard(Keyword::EqualSign);
        nextHard();

        auto insertIntoExclusiveMap = [&](auto* container, auto val)
        {
            auto it = container->find(declaredName);

            if(it != container->end())
            {
                nameConflict("Variable name", val.definedAt, it->second.definedAt);
                return;
            }

            container->operator[](declaredName) = val;
        };

        switch(symbol)
        {
        case Keyword::Graph:
        {
            auto optionalGraph = parseGraph();

            if(!optionalGraph) throw SyntaxError(getCurrentToken().codePosition,
                                                 "Was expecting graph literal.");

            insertIntoExclusiveMap(precursor ? &precursor->vars.graphs
                                             : &vars->graphs,
                                   *optionalGraph);
            break;
        }
        case Keyword::Expression:
        {
            insertIntoExclusiveMap(precursor ? &precursor->vars.defines
                                             : &vars->defines,
                                   parseExpression());
            break;
        }
        case Keyword::ScalarKeyword:
        {
            if(!precursor) throw SyntaxError(getCurrentToken().codePosition,
                                             "Scalar declarations can only appear within the default "
                                             "state of elements.");

            //TODO: Name collision checks? It seems idiomatic that the last one would just overwrite
            // the previous. Maybe post a warning for the linter
            precursor->elem.setInitialExpression(declaredName, parseExpression());
            break;
        }
        default: throw SyntaxError(typePos, "This type is invalid for this case, you absolute madperson.");
        }

        matchSymbolHard(Keyword::Semicolon);
        next();

        return true;
    };

    auto parseAttributeWithExpressionType = [&](const int key, ReferenceAttribute* attr)
    {
        if(!attrKeyToBodyTypeMap.at(key).expression) return false;

        typedef bool lol;
        lol gotem = false;

        auto processNoTarget = [&]()
        {
            gotem = true;
            attr->active = true;
            return parseExpression();
        };

        auto processConstraintTarget = [&]()
        {
            matchNextAdvance(Keyword::EqualSign);
            nextHard();
            auto optional = processNoTarget();
            return optional;
        };

             if(matchSymbolSomft(Keyword::Start))   attr->starting = processConstraintTarget();
        else if(matchSymbolSomft(Keyword::Min))     attr->min = processConstraintTarget();
        else if(matchSymbolSomft(Keyword::Max))     attr->max = processConstraintTarget();
        else                                        attr->expr = processNoTarget();

        return gotem;
    };

    auto parseTimeInMilliseconds = [&]
    {
        if(!matchSymbolSomft(Keyword::ScalarType)) return std::optional<int>();

        int result = getCurrentToken().userNumber;
        nextHard();

        if(matchSymbolSomft(Keyword::UnitMilliseconds));
        else if(matchSymbolSomft(Keyword::UnitSeconds))
            result = result * 1000;
        else throw SyntaxError(getCurrentToken(), "Was expecting chrono type");

        next();

        return std::optional<int>(result);
    };

    auto parseAttributeDelay = [&](ReferenceAttribute* attr)
    {
        if(!matchSymbolSomft(Keyword::Delay)) return false;
        nextHard();
        matchSymbolHard(Keyword::OpenParen);
        nextHard();

        attr->delay = parseTimeInMilliseconds();

        //Actually, I'm in a forgiving mood. I don't really care if it's empty.
        if(attr->delay) attr->active = true;

        matchSymbolHard(Keyword::CloseParen);
        next();
        return true;
    };

    struct EasePair
    {
        std::optional<Animation::Transition> easeIn;
        std::optional<Animation::Transition> easeOut;
    };

    auto parseAttributeEase = [&](const int key, ScopedVariables* scope)
    {
        std::optional<EasePair> optionalResult;

        if(!attrKeyToBodyTypeMap.at(key).expression) return optionalResult;
        if(!matchSymbolSomft(Keyword::EaseIn) && !matchSymbolSomft(Keyword::EaseOut)) return optionalResult;

        const int easeType = getCurrentToken().symbol;
        nextHard();
        matchSymbolHard(Keyword::OpenParen);
        nextHard();

        auto optionalTime = parseTimeInMilliseconds();
        if(!optionalTime) throw SyntaxError(getCurrentToken(), "Ease statements require time as the first arguement.");

        matchSymbolHard(Keyword::Comma);
        nextHard();
        auto optionalGraph = parseGraph(scope);

        if(!optionalGraph) throw SyntaxError(getCurrentToken(), "Ease statements require a graph. "
                                                         "Use \"delay\" if you just want to delay.");


        matchSymbolHard(Keyword::CloseParen);
        next();

        Animation::Transition myTransition = {*optionalTime, *optionalGraph};

        EasePair result;

        if(easeType == Keyword::EaseIn)       result.easeIn  = myTransition;
        else if(easeType == Keyword::EaseOut) result.easeOut = myTransition;
        else throw std::logic_error("hahahahahahahaahahhh, whew *whipes tear* heh");

        return std::optional<EasePair>(result);
    };

    auto parseAttributeBodyForKey = [&](const int key, ReferenceAttribute* attr)
    {
        const CodePosition startPos = getCurrentToken().codePosition;

        const AttributeBodyTypes types = attrKeyToBodyTypeMap.at(key);

        auto parseStateKeyListType = [&](const int key, ReferenceAttribute* attr)
        {
            if(!types.stateKeyList) return false;

            attr->active = true;
            attr->stateModifierAttr = true;

            attr->keys.push_back(parseScopedValueKeyForStateKey());
            return true;
        };

        auto parseAttributeWithSingleScopedValueKeyType = [&](const int key, ReferenceAttribute* attr)
        {
            if(!types.singleScopedValueKey) return false;

            attr->active = true;
            attr->singleKey = parseScopedValueKeyForExpression();
            return true;
        };

        auto parseAttributeWithUserTokenListType = [&](const int key, ReferenceAttribute* attr)
        {
            if(!types.userTokenList) return false;
            if(!matchSymbolSomft(Keyword::UserToken)) return false;

            attr->active = true;
            attr->literalList.push_back(getCurrentToken().userToken);
            next();

            return true;
        };

        auto parseAttributeWithUserTokenType = [&](const int key, ReferenceAttribute* attr)
        {
            if(!types.userToken) return false;
            if(!matchSymbolSomft(Keyword::UserToken)) return false;

            attr->active = true;
            attr->literal = getCurrentToken().userToken;
            next();

            return true;
        };

        auto parsePositionWithinType = [&](const int key, ReferenceAttribute* attr)
        {
            if(!types.positionWithin) return false;

            attr->active = true;
            attr->singleKey = parseScopedValueKeyForPositionWithin();

            //position within is the only case where a single key isn't a VALUE, but instead a
            // path. This complicates things considerably.

            return true;
        };


        auto parseAttributeWithColorType = [&](const int key, ReferenceAttribute* attr)
        {
            if(!types.color) return false;
            if(!matchSymbolSomft(Keyword::ColorLiteral))
            {
                if(!matchSymbolSomft(Keyword::UserToken)) return false;
                //Colors can now be tokens, and an arbitrary number of them to boot...

                std::string builtColorKey;

                while(matchSymbolSomft(Keyword::UserToken))
                {
                    if(builtColorKey.size()) builtColorKey.push_back(' ');
                    builtColorKey += getCurrentToken().userToken;

                    next();
                }

                attr->active = true;
                attr->color = Color(builtColorKey); //May or may not be correct.
                return true;

            }

            attr->active = true;
            attr->color = getCurrentToken().userColor;
            next();

            return true;
        };

        auto parseAttributeWithStringLiteralType = [&](const int key, ReferenceAttribute* attr)
        {
            if(!types.stringLiteral) return false;
            if(!matchSymbolSomft(Keyword::UserString)) return false;

            attr->active = true;
            attr->literal = getCurrentToken().userString;
            next();

            return true;
        };

        auto parseAttributeWithPropertyType = [&](const int key, ReferenceAttribute* attr)
        {
            if(!types.property) return false;

            const int sym = getCurrentToken().symbol;
            //XXXXXXXXXXXX sym not key!!!! XXX
            // I think we can remove key from all of these then
            if(!(sym >= Property::PropertiesStart && key <= Property::PropertiesEnd)) return false;

            attr->active = true;
            attr->property = sym;
            next();

            return true;
        };

        //parseAttributeWithExpression is the only one that fails hard, but it's also exclusive,
        // an attribute can't***** be an expression type in addition to any other.
        //*****currently...


        //Need to match the appropriate parsers with the key and then have a specific
        // message if it fails~

             if(parseAttributeWithPropertyType(key, attr));
        else if(parseAttributeWithColorType(key, attr));
        else if(parseAttributeWithUserTokenType(key, attr));
        else if(parseAttributeWithStringLiteralType(key, attr));
        else if(parseAttributeWithUserTokenListType(key, attr));
        else if(parsePositionWithinType(key, attr));
        else if(parseStateKeyListType(key, attr));
        else if(parseAttributeWithSingleScopedValueKeyType(key, attr));
        else if(parseAttributeWithExpressionType(key, attr));
        else
        {
            std::stringstream ss;

            ss << "Malformed attribute body, expected ";

            bool doneDidIt = false;
            auto additionally = [&](const std::string str)
            {
                if(doneDidIt) ss << ", or ";
                ss << str;
                doneDidIt = true;
            };

            if(types.stateKeyList)        additionally("State Key List");
            if(types.userTokenList)       additionally("User Token List");
            if(types.userToken)           additionally("User Token");
            if(types.positionWithin)      additionally("Position Within Key");
            if(types.color)               additionally("Color Literal");
            if(types.stringLiteral)       additionally("String Literal");
            if(types.property)            additionally("Property");
            if(types.expression)          additionally("Expression");

            ss << ".";

            throw SyntaxError(startPos, ss.str());
        }
    };

    auto parseUnnaturalAttributeBody = [&](const int key, ReferenceAttributeSet* attrs, ScopedVariables* scope)
    {
        //We just assume we're not merging in, here.
        //key: one;
        //key: one, one, one, one;

        const CodePosition startPos = getCurrentToken().codePosition;
        const int baseKey = unnaturalKeyMap.at(key)[0]; //xxx

        std::vector<std::optional<ReferenceAttribute>> myAttributes;

        while(true)
        {
            if(matchSymbolSomft(Keyword::Pass))
            {
                myAttributes.emplace_back();
                next();
            }
            else if(matchSymbolSomft(Keyword::Clear))
            {
                ReferenceAttribute temp;
                temp.active = true;
                temp.clear  = true;
                myAttributes.emplace_back(temp);
                next();
            }
            else if(parseAttributeEase(key, scope))
                throw SyntaxError(startPos, "Unnatural key cannot contain ease specifiers.");
            else if(ReferenceAttribute dummy; parseAttributeDelay(&dummy))
                throw SyntaxError(startPos, "Unnatural key cannot contain delay specifiers.");
            else
            {
                ReferenceAttribute temp;
                temp.active = true;
                parseAttributeBodyForKey(key, &temp);
                myAttributes.emplace_back(temp);
            }

            if(matchSymbolSomft(Keyword::Comma))
            {
                nextHard();
                continue;
            }
            break;
        }


        if(myAttributes.size() == 1)
        {
            for(const int natKey : unnaturalKeyMap.at(key))
                if(myAttributes.front())
                    attrs->insertAttribute(*myAttributes.front(), natKey);
        }
        else if(myAttributes.size() == unnaturalKeyMap.at(key).size())
        {
            auto it = myAttributes.begin();
            for(const int natKey : unnaturalKeyMap.at(key))
            {
                if(it->has_value()) attrs->insertAttribute(**it, natKey);
                ++it;
            }
        }
        else
        {
            std::stringstream ss;
            ss << "Invalid number of arguments to unatural key "
               << getLiteralForSymbol(key) << ", must be 1 or "
               << unnaturalKeyMap.at(key).size() << ".";
            throw SyntaxError(startPos, ss.str());
        }

        matchSymbolHard(Keyword::Semicolon);
        next();
    };

    auto parseAttribute = [&](ReferenceAttributeSet* attrs, ScopedVariables* scope)
    {
        const int key = getCurrentToken().symbol;

        if(!attrKeyToBodyTypeMap.count(key))
            return false;

        nextHard();
        matchSymbolHard(Keyword::Colon);
        nextHard();

        if(matchSymbolSomft(Keyword::Semicolon))
        {
            next();
            return true;
        }

        if(attrKeyToBodyTypeMap.at(key).unnatural)
        {
            parseUnnaturalAttributeBody(key, attrs, scope);
            return true;
        }

        ReferenceAttribute myAttr = attrs->getAttribute(key);

        while(true)
        {
            if(matchSymbolSomft(Keyword::Clear))
            {
                myAttr.active = true;
                myAttr.clear  = true;
                nextHard();
            }
            else if(matchSymbolSomft(Keyword::Pass)) nextHard(); //Inline parsing lmfao
            else if(auto pair = parseAttributeEase(key, scope))
            {
                myAttr.active = true; //Really?
                if(pair->easeIn)  myAttr.ease = pair->easeIn;
                assert(!pair->easeOut); //Oof just a quickie I'm busy with something else right now.
            }
            else if(parseAttributeDelay(&myAttr));
            else parseAttributeBodyForKey(key, &myAttr);

            if(matchSymbolSomft(Keyword::Comma))
            {
                nextHard();
                continue;
            }

            break;
        }

        matchSymbolHard(Keyword::Semicolon);
        next();

        attrs->insertAttribute(myAttr, key);

        return true;
    };

    typedef VirtualStateKeyPrecursor::Node VstateKeyNode;

    std::function<VstateKeyNode(const int)> parseVirtualStateKey = [&](const int parenLevel)
    {
        //Left node is always either just a ScopedValueKey or a (Node).
        //Right node is always parsed as a full VirtualStateKey
        // so that (1 & 1 & 1) == (1 & (1 & (1)))

        auto parseValueNode = [&]()
        {
            bool negation = false;
            if(matchSymbolSomft(Keyword::Not))
            {
                nextHard();
                negation = true;
            }

            if(matchSymbolSomft(Keyword::OpenParen))
            {
                nextHard();

                VstateKeyNode node = parseVirtualStateKey(parenLevel + 1);

                if(negation) node.negation = !node.negation;
                return node;
            }

            VstateKeyNode node;
            node.operation = Keyword::ScopedValueKey;
            node.negation = negation;
            node.stateKeyPrecursor = parseScopedValueKeyForStateKey();
            return node;
        };

        VstateKeyNode resultNode = parseValueNode();

        while(!matchSymbolSomft(Keyword::Colon) &&
              !matchSymbolSomft(Keyword::CloseParen))
        {
            VstateKeyNode operatorNode;
            operatorNode.left = std::make_unique<VstateKeyNode>(resultNode);

            if(matchSymbolSomft(Keyword::And))      operatorNode.operation = Keyword::And;
            else if(matchSymbolSomft(Keyword::Or))  operatorNode.operation = Keyword::Or;
            else if(matchSymbolSomft(Keyword::Xor)) operatorNode.operation = Keyword::Xor;
            else throw SyntaxError(getCurrentToken(), "Was expecting logical operator for compound state key.");

            next();

            operatorNode.right = std::make_unique<VstateKeyNode>(parseValueNode());
            resultNode = operatorNode;
        }

        if(parenLevel) matchSymbolHard(Keyword::CloseParen);
        else matchSymbolHard(Keyword::Colon);
        next();

        return resultNode;
    };

    auto parseElementBody = [&](ElementPrecursor* precursor)
    {
        if(matchSymbolSomft(Keyword::Semicolon))
        {
            next();
            return;
        }

        if(!matchSymbolSomft(Keyword::OpenBracket))
        {
            const CodePosition codeMarker = getCurrentToken().codePosition;
            postSyntaxError(SyntaxError(codeMarker, "Malformed element body. Was expecting { or ;\n"
                                                    "Attempting to make sense of this..."));

            advanceTo({Keyword::OpenBracket, Keyword::CloseBracket});
            if(Keyword::CloseBracket)
            {
                next();
                return;
            }
        }

        matchSymbolHard(Keyword::OpenBracket);
        nextHard();

        ScopedValueKey currentStateKey{};

        while(true)
        {
            try
            {
                const bool defaultState = !currentStateKey.key.has_value();
                ReferenceAttributeSet* currentState = &precursor->elem.getAttributeSetForStateKey(currentStateKey);

                if(defaultState)
                {
                    while(parseDeclare(&precursor->vars, precursor) ||
                          parseAttribute(currentState, &precursor->vars));
                }
                else
                {
                    while(parseSet(&currentState->setModifiers) ||
                          parseAttribute(currentState, &precursor->vars));
                }

                if(matchSymbolSomft(Keyword::State))
                {
                    nextHard();

                    VirtualStateKeyPrecursor vstatekey;

                    //Alright furries, hold onto your peets.
                    vstatekey.root = parseVirtualStateKey(0);

                    if(vstatekey.checkIsVirtual()) currentStateKey = vstatekey.root.stateKeyPrecursor;
                    else
                    {
                        currentStateKey = ScopedValueKey{};
                        std::stringstream ss;
                        ss << "generated-virtual-state-" << precursor->getFreshVirtualStateStamp();
                        currentStateKey.myScope = ScopedValueKey::Scope::Element;
                        currentStateKey.key = ss.str();
                        vstatekey.proxyStateKeyPrecursor = currentStateKey;

                        precursor->elem.addVirtualStateKey(vstatekey);
                    }

                    continue;
                }

                if(!matchSymbolSomft(Keyword::CloseBracket))
                {
                    if(matchSymbolSomft(Keyword::Set) && defaultState)
                    {
                        postSyntaxError(SyntaxError(getCurrentToken(), "Set not allowed in default state."));
                        SetContainer dump;
                        parseSet(&dump); //But aye, might as well compile it, eh?
                        continue; //Hey! We can recover ourselves here!
                    }
                    else if(matchSymbolSomft(Keyword::Declare) && !defaultState)
                        throw SyntaxError(getCurrentToken(), "Declare not allowed in non-default state.");
                    else throw SyntaxError(getCurrentToken(), "Token is not a state or variable modifier or attribute key.");
                }

                break;
            }
            catch(const SyntaxError& e)
            {
                postSyntaxError(e);
                //We can look into the symbol and offer hints! HOW CUTE IS THAAAAAT
                //"hint: did you mean [x]?" *squee*
                if(!matchSymbolSomft(Keyword::Semicolon))
                    advanceTo({Keyword::Semicolon, Keyword::CloseBracket, Keyword::State});
                if(matchSymbolSomft(Keyword::Semicolon)) next();
                continue;
            }
        }

        matchSymbolHard(Keyword::CloseBracket);
        next();
    };

    auto tryWithinHeader = [&](auto fun)
    {
        try
        {
            fun();
        }
        catch(SyntaxError& e)
        {
            //TODO would like to add "Advancing to X to the error".
            //Maybe pass the error object to the advanceTo function?

            postSyntaxError(e);
            advanceTo({Keyword::Semicolon, Keyword::OpenBracket, Keyword::CloseBracket});
        }
    };

    auto parseElementHeaderClass = [&](ElementPrecursor* precursor)
    {
        tryWithinHeader([&]
        {
            //: usertoken (, usertoken (...))
            if(!matchSymbolSomft(Keyword::Colon)) return;
            nextHard();

            while(true)
            {
                matchSymbolHard(Keyword::UserToken);
                precursor->myClasses.push_back({getCurrentToken().codePosition, getCurrentToken().userToken});

                next();
                if(matchSymbolSomft(Keyword::Comma))
                {
                    next();
                    continue;
                }
                break;
            }
        });
    };

    auto parseElementHeaderModel = [&](ElementPrecursor* precursor)
    {
        //usertoken (:: usertoken (:: usertoken (...)))
        tryWithinHeader([&]
        {
            if(!matchSymbolSomft(Keyword::Arrow)) return;
            nextHard();

            precursor->myModel = parseScopedValueKeyForModelPath();
        });
    };

    auto parseElement = [&]
    {
        //# (|elementName) (HEADER|BODY)
        if(!matchSymbolSomft(Keyword::Pound)) return false;

        ElementPrecursor precursor(allocateElementPrecursor());
        precursor.myType = ElementType::IsInstance;
        nextHard(); //Commited now boiii

        if(matchSymbolSomft(Keyword::UserToken))
        {
            precursor.name = getCurrentToken().userToken;
            nextHard();
        }
        else
        {
            precursor.autoGenerateRandomName();
        }

        parseElementHeaderModel(&precursor);
        parseElementHeaderClass(&precursor);
        parseElementBody(&precursor);
        precursors.push_back(precursor);

        return true;
    };

    auto parseClass = [&]
    {
        if(!matchSymbolSomft(Keyword::Dot)) return false;

        ElementPrecursor precursor(allocateElementPrecursor());
        precursor.myType = ElementType::IsClass;

        nextHard();
        matchSymbolHard(Keyword::UserToken);

        precursor.name = getCurrentToken().userToken;

        next(); //name

        parseElementHeaderClass(&precursor);
        parseElementBody(&precursor);
        precursors.push_back(precursor);

        return true;
    };

    auto parseRemora = [&]
    {
        //@ remoraHost . USERTOKEN (HEADER|BODY)
        //@remoraHost (header|body) //Anonymous
        if(!matchSymbolSomft(Keyword::SchoolOperator)) return false;
        nextHard();
        matchSymbolHard(Keyword::UserToken);

        ElementPrecursor precursor(allocateElementPrecursor());
        precursor.myType = ElementType::IsRemora;
        precursor.hostName = getCurrentToken();

        next();

        if(peekSymbolSequence({Keyword::Dot, Keyword::UserToken}))
        {
            next();
            precursor.name = getCurrentToken().userToken;
            next();
        }
        else precursor.autoGenerateRandomName();

        parseElementHeaderModel(&precursor);
        parseElementHeaderClass(&precursor);
        parseElementBody(&precursor);
        precursors.push_back(precursor);

        return true;
    };

    try
    {
        if(!matchSymbolSomft(Spec::spec))
            throw SyntaxError(getCurrentToken(), "Expected Spec declaration.");

        nextHard();

        //TODO: Put the version specification somewhere else
        if(!matchSymbolSomft(Spec::bleeding))
            throw SyntaxError(getCurrentToken(),
                              "IVD version mismatch. Current version is ""bleeding"" Aborting...");

        nextHard();
        matchSymbolHard(Keyword::Semicolon);
        nextHard();

        while(!reachedEnd())
        {
                 if(parseElement());
            else if(parseClass());
            else if(parseRemora());
            else if(parseDeclare(&varsForTranslationUnit));
            else
            {
                postSyntaxError(SyntaxError(getCurrentToken(), "Invalid token for base context."));
                break;
            }
        }
    }
    catch(SyntaxError& e)
    {
        postSyntaxError(e);
    }
    catch(UnexpectedEOF& e)
    {
        errorMessages.push_back("Unexpected EOF, aborting...");
    }
    catch(std::logic_error& e)
    {
        std::string es = "Logic error: ";
        es += e.what();
        errorMessages.push_back(es);
    }

    return precursors;
}

void Compiler::finalizePrecursors(std::vector<Compiler::ElementPrecursor> precursors)
{
    std::map<std::string, ElementPrecursor> virginClasses;

    for(ElementPrecursor& preElem : precursors)
    {
        {
            //This is kind of dumb... TODO?
            //later: Uh, I tried... But uh... Is it really that dumb?
            preElem.elem.getDefaultAttr().declareModifiers = preElem.vars.defines;
        }

        //Backwards because Element::deriveFrom is kind of backwards.
        for(auto rit = preElem.myClasses.rbegin(); rit != preElem.myClasses.rend(); ++rit)
        {

            const UserToken parentClassName = *rit;
            {
                //First request for the class finalizes it.
                auto it = virginClasses.find(parentClassName.value);
                virginClasses.count(parentClassName.value);
                if(it != virginClasses.end())
                {
                    //Already deduplicated before inserting into virginClasses.
                    myClasses.emplace(parentClassName.value, it->second);
                    virginClasses.erase(it);
                }
            }

            auto it = myClasses.find(parentClassName.value);
            if(it == myClasses.end())
            {
                postSyntaxError(SyntaxError(parentClassName.pos, "Parent class undefined."));
                continue;
            }

            preElem.elem.deriveFrom(it->second.elem);

            {
                std::map<std::string, CodePosition> names;

                for(auto remora : preElem.remoras)
                {
                    auto it = names.find(remora.name);

                    if(it == names.end()) names[remora.name] = remora.codePosition;
                    else nameConflict("Remora name", remora.codePosition, it->second);
                }
            }

            IrisUtils::Routine::appendContainer(preElem.remoras, it->second.remoras);
        }

        if(preElem.myType == ElementType::IsClass)
        {
            //Name collision cheeeeeeck...
            auto virginIt = virginClasses.find(preElem.name);

            if(virginIt != virginClasses.end())
            {
                nameConflict("Class name",
                             preElem.codePosition,
                             virginIt->second.codePosition);

                continue;
            }

            auto finalIt = myClasses.find(preElem.name);

            if(finalIt != myClasses.end())
            {
                nameConflict("Class name",
                             preElem.codePosition,
                             finalIt->second.codePosition);

                continue;
            }

            virginClasses.emplace(preElem.name, preElem);
        }
        else if(preElem.myType == ElementType::IsRemora)
        {
            auto it = virginClasses.find(preElem.hostName.value);
            if(it == virginClasses.end())
            {
                if(myClasses.find(preElem.hostName.value) != myClasses.end())
                     postSyntaxError(SyntaxError(preElem.hostName.pos,
                                                 "Remora host class already finalized."));
                else postSyntaxError(SyntaxError(preElem.hostName.pos,
                                                 "Remora host class undefined."));
                continue;
            }

            it->second.remoras.emplace_back(preElem);
        }
        else if(preElem.myType == ElementType::IsInstance)
        {
            if(!preElem.myModel.empty() && preElem.myModel.myScope != ScopedValueKey::Scope::Global)
            {
                postSyntaxError(SyntaxError(preElem.myModel.definedAt,
                                            "Cannot enumerate free element across non-global model."));
                continue;
            }

            {
                auto nameIt = elementNamePositions.find(preElem.name);

                if(nameIt != elementNamePositions.end())
                {
                    nameConflict("Element name",
                                 preElem.codePosition,
                                 nameIt->second);
                    continue;
                }
            }

            if(preElem.myModel.path) preElem.elem.setModelPath(*preElem.myModel.path);
            preElem.elem.setPath({preElem.name});

            auto substituteValueKeys = [&](ScopedValueKey& vkey, const ElementPrecursor& parent)
            {
                if(vkey.myScope == ScopedValueKey::Scope::RemorialClass)
                {
                    if(!vkey.path) vkey.path = ValueKeyPath();

                    ValueKeyPath mewPath;
                    mewPath = parent.elem.getPath();
                    IrisUtils::Routine::appendContainer(mewPath, *vkey.path);
                    vkey.path = mewPath;
                    vkey.myScope = ScopedValueKey::Scope::Global;
                }
            };

            auto substituteParentKeys = std::bind(substituteValueKeys, std::placeholders::_1, preElem);

            preElem.elem.applyToEachScopedValueKey(substituteParentKeys);

            std::function<void(ElementPrecursor&)> finalizeRemoras = [&](ElementPrecursor& parentElem)
            {
                for(ElementPrecursor& remora : parentElem.remoras)
                {
                    {
                        ValueKeyPath mewPath = parentElem.elem.getPath();
                        IrisUtils::Routine::appendContainer(mewPath, {remora.name});
                        remora.elem.setPath(mewPath);
                    }

                    remora.elem.applyToEachScopedValueKey(std::bind(substituteValueKeys, std::placeholders::_1, parentElem));

                    //MODEL NAMES, PEOPLE
                    if(!remora.myModel.empty())
                    {
                        if(remora.myModel.myScope == ScopedValueKey::Scope::RemorialClass)
                        {
                            if(parentElem.elem.getModelPath().empty())
                            {
                                postSyntaxError(SyntaxError(parentElem.codePosition, "Remorial instance does not declare a model."));
                                postSyntaxError(SyntaxError(remora.myModel.definedAt, "But remora requires one."));
                            }
                            else
                            {
                                ValueKeyPath modelPath = parentElem.elem.getModelPath();
                                if(remora.myModel.path)
                                    IrisUtils::Routine::appendContainer(modelPath, *remora.myModel.path);
                                remora.elem.setModelPath(modelPath);
                            }
                        }
                        else
                        {
                            throw SyntaxError(remora.myModel.definedAt,
                                              "Attempt to enumerate remora across non-host model.");
                        }
                    }

                    finalizeRemoras(remora);
                }
            };

            try
            {
                finalizeRemoras(preElem);

                std::function<void(ElementPrecursor&)> recursivelyInsert = [&](ElementPrecursor& preElem)
                {
                    elementNamePositions[preElem.name] = preElem.codePosition;

                    preElem.elem.applyToEachScopedValueKey([&](ScopedValueKey& key)
                    {
                        if(key.myScope == ScopedValueKey::Scope::RemorialClass)
                            throw std::logic_error("shit.");

                        if(!preElem.myModel.path &&
                           key.myScope == ScopedValueKey::Scope::Model)
                        {
                            postSyntaxError(SyntaxError(key.definedAt,
                                                        "Model scoped value key in non-enumerated element."));
                        }
                    });

                    finalizedElements.push_back(preElem.elem);
                    for(ElementPrecursor& remora : preElem.remoras)
                        recursivelyInsert(remora);
                };
                recursivelyInsert(preElem);
            }
            catch(SyntaxError& e)
            { postSyntaxError(e); }
        } else throw std::logic_error("Internal error: Unrecognized type for element precursor.");
    }

    //Any classes that aren't already finalized need that to happen now,
    // or else they'll turn into a pumpkin.
    for(auto& pair : virginClasses) myClasses.emplace(pair.first, pair.second);
}

bool Compiler::compileFile(const char* path)
{
    std::ifstream myFile;
    myFile.open(path);

    std::string code;

    if(!myFile) return false;

    while(true)
    {
        std::string buffer;
        std::getline(myFile, buffer);

        if(!myFile)
            break;

        code += buffer;
        code += '\n'; //For tracking line numbers.
    }

    compile(code.c_str());
    return true;
}

void Compiler::compile(std::string code)
{
    {
        //If you use tabs 🔪
        std::string cleanedCode;
        for(auto it = code.begin(); it != code.end(); ++it)
        {
            if(*it == '\t') cleanedCode += "    ";
            else            cleanedCode += *it;
        }
        code = cleanedCode;
    }

    finalizePrecursors(parseTokens(tokenizeInput(code)));

    for(const SyntaxError& e : myErrors) errorMessages.push_back(e.printout(code.c_str()));
    myErrors.clear();
}

const std::string& Compiler::getErrorMessageDigest()
{
   publicErrorBuffer.clear();

   for(const auto err : getErrorMessages())
   {
       publicErrorBuffer += err;
       publicErrorBuffer += '\n';
   }

   return publicErrorBuffer;
}

}//IVD
