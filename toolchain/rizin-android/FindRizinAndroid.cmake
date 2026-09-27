# FindRizinAndroid.cmake
# Locates the prebuilt rizin produced by build-scripts/build.sh and exposes
# an imported target RizinAndroid::Rizin (static librz archives + headers).
#
# Inputs (optional):
#   RIZIN_ANDROID_ROOT - directory containing prebuilt/ (default: this file's dir)
#   ANDROID_ABI        - arm64-v8a | armeabi-v7a   (default: arm64-v8a)
# Outputs:
#   RizinAndroid_FOUND, RizinAndroid_INCLUDE_DIR, RizinAndroid_LIBRARIES
#   RizinAndroid::Rizin

set(RizinAndroid_ROOT_DIR "${RIZIN_ANDROID_ROOT}")
if(NOT RizinAndroid_ROOT_DIR)
  get_filename_component(RizinAndroid_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
endif()
if(NOT ANDROID_ABI)
  set(ANDROID_ABI "arm64-v8a")
endif()
set(_rizin_prefix "${RizinAndroid_ROOT_DIR}/prebuilt/${ANDROID_ABI}")

# Direct EXISTS check instead of find_path: NDK toolchain sets
# CMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY, which re-roots find_path away
# from our prebuilt dir.
set(RizinAndroid_INCLUDE_DIR "${_rizin_prefix}/include/librz")
if(NOT EXISTS "${RizinAndroid_INCLUDE_DIR}/rz_asm.h")
  set(RizinAndroid_INCLUDE_DIR "RizinAndroid_INCLUDE_DIR-NOTFOUND")
endif()

file(GLOB _rizin_libs "${_rizin_prefix}/lib/*.a")
if(_rizin_libs)
  set(RizinAndroid_LIBRARIES "${_rizin_libs}")
else()
  set(RizinAndroid_LIBRARIES "RizinAndroid_LIBRARIES-NOTFOUND")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RizinAndroid
  REQUIRED_VARS RizinAndroid_INCLUDE_DIR RizinAndroid_LIBRARIES)

if(RizinAndroid_FOUND AND NOT TARGET RizinAndroid::Rizin)
  add_library(RizinAndroid::Rizin INTERFACE IMPORTED)
  # --start-group/--end-group: the static librz archives reference each other
  # cyclically (core <-> bin <-> io ...), the group makes the linker resolve
  # them regardless of archive order.
  # librz/sdb is needed too: rz_cons.h et al. do #include <sdb.h>, which
  # installs under include/librz/sdb/ (same as rz_util.pc's Cflags).
  set_target_properties(RizinAndroid::Rizin PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${RizinAndroid_INCLUDE_DIR};${RizinAndroid_INCLUDE_DIR}/sdb"
    INTERFACE_LINK_LIBRARIES "-Wl,--start-group;${RizinAndroid_LIBRARIES};-Wl,--end-group;m;dl")
endif()

mark_as_advanced(RizinAndroid_INCLUDE_DIR RizinAndroid_LIBRARIES)
