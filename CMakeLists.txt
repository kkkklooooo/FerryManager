cmake_minimum_required(VERSION 3.10)
project(EcoSim VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB IMGUI_SOURCES imgui/*.cpp)

set(SOURCES
    core/OrganismDefine.cpp
    core/EnvironmentDefine.cpp
    core/WorldDefine.cpp
    MAKE_IMAGE/make_image.cpp
    ui/SetupUI.cpp
    boids/boids.cpp
    utils/img_utils.cpp
    ${IMGUI_SOURCES}
    imgui/backends/imgui_impl_win32.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

set(HEADERS
    core/Registry.h
    core/Organism.h
    core/Environment.h
    core/World.h
    stb_image/stb_image.h
    stb_image/std_image_write.h
    MAKE_IMAGE/make_image.h
)

add_executable(ecosim ${SOURCES} ${HEADERS}
    "operator/MyOperator.h" "operator/MyOperatorDefine.cpp"
    "core/Animals.h" "core/Plants.h" 
    "config/ConflgDefine.cpp" "config/Config.h" "test_main.cpp"
    "implot/implot.cpp" "implot/implot_items.cpp" "implot/implot.h" "data/game_struct.h")

target_include_directories(ecosim PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/ui
    ${CMAKE_SOURCE_DIR}/operator
    ${CMAKE_SOURCE_DIR}/vendor
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/imgui/backends
    ${CMAKE_SOURCE_DIR}/implot
    ${CMAKE_SOURCE_DIR}/stb_image
)

add_executable(ecosim2 ${SOURCES} ${HEADERS}
    "operator/MyOperator.h" "operator/MyOperatorDefine.cpp"
    "core/Animals.h" "core/Plants.h"
    "config/ConflgDefine.cpp" "config/Config.h" "imguMe/myui.cpp"
     "implot/implot.cpp" "implot/implot_items.cpp" "implot/implot.h" "data/game_struct.h"
     utils/img_utils.h)

target_include_directories(ecosim2 PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/ui
    ${CMAKE_SOURCE_DIR}/operator
    ${CMAKE_SOURCE_DIR}/vendor
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/imgui/backends
    ${CMAKE_SOURCE_DIR}/implot
    ${CMAKE_SOURCE_DIR}/stb_image
)

target_link_libraries(ecosim PRIVATE opengl32 gdi32 dwmapi)


target_link_libraries(ecosim2 PRIVATE opengl32 gdi32 dwmapi)

# ---- headless debug sim ----
add_executable(debug_sim debug_main.cpp
    "operator/MyOperator.h" "operator/MyOperatorDefine.cpp"
    "core/Animals.h" "core/Plants.h"
    core/OrganismDefine.cpp core/EnvironmentDefine.cpp core/WorldDefine.cpp
    config/ConflgDefine.cpp config/Config.h boids/boids.cpp  "data/game_struct.h")

target_include_directories(debug_sim PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/operator
    ${CMAKE_SOURCE_DIR}/vendor
)

# ---- standalone boids test ----
add_executable(boids_test
    boids/test.cpp
    boids/boids.cpp
    ${IMGUI_SOURCES}
    imgui/backends/imgui_impl_win32.cpp
    imgui/backends/imgui_impl_opengl3.cpp "data/game_struct.h")

target_include_directories(boids_test PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/operator
    ${CMAKE_SOURCE_DIR}/vendor
    ${CMAKE_SOURCE_DIR}/boids
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/imgui/backends
)

target_link_libraries(boids_test PRIVATE opengl32 gdi32 dwmapi)

if(MSVC)
    target_compile_options(ecosim PRIVATE /EHsc /utf-8)
    target_compile_options(ecosim2 PRIVATE /EHsc /utf-8)
    target_compile_options(debug_sim PRIVATE /EHsc /utf-8)
    target_compile_options(boids_test PRIVATE /EHsc /utf-8)
endif()

# Copy default_config.json to build directory so the exe can find it
add_custom_command(TARGET ecosim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config/default_config.json"
        "$<TARGET_FILE_DIR:ecosim>/default_config.json"
)

add_custom_command(TARGET ecosim2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config/default_config.json"
        "$<TARGET_FILE_DIR:ecosim2>/default_config.json"
)

add_custom_command(TARGET debug_sim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config/default_config.json"
        "$<TARGET_FILE_DIR:debug_sim>/default_config.json"
)

# ---- shared library for distribution ----
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(LIB_HEADERS
    core/Registry.h
    core/Organism.h
    core/Environment.h
    core/World.h
    core/Animals.h
    core/Plants.h
    config/Config.h
    data/game_struct.h
    operator/MyOperator.h
    boids/boids.h
    boids/Genes.h
    boids/GA.h
)

add_library(ecosim_lib SHARED
    core/OrganismDefine.cpp
    core/EnvironmentDefine.cpp
    core/WorldDefine.cpp
    config/ConflgDefine.cpp
    operator/MyOperatorDefine.cpp
    boids/boids.cpp
    ${LIB_HEADERS}
)

target_include_directories(ecosim_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/core>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/config>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/operator>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/vendor>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/boids>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/data>
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/core
        ${CMAKE_SOURCE_DIR}/config
        ${CMAKE_SOURCE_DIR}/operator
        ${CMAKE_SOURCE_DIR}/vendor
        ${CMAKE_SOURCE_DIR}/boids
)

# Copy all project headers to includes/ for distribution
add_custom_command(TARGET ecosim_lib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/includes/core"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/includes/config"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/includes/data"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/includes/operator"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/includes/boids"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/includes/vendor"
    # core/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/core/Registry.h"       "${CMAKE_BINARY_DIR}/includes/core/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/core/Organism.h"       "${CMAKE_BINARY_DIR}/includes/core/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/core/Environment.h"     "${CMAKE_BINARY_DIR}/includes/core/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/core/World.h"           "${CMAKE_BINARY_DIR}/includes/core/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/core/Animals.h"         "${CMAKE_BINARY_DIR}/includes/core/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/core/Plants.h"          "${CMAKE_BINARY_DIR}/includes/core/"
    # config/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config/Config.h"        "${CMAKE_BINARY_DIR}/includes/config/"
    # data/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/data/game_struct.h"     "${CMAKE_BINARY_DIR}/includes/data/"
    # operator/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/operator/MyOperator.h"  "${CMAKE_BINARY_DIR}/includes/operator/"
    # boids/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/boids/boids.h"          "${CMAKE_BINARY_DIR}/includes/boids/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/boids/Genes.h"          "${CMAKE_BINARY_DIR}/includes/boids/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/boids/GA.h"             "${CMAKE_BINARY_DIR}/includes/boids/"
    # vendor/ (needed by Config.h and game_struct.h)
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/vendor/json.hpp"        "${CMAKE_BINARY_DIR}/includes/vendor/"
    # also copy default_config.json alongside the DLL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config/default_config.json"
        "$<TARGET_FILE_DIR:ecosim_lib>/default_config.json"
    COMMENT "Copied library headers to ${CMAKE_BINARY_DIR}/includes/"
)

# MSVC compile options for the lib
if(MSVC)
    target_compile_options(ecosim_lib PRIVATE /EHsc /utf-8)
endif()
