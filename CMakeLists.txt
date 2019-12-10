cmake_minimum_required(VERSION 3.0)
project(start-opengl VERSION 1.0.0 LANGUAGES C CXX)

include(cmake/global.cmake)

set(MY_ROOT "${PROJECT_SOURCE_DIR}")

# cmake options

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()
if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX d)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
#set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# flags

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wformat")
if(NOT HOST_OS STREQUAL "Win")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
endif()
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

string(STRIP "${CMAKE_CXX_FLAGS}" CMAKE_CXX_FLAGS)
status("CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

# packages

LIST(APPEND CMAKE_MODULE_PATH "${MY_ROOT}/cmake/modules")

## glew

find_package(GLEW REQUIRED)

## glfw3

find_package(GLFW3 REQUIRED)

## glm

# libglm-dev is outdated (ext/), use explicit version instead
#find_package(GLM REQUIRED)
set(GLM_ROOT "${MY_ROOT}/third_party/glm")

## stb

set(STB_ROOT "${MY_ROOT}/third_party/stb")

## assimp

find_package(ASSIMP REQUIRED)

## libs

if(HOST_OS STREQUAL "Win")
  set(GL_LIBS glfw3 opengl32)
elseif(HOST_OS STREQUAL "Linux")
  find_package(OpenGL REQUIRED)
  add_definitions(${OPENGL_DEFINITIONS})
  find_package(X11 REQUIRED)
  # note that the order is important for setting the libs
  # use pkg-config --libs $(pkg-config --print-requires --print-requires-private glfw3) in a terminal to confirm
  set(GL_LIBS ${GLFW3_LIBRARY} X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL dl pthread)
  set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
elseif(HOST_OS STREQUAL "Mac")
  status_var(CMAKE_OSX_SYSROOT)
  include_directories(/System/Library/Frameworks)
  find_library(COCOA_LIBRARY Cocoa)
  find_library(OpenGL_LIBRARY OpenGL)
  find_library(IOKit_LIBRARY IOKit)
  find_library(CoreVideo_LIBRARY CoreVideo)
  mark_as_advanced(COCOA_LIBRARY OpenGL_LIBRARY)
  set(APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY})
  set(APPLE_LIBS ${APPLE_LIBS} ${GLFW3_LIBRARY})
  set(GL_LIBS ${GL_LIBS} ${APPLE_LIBS})
else()
  set(GL_LIBS )
endif()

list(APPEND GL_LIBS GLEW::GLEW)
status("GL_LIBS: ${GL_LIBS}")

# targets

set(MY_OUTPUT "${MY_ROOT}/_output")
set_outdir(
  ARCHIVE "${MY_OUTPUT}/lib"
  LIBRARY "${MY_OUTPUT}/lib"
  RUNTIME "${MY_OUTPUT}/bin"
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

include_directories(
  ${GLM_ROOT}
  ${STB_ROOT}
  ${ASSIMP_INCLUDE_DIR}
  ${MY_ROOT}/include
)

add_subdirectory(src)
