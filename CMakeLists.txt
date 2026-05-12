cmake_minimum_required(VERSION 3.10)
project(EcoSim VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB IMGUI_SOURCES imgui/*.cpp)

set(SOURCES
    OrganismDefine.cpp
    EnvironmentDefine.cpp
    WorldDefine.cpp
    test_main.cpp
    ${IMGUI_SOURCES}
    imgui/backends/imgui_impl_win32.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

set(HEADERS
    Registry.h
    Organism.h
    Environment.h
    World.h
)

add_executable(ecosim ${SOURCES} ${HEADERS} "MyOperator.h" "MyOperatorDefine.cpp" "Animals.h"  "ConflgDefine.cpp")

target_include_directories(ecosim PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/imgui/backends
)

target_link_libraries(ecosim PRIVATE opengl32 gdi32 dwmapi)

if(MSVC)
    target_compile_options(ecosim PRIVATE /EHsc)
endif()

# ---- standalone boids test ----
add_executable(boids_test
    boids/test.cpp
    ${IMGUI_SOURCES}
    imgui/backends/imgui_impl_win32.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(boids_test PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/boids
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/imgui/backends
)

target_link_libraries(boids_test PRIVATE opengl32 gdi32 dwmapi)

if(MSVC)
    target_compile_options(boids_test PRIVATE /EHsc)
endif()
