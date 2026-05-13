cmake_minimum_required(VERSION 3.10)
project(EcoSim VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB IMGUI_SOURCES imgui/*.cpp)

set(SOURCES
    OrganismDefine.cpp
    EnvironmentDefine.cpp
    WorldDefine.cpp
    SetupUI.cpp
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

add_executable(debug_sim debug_main.cpp "MyOperator.h" "MyOperatorDefine.cpp" "Animals.h" "Plants.h"
    OrganismDefine.cpp EnvironmentDefine.cpp WorldDefine.cpp ConflgDefine.cpp)

target_include_directories(ecosim PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/imgui/backends
)

target_link_libraries(ecosim PRIVATE opengl32 gdi32 dwmapi)

target_include_directories(debug_sim PRIVATE ${CMAKE_SOURCE_DIR})

if(MSVC)
    target_compile_options(ecosim PRIVATE /EHsc)
    target_compile_options(debug_sim PRIVATE /EHsc)
endif()

# Copy default_config.json to build directory so the exe can find it
add_custom_command(TARGET ecosim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/default_config.json"
        "$<TARGET_FILE_DIR:ecosim>/default_config.json"
)

add_custom_command(TARGET debug_sim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/default_config.json"
        "$<TARGET_FILE_DIR:debug_sim>/default_config.json"
)
