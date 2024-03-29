# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#  Hank Childs, Tue Jan  5 07:13:58 PST 2010
#  Remove xml2makefile and xml2projectfile.
#
#  Kathleen Bonnell, Wed Feb  3 17:22:12 PST 2010
#  Add install for visit_exe.
#
#  Kathleen Bonnell, Sun Mar  14 14:06:51 MST 2010
#  Change copy_directory for VISIT_BIN_DIRECTORIES to a FILE(COPY 
#  so that '.svn' can be skipped.
#
#  Kathleen Bonnell, Mon Mar  22 11:08:46 MST 2010
#  Don't copy VISIT_BIN_FILES on windows, instead only copy makemovie.py,
#  makemoviemain.py and visitdiff.py.
#
#  Eric Brugger, Tue Apr  6 14:22:02 PDT 2010
#  I changed the installation location of makemovie.py and makemoviemain.py
#  to ${VISIT_INSTALLED_VERSION_BIN} instead of bin.
#
#  Cyrus Harrison, Wed Apr 21 12:25:20 PDT 2010
#  Added copy of qt.conf to ${VISIT_INSTALLED_VERSION_BIN}
#  This file is used to prevent qt from loading plugins from other qt
#  installs.
#
#  Eric Brugger, Wed Feb 16 11:50:39 PST 2011
#  I changed the installation location of visitdiff.py to the same location
#  that makemovie.py and makemoviemain.py get installed.
#
#  Brad Whitlock, Tue May  1 12:01:49 PDT 2012
#  I moved some directories up into resources.
#
#  Brad Whitlock, Fri May 18 16:09:34 PST 2012
#  Use different resource file.
#
#  Brad Whitlock, Wed Mar 13 11:41:38 PDT 2013
#  Install frontendlauncher.py
#
#  Kathleen Biagas, Thu Mar 24 16:29:50 PDT 2016
#  Remove qt.conf in favor of having FindVisItQtx.cmake create one on the fly.
#
#  Alister Maguire, Tue Oct 15 10:09:02 PDT 2019
#  Copy over the makemili_driver to the exe directory. 
#
#  Eric Brugger, Thu Dec 19 14:57:28 PST 2019
#  I added mcurvit.
#
#  Eric Brugger, Wed Feb 12 15:37:23 PST 2020
#  I renamed makemili_driver to makemili_driver_ser since that is what
#  the frontendlauncher is expecting it to be named. I also explicitly
#  create the exe directory before copying it the directory, since the
#  creation of the directory is non deterministic with regards to different
#  operating systems and versions.
#
#  Eric Brugger, Fri Jan 21 13:16:48 PST 2022
#  I added qtviswinExample, qtvtkExample and qvtkopenglExample.
#
#  Kathleen Biagas, Tue Jan 31, 2023
#  Change how files are copied on Windows.
#
#****************************************************************************

#-----------------------------------------------------------------------------
# Copy necessary bin files & directories to the build dir.
#----------------------------------------------------------------------------

if(NOT WIN32)
    set(VISIT_BIN_FILES
        Doxyfile
        curv3dprep
        frontendlauncher
        frontendlauncher.py
        makemili_driver
        mcurvit
        mkgrdbl
        mpeg2encode
        qtviswinExample
        qtvtkExample
        qvtkopenglExample
        regen-ajp
        run_doxygen
        surfcomp
        text2polys
        time_annotation
        visit
        visit_composite
        visit_transition
        visitconvert
        visitprotocol
        xml2atts
        xml2avt
        xml2cmake
        xml2info
        xml2java
        xml2plugin
        xml2python
        xml2window
        xmledit
        xmltest)

    if(APPLE)
        #--------------------------------------------------------------------------
        # Create launcher binary for macos. This is the thing double-clicking the
        # icon actually has the effect of executing.
        #--------------------------------------------------------------------------
        add_executable(visit_macos_launcher ${VISIT_APPLICATION_STYLE} visit_macos_launcher.c ${VISIT_VISIT_RESOURCE_FILE})
        set_source_files_properties(visit_macos_launcher.c PROPERTIES LANGUAGE C)
    endif()

    #
    # Copy files to the bin and exe directories. We have to explicitly
    # create the exe directory since we can't control when it gets created
    # otherwise.
    #
    # this is copying to build dir
    foreach(VISIT_BIN_ITEM ${VISIT_BIN_FILES} internallauncher makemovie.py makemoviemain.py visitcinema.py visitcinemamain.py visitdiff.py)
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                                ${CMAKE_CURRENT_SOURCE_DIR}/${VISIT_BIN_ITEM}
                                ${CMAKE_CURRENT_BINARY_DIR}/${VISIT_BIN_ITEM})
    endforeach()

    file(MAKE_DIRECTORY ${VISIT_EXECUTABLE_DIR})
    configure_file(makemili_driver_ser ${VISIT_EXECUTABLE_DIR} COPYONLY)

    #
    # Install the files. This happens with make install or make package.
    #
    install(FILES ${VISIT_BIN_FILES}
            DESTINATION bin
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_WRITE GROUP_EXECUTE
                        WORLD_READ             WORLD_EXECUTE) 

    install(FILES internallauncher
        DESTINATION ${VISIT_VERSION}/bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_WRITE GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE)

    install(FILES makemovie.py makemoviemain.py visitcinema.py visitcinemamain.py
                  visitdiff.py makemili_driver_ser
        DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_WRITE GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE)

    unset(VISIT_BIN_FILES)
else()

    #--------------------------------------------------------------------------
    # Create VisIt launcher binary for windows
    #--------------------------------------------------------------------------
    add_executable(visit_exe ${VISIT_APPLICATION_STYLE} visit.c ${VISIT_VISIT_RESOURCE_FILE})
    set_target_properties(visit_exe PROPERTIES OUTPUT_NAME visit)
    set_source_files_properties(visit.c PROPERTIES LANGUAGE CXX)
    target_include_directories(visit_exe PRIVATE ${VISIT_COMMON_INCLUDES})
    target_link_libraries(visit_exe PRIVATE shlwapi)
    VISIT_INSTALL_TARGETS(visit_exe)

    #--------------------------------------------------------------------------
    # Install and copy to build the movie helper scripts
    #--------------------------------------------------------------------------
    set(VISIT_BIN_WIN_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/makemovie.py
        ${CMAKE_CURRENT_SOURCE_DIR}/makemoviemain.py
        ${CMAKE_CURRENT_SOURCE_DIR}/visitcinema.py
        ${CMAKE_CURRENT_SOURCE_DIR}/visitcinemamain.py
        ${CMAKE_CURRENT_SOURCE_DIR}/visitdiff.py)

    install(FILES ${VISIT_BIN_WIN_FILES}
            DESTINATION ${VISIT_INSTALLED_VERSION}
            PERMISSIONS OWNER_READ OWNER_WRITE
                        GROUP_READ GROUP_WRITE
                        WORLD_READ)

    add_custom_target(copy_movie_files ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VISIT_BIN_WIN_FILES}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
    visit_add_to_util_builds(copy_movie_files)
    unset(VISIT_BIN_WIN_FILES)
endif()

