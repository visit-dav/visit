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
#****************************************************************************

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/DataBinning
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/Database/Ghost
${VISIT_SOURCE_DIR}/avt/Expressions/Abstract
${VISIT_SOURCE_DIR}/avt/Expressions/CMFE
${VISIT_SOURCE_DIR}/avt/Expressions/Conditional
${VISIT_SOURCE_DIR}/avt/Expressions/Derivations
${VISIT_SOURCE_DIR}/avt/Expressions/General
${VISIT_SOURCE_DIR}/avt/Expressions/ImageProcessing
${VISIT_SOURCE_DIR}/avt/Expressions/Management
${VISIT_SOURCE_DIR}/avt/Expressions/Math
${VISIT_SOURCE_DIR}/avt/Expressions/MeshQuality
${VISIT_SOURCE_DIR}/avt/FileWriter
${VISIT_SOURCE_DIR}/avt/Filters
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/Plotter
${VISIT_SOURCE_DIR}/avt/Queries/Abstract
${VISIT_SOURCE_DIR}/avt/Queries/Misc
${VISIT_SOURCE_DIR}/avt/Queries/Pick
${VISIT_SOURCE_DIR}/avt/Queries/Queries
${VISIT_SOURCE_DIR}/avt/View
${VISIT_SOURCE_DIR}/avt/VisWindow/Colleagues
${VISIT_SOURCE_DIR}/avt/VisWindow/Proxies
${VISIT_SOURCE_DIR}/avt/VisWindow/Tools
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VTK_INCLUDE_DIRS}
)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY}
${ALL_THIRDPARTY_IO_LIBRARY_DIR}
${ICET_LIBRARY_DIR}
)

SET(VTK_EXTRA_LIBS
${vtkjpeg_LIBRARIES}
${vtkpng_LIBRARIES}
${VTK_FREETYPE_LIBRARIES}
)

IF(VISIT_STATIC)
    SET(CONVERT_STATIC_SOURCES ConvertStaticSymbolLocator.C)
ENDIF(VISIT_STATIC)

#********************************* SERIAL ************************************
ADD_EXECUTABLE(visitconvert_ser convert.C ${CONVERT_STATIC_SOURCES})
VISIT_INSTALL_TARGETS(visitconvert_ser)
VISIT_DATA_ADD_FOLDER(convert visitconvert_ser)

IF(VISIT_DBIO_ONLY)
    SET_TARGET_PROPERTIES(visitconvert_ser PROPERTIES OUTPUT_NAME visitconvert_ser_lite)
    TARGET_LINK_LIBRARIES(visitconvert_ser
        ${VISIT_EXE_LINKER_FLAGS}
        ${engine_ser_exe_IDatabase}
        ${engine_ser_exe_EDatabase_ser}
        avtdatabase_ser
        avtdbatts
        avtpipeline_ser
        lightweight_visit_vtk
        visitcommon
        visit_vtk
        ${ALL_THIRDPARTY_IO_LIB}
        ${VTK_EXTRA_LIBS}
        ${CMAKE_THREAD_LIBS}
        ${ZLIB_LIBRARY}
    )
ELSE()
    TARGET_LINK_LIBRARIES(visitconvert_ser
        ${VISIT_EXE_LINKER_FLAGS}
        ${engine_ser_exe_IDatabase}
        ${engine_ser_exe_EDatabase_ser}
        avtdatabase_ser
        avtdbatts
        avtdbin_ser
        avtexpressions_ser
        avtfilters_ser
        avtmath
        avtmir_ser
        avtpipeline_ser
        avtplotter_ser
        avtview
        lightweight_visit_vtk
        visitcommon
        visit_verdict
        visit_vtk
        ${ALL_THIRDPARTY_IO_LIB}
        ${VTK_EXTRA_LIBS}
        ${OPENGL_LIBRARIES}
        ${CMAKE_THREAD_LIBS}
        ${ZLIB_LIBRARY}
    )
    if(HAVE_OSMESA)
        target_link_libraries(visitconvert_ser ${OSMESA_LIBRARIES})
    elseif(HAVE_EGL)
        target_link_libraries(visitconvert_ser ${EGL_LIBRARIES})
    endif()
ENDIF()

# If we're building statically then convert can't be linked until the plugin
# sources are built
IF(VISIT_STATIC)
    ADD_DEPENDENCIES(visitconvert_ser
        ${engine_ser_exe_IDatabase}
        ${engine_ser_exe_EDatabase_ser}
    )
ENDIF(VISIT_STATIC)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_EXECUTABLE(visitconvert_par convert.C ${CONVERT_STATIC_SOURCES})
    VISIT_INSTALL_TARGETS(visitconvert_par)
    VISIT_DATA_ADD_FOLDER(convert visitconvert_par)

    IF(VISIT_DBIO_ONLY)
        SET_TARGET_PROPERTIES(visitconvert_par PROPERTIES OUTPUT_NAME visitconvert_par_lite)
        PARALLEL_EXECUTABLE_LINK_LIBRARIES(visitconvert_par
            ${engine_par_exe_IDatabase_par}
            ${engine_par_exe_EDatabase_par}
            avtdatabase_par
            avtdbatts
            avtpipeline_par
            lightweight_visit_vtk
            visitcommon
            visit_verdict
            visit_vtk
            ${ALL_THIRDPARTY_IO_LIB}
            ${VTK_EXTRA_LIBS}
            ${CMAKE_THREAD_LIBS}
            ${ZLIB_LIBRARY}
        )
    ELSE()
        PARALLEL_EXECUTABLE_LINK_LIBRARIES(visitconvert_par
            ${engine_par_exe_IDatabase_par}
            ${engine_par_exe_EDatabase_par}
            avtdatabase_par
            avtdbatts
            avtdbin_par
            avtexpressions_par
            avtfilters_par
            avtmath
            avtmir_par
            avtpipeline_par
            avtplotter_par
            avtview
            lightweight_visit_vtk
            visitcommon
            visit_verdict
            visit_vtk
            ${ALL_THIRDPARTY_IO_LIB}
            ${VTK_EXTRA_LIBS}
            ${OPENGL_LIBRARIES}
            ${CMAKE_THREAD_LIBS}
            ${ZLIB_LIBRARY}
        )
        if(HAVE_OSMESA)
            target_link_libraries(visitconvert_par ${OSMESA_LIBRARIES})
        elseif(HAVE_EGL)
            target_link_libraries(visitconvert_par ${EGL_LIBRARIES})
        endif()
    ENDIF()

    # If we're building statically then visitconvert_par can't be linked until the 
    # plugin sources are built
    IF(VISIT_STATIC)
        ADD_DEPENDENCIES(visitconvert_par
            ${engine_par_exe_IDatabase_par}
            ${engine_par_exe_EDatabase_par}
        )
    ENDIF(VISIT_STATIC)
ENDIF(VISIT_PARALLEL)
