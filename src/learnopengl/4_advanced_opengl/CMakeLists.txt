# https://learnopengl.com

set(MY_CURR ${CMAKE_CURRENT_LIST_DIR})

set(MY_NAME "learnopengl/4_advanced_opengl")
set(_outdir "${MY_ROOT}/_output")
set_outdir(
  ARCHIVE "${_outdir}/lib/${MY_NAME}"
  LIBRARY "${_outdir}/lib/${MY_NAME}"
  RUNTIME "${_outdir}/bin/${MY_NAME}"
)
unset(_outdir)

## targets

set(gl_names
  1_1_depth_testing
  2_stencil_testing
  3_1_blending_discard
  3_2_blending_sorted
  4_face_culling
  5_1_framebuffers
  6_1_cubemaps_skybox
  8_advanced_glsl_ubo
  9_1_geometry_shader_houses
  10_1_instancing_quads
  11_anti_aliasing_multisampling
  11_anti_aliasing_offscreen
)
foreach(gl_name IN LISTS gl_names)
  add_gl_executable(${gl_name})
endforeach()

set(gl2_names
  6_2_cubemaps_environment_mapping
  6_3_cubemaps_reflection_model
  9_2_geometry_shader_exploding
  9_3_normal_visualization
  10_2_asteroids
  10_3_asteroids_instanced
)
foreach(gl_name IN LISTS gl2_names)
  add_gl_executable(${gl_name} LIBS ${ASSIMP_LIBRARIES})
endforeach()

## install

install(TARGETS ${gl_names} ${gl2_names}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/${MY_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${MY_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${MY_NAME}
)
