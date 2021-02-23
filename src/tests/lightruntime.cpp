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

#include <iostream>

#include "user_include/cpp/IVD_cpp.h"

#include "widgets/boxlayout.h"
#include "widgets/stacklayout.h"

int main(int argc, char** argv)
{

    if(argc == 1)
    {
        std::cout << "Usage: lightruntime sourcefile.ivd" << std::endl;
        return 0;
    }
    else if(argc != 2)
    {
        std::cout << "Too many arguements" << std::endl;
        return 0;
    }

    const std::string path = argv[1];

    IVD::bindings::Environment rt;

    rt.register_layout<IVD::std_widgets::HboxLayout>("hbox");
    rt.register_layout<IVD::std_widgets::VboxLayout>("vbox");
    rt.register_layout<IVD::std_widgets::StackLayout>("stack");

    const int stat = rt.load_IVD_from_file(path.c_str());

    if(stat == IVD_STATUS_SUCCESS)
        rt.run();
    else if(stat == IVD_STATUS_FILE_NOT_FOUND)
        std::cout << "File note found: " << path << std::endl;
    else if(stat == IVD_STATUS_COMPILE_ERROR)
        std::cout << "Erorrs in code: " << std::endl << rt.get_compiler_errors() << std::endl;
    else
        std::cout << "An unknown error occoured" << std::endl;

    return 0;
}
