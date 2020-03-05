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

#include "user_include/IVD_cpp.h"

TEST_CASE("Nested models", "[manual][nested-models]") //May god have mercy on our souls
{
    IVD::Runtime rt;

    IVD::Model windowModel = rt.add_model("windows");
    IVD::Instance window = windowModel.add_instance();

    IVD::Model childModel = window.actualize_child_model("child");

    IVD::Instance widget1 = childModel.add_instance();
    std::string blep1 = "Child 1";
    widget1.bind_accessors("the-text", blep1);

    IVD::Instance widget2 = childModel.add_instance();
    std::string blep2 = "Child 2";
    widget2.bind_accessors("the-text", blep2);

    IVD::Instance widget3 = childModel.add_instance();
    const std::string blep3 = "Child 3";
    widget3.bind_accessors("the-text", blep3);

    IVD::Instance widget4 = childModel.add_instance();
    widget4.set_state("last");

    IVD::Model deep = widget4.actualize_child_model("deep-kid");
    IVD::Instance innermostwidget = deep.add_instance();

    const std::string blep14 = "deepest depths";
    innermostwidget.bind_accessors("thine-text", blep14);

    SECTION("Straight nested")
    {
        REQUIRE(rt.load_IVD_from_file("model/nested.ivd") == IVD_STATUS_SUCCESS);
        rt.run();
    }

    SECTION("Deep parent deduction")
    {
        REQUIRE(rt.load_IVD_from_file("model/deeprootnested.ivd") == IVD_STATUS_SUCCESS);
        rt.run();
    }

    REQUIRE(true); //If we made it this far without crashing, it works.
}
