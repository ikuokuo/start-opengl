# http://www.opengl-tutorial.org

set(MY_CURR ${CMAKE_CURRENT_LIST_DIR})

set(MY_NAME "opengl-tutorial")
set(_outdir "${MY_ROOT}/_output")
set_outdir(
  ARCHIVE "${_outdir}/lib/${MY_NAME}"
  LIBRARY "${_outdir}/lib/${MY_NAME}"
  RUNTIME "${_outdir}/bin/${MY_NAME}"
)
unset(_outdir)

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
