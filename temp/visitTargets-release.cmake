#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "visit::bow" for configuration "Release"
set_property(TARGET visit::bow APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(visit::bow PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/bow.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/bow.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS visit::bow )
list(APPEND _IMPORT_CHECK_FILES_FOR_visit::bow "${_IMPORT_PREFIX}/lib/bow.lib" "${_IMPORT_PREFIX}/bin/bow.dll" )

# Import target "visit::visitcommon" for configuration "Release"
set_property(TARGET visit::visitcommon APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(visit::visitcommon PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/visitcommon.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/visitcommon.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS visit::visitcommon )
list(APPEND _IMPORT_CHECK_FILES_FOR_visit::visitcommon "${_IMPORT_PREFIX}/lib/visitcommon.lib" "${_IMPORT_PREFIX}/bin/visitcommon.dll" )

# Import target "visit::xmledit" for configuration "Release"
set_property(TARGET visit::xmledit APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(visit::xmledit PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/xmledit.exe"
  )

list(APPEND _IMPORT_CHECK_TARGETS visit::xmledit )
list(APPEND _IMPORT_CHECK_FILES_FOR_visit::xmledit "${_IMPORT_PREFIX}/bin/xmledit.exe" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
