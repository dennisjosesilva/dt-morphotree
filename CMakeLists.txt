project(dt-morphotree)

cmake_minimum_required(VERSION 3.10)

set(CMACKE_CXX_STANDARD 14)

include_directories(include)

# OPENGL and glfw
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

# CONAN CONFIG
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

file(GLOB_RECURSE PROJECT_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
set(PROJECT_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM PROJECT_SOURCE ${PROJECT_MAIN})

set(BF_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/src/bf.cpp")
list(REMOVE_ITEM PROJECT_SOURCE ${BF_MAIN})


add_library(dt-morphotree STATIC ${PROJECT_SOURCE})
target_link_libraries(dt-morphotree PUBLIC
  ${CONAN_LIBS}
  OpenGL::GL
  glfw)

add_executable(dt-morphotree-app ${PROJECT_MAIN})
target_link_libraries(dt-morphotree-app dt-morphotree)

add_executable(bf ${BF_MAIN})
target_link_libraries(bf dt-morphotree)