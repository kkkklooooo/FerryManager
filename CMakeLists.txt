cmake_minimum_required(VERSION 3.10)
project(EcoSim VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB IMGUI_SOURCES imgui/*.cpp)

set(SOURCES
    OrganismDefine.cpp
    EnvironmentDefine.cpp
    WordDefine.cpp
    test_main.cpp
    ${IMGUI_SOURCES}
)

# ͷ�ļ�����ΪIDE�Ѻã���������룩
set(HEADERS
    Registry.h
    Organism.h
    Environment.h
    Word.h
)

add_executable(ecosim ${SOURCES} ${HEADERS})

target_include_directories(ecosim PRIVATE
    ${CMAKE_SOURCE_DIR}
)
# ���ʹ��MSVC����������һЩѡ��
if(MSVC)
    target_compile_options(ecosim PRIVATE /EHsc)
endif()
