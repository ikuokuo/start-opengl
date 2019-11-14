# https://learnopengl.com

set(MY_CURR ${CMAKE_CURRENT_LIST_DIR})

# add_gl_executable(NAME outdir LIBS libs)
macro(add_gl_executable NAME)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs LIBS)
  cmake_parse_arguments(THIS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_executable(${NAME}
    ${MY_ROOT}/src/base/glfw_base.cpp
    ${MY_CURR}/${NAME}.cpp
  )
  target_include_directories(${NAME} PUBLIC
    "$<BUILD_INTERFACE:${MY_ROOT}/src>"
    "$<BUILD_INTERFACE:${MY_CURR}/..>"
  )
  target_link_libraries(${NAME} ${GL_LIBS} ${THIS_LIBS})
  target_compile_definitions(${NAME} PUBLIC
    MY_DIR="${MY_CURR}/.."
  )
endmacro()

add_subdirectory(${MY_CURR}/1_getting_started)
add_subdirectory(${MY_CURR}/2_lighting)
add_subdirectory(${MY_CURR}/3_model_loading)
add_subdirectory(${MY_CURR}/4_advanced_opengl)
