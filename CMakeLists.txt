# Copyright 2020 Iris Chase

# Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

cmake_minimum_required(VERSION 3.12)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()


project(IVD CXX)
set(CMAKE_CXX_STANDARD 17)

list(APPEND ivd_core_sources
src/attribute.cpp
src/attribute.h
src/attributepositionpair.h
src/attributeset.cpp
src/attributeset.h
src/canvas.h
src/color.cpp
src/color.h
src/compiler.cpp
src/compiler.h
src/defaults.cpp
src/defaults.h
src/displayitem.cpp
src/displayitem.h
src/driver.h
src/element.h
src/environment.cpp
src/environment.h
src/event.h
src/expression.cpp
src/expression.h
src/geometry.h
src/geometryproposal.h
src/keywords.h
src/material.cpp
src/material.h
src/standardstatekeys.h
src/statekey.h
src/statemanager.h
src/statemanager.cpp
src/states.h
src/text.cpp
src/text.h
src/valuekey.cpp
src/valuekey.h
src/modelcontainer.h
src/modelitembase.h
src/modelitem.cpp
src/corefonts.h
src/graph.cpp
src/graph.h
src/codeposition.h
src/virtualstatekey.h
src/virtualstatekey.cpp
src/binaryexpressionprinter.h
src/cbindings.cpp

src/user_include/IVD_c.h
src/user_include/IVD_cpp.h
src/user_include/IVD_status.h

src/shaping/line.h

src/material/rowlayoutmaterial.cpp
src/material/rowlayoutmaterial.h
src/material/simplematerial.h
src/material/simplematerial.cpp
src/material/fallbackparagraphmaterial.cpp
src/material/fallbackparagraphmaterial.h
src/material/inlinelayoutmaterial.h
src/material/inlinelayoutmaterial.cpp
src/material/textrunmaterial.h
src/material/textrunmaterial.cpp
src/material/imagematerial.h
src/material/imagematerial.cpp
src/material/freelayoutmaterial.h
src/material/freelayoutmaterial.cpp
src/material/stacklayoutmaterial.h
src/material/stacklayoutmaterial.cpp

src/allmaterials.h

src/specific_driver_sdl/cairocanvas.cpp
src/specific_driver_sdl/cairocanvas.h
src/specific_driver_sdl/sdldriver.cpp
src/specific_driver_sdl/sdldriver.h
src/specific_driver_sdl/sdlwindow.cpp
src/specific_driver_sdl/sdlwindow.h
src/specific_driver_sdl/textdriver.cpp

${CMAKE_CURRENT_BINARY_DIR}/corefontsansbinary.cpp
${CMAKE_CURRENT_BINARY_DIR}/corefontsansbinary.cpp
${CMAKE_CURRENT_BINARY_DIR}/corefontsansboldbinary.cpp
${CMAKE_CURRENT_BINARY_DIR}/corefontsansitalicbinary.cpp
${CMAKE_CURRENT_BINARY_DIR}/corefontsansbolditalicbinary.cpp
${CMAKE_CURRENT_BINARY_DIR}/corefontserifbinary.cpp
${CMAKE_CURRENT_BINARY_DIR}/corefontserifboldbinary.cpp
${CMAKE_CURRENT_BINARY_DIR}/corefontserifitalicbinary.cpp
${CMAKE_CURRENT_BINARY_DIR}/corefontserifbolditalicbinary.cpp
${CMAKE_CURRENT_BINARY_DIR}/corefontmonobinary.cpp
${CMAKE_CURRENT_BINARY_DIR}/corefontmonoboldbinary.cpp
${CMAKE_CURRENT_BINARY_DIR}/corefontmonoitalicbinary.cpp
${CMAKE_CURRENT_BINARY_DIR}/corefontmonobolditalicbinary.cpp

contrib/irisutils/lexcompare.h
contrib/irisutils/routine.h
contrib/irisutils/easyuniquepointer.h
)

find_package(reprodyne 1.0.0 REQUIRED)
find_package(Freetype REQUIRED)
find_package(Catch2 REQUIRED)

SET(IVD_COMMON_LIBS
  SDL2
  cairo
  harfbuzz
  freetype
  OpenImageIO)

SET(IVD_INTERNAL_INCLUDES
  src
  contrib
  ${REPRODYNE_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS})

add_library(ivd SHARED ${ivd_core_sources})
target_include_directories(ivd PRIVATE PRIVATE ${IVD_INTERNAL_INCLUDES})
target_link_libraries(ivd PRIVATE ${IVD_COMMON_LIBS})

set_target_properties(ivd PROPERTIES PUBLIC_HEADER
  "${PROJECT_SOURCE_DIR}/src/user_include/IVD_c.h;${PROJECT_SOURCE_DIR}/src/user_include/IVD_cpp.h;${PROJECT_SOURCE_DIR}/src/user_include/IVD_status.h")


