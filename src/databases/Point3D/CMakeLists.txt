# DO NOT EDIT THIS FILE! THIS FILE IS AUTOMATICALLY GENERATED BY xml2cmake
PROJECT(Point3D_database)

ADD_DATABASE_CODE_GEN_TARGETS(Point3D)

SET(COMMON_SOURCES
Point3DPluginInfo.C
Point3DCommonPluginInfo.C
)

SET(LIBI_SOURCES
Point3DPluginInfo.C
)

SET(LIBM_SOURCES
Point3DMDServerPluginInfo.C
${COMMON_SOURCES}
avtPoint3DFileFormat.C
)

SET(LIBE_SOURCES
Point3DEnginePluginInfo.C
${COMMON_SOURCES}
avtPoint3DFileFormat.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_DATABASE_INCLUDES}
)

LINK_DIRECTORIES(${VISIT_LIBRARY_DIR} )

ADD_LIBRARY(IPoint3DDatabase ${LIBI_SOURCES})
TARGET_LINK_LIBRARIES(IPoint3DDatabase visitcommon)
SET(INSTALLTARGETS IPoint3DDatabase)

IF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    ADD_LIBRARY(MPoint3DDatabase ${LIBM_SOURCES})
    TARGET_LINK_LIBRARIES(MPoint3DDatabase visitcommon avtdbatts avtdatabase_ser )
    ADD_TARGET_DEFINITIONS(MPoint3DDatabase MDSERVER)
    SET(INSTALLTARGETS ${INSTALLTARGETS} MPoint3DDatabase)
ENDIF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)

ADD_LIBRARY(EPoint3DDatabase_ser ${LIBE_SOURCES})
TARGET_LINK_LIBRARIES(EPoint3DDatabase_ser visitcommon avtpipeline_ser avtdatabase_ser )
ADD_TARGET_DEFINITIONS(EPoint3DDatabase_ser ENGINE)
SET(INSTALLTARGETS ${INSTALLTARGETS} EPoint3DDatabase_ser)

IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(EPoint3DDatabase_par ${LIBE_SOURCES})
    TARGET_LINK_LIBRARIES(EPoint3DDatabase_par visitcommon avtpipeline_par avtdatabase_par )
    ADD_TARGET_DEFINITIONS(EPoint3DDatabase_par ENGINE)
    SET(INSTALLTARGETS ${INSTALLTARGETS} EPoint3DDatabase_par)
ENDIF(VISIT_PARALLEL)

VISIT_INSTALL_DATABASE_PLUGINS(${INSTALLTARGETS})
VISIT_PLUGIN_TARGET_OUTPUT_DIR(databases ${INSTALLTARGETS})
VISIT_PLUGIN_TARGET_FOLDER(databases Point3D ${INSTALLTARGETS})

