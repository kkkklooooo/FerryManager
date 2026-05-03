cmake_minimum_required(VERSION 3.10)
project(EcoSim VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源文件
set(SOURCES
    OrganismDefine.cpp
    EnvironmentDefine.cpp
    WordDefine.cpp
    test_main.cpp   # 测试入口
)

# 头文件（仅为IDE友好，不参与编译）
set(HEADERS
    Registry.h
    Organism.h
    Environment.h
    Word.h
)

add_executable(ecosim ${SOURCES} ${HEADERS})

target_include_directories(ecosim PRIVATE ${CMAKE_SOURCE_DIR})

# 如果使用MSVC，可以添加一些选项
if(MSVC)
    target_compile_options(ecosim PRIVATE /EHsc)
endif()
