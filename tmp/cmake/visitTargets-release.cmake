#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "visitcommon" for configuration "Release"
set_property(TARGET visitcommon APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(visitcommon PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/3.3.3/linux-x86_64/lib/libvisitcommon.so"
  IMPORTED_SONAME_RELEASE "libvisitcommon.so"
  )

list(APPEND _cmake_import_check_targets visitcommon )
list(APPEND _cmake_import_check_files_for_visitcommon "${_IMPORT_PREFIX}/3.3.3/linux-x86_64/lib/libvisitcommon.so" )

# Import target "lightweight_visit_vtk" for configuration "Release"
set_property(TARGET lightweight_visit_vtk APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(lightweight_visit_vtk PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/3.3.3/linux-x86_64/lib/liblightweight_visit_vtk.so"
  IMPORTED_SONAME_RELEASE "liblightweight_visit_vtk.so"
  )

list(APPEND _cmake_import_check_targets lightweight_visit_vtk )
list(APPEND _cmake_import_check_files_for_lightweight_visit_vtk "${_IMPORT_PREFIX}/3.3.3/linux-x86_64/lib/liblightweight_visit_vtk.so" )

# Import target "visit_vtk" for configuration "Release"
set_property(TARGET visit_vtk APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(visit_vtk PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/3.3.3/linux-x86_64/lib/libvisit_vtk.so"
  IMPORTED_SONAME_RELEASE "libvisit_vtk.so"
  )

list(APPEND _cmake_import_check_targets visit_vtk )
list(APPEND _cmake_import_check_files_for_visit_vtk "${_IMPORT_PREFIX}/3.3.3/linux-x86_64/lib/libvisit_vtk.so" )

# Import target "avtdbatts" for configuration "Release"
set_property(TARGET avtdbatts APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(avtdbatts PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/3.3.3/linux-x86_64/lib/libavtdbatts.so"
  IMPORTED_SONAME_RELEASE "libavtdbatts.so"
  )

list(APPEND _cmake_import_check_targets avtdbatts )
list(APPEND _cmake_import_check_files_for_avtdbatts "${_IMPORT_PREFIX}/3.3.3/linux-x86_64/lib/libavtdbatts.so" )

# Import target "avtmath" for configuration "Release"
set_property(TARGET avtmath APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(avtmath PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/3.3.3/linux-x86_64/lib/libavtmath.so"
  IMPORTED_SONAME_RELEASE "libavtmath.so"
  )

list(APPEND _cmake_import_check_targets avtmath )
list(APPEND _cmake_import_check_files_for_avtmath "${_IMPORT_PREFIX}/3.3.3/linux-x86_64/lib/libavtmath.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
