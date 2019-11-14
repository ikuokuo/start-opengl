set(MY_CURR ${CMAKE_CURRENT_LIST_DIR})

## glfw_demo

add_executable(glfw_demo
  ${MY_CURR}/base/glfw_base.cpp
  ${MY_CURR}/glfw_demo.cpp
)

target_include_directories(glfw_demo PUBLIC
  "$<BUILD_INTERFACE:${MY_CURR}>"
)

target_link_libraries(glfw_demo ${GL_LIBS})

## install

install(TARGETS glfw_demo
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
