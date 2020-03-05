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

#include "user_include/IVD_cpp.h"
#include <reprodyne.h>


int main(int argc, char** argv)
{
    bool status = argc == 4;

    std::string mode;
    std::string ivdFile;
    std::string path;

    if(status)
    {
        mode = argv[1];
        ivdFile = argv[2];
        path = argv[3];
    }

    if(mode == "play")
    {
        reprodyne_play(path.c_str());
    }
    else if(mode == "record")
    {
        reprodyne_record();
    }
    else status = false;

    if(!status)
    {
        std::cerr << "Usage: [play|record] IVD_SOURCE X3TEST_SERIAL" << std::endl;
        return -1;
    }

    //Run
    IVD::Runtime rt;
    rt.load_IVD_from_file(ivdFile);
    rt.run();

    if(mode == "record")
    {
        reprodyne_save(path.c_str());
    }
    else if(mode == "play")
    {
        reprodyne_assert_complete_read();
    }

    return 0;
}
