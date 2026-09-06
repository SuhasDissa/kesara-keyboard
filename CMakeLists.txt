cmake_minimum_required(VERSION 3.16)
project(kesara VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CTest)

option(BUILD_IBUS "Install the IBus/m17n table" ON)
set(BUILD_FCITX4 "AUTO" CACHE STRING "Build Fcitx 4 addon: ON, OFF, or AUTO")
set(BUILD_FCITX5 "AUTO" CACHE STRING "Build Fcitx 5 addon: ON, OFF, or AUTO")
option(BUILD_TESTS "Build engine tests" ON)

add_library(kesara_engine STATIC src/engine/kesara.c)
target_include_directories(kesara_engine PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/engine>)
set_target_properties(kesara_engine PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(BUILD_IBUS)
  install(FILES m17n/si-kesara.mim
          DESTINATION ${CMAKE_INSTALL_DATADIR}/m17n
          COMPONENT ibus)
endif()

find_package(PkgConfig QUIET)

# ---- Fcitx 5 ----
set(BUILD_FCITX5_REAL OFF)
if(BUILD_FCITX5 STREQUAL "OFF")
  # skip
elseif(BUILD_FCITX5 STREQUAL "ON")
  find_package(Fcitx5Core REQUIRED)
  set(BUILD_FCITX5_REAL ON)
else()
  find_package(Fcitx5Core QUIET)
  if(Fcitx5Core_FOUND)
    set(BUILD_FCITX5_REAL ON)
  endif()
endif()

if(BUILD_FCITX5_REAL)
  include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")
  add_library(kesara MODULE
    src/fcitx5/kesara-im.cpp)
  target_link_libraries(kesara PRIVATE kesara_engine Fcitx5::Core)
  set_target_properties(kesara PROPERTIES PREFIX "")
  configure_file(src/fcitx5/kesara-addon.conf.in
                 ${CMAKE_CURRENT_BINARY_DIR}/kesara-addon.conf @ONLY)
  install(TARGETS kesara
          DESTINATION "${FCITX_INSTALL_LIBDIR}/fcitx5"
          COMPONENT fcitx5)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kesara-addon.conf"
          DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon"
          RENAME kesara.conf
          COMPONENT fcitx5)
  install(FILES src/fcitx5/kesara.conf
          DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod"
          COMPONENT fcitx5)
endif()

# ---- Fcitx 4 ----
set(BUILD_FCITX4_REAL OFF)
if(BUILD_FCITX4 STREQUAL "OFF")
  # skip
else()
  if(NOT COMMAND pkg_check_modules)
    if(BUILD_FCITX4 STREQUAL "ON")
      find_package(PkgConfig REQUIRED)
    else()
      find_package(PkgConfig QUIET)
    endif()
  endif()
  if(COMMAND pkg_check_modules)
    if(BUILD_FCITX4 STREQUAL "ON")
      pkg_check_modules(FCITX REQUIRED fcitx)
      set(BUILD_FCITX4_REAL ON)
    else()
      pkg_check_modules(FCITX fcitx)
      if(FCITX_FOUND)
        set(BUILD_FCITX4_REAL ON)
      endif()
    endif()
  elseif(BUILD_FCITX4 STREQUAL "ON")
    message(FATAL_ERROR "BUILD_FCITX4=ON but pkg-config is not available")
  endif()
endif()

if(BUILD_FCITX4_REAL)
  add_library(fcitx-kesara MODULE src/fcitx4/eim.c)
  target_include_directories(fcitx-kesara PRIVATE ${FCITX_INCLUDE_DIRS})
  target_compile_options(fcitx-kesara PRIVATE ${FCITX_CFLAGS_OTHER})
  target_link_libraries(fcitx-kesara PRIVATE kesara_engine ${FCITX_LIBRARIES})
  set_target_properties(fcitx-kesara PROPERTIES PREFIX "")
  if(FCITX_LIBRARY_DIRS)
    target_link_directories(fcitx-kesara PRIVATE ${FCITX_LIBRARY_DIRS})
  endif()
  pkg_get_variable(FCITX_ADDONDIR fcitx addondir)
  pkg_get_variable(FCITX_DATADIR fcitx pkgdatadir)
  if(NOT FCITX_ADDONDIR)
    set(FCITX_ADDONDIR "${CMAKE_INSTALL_LIBDIR}/fcitx")
  endif()
  if(NOT FCITX_DATADIR)
    set(FCITX_DATADIR "${CMAKE_INSTALL_DATADIR}/fcitx")
  endif()
  install(TARGETS fcitx-kesara
          DESTINATION "${FCITX_ADDONDIR}"
          COMPONENT fcitx4)
  install(FILES src/fcitx4/fcitx-kesara.conf
          DESTINATION "${FCITX_DATADIR}/addon"
          COMPONENT fcitx4)
endif()

if(BUILD_TESTS)
  add_executable(test_kesara tests/test_kesara.c)
  target_link_libraries(test_kesara PRIVATE kesara_engine)
  add_test(NAME kesara_engine COMMAND test_kesara)
endif()

message(STATUS "BUILD_IBUS=${BUILD_IBUS}")
message(STATUS "BUILD_FCITX4_REAL=${BUILD_FCITX4_REAL}")
message(STATUS "BUILD_FCITX5_REAL=${BUILD_FCITX5_REAL}")
