cmake_minimum_required(VERSION 3.8)

project(fpslogger LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_BASE REQUIRED gstreamer-base-1.0)

add_library(${PROJECT_NAME} SHARED
    gstfpslogger.cpp
    fpslogger.cpp
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${GLIB_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_BASE_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${GLIB_LIBRARIES}
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_BASE_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
    COMPONENT libfpslogger
    LIBRARY 
      DESTINATION /lib/x86_64-linux-gnu/gstreamer-1.0
)