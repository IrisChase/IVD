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

TEST_CASE("model-environment-tests")
{
    IVD::Runtime myEnv;
    IVD::Model model = myEnv.add_model("test-model");
    IVD::Instance inst = model.add_instance();

    inst.set_state("first-grab");

    bool passed = false;
    inst.set_trigger("accept", [&passed, inst]()
    {
        passed = true;
        inst.set_state("passed");
    });

    int count = 0;
    inst.set_trigger("once-only", [&]()
    {
        ++count;
    });

    auto RUN_IVD_TEST = [&](const std::string& path)
    {
        REQUIRE(myEnv.load_IVD_from_file(path) == IVD_STATUS_SUCCESS);
        myEnv.run();
        REQUIRE(passed);
    };

    SECTION("automatic")
    {
        SECTION("Trigger state tests") //that doesn't really mean anything...
        {
            inst.set_trigger("set-the-state", [inst]()
            {
                inst.set_state("the-model-state");
            });

            SECTION("Trigger state")
            {
                RUN_IVD_TEST("model/triggerstate.ivd");
            }

            SECTION("Single model item element instancing")
            {
                RUN_IVD_TEST("model/modelinstancing.ivd");
            }

            SECTION("Common model element parent deduction")
            {
                RUN_IVD_TEST("model/commonparentdeduction.ivd");
            }

            SECTION("Common sibling deduction")
            {
                model.add_instance();
                model.add_instance();
                model.add_instance();
                model.add_instance();
                RUN_IVD_TEST("model/commonsiblingdeduction.ivd");

                //TODO: add a hook to only-once to guarantee only one got deduced
                //not really necessary given the current implementation but'cha never know.
            }
        }

        SECTION("Test singular not state")
        {
            RUN_IVD_TEST("model/notstate.ivd");
        }

        SECTION("Set number on model from IVD")
        {
            double num = 0;
            inst.bind_accessors("fortytwo", num);

            RUN_IVD_TEST("model/setnum.ivd");
            REQUIRE(inst.get_number("fortytwo") == 42);
        }

        SECTION("Only call trigger once")
        {
            SECTION("From single state")
            {
                RUN_IVD_TEST("model/singletriggercall.ivd");
                REQUIRE(count == 1);
            }

            SECTION("From compound state")
            {
                RUN_IVD_TEST("model/singletriggercallcompound.ivd");
                REQUIRE(count == 1);
            }
        }

        SECTION("state suck")
        {
           inst.set_trigger("trig-hook", [=]()
           {
               inst.set_state("hook-back");
           });

           RUN_IVD_TEST("model/statesuck.ivd");
        }
    }
    SECTION("manual")
    {
        SECTION("Bind to model geometry")
        {
            double width = 600;
            double height = 69; //tehe I'm 12

            inst.bind_accessors("the-width", width);
            inst.bind_accessors("the-height", height);

            inst.set_trigger("update-size", [&]()
            {
                inst.set_number("the-width", 700);
                inst.set_number("the-height", 100);
            });

            RUN_IVD_TEST("model/bindnum.ivd");
            REQUIRE((width == 700 && height == 100));
        }
        SECTION("Bind to model text")
        {
            std::string theString = "Press F to get instructions for validation.";
            const std::string secondString = "If the window has resized properly, press N to accept the test.";
            inst.bind_accessors("the-string", theString);

            inst.set_trigger("change-text", [&]()
            {
                inst.set_string("the-string", secondString);
            });

            RUN_IVD_TEST("model/stringbinding.ivd");
            REQUIRE(theString == secondString);
        }

        SECTION("Model ordering/reordering")
        {
            IVD::Instance inst2 = model.add_instance();

            inst.set_trigger("swap", [&]()
            {
                model.swap_instances(inst, inst2);
                inst.set_state("swapped");
                inst2.set_state("swapped");
            });

            RUN_IVD_TEST("model/order.ivd");
        }
        SECTION("Dynamic model item instantiation and destruction")
        {
            IVD::Instance lastInstance;

            bool insertedOne = false;
            bool erasedOne = false;

            inst.set_trigger("insert-one", [&]
            {
                lastInstance = model.add_instance();
                insertedOne = true;
            });

            inst.set_trigger("erase-one", [&]
            {
                if(model.size() < 2) return;

                model.erase_later(*(++model.begin()));

                erasedOne = true;
                passed = true; //eehhh
            });

            RUN_IVD_TEST("model/instdestruct.ivd");
            REQUIRE((insertedOne && erasedOne));
        }
    }
}
