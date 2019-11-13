#!/usr/bin/env bash

# _VERBOSE_=1
# _INIT_LINTER_=1
# _FORCE_INSRALL_=1
_INSTALL_OPTIONS_=$@

BASE_DIR=$(cd "$(dirname "$0")" && pwd)
ROOT_DIR=$(realpath "$BASE_DIR/..")

source "$BASE_DIR/init_tools.sh"

## deps

_echo_s "Init deps"

if [ "$HOST_OS" = "Linux" ]; then
  # On Trusty, please install glfw3 from source
  #   E: Unable to locate package libglfw3-dev
  _install_deps "$SUDO apt-get install" libglew-dev libglfw3-dev
elif [ "$HOST_OS" = "Mac" ]; then
  _install_deps "brew install" glew glfw
elif [ "$HOST_OS" = "Win" ]; then
  if [ "$HOST_NAME" = "MINGW" ]; then
    if [ "$HOST_ARCH" = "x64" ]; then
      _install_deps "pacman -S" mingw-w64-x86_64-glew mingw-w64-x86_64-glfw
    elif [ "$HOST_ARCH" = "x86" ]; then
      _install_deps "pacman -S" mingw-w64-i686-glew mingw-w64-i686-glfw
    else
      _echo_e "Unknown host arch :("
      exit 1
    fi
  fi
else  # unexpected
  _echo_e "Unknown host os :("
  exit 1
fi

## cmake version

_cmake_version=`cmake --version | head -1`
_cmake_version=${_cmake_version#*version}
_cmake_version_major=`echo ${_cmake_version} | cut -d'.' -f1`
if [ "$_cmake_version_major" -lt "3" ]; then
  _echo_s "Expect cmake version >= 3.0.0"
  if [ "$HOST_NAME" = "Ubuntu" ]; then
    # sudo apt remove cmake
    _echo "How to upgrade cmake in Ubuntu"
    _echo "  https://askubuntu.com/questions/829310/how-to-upgrade-cmake-in-ubuntu"
  fi
fi

exit 0
