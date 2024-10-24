# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#    Gunther H. Weber, Thu Jan 28 14:33:36 PST 2010
#    Added hack/workaround that ensures that the static MPI libraries get added
#    to the end of the link line. In essence, we add a dummy/empty library
#    that has the MPI libraries as dependencies. This libary is added to the
#    end of the link line of parallel executables. Doing so ensures that cmake
#    will add the depencies of this dummy library, i.e., the MPI libraries, to
#    the end of the link line.
#
#    Hank Childs, Sat Aug 21 11:56:26 PDT 2010
#    Rename DDF as DataBinning.
#
#    Brad Whitlock, Mon Dec 13 16:06:42 PST 2010
#    I added CumulativeQueryNamedSelectionExtension.
#
#    Brad Whitlock, Fri May 18 16:09:34 PST 2012
#    Use different resource file.
#
#    Kathleen Biagas, Thu Jun 21 15:19:02 MST 2012
#    Call ADD_TARGET_DEFINTIONS to add "HAVE_ICET", as the function deals
#    correctly with a target already having some COMPILE_DEFINITIONS set.
#
#    Brad Whitlock, Thu Apr 11 15:59:49 PDT 2013
#    Add check for X11. Change how we get zlib.
#
#    Eric Brugger, Tue Sep 30 15:03:30 PDT 2014
#    Add the EAVL library to the list of link libraries.
#
#    Burlen Loring, Thu Sep  3 22:35:11 PDT 2015
#    I added the ProgrammableCompositer, a helper class
#    used by the NetworkManager.
#
#    Eric Brugger, Tue Jul 20 11:55:04 PDT 2021
#    Added OPENGL_INCLUDE_DIR to the include directories if building IceT
#    with MesaGL.
#
#    Justin Privitera, Wed Aug 24 11:08:51 PDT 2022
#    Added logic to be aware of avtBP lib.
#
#    Eric Brugger, Fri Feb 24 14:57:15 PST 2023
#    Replace vtkjpeg with vtkjpeg_LIBRARIES and vtkpng with vtkpng_LIBRARIES.
#
#    Eric Brugger, Tue Jul 25 11:23:55 EDT 2023
#    Change CMAKE_THREAD_LIBS to Threads::Threads.
#
#    Kathleen Biagas, Tue Sep 5, 2023
#    Remove untested, unused EGL logic.
#
#    Kathleen Biagas, Thu Oct 3, 2024
#    Utilize visit_add_library.
#
#****************************************************************************

set(LIBENGINE_SOURCES
    ClonedDataNetwork.C
    CumulativeQueryNamedSelectionExtension.C
    DataNetwork.C
    Engine.C
    EngineBase.C
    LoadBalancer.C
    MesaDisplay.C
    Netnodes.C
    NetworkManager.C
    ProgrammableCompositer.C
    VisItDisplay.C)

set(LIBENGINE_HEADERS
    ClonedDataNetwork.h
    CumulativeQueryNamedSelectionExtension.h
    DataNetwork.h
    Engine.h
    EngineBase.h
    EngineRPCExecutor.h
    Executors.h
    LoadBalancer.h
    MesaDisplay.h
    Netnodes.h
    NetworkManager.h
    ProgrammableCompositer.h
    VisItDisplay.h
    engine_main_exports.h)

# If the engine is static then we need some static symbol lookup functions
if(VISIT_STATIC)
    list(APPEND LIBENGINE_SOURCES EngineStaticSymbolLocator.C)
endif()

if(NOT WIN32 AND NOT APPLE AND VISIT_USE_X)
    list(APPEND LIBENGINE_SOURCES XDisplay.C)
    list(APPEND LIBENGINE_HEADERS XDisplay.h)
endif()


set(LIBENGINE_INCLUDES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/engine/main>)

set(LIBENGINE_DEPENDS
    visitcommon
    enginerpc
    avtview)

