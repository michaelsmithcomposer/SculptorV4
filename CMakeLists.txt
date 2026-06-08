cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(SculptorV4 VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES} 
    "src/lib/ObjectState.cpp" 
    "src/mod/Form/Component.cpp" 
    "src/mod/Form/Modulator.cpp" 
    "src/mod/Form/Layer.cpp" 
    "src/mod/Manager.cpp" 
    "src/mod/Property.cpp"
    "src/external/clipper.engine.cpp" 
    "src/external/clipper.triangulation.cpp"  "src/lib/UINode.cpp" "src/hooks/EditorUI.cpp" "src/lib/Utilities.cpp" "src/mod/UI/UI.cpp" "src/mod/Form/Form.cpp" "src/mod/Form/VectorEditor.cpp" "src/mod/Serialization.cpp" "src/hooks/EditorPauseLayer.cpp" "src/lib/Grid.cpp" "src/mod/UI/PropertyInput.cpp" "src/mod/UI/ModulatablePropertyInput.cpp" "src/mod/UI/SelectionPopup.cpp" "src/lib/Geometry2.cpp" "src/mod/UI/PoolInput.cpp" "src/mod/Chain.cpp" )

target_include_directories(${PROJECT_NAME} PRIVATE src)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_PDB_FILE:${PROJECT_NAME}>
        "C:/Program Files (x86)/Steam/steamapps/common/Geometry Dash/geode/unzipped/michael.sculptorv4/"
)
