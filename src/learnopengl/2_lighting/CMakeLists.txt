# https://learnopengl.com

set(MY_CURR ${CMAKE_CURRENT_LIST_DIR})

set(MY_NAME "learnopengl/2_lighting")
set_outdir(
  ARCHIVE "${MY_OUTPUT}/lib/${MY_NAME}"
  LIBRARY "${MY_OUTPUT}/lib/${MY_NAME}"
  RUNTIME "${MY_OUTPUT}/bin/${MY_NAME}"
)

## targets

set(gl_names
  1_colors
  2_1_basic_lighting_diffuse
  2_2_basic_lighting_specular
  3_1_materials
  4_1_lighting_maps_diffuse
  4_2_lighting_maps_specular
  5_1_light_casters_directional
  5_2_light_casters_point
  5_3_light_casters_spot
  5_4_light_casters_spot_soft
  6_multiple_lights
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
