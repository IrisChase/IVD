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

#ifndef IRISUTILS_LEXCOMPARE
#define IRISUTILS_LEXCOMPARE

#include <tuple>

//Once upon a time I thought 7 outta be enough. But it proved insufficient, so then I made it 17.
//... Then that wasn't enough... So now it's 22...

#define IRISUTILS_MACRO_OVERLOAD(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, MACRO, ...) MACRO

#define IRISUTILS_UNPACK_FINAL(object, field) object . field

#define IRISUTILS_UNPACK22(object, field22, field21, field20, field19, field18, field17, field16, field15, field14, field13, field12, field11, field10, field9, field8, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field22) , \
    IRISUTILS_UNPACK_FINAL(object, field21) , \
    IRISUTILS_UNPACK_FINAL(object, field20) , \
    IRISUTILS_UNPACK_FINAL(object, field19) , \
    IRISUTILS_UNPACK_FINAL(object, field18) , \
    IRISUTILS_UNPACK_FINAL(object, field17) , \
    IRISUTILS_UNPACK_FINAL(object, field16) , \
    IRISUTILS_UNPACK_FINAL(object, field15) , \
    IRISUTILS_UNPACK_FINAL(object, field14) , \
    IRISUTILS_UNPACK_FINAL(object, field13) , \
    IRISUTILS_UNPACK_FINAL(object, field12) , \
    IRISUTILS_UNPACK_FINAL(object, field11) , \
    IRISUTILS_UNPACK_FINAL(object, field10) , \
    IRISUTILS_UNPACK_FINAL(object, field9) , \
    IRISUTILS_UNPACK_FINAL(object, field8) , \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK21(object, field21, field20, field19, field18, field17, field16, field15, field14, field13, field12, field11, field10, field9, field8, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field21) , \
    IRISUTILS_UNPACK_FINAL(object, field20) , \
    IRISUTILS_UNPACK_FINAL(object, field19) , \
    IRISUTILS_UNPACK_FINAL(object, field18) , \
    IRISUTILS_UNPACK_FINAL(object, field17) , \
    IRISUTILS_UNPACK_FINAL(object, field16) , \
    IRISUTILS_UNPACK_FINAL(object, field15) , \
    IRISUTILS_UNPACK_FINAL(object, field14) , \
    IRISUTILS_UNPACK_FINAL(object, field13) , \
    IRISUTILS_UNPACK_FINAL(object, field12) , \
    IRISUTILS_UNPACK_FINAL(object, field11) , \
    IRISUTILS_UNPACK_FINAL(object, field10) , \
    IRISUTILS_UNPACK_FINAL(object, field9) , \
    IRISUTILS_UNPACK_FINAL(object, field8) , \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK20(object, field20, field19, field18, field17, field16, field15, field14, field13, field12, field11, field10, field9, field8, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field20) , \
    IRISUTILS_UNPACK_FINAL(object, field19) , \
    IRISUTILS_UNPACK_FINAL(object, field18) , \
    IRISUTILS_UNPACK_FINAL(object, field17) , \
    IRISUTILS_UNPACK_FINAL(object, field16) , \
    IRISUTILS_UNPACK_FINAL(object, field15) , \
    IRISUTILS_UNPACK_FINAL(object, field14) , \
    IRISUTILS_UNPACK_FINAL(object, field13) , \
    IRISUTILS_UNPACK_FINAL(object, field12) , \
    IRISUTILS_UNPACK_FINAL(object, field11) , \
    IRISUTILS_UNPACK_FINAL(object, field10) , \
    IRISUTILS_UNPACK_FINAL(object, field9) , \
    IRISUTILS_UNPACK_FINAL(object, field8) , \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK19(object, field19, field18, field17, field16, field15, field14, field13, field12, field11, field10, field9, field8, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field19) , \
    IRISUTILS_UNPACK_FINAL(object, field18) , \
    IRISUTILS_UNPACK_FINAL(object, field17) , \
    IRISUTILS_UNPACK_FINAL(object, field16) , \
    IRISUTILS_UNPACK_FINAL(object, field15) , \
    IRISUTILS_UNPACK_FINAL(object, field14) , \
    IRISUTILS_UNPACK_FINAL(object, field13) , \
    IRISUTILS_UNPACK_FINAL(object, field12) , \
    IRISUTILS_UNPACK_FINAL(object, field11) , \
    IRISUTILS_UNPACK_FINAL(object, field10) , \
    IRISUTILS_UNPACK_FINAL(object, field9) , \
    IRISUTILS_UNPACK_FINAL(object, field8) , \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK18(object, field18, field17, field16, field15, field14, field13, field12, field11, field10, field9, field8, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field18) , \
    IRISUTILS_UNPACK_FINAL(object, field17) , \
    IRISUTILS_UNPACK_FINAL(object, field16) , \
    IRISUTILS_UNPACK_FINAL(object, field15) , \
    IRISUTILS_UNPACK_FINAL(object, field14) , \
    IRISUTILS_UNPACK_FINAL(object, field13) , \
    IRISUTILS_UNPACK_FINAL(object, field12) , \
    IRISUTILS_UNPACK_FINAL(object, field11) , \
    IRISUTILS_UNPACK_FINAL(object, field10) , \
    IRISUTILS_UNPACK_FINAL(object, field9) , \
    IRISUTILS_UNPACK_FINAL(object, field8) , \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK17(object, field17, field16, field15, field14, field13, field12, field11, field10, field9, field8, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field17) , \
    IRISUTILS_UNPACK_FINAL(object, field16) , \
    IRISUTILS_UNPACK_FINAL(object, field15) , \
    IRISUTILS_UNPACK_FINAL(object, field14) , \
    IRISUTILS_UNPACK_FINAL(object, field13) , \
    IRISUTILS_UNPACK_FINAL(object, field12) , \
    IRISUTILS_UNPACK_FINAL(object, field11) , \
    IRISUTILS_UNPACK_FINAL(object, field10) , \
    IRISUTILS_UNPACK_FINAL(object, field9) , \
    IRISUTILS_UNPACK_FINAL(object, field8) , \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK16(object, field16, field15, field14, field13, field12, field11, field10, field9, field8, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field16) , \
    IRISUTILS_UNPACK_FINAL(object, field15) , \
    IRISUTILS_UNPACK_FINAL(object, field14) , \
    IRISUTILS_UNPACK_FINAL(object, field13) , \
    IRISUTILS_UNPACK_FINAL(object, field12) , \
    IRISUTILS_UNPACK_FINAL(object, field11) , \
    IRISUTILS_UNPACK_FINAL(object, field10) , \
    IRISUTILS_UNPACK_FINAL(object, field9) , \
    IRISUTILS_UNPACK_FINAL(object, field8) , \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK15(object, field15, field14, field13, field12, field11, field10, field9, field8, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field15) , \
    IRISUTILS_UNPACK_FINAL(object, field14) , \
    IRISUTILS_UNPACK_FINAL(object, field13) , \
    IRISUTILS_UNPACK_FINAL(object, field12) , \
    IRISUTILS_UNPACK_FINAL(object, field11) , \
    IRISUTILS_UNPACK_FINAL(object, field10) , \
    IRISUTILS_UNPACK_FINAL(object, field9) , \
    IRISUTILS_UNPACK_FINAL(object, field8) , \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK14(object, field14, field13, field12, field11, field10, field9, field8, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field14) , \
    IRISUTILS_UNPACK_FINAL(object, field13) , \
    IRISUTILS_UNPACK_FINAL(object, field12) , \
    IRISUTILS_UNPACK_FINAL(object, field11) , \
    IRISUTILS_UNPACK_FINAL(object, field10) , \
    IRISUTILS_UNPACK_FINAL(object, field9) , \
    IRISUTILS_UNPACK_FINAL(object, field8) , \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK13(object, field13, field12, field11, field10, field9, field8, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field13) , \
    IRISUTILS_UNPACK_FINAL(object, field12) , \
    IRISUTILS_UNPACK_FINAL(object, field11) , \
    IRISUTILS_UNPACK_FINAL(object, field10) , \
    IRISUTILS_UNPACK_FINAL(object, field9) , \
    IRISUTILS_UNPACK_FINAL(object, field8) , \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK12(object, field12, field11, field10, field9, field8, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field12) , \
    IRISUTILS_UNPACK_FINAL(object, field11) , \
    IRISUTILS_UNPACK_FINAL(object, field10) , \
    IRISUTILS_UNPACK_FINAL(object, field9) , \
    IRISUTILS_UNPACK_FINAL(object, field8) , \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK11(object, field11, field10, field9, field8, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field11) , \
    IRISUTILS_UNPACK_FINAL(object, field10) , \
    IRISUTILS_UNPACK_FINAL(object, field9) , \
    IRISUTILS_UNPACK_FINAL(object, field8) , \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK10(object, field10, field9, field8, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field10) , \
    IRISUTILS_UNPACK_FINAL(object, field9) , \
    IRISUTILS_UNPACK_FINAL(object, field8) , \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK9(object, field9, field8, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field9) , \
    IRISUTILS_UNPACK_FINAL(object, field8) , \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK8(object, field8, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field8) , \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK7(object, field7, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field7) , \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK6(object, field6, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field6) , \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK5(object, field5, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field5) , \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK4(object, field4, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field4) , \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK3(object, field3, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field3) , \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK2(object, field2, field1) \
    IRISUTILS_UNPACK_FINAL(object, field2) , \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK1(object, field1) \
    IRISUTILS_UNPACK_FINAL(object, field1)

