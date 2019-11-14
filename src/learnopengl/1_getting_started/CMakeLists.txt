# https://learnopengl.com

set(MY_CURR ${CMAKE_CURRENT_LIST_DIR})

set(MY_NAME "learnopengl/1_getting_started")
set(_outdir "${MY_ROOT}/_output")
set_outdir(
  ARCHIVE "${_outdir}/lib/${MY_NAME}"
  LIBRARY "${_outdir}/lib/${MY_NAME}"
  RUNTIME "${_outdir}/bin/${MY_NAME}"
)
unset(_outdir)

## targets

set(gl_names
  2_1_hello_triangle
  3_1_shaders_uniform
  3_2_shaders_interpolation
  4_1_textures
  4_2_textures_combined
  5_1_transformations
  6_1_coordinate_systems
  6_2_coordinate_systems_depth
  6_3_coordinate_systems_multiple
  7_1_camera_circle
  7_2_camera_keyboard_dt
  7_3_camera_mouse_zoom
  7_4_camera_class
)

foreach(gl_name IN LISTS gl_names)
  add_gl_executable(${gl_name})
endforeach()

## install

install(TARGETS ${gl_names}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/${MY_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${MY_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${MY_NAME}
)
