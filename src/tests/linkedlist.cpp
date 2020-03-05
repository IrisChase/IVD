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

#include <catch2/catch.hpp>
#include "modelcontainer.h"

TEST_CASE("Model linked list tests", "[automatic][containerLinkedList]")
{
    IVD::EventQueue myQueue;
    IVD::ModelContainer container(&myQueue, "no name");

    auto* first  = container.push_back_new();
    auto* second = container.push_back_new();
    auto* third  = container.push_back_new();
    auto* forth  = container.push_back_new();
    auto* fifth  = container.push_back_new();

    auto compare = [&](std::vector<IVD::ModelItemBase*> referenceSet)
    {
        REQUIRE(container.size() == referenceSet.size());

        auto it1 = container.begin();
        auto it2 = referenceSet.begin();

        while(it1 != container.end() && it2 != referenceSet.end())
        {
            REQUIRE(*it1 == *it2);
            ++it1;
            ++it2;
        }
        REQUIRE((it1 == container.end() && it2 == referenceSet.end()));
    };

    SECTION("Adjacent swap back")
    {
        container.swap(first, second);
        compare({second, first, third, forth, fifth});
    }
    SECTION("Adjacent swap front")
    {
        container.swap(forth, fifth);
        compare({first, second, third, fifth, forth});
    }
    SECTION("Adjacent swap middle")
    {
        container.swap(second, third);
        compare({first, third, second, forth, fifth});
    }

    //UNO REVERSE CARD
    SECTION("Adjacent swap back, inputs reversed")
    {
        container.swap(second, first);
        compare({second, first, third, forth, fifth});
    }
    SECTION("Adjacent swap front, inputs reversed")
    {
        container.swap(fifth, forth);
        compare({first, second, third, fifth, forth});
    }
    SECTION("Adjacent swap middle, inputs reversed")
    {
        container.swap(third, second);
        compare({first, third, second, forth, fifth});
    }

    //
    SECTION("Delete back")
    {
        container.safeToDelete(first);
        compare({second, third, forth, fifth});
    }
    SECTION("Delete front")
    {
        container.safeToDelete(fifth);
        compare({first, second, third, forth});
    }
    SECTION("Delete middle")
    {
        container.safeToDelete(third);
        compare({first, second, forth, fifth});
    }
}
