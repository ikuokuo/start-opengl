# - Try to find Assimp
# Once done, this will define
#
# ASSIMP_FOUND - system has Assimp
# ASSIMP_INCLUDE_DIR - the Assimp include directories
# ASSIMP_LIBRARIES - link these to use Assimp

FIND_PATH(ASSIMP_INCLUDE_DIR assimp/mesh.h
  /usr/include
  /usr/local/include
  /opt/local/include
  ${CMAKE_SOURCE_DIR}/includes
)

FIND_LIBRARY(ASSIMP_LIBRARY assimp
  /usr/lib64
  /usr/lib
  /usr/local/lib
  /opt/local/lib
  ${CMAKE_SOURCE_DIR}/lib
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ASSIMP DEFAULT_MSG ASSIMP_INCLUDE_DIR ASSIMP_LIBRARY)

IF(ASSIMP_FOUND)
  SET(ASSIMP_INCLUDE_DIRS ${ASSIMP_INCLUDE_DIR})
  SET(ASSIMP_LIBRARIES ${ASSIMP_LIBRARY})
ENDIF()
