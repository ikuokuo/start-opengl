# https://learnopengl.com

set(MY_CURR ${CMAKE_CURRENT_LIST_DIR})

set(MY_NAME "learnopengl/3_model_loading")
set(_outdir "${MY_ROOT}/_output")
set_outdir(
  ARCHIVE "${_outdir}/lib/${MY_NAME}"
  LIBRARY "${_outdir}/lib/${MY_NAME}"
  RUNTIME "${_outdir}/bin/${MY_NAME}"
)
unset(_outdir)

## targets

set(gl_names
  1_model_loading
)

foreach(gl_name IN LISTS gl_names)
  add_gl_executable(${gl_name} LIBS ${ASSIMP_LIBRARIES})
endforeach()

## install

install(TARGETS ${gl_names}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/${MY_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${MY_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${MY_NAME}
)
