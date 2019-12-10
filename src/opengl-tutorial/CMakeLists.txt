# http://www.opengl-tutorial.org

set(MY_CURR ${CMAKE_CURRENT_LIST_DIR})

set(MY_NAME "opengl-tutorial")
set_outdir(
  ARCHIVE "${MY_OUTPUT}/lib/${MY_NAME}"
  LIBRARY "${MY_OUTPUT}/lib/${MY_NAME}"
  RUNTIME "${MY_OUTPUT}/bin/${MY_NAME}"
)

## t03_matrices

add_executable(t03_matrices
  ${MY_ROOT}/src/base/glfw_base.cpp
  ${MY_CURR}/common/shader.cpp
  ${MY_CURR}/t03_matrices/t03_matrices.cpp
)
target_include_directories(t03_matrices PUBLIC
  "$<BUILD_INTERFACE:${MY_ROOT}/src>"
  "$<BUILD_INTERFACE:${MY_CURR}>"
)
target_link_libraries(t03_matrices ${GL_LIBS})

## install

install(TARGETS t03_matrices
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/${MY_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${MY_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${MY_NAME}
)
