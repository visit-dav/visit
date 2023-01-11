# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

target_sources(visitcommon PRIVATE
    AtomicProperties.C
    CoordUtils.C
    GeometricHelpers.C
    Namescheme.C
    StringHelpers.C
    Utility.C
    visit_gzstream.C
    void_ref_ptr.C)

if(WIN32)
    target_sources(visitcommon PRIVATE win32-regex.c)
    set_source_files_properties(win32-regex.c
        PROPERTIES LANGUAGE CXX)
endif()

target_include_directories(visitcommon PUBLIC .)

if(VISIT_ENABLE_UNIT_TESTS)
    add_executable(MRUCache_test MRUCache_test.C)
    set_target_properties(MRUCache_test PROPERTIES FOLDER unit_tests)

    add_executable(Namescheme_test Namescheme_test.C)
    target_link_libraries(Namescheme_test ${VISIT_EXE_LINKER_FLAGS}
         visitcommon ${ZLIB_LIBRARY} ${CMAKE_THREAD_LIBS} ${DL_LIB})
    set_target_properties(Namescheme_test PROPERTIES FOLDER unit_tests)

    add_executable(Utility_test Utility_test.C)
    target_link_libraries(Utility_test ${VISIT_EXE_LINKER_FLAGS} visitcommon
        ${ZLIB_LIBRARY} ${CMAKE_THREAD_LIBS} ${DL_LIB})
    set_target_properties(Utility_test PROPERTIES FOLDER unit_tests)

    add_executable(StringHelpers_test StringHelpers_test.C)
    target_link_libraries(StringHelpers_test ${VISIT_EXE_LINKER_FLAGS}
            visitcommon ${ZLIB_LIBRARY} ${CMAKE_THREAD_LIBS} ${DL_LIB})
    set_target_properties(StringHelpers_test PROPERTIES FOLDER unit_tests)
endif()

