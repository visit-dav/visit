# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu March 7, 2019
#   Added sphinx_launcher (python executable) for Windows.
#
#   Mark C. Miller, Mon Jun  8 16:45:05 PDT 2020
#   Added linkcheck builder
#
#   Kathleen Biagas, Wed Oct 21, 2020
#   Replaced old logic with python-3 creation of manuals, which was moved
#   from root CMakeLists.txt.
#
#   Kathleen Biagas, Thu Jan 7, 2021
#   Merge logic for creating the build and install commands, they are the
#   same for all platforms, except the sphinx build command which is
#   different on Windows.  Added logic to ensure sphinx build command exists.
#
#   Eric Brugger, Wed Feb 10 15:07:41 PST 2021
#   Update for switch to Python 3.
#
#   Kathleen Biagas, Thu Apr 22, 2021
#   Don't build manuals for VISIT_STATIC, as sphinx won't be built.
#
#   Kathleen Biagas, Thu July 1, 2021
#   Change FATAL_ERROR to WARNING with early return.
#
#   Cyrus Harrison, Wed Jan 19 12:56:45 PST 2022
#   Make sure `make install` will call docs build target, otherwise
#   make install can fail unless "make manuals" is called manually. 
#
#   Cyrus Harrison, Fri Feb 25 14:49:03 PST 2022
#   Add a `docs` cmake target that is an alais for `manuals`
#
#   Kathleen Biagas, Thu Mar 24, 2022
#   Use 'manuals.vcxproj' for building manuals during an install on Windows
#   with MSVC.
#
#   Kathleen Biagas, Wed Nov 15, 2023
#   Use sphinx-build.exe on Windows, due to newer python and sphinx.
#
#   Kathleen Biagas, Thue Dec 14, 2023
#   Revert back to using sphinx-build-script.py on Windows.  Though sphinx
#   didn't provide the script, the previous version works, with the version
#   number updated to current.  The sphinx-build executable doesn't work
#   on other machines, as it attempts to run python from the location where
#   sphinx/python was built.
#
#****************************************************************************

if(VISIT_PYTHON_DIR AND VISIT_ENABLE_MANUALS AND NOT VISIT_STATIC)
    message(STATUS "Configure manuals targets")
    set(errmsgtail "To remove this warning, either install sphinx or set VISIT_ENABLE_MANUALS to false.")
    if(WIN32)
        # Need a different sphinx build command for windows
        if(NOT EXISTS ${VISIT_PYTHON_DIR}/Scripts/sphinx-build-script.py)
            message(WARNING "Manuals are enabled but"
                   " ${VISIT_PYTHON_DIR}/Scripts/sphinx-build-script.py"
                   " does not exist so manuals will not be built. ${errmsgtail}")
            return()
        endif()
        set(sphinx_build_cmd "${VISIT_PYTHON_DIR}/python.exe \ "
              "${VISIT_PYTHON_DIR}/Scripts/sphinx-build-script.py")
    else()
        if(NOT EXISTS ${VISIT_PYTHON_DIR}/bin/sphinx-build)
            message(WARNING "Manuals are enabled but"
                    " ${VISIT_PYTHON_DIR}/bin/sphinx-build"
                    " does not exist so manuals will not be built. ${errmsgtail}")
            return()
        endif()
        set(sphinx_build_cmd ${VISIT_PYTHON_DIR}/bin/sphinx-build)
    endif()

    # Add custom target to build the manuals
    add_custom_target(manuals COMMAND ${sphinx_build_cmd}
        -b html ${VISIT_SOURCE_DIR}/doc
        ${VISIT_BINARY_DIR}/resources/help/en_US/manual -a)

    #
    # Allow `make docs` to be an alias for `make manuals`
    #
    add_dependencies(docs manuals)


    ######
    # This snippet makes sure if we do a make install w/o the optional "docs"
    # target built, it will be built during the install process.
    ######

    if(MSVC)
        install(CODE "execute_process(COMMAND \"${CMAKE_BUILD_TOOL}\" manuals.vcxproj WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\")")
    else()
        install(CODE "execute_process(COMMAND \"${CMAKE_BUILD_TOOL}\" manuals WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\")")
    endif()

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
endif()
