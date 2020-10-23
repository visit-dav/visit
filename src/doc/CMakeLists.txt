# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu March 7, 2019
#   Added sphinx_launcher (python executable) for Windows.
#
#   Kathleen Biagas, Wed Oct 21, 2020
#   Replaced old logic with python-3 creation of manuals, which was moved
#   from root CMakeLists.txt.
#
#****************************************************************************

if(VISIT_PYTHON3_DIR)
  if(WIN32)
    # Need a different path for windows because sphinx-build
    # is in a different location.

    # don't pollute the src repo, build the docs in the binary dir.
    # This doesn't allow the manuals to be found if running a dev build,
    # so location may need to be rethought, but other than building them
    # in exe/Release and in exe/Debug, thought this location was best.
    add_custom_target(manuals
        COMMAND ${VISIT_PYTHON3_DIR}/python.exe
                ${VISIT_PYTHON3_DIR}/Scripts/sphinx-build-script.py
        -b html ${VISIT_SOURCE_DIR}/doc 
        ${VISIT_BINARY_DIR}/resources/help/en_US/manual -a)
    # Add command to ensure the manuals are installed to the correct location
    install(DIRECTORY ${VISIT_BINARY_DIR}/resources/help/en_US/manual
        DESTINATION ${VISIT_INSTALLED_VERSION_RESOURCES}/help/en_US/
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE
                         GROUP_READ GROUP_WRITE
                         WORLD_READ
        DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                              GROUP_READ GROUP_WRITE GROUP_EXECUTE
                              WORLD_READ             WORLD_EXECUTE
        CONFIGURATIONS Debug Release RelWithDebInfo MinSizeRel)
  elseif(APPLE)
    add_custom_target(manuals
                  COMMAND ${VISIT_PYTHON3_DIR}/bin/sphinx-build
                  -b html ${VISIT_SOURCE_DIR}/doc
                  ${VISIT_BINARY_DIR}/resources/help/en_US/manual -a)
    # Add command to ensure the manuals are installed to the correct location
    install(DIRECTORY ${VISIT_BINARY_DIR}/resources/help/en_US/manual
        DESTINATION ${VISIT_INSTALLED_VERSION_RESOURCES}/help/en_US/
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE
                         GROUP_READ GROUP_WRITE
                         WORLD_READ
        DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                              GROUP_READ GROUP_WRITE GROUP_EXECUTE
                              WORLD_READ             WORLD_EXECUTE
        CONFIGURATIONS Debug Release RelWithDebInfo MinSizeRel)
  else()
    add_custom_target(manuals
                  COMMAND ${VISIT_PYTHON3_DIR}/bin/sphinx-build
                  -b html ${VISIT_SOURCE_DIR}/doc
                  ${VISIT_BINARY_DIR}/resources/help/en_US/manual -a)
  endif()
endif()

