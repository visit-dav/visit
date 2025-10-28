# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#   Hank Childs, Wed Jan 13 11:23:39 CST 2010
#   Rename convert to visitconvert, to avoid name conflicts with other tools.
#
#   Gunther H. Weber, Thu Jan 28 14:33:36 PST 2010
#   Added hack/workaround that ensures that the static MPI libraries get added
#   to the end of the link line. In essence, we add a dummy/empty library
#   that has the MPI libraries as dependencies. This libary is added to the
#   end of the link line of parallel executables. Doing so ensures that cmake
#   will add the depencies of this dummy library, i.e., the MPI libraries, to
#   the end of the link line.
#
#   Hank Childs, Sat Aug 21 14:20:04 PDT 2010
#   Rename ddf to dbin/DDF to DataBinning.
#
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#   Eric Brugger, Fri Feb 24 14:57:15 PST 2023
#   Replaced vtkjpeg with vtkjpeg_LIBRARIES and vtkpng with vtkpng_LIBRARIES.
#
#   Eric Brugger, Tue Jul 25 11:23:55 EDT 2023
#   Change CMAKE_THREAD_LIBS to Threads::Threads.
#
#   Kathleen Biagas, Tue Sep 5, 2023 
#   Remove untested, unused EGL logic.
#
#****************************************************************************

if(VISIT_STATIC)
    set(visitconvert_STATIC_SOURCES ConvertStaticSymbolLocator.C)
endif()

set(visitconvert_DEPENDS
        avtdbatts
        lightweight_visit_vtk
        visitcommon
        visit_vtk
        ${ALL_THIRDPARTY_IO_LIB}
        ${vtkjpeg_LIBRARIES}
        ${vtkpng_LIBRARIES}
        Threads::Threads)

set(visitconvert_DEPENDS_SER
        ${engine_ser_exe_IDatabase}
        ${engine_ser_exe_EDatabase_ser}
        avtdatabase_ser
        avtpipeline_ser)

set(visitconvert_DEPENDS_PAR
        ${engine_par_exe_IDatabase}
        ${engine_par_exe_EDatabase_par}
        avtdatabase_par
        avtpipeline_par)

IF(NOT VISIT_DBIO_ONLY)
    list(APPEND visitconvert_DEPENDS
        avtmath
        avtview
        visit_verdict)

    list(APPEND visitconvert_DEPENDS_SER
        avtdbin_ser
        avtexpressions_ser
        avtfilters_ser
        avtmir_ser
        avtplotter_ser)

    list(APPEND visitconvert_DEPENDS_PAR
        avtdbin_par
        avtexpressions_par
        avtfilters_par
        avtmir_par
        avtplotter_par)
endif()

#********************************* SERIAL ************************************
visit_add_executable(
    NAME        visitconvert_ser
    SOURCES     convert.C
                ${visitconvert_STATIC_SOURCES}
    DEPENDS_ON  ${visitconvert_DEPENDS}
                ${visitconvert_DEPENDS_SER}
    FOLDER      tools/data/convert)

if(VISIT_DBIO_ONLY)
    set_target_properties(visitconvert_ser PROPERTIES
        OUTPUT_NAME visitconvert_ser_lite)
endif()

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    visit_add_parallel_executable(
        NAME        visitconvert_par
        SOURCES     convert.C
                    ${visitconvert_STATIC_SOURCES}
        DEPENDS_ON  ${visitconvert_DEPENDS}
                    ${visitconvert_DEPENDS_PAR}
        FOLDER      tools/data/convert)

    if(VISIT_DBIO_ONLY)
        set_target_properties(visitconvert_par PROPERTIES
            OUTPUT_NAME visitconvert_par_lite)
    endif()
endif()

