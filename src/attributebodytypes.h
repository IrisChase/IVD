#pragma once

#include <map>

namespace IVD
{

typedef unsigned int KeyType;

struct AttributeBodyTypes
{
    bool expression;
    bool property;
    bool stringLiteral;
    bool userToken;
    bool userTokenList;
    bool stateKeyList;
    bool singleScopedValueKey;
    bool color;
    bool unnatural;
    bool positionWithin;
};

typedef std::map<KeyType, AttributeBodyTypes> AttributeBodyTypeMap;

}//IVD
