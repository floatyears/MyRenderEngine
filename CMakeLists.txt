cmake_minimum_required(VERSION 3.4)

project(MyRenderEngine)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(BOOST_ROOT /usr/local/Cellar/boost/1.57.0)
find_package(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

set(SOURCE_FILES Render/Render/sample1cpp.cpp)
add_executable(ModelLoader ${SOURCE_FILES})
