# DO NOT EDIT THIS FILE! THIS FILE IS AUTOMATICALLY GENERATED BY xml2cmake
PROJECT(MRTest_database)

ADD_DATABASE_CODE_GEN_TARGETS(MRTest)

SET(COMMON_SOURCES
MRTestPluginInfo.C
MRTestCommonPluginInfo.C
)

SET(LIBI_SOURCES
MRTestPluginInfo.C
)

SET(LIBM_SOURCES
MRTestMDServerPluginInfo.C
${COMMON_SOURCES}
avtMRTestFileFormat.C
)

SET(LIBE_SOURCES
MRTestEnginePluginInfo.C
${COMMON_SOURCES}
avtMRTestFileFormat.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_DATABASE_INCLUDES}
)

LINK_DIRECTORIES(${VISIT_LIBRARY_DIR} )

ADD_LIBRARY(IMRTestDatabase ${LIBI_SOURCES})
TARGET_LINK_LIBRARIES(IMRTestDatabase visitcommon)
SET(INSTALLTARGETS IMRTestDatabase)

IF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    ADD_LIBRARY(MMRTestDatabase ${LIBM_SOURCES})
    TARGET_LINK_LIBRARIES(MMRTestDatabase visitcommon avtdbatts avtdatabase_ser )
    ADD_TARGET_DEFINITIONS(MMRTestDatabase MDSERVER)
    SET(INSTALLTARGETS ${INSTALLTARGETS} MMRTestDatabase)
ENDIF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)

ADD_LIBRARY(EMRTestDatabase_ser ${LIBE_SOURCES})
TARGET_LINK_LIBRARIES(EMRTestDatabase_ser visitcommon avtpipeline_ser avtdatabase_ser )
ADD_TARGET_DEFINITIONS(EMRTestDatabase_ser ENGINE)
SET(INSTALLTARGETS ${INSTALLTARGETS} EMRTestDatabase_ser)

IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(EMRTestDatabase_par ${LIBE_SOURCES})
    TARGET_LINK_LIBRARIES(EMRTestDatabase_par visitcommon avtpipeline_par avtdatabase_par )
    ADD_TARGET_DEFINITIONS(EMRTestDatabase_par ENGINE)
    SET(INSTALLTARGETS ${INSTALLTARGETS} EMRTestDatabase_par)
ENDIF(VISIT_PARALLEL)

VISIT_INSTALL_DATABASE_PLUGINS(${INSTALLTARGETS})
VISIT_PLUGIN_TARGET_OUTPUT_DIR(databases ${INSTALLTARGETS})
VISIT_PLUGIN_TARGET_FOLDER(databases MRTest ${INSTALLTARGETS})