#define IRISUTILS_UNPACK(object, fields...) \
    IRISUTILS_MACRO_OVERLOAD(fields, \
                             IRISUTILS_UNPACK22, \
                             IRISUTILS_UNPACK21, \
                             IRISUTILS_UNPACK20, \
                             IRISUTILS_UNPACK19, \
                             IRISUTILS_UNPACK18, \
                             IRISUTILS_UNPACK17, \
                             IRISUTILS_UNPACK16, \
                             IRISUTILS_UNPACK15, \
                             IRISUTILS_UNPACK14, \
                             IRISUTILS_UNPACK13, \
                             IRISUTILS_UNPACK12, \
                             IRISUTILS_UNPACK11, \
                             IRISUTILS_UNPACK10, \
                             IRISUTILS_UNPACK9, \
                             IRISUTILS_UNPACK8, \
                             IRISUTILS_UNPACK7, \
                             IRISUTILS_UNPACK6, \
                             IRISUTILS_UNPACK5, \
                             IRISUTILS_UNPACK4, \
                             IRISUTILS_UNPACK3, \
                             IRISUTILS_UNPACK2, \
                             IRISUTILS_UNPACK1)(object, fields)


#define IRISUTILS_DEFINE_COMP(type, fields...) \
    friend bool operator<(const type & left, const type & right) \
    {   \
        return std::tie(IRISUTILS_UNPACK(left, fields)) < \
               std::tie(IRISUTILS_UNPACK(right, fields)); \
    } \
    friend bool operator!=(const type & left, const type & right) \
    {   \
        return left < right || right < left; \
    }   \
    friend bool operator==(const type & left, const type & right) \
    {   \
        return !(left != right); \
    }

#endif // IRISUTILS_LEXCOMPARE