if(CONDUIT_FOUND AND MFEM_FOUND)
    list(APPEND LIBENGINE_DEPENDS mfem avtblueprint)
endif()

if(HAVE_OSMESA)
    list(APPEND LIBENGINE_DEPENDS visit_vtk_offscreen)
endif()

#************************** SERIAL LIBRARY ************************************
visit_add_library(
    NAME        engine_ser
    SOURCES     ${LIBENGINE_SOURCES}
    HEADERS     ${LIBENGINE_HEADERS}
    INCLUDES    ${LIBENGINE_INCLUDES}
    DEPENDS_ON  ${LIBENGINE_DEPENDS}
                avtwriter_ser
                avtplotter_ser
                avtquery_ser
                avtviswindow_ser
                avtpipeline_ser)

if(WIN32)
    visit_patch_target(NAME engine_ser OUTPUT_NAME enginelib_ser)
else()
    # This keeps comm's exceptions visible when using -fvisibility=hidden
    visit_patch_target(NAME engine_ser DEFINES COMM_EXPORTS)
endif()


#************************** PARALLEL LIBRARY **********************************

if(VISIT_PARALLEL)
    # If we have Ice-T then add more sources to the build
    if(ICET_FOUND)
        set(ICET_SOURCES IceTNetworkManager.C)
        set(ICET_HEADERS IceTNetworkManager.h)
        if(APPLE)
            # We need to link with OpenGL on Mac when we have IceT.
            set(ICET_OPENGL ${OPENGL_LIBRARIES})
        endif()

        if(VISIT_STATIC)
            set(ICET_STATIC_LIB ${ICET_LIB} ${ICET_OPENGL} ${VISIT_PARALLEL_LIBS})
        else()
            set(ICET_DYNAMIC_LIB ${ICET_LIB} ${ICET_OPENGL})
        endif()
    endif()

    visit_add_library(
        DO_PARALLEL
        NAME        engine_par
        SOURCES     ${LIBENGINE_SOURCES}
                    ${ICET_SOURCES}
                    MPIXfer.C
        HEADERS     ${LIBENGINE_HEADERS}
                    ${ICET_HEADERS}
                    MPIXfer.h
        INCLUDES    ${LIBENGINE_INCLUDES}
        DEPENDS_ON  ${LIBENGINE_DEPENDS}
                    avtwriter_par
                    avtplotter_par
                    avtquery_par
                    avtviswindow_par
                    avtpipeline_par
                    ${ICET_DYNAMIC_LIB}
                    cognomen)

    if(WIN32)
        visit_patch_target(NAME engine_par OUTPUT_NAME enginelib_par)
    endif()

    if(ICET_FOUND)
        visit_patch_target(
            NAME        engine_par
            INCLUDES    $<BUILD_INTERFACE:${ICET_INCLUDE_DIR}>
                        $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/icet>
            DEFINES     HAVE_ICET
            LINKDIR     ${ICET_LIBRARY_DIR})
        if(VISIT_MESAGL_DIR)
            visit_patch_target(
                NAME engine_par
                INCLUDES $<BUILD_INTERFACE:${OPENGL_INCLUDE_DIR}>)
        endif()
    endif()
endif()


#*********************** SERIAL EXECUTABLE ************************************

# These 3 variables are only defined when building statically.
# engine_ser_exe_EDatabase_ser = The list of database libE libraries needed to link
# engine_ser_exe_EOperator_ser = The list of operator libE libraries needed to link
# engine_ser_exe_EPlot_ser = The list of plot libE libraries needed to link

add_executable(engine_ser_exe ${VISIT_APPLICATION_STYLE} main.C ${ENGINE_STATIC_SOURCES} ${VISIT_ENGINE_SER_RESOURCE_FILE})
set_target_properties(engine_ser_exe PROPERTIES OUTPUT_NAME engine_ser)

if(WIN32 AND MSVC)
    set_target_properties(engine_ser_exe PROPERTIES LINK_FLAGS "/STACK:4000000")
