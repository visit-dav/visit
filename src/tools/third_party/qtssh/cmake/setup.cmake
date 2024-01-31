# Forcibly re-enable assertions, even if we're building in release
# mode. This is a security project - assertions may be enforcing
# security-critical constraints. A backstop #ifdef in defs.h should
# give a #error if this manoeuvre doesn't do what it needs to.

# LLNL - changing defs.h to #undefine NDEBUG instead of mucking with
# CMAKE_C_FLAGS_XXX because that isn't working correctly within the context
# of building this within VisIt.


set(PUTTY_IPV6 ON CACHE INTERNAL
    "Build PuTTY with IPv6 support if possible")
set(PUTTY_DEBUG OFF CACHE INTERNAL
    "Build PuTTY with debug() statements enabled")
set(PUTTY_FUZZING OFF CACHE INTERNAL
   "Build PuTTY binaries suitable for fuzzing, NOT FOR REAL USE")
set(PUTTY_COVERAGE OFF CACHE INTERNAL
    "Build PuTTY binaries suitable for code coverage analysis")
set(STRICT OFF CACHE INTERNAL
   "Enable extra compiler warnings and make them errors")


set(GENERATED_SOURCES_DIR ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY})

#set(GENERATED_LICENCE_H ${GENERATED_SOURCES_DIR}/licence.h)
#set(INTERMEDIATE_LICENCE_H ${GENERATED_LICENCE_H}.tmp)
#add_custom_command(OUTPUT ${INTERMEDIATE_LICENCE_H}
#  COMMAND ${CMAKE_COMMAND}
#    -DLICENCE_FILE=${putty_SOURCE_DIR}/LICENCE
#    -DOUTPUT_FILE=${INTERMEDIATE_LICENCE_H}
#    -P ${putty_SOURCE_DIR}/cmake/licence.cmake
#  DEPENDS ${putty_SOURCE_DIR}/cmake/licence.cmake ${putty_SOURCE_DIR}/LICENCE)
#add_custom_target(generated_licence_h
#  BYPRODUCTS ${GENERATED_LICENCE_H}
#  COMMAND ${CMAKE_COMMAND} -E copy_if_different
#    ${INTERMEDIATE_LICENCE_H} ${GENERATED_LICENCE_H}
#  DEPENDS ${INTERMEDIATE_LICENCE_H}
#  COMMENT "Updating licence.h")
#VISIT_TOOLS_ADD_FOLDER(qtssh generated_licence_h)

function(add_sources_from_current_dir target)
  set(sources)
  foreach(i ${ARGN})
    set(sources ${sources} ${CMAKE_CURRENT_SOURCE_DIR}/${i})
  endforeach()
  target_sources(${target} PRIVATE ${sources})
endfunction()

set(extra_dirs)
if(CMAKE_SYSTEM_NAME MATCHES "Windows" OR WINELIB)
  set(platform windows)
else()
  set(platform unix)
endif()

function(be_list TARGET NAME)
  cmake_parse_arguments(OPT "SSH;SERIAL;OTHERBACKENDS" "" "" "${ARGN}")
  add_library(${TARGET}-be-list OBJECT ${putty_SOURCE_DIR}/be_list.c)
  foreach(setting SSH SERIAL OTHERBACKENDS)
    if(OPT_${setting})
      target_compile_definitions(${TARGET}-be-list PRIVATE ${setting}=1)
    else()
      target_compile_definitions(${TARGET}-be-list PRIVATE ${setting}=0)
    endif()
  endforeach()
  target_compile_definitions(${TARGET}-be-list PRIVATE APPNAME=${NAME})
  target_include_directories(${TARGET}-be-list PRIVATE
      ${putty_SOURCE_DIR}/${platform}
      ${GENERATED_SOURCES_DIR})
  target_sources(${TARGET} PRIVATE $<TARGET_OBJECTS:${TARGET}-be-list>)
  VISIT_TOOLS_ADD_FOLDER(qtssh ${TARGET}-be-list)
endfunction()

include(cmake/platforms/${platform}.cmake)

if(PUTTY_DEBUG)
  add_compile_definitions(DEBUG)
endif()
if(PUTTY_FUZZING)
  add_compile_definitions(FUZZING)
endif()
if(PUTTY_COVERAGE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -g ")
endif()
