project(libmchat C)
cmake_minimum_required(VERSION 2.8)

# Turns out cmake version before 3.1 did NOT include CMAKE_C_STANDARD as a property.
# This macro is to make up for this shortfall.
# Borrowed From: https://stackoverflow.com/questions/24840030/forcing-c99-in-cmake-to-use-for-loop-initial-declaration
macro(use_c99)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_C_FLAGS "--std=gnu99 ${CMAKE_C_FLAGS}")
    endif ()
  else ()
    set (CMAKE_C_STANDARD 99)
  endif ()
endmacro(use_c99)

# We need pkg-config for glib2 and doxygen to build documentation
find_package(PkgConfig REQUIRED)
find_package(Doxygen)

# Find glib2 with pkg-config
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GIO2 REQUIRED gio-2.0)
pkg_check_modules(GOBJECT2 REQUIRED gobject-2.0)
link_directories(${GLIB2_LIBRARY_DIRS} ${GIO2_LIBRARY_DIRS} ${GOBJECT2_LIBRARY_DIRS})
add_definitions(${GLIB2_CFLAGS_OTHER} ${GIO2_CFLAGS_OTHER} ${GOBJECT2_CFLAGS_OTHER})
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src LIBMCHAT_SRC_LIST)
include_directories(${GLIB2_INCLUDE_DIRS} ${GIO2_INCLUDE_DIRS} ${GOBJECT2_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB LIBMCHAT_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

# Build objects first (so a static and dynamic library can be built quickly)
add_library(${PROJECT_NAME}_obj OBJECT ${LIBMCHAT_SRC_LIST} ${LIBMCHAT_HEADER_FILES})
use_c99()
set_property(TARGET ${PROJECT_NAME}_obj PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(${PROJECT_NAME}_static STATIC $<TARGET_OBJECTS:${PROJECT_NAME}_obj>)
set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME mchat)

# Build shared library
add_library(${PROJECT_NAME}_shared SHARED $<TARGET_OBJECTS:${PROJECT_NAME}_obj>)
target_link_libraries(${PROJECT_NAME}_shared ${GLIB2_LIBRARIES} ${GIO2_LIBRARIES} ${GOBJECT2_LIBRARIES})
set_target_properties(${PROJECT_NAME}_shared PROPERTIES OUTPUT_NAME mchat)

#Build documentation
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(libmchat_doc ALL ${DOXYGEN_EXECUTABLE}
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API Documentation with Doxygen" VERBATIM)
else(DOXYGEN_FOUND)
    message(WARNING "Doxygen was not found so documentation was not built")
endif(DOXYGEN_FOUND)

#Qt Creator (Sean's IDE of choice) specific directives (make header files show up in source listing)
file(GLOB LIBMCHAT_READMES "${CMAKE_CURRENT_SOURCE_DIR}/README.md" "${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG")
add_custom_target(libmchat_headers SOURCES ${LIBMCHAT_HEADER_FILES})
add_custom_target(libmchat_nonbuild SOURCES ${LIBMCHAT_READMES})
