cmake_minimum_required(VERSION 3.10)
project(EcoSim VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB IMGUI_SOURCES imgui/*.cpp)

set(SOURCES
    core/OrganismDefine.cpp
    core/EnvironmentDefine.cpp
    core/WorldDefine.cpp
    ui/SetupUI.cpp
    test_main.cpp
    boids/boids.cpp
    ${IMGUI_SOURCES}
    imgui/backends/imgui_impl_win32.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

set(HEADERS
    core/Registry.h
    core/Organism.h
    core/Environment.h
    core/World.h
)

add_executable(ecosim ${SOURCES} ${HEADERS}
    "operator/MyOperator.h" "operator/MyOperatorDefine.cpp"
    "core/Animals.h" "core/Plants.h"
    "config/ConflgDefine.cpp" "config/Config.h" "imguMe/myui.cpp"
    "implot/implot.cpp" "implot/implot_items.cpp" "implot/implot.h")

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
)

target_link_libraries(ecosim PRIVATE opengl32 gdi32 dwmapi)

# ---- headless debug sim ----
add_executable(debug_sim debug_main.cpp
    "operator/MyOperator.h" "operator/MyOperatorDefine.cpp"
    "core/Animals.h" "core/Plants.h"
    core/OrganismDefine.cpp core/EnvironmentDefine.cpp core/WorldDefine.cpp
    config/ConflgDefine.cpp config/Config.h boids/boids.cpp "imguMe/myui.cpp")

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
    imgui/backends/imgui_impl_opengl3.cpp
 "imguMe/myui.cpp")

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
    target_compile_options(ecosim PRIVATE /EHsc)
    target_compile_options(debug_sim PRIVATE /EHsc)
    target_compile_options(boids_test PRIVATE /EHsc)
endif()

# Copy default_config.json to build directory so the exe can find it
add_custom_command(TARGET ecosim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config/default_config.json"
        "$<TARGET_FILE_DIR:ecosim>/default_config.json"
)

add_custom_command(TARGET debug_sim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config/default_config.json"
        "$<TARGET_FILE_DIR:debug_sim>/default_config.json"
)
