cmake_minimum_required(VERSION 3.15)

project("ubatt" C)

include(FindPkgConfig)

pkg_check_modules(GLIB "glib-2.0")
pkg_check_modules(UPOWER "upower-glib")

include_directories(${GLIB_INCLUDE_DIRS} ${UPOWER_INCLUDE_DIRS})
set(CMAKE_C_FLAGS "-std=gnu99 -Werror=implicit")

add_executable("ubatt" "ubatt.c")
target_link_libraries("ubatt" ${GLIB_LIBRARIES} ${UPOWER_LIBRARIES} "m")

install(TARGETS "ubatt" DESTINATION "bin")