add_executable(ivdRepro ${ivd_core_sources} src/tests/recordplayback.cpp)
target_include_directories(ivdRepro PRIVATE ${IVD_INTERNAL_INCLUDES})
target_link_libraries(ivdRepro PRIVATE ${IVD_COMMON_LIBS} reprodyne)
target_compile_definitions(ivdRepro PUBLIC REPRODYNE_AVAILABLE)



find_package(Python3 REQUIRED)
function(addFont sourcettf constantsymbol)
    set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${constantsymbol}binary.cpp)
    add_custom_command(DEPENDS ${sourcettf} binarysourcegenerator.py
                       COMMAND ${Python3_EXECUTABLE} binarysourcegenerator.py
                               ${sourcettf} corefonts.h ${constantsymbol} ${outfile}
                       OUTPUT ${outfile}
                       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

addFont("contrib/fonts/LiberationSans-Regular.ttf"    "corefontsans")
addFont("contrib/fonts/LiberationSans-Bold.ttf"       "corefontsansbold")
addFont("contrib/fonts/LiberationSans-Italic.ttf"     "corefontsansitalic")
addFont("contrib/fonts/LiberationSans-BoldItalic.ttf" "corefontsansbolditalic")

addFont("contrib/fonts/LiberationSerif-Regular.ttf"    "corefontserif")
addFont("contrib/fonts/LiberationSerif-Bold.ttf"       "corefontserifbold")
addFont("contrib/fonts/LiberationSerif-Italic.ttf"     "corefontserifitalic")
addFont("contrib/fonts/LiberationSerif-BoldItalic.ttf" "corefontserifbolditalic")

addFont("contrib/fonts/LiberationMono-Regular.ttf"    "corefontmono")
addFont("contrib/fonts/LiberationMono-Bold.ttf"       "corefontmonobold")
addFont("contrib/fonts/LiberationMono-Italic.ttf"     "corefontmonoitalic")
addFont("contrib/fonts/LiberationMono-BoldItalic.ttf" "corefontmonobolditalic")

configure_file("contrib/test-images/jpeg_test_article.jpg" "jpeg_test_article.jpg" COPYONLY)
configure_file("contrib/test-images/png_test_article.png" "png_test_article.png" COPYONLY)

add_executable(ivdruntime src/tests/lightruntime.cpp)
target_include_directories(ivdruntime PRIVATE src)
target_link_libraries(ivdruntime PRIVATE ivd)

add_executable(ivdserializingcompiler src/tests/serializingcompiler.cpp)
target_include_directories(ivdserializingcompiler PRIVATE src contrib)
target_link_libraries(ivdserializingcompiler PRIVATE ivd)

add_executable(ivdmodeltests
    src/tests/modeltests.cpp
    src/tests/catch2testmain.cpp
    src/tests/linkedlist.cpp
    src/tests/nestedmodels.cpp)
target_include_directories(ivdmodeltests PRIVATE src contrib)
target_link_libraries(ivdmodeltests PRIVATE ivd)



include(TestBigEndian)
TEST_BIG_ENDIAN(systemIsBigEndian)
if(systemIsBigEndian)
    add_compile_definitions(BIG_ENDIAN_SYSTEM)
else()
    add_compile_definitions(LITTLE_ENDIAN_SYSTEM)
endif()

set(IVD_MAJOR_VERSION 0)
set(IVD_MINOR_VERSION 0)
set(IVD_PATCH_VERSION 0)

set(IVD_API_VERSION ${IVD_MAJOR_VERSION}.${IVD_MINOR_VERSION})
set(IVD_VERSION ${IVD_API_VERSION}.${IVD_PATCH_VERSION})

set(IVD_DEST_NAME IVD-${IVD_API_VERSION})

set(IVD_USER_INCLUDE_DIRS include/${IVD_DEST_NAME})

install(TARGETS ivd EXPORT ivd-targets
        ARCHIVE
            DESTINATION lib/${IVD_DEST_NAME}
        PUBLIC_HEADER
            DESTINATION ${IVD_USER_INCLUDE_DIRS})

include(CMakePackageConfigHelpers)

write_basic_package_version_file(ivd-config-version.cmake
  VERSION ${IVD_VERSION}
  COMPATIBILITY SameMajorVersion)

configure_file(ivd-config.cmake.in
    ${CMAKE_BINARY_DIR}/ivd-config.cmake @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/ivd-config.cmake ${CMAKE_BINARY_DIR}/ivd-config-version.cmake
    DESTINATION lib/cmake/${IVD_DEST_NAME})

install(EXPORT ivd-targets
    DESTINATION lib/cmake/${IVD_DEST_NAME})