endif()

target_link_libraries(engine_ser_exe
    ${VISIT_EXE_LINKER_FLAGS}
    ${engine_ser_exe_IDatabase}
    ${engine_ser_exe_EDatabase_ser}
    ${engine_ser_exe_IOperator}
    ${engine_ser_exe_EOperator_ser}
    ${engine_ser_exe_IPlot}
    ${engine_ser_exe_EPlot_ser}
    engine_ser
    visit_verdict
    ${vtkjpeg_LIBRARIES}
    ${vtkpng_LIBRARIES}
    ${ALL_THIRDPARTY_IO_LIB}
    Threads::Threads
    ${ZLIB_LIBRARY}
)

# If we're building statically then the engine can't be linked until the plugin
# sources are built
if(VISIT_STATIC)
    add_dependencies(engine_ser_exe
        ${engine_ser_exe_IDatabase}
        ${engine_ser_exe_EDatabase_ser}
        ${engine_ser_exe_IOperator}
        ${engine_ser_exe_EOperator_ser}
        ${engine_ser_exe_IPlot}
        ${engine_ser_exe_EPlot_ser}
    )
endif()

VISIT_INSTALL_TARGETS(engine_ser_exe)

#************************ PARALLEL EXECUTABLE *********************************

if(VISIT_PARALLEL)
    if(VISIT_NOLINK_MPI_WITH_LIBRARIES)
        add_library(link_mpi_libs STATIC empty.c)
        target_link_libraries(link_mpi_libs ${VISIT_PARALLEL_LIBS})
    endif()

    # For static builds with icet, don't add the parallel libs automatically. We
    # try to control it explicitly so icet comes before mpi on the link line.
    if(ICET_FOUND AND VISIT_STATIC)
        set(tmpval ${VISIT_NOLINK_MPI_WITH_LIBRARIES})
        set(VISIT_NOLINK_MPI_WITH_LIBRARIES OFF)
    endif()

    ADD_PARALLEL_EXECUTABLE(engine_par_exe ${VISIT_APPLICATION_STYLE} main.C ${ENGINE_STATIC_SOURCES} ${VISIT_ENGINE_PAR_RESOURCE_FILE})

    if(ICET_FOUND AND VISIT_STATIC)
        set(VISIT_NOLINK_MPI_WITH_LIBRARIES ${tmpval})
    endif(ICET_FOUND AND VISIT_STATIC)

    set_target_properties(engine_par_exe PROPERTIES OUTPUT_NAME engine_par)
    if(WIN32 AND MSVC)
        set_target_properties(engine_par_exe PROPERTIES
            LINK_FLAGS "/STACK:4000000")
    endif()

    PARALLEL_EXECUTABLE_LINK_LIBRARIES(engine_par_exe
        ${engine_par_exe_IDatabase_par}
        ${engine_par_exe_EDatabase_par}
        ${engine_par_exe_IOperator_par}
        ${engine_par_exe_EOperator_par}
        ${engine_par_exe_IPlot_par}
        ${engine_par_exe_EPlot_par}
        engine_par
        visit_verdict
        ${vtkjpeg_LIBRARIES}
        ${vtkpng_LIBRARIES}
        ${ALL_THIRDPARTY_IO_LIB}
        ${ICET_STATIC_LIB}
        Threads::Threads
        ${ZLIB_LIBRARY}
    )

    # If we're building statically then the engine can't be linked until the plugin
    # sources are built
    if(VISIT_STATIC)
        add_dependencies(engine_par_exe
            ${engine_par_exe_IDatabase_par}
            ${engine_par_exe_EDatabase_par}
            ${engine_par_exe_IOperator_par}
            ${engine_par_exe_EOperator_par}
            ${engine_par_exe_IPlot_par}
            ${engine_par_exe_EPlot_par}
        )
    endif()

    VISIT_INSTALL_TARGETS(engine_par_exe)
endif()
