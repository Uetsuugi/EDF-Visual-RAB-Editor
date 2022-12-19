cmake_minimum_required(VERSION 3.23)
project(EDF-Visual-RAB-Editor)

set(CMAKE_CXX_STANDARD 20)

add_executable(EDF_RAB_Editor main.cpp utility.h utility.cpp CMPLHandler.cpp CMPLHandler.h Window.hpp Window.cpp UI.cpp UI.h RAB.cpp RAB.h portable-file-dialogs.h)

#SFML STUFF
message("-- Linking SFML")
set(SFML_USE_STATIC_LIBS OFF)
find_package(SFML COMPONENTS graphics window system)

if(NOT SFML_FOUND)
    message(FATAL_ERROR "Could not find SFML")
endif()

#IMGUI-SFML
message("-- Linking ImGui-SFML")
set(ImGui-SFML_USE_STATIC_LIBS ON)
find_package(ImGui-SFML REQUIRED)

if(NOT ImGui-SFML_FOUND)
    message(FATAL_ERROR "Could not find ImGui-SFML")
endif()

#JSONCPP
find_package(JSON-C COMPONENTS jsoncpp)

target_link_libraries(EDF_RAB_Editor jsoncpp sfml-graphics sfml-window sfml-system ImGui-SFML::ImGui-SFML)