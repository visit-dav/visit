# DO NOT EDIT THIS FILE! THIS FILE IS AUTOMATICALLY GENERATED BY xml2cmake
PROJECT(Histogram_plot)

ADD_PLOT_CODE_GEN_TARGETS(Histogram)

SET(COMMON_SOURCES
HistogramPluginInfo.C
HistogramCommonPluginInfo.C
HistogramAttributes.C
)

SET(LIBI_SOURCES
HistogramPluginInfo.C
)

SET(LIBG_SOURCES
HistogramGUIPluginInfo.C
QvisHistogramPlotWindow.C
${COMMON_SOURCES}
)

SET(LIBV_SOURCES
HistogramViewerEnginePluginInfo.C
HistogramViewerPluginInfo.C
avtHistogramPlot.C
avtHistogramFilter.C
avtHistogramMapper.C
${COMMON_SOURCES}
)

SET(LIBE_SOURCES
HistogramViewerEnginePluginInfo.C
HistogramEnginePluginInfo.C
avtHistogramPlot.C
avtHistogramFilter.C
avtHistogramMapper.C
${COMMON_SOURCES}
)


INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_PLOT_INCLUDES}
${VISIT_INCLUDE_DIR}/avt/Expressions/MeshQuality
${VISIT_INCLUDE_DIR}/avt/Expressions/Abstract
)

LINK_DIRECTORIES(${VISIT_LIBRARY_DIR} )

ADD_LIBRARY(IHistogramPlot ${LIBI_SOURCES})
TARGET_LINK_LIBRARIES(IHistogramPlot visitcommon)
SET(INSTALLTARGETS IHistogramPlot)

IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    ADD_LIBRARY(GHistogramPlot ${LIBG_SOURCES})
    set_target_properties(GHistogramPlot PROPERTIES AUTOMOC ON)
    TARGET_LINK_LIBRARIES(GHistogramPlot visitcommon gui )

    ADD_LIBRARY(VHistogramPlot ${LIBV_SOURCES})
    ADD_TARGET_DEFINITIONS(VHistogramPlot VIEWER)
    TARGET_LINK_LIBRARIES(VHistogramPlot visitcommon viewer )

    SET(INSTALLTARGETS ${INSTALLTARGETS} GHistogramPlot VHistogramPlot)

    IF(VISIT_PYTHON_SCRIPTING)
        SET(LIBS_SOURCES
            HistogramScriptingPluginInfo.C
            PyHistogramAttributes.C
            ${COMMON_SOURCES}
        )
        ADD_LIBRARY(SHistogramPlot ${LIBS_SOURCES})
        IF(WIN32)
            # This prevents python from #defining snprintf as _snprintf
            SET_TARGET_PROPERTIES(SHistogramPlot PROPERTIES COMPILE_DEFINITIONS HAVE_SNPRINTF)
        ENDIF()
        TARGET_LINK_LIBRARIES(SHistogramPlot visitcommon visitpy ${PYTHON_LIBRARY})
        SET(INSTALLTARGETS ${INSTALLTARGETS} SHistogramPlot)
    ENDIF(VISIT_PYTHON_SCRIPTING)

    IF(VISIT_JAVA)
        FILE(COPY HistogramAttributes.java DESTINATION ${JavaClient_BINARY_DIR}/src/plots)
        ADD_CUSTOM_TARGET(JavaHistogram ALL ${Java_JAVAC_EXECUTABLE} ${VISIT_Java_FLAGS} -d ${JavaClient_BINARY_DIR} -classpath ${JavaClient_BINARY_DIR} -sourcepath ${JavaClient_BINARY_DIR} HistogramAttributes.java
            DEPENDS JavaClient
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    ENDIF(VISIT_JAVA)
ENDIF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)

ADD_LIBRARY(EHistogramPlot_ser ${LIBE_SOURCES})
TARGET_LINK_LIBRARIES(EHistogramPlot_ser visitcommon avtpipeline_ser avtplotter_ser avtexpressions_ser )
ADD_TARGET_DEFINITIONS(EHistogramPlot_ser ENGINE)
SET(INSTALLTARGETS ${INSTALLTARGETS} EHistogramPlot_ser)

IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(EHistogramPlot_par ${LIBE_SOURCES})
    TARGET_LINK_LIBRARIES(EHistogramPlot_par visitcommon avtpipeline_par avtplotter_par avtexpressions_par )
    ADD_TARGET_DEFINITIONS(EHistogramPlot_par ENGINE)
    SET(INSTALLTARGETS ${INSTALLTARGETS} EHistogramPlot_par)
ENDIF(VISIT_PARALLEL)

VISIT_INSTALL_PLOT_PLUGINS(${INSTALLTARGETS})
VISIT_PLUGIN_TARGET_OUTPUT_DIR(plots ${INSTALLTARGETS})
VISIT_PLUGIN_TARGET_FOLDER(plots Histogram ${INSTALLTARGETS})

