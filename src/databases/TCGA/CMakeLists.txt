# DO NOT EDIT THIS FILE! THIS FILE IS AUTOMATICALLY GENERATED BY xml2cmake
PROJECT(TCGA_database)

ADD_DATABASE_CODE_GEN_TARGETS(TCGA)

SET(COMMON_SOURCES
TCGAPluginInfo.C
TCGACommonPluginInfo.C
)

SET(LIBI_SOURCES
TCGAPluginInfo.C
)

SET(LIBM_SOURCES
TCGAMDServerPluginInfo.C
${COMMON_SOURCES}
avtTCGAFileFormat.C
)

SET(LIBE_SOURCES
TCGAEnginePluginInfo.C
${COMMON_SOURCES}
avtTCGAFileFormat.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_DATABASE_INCLUDES}
)

LINK_DIRECTORIES(${VISIT_LIBRARY_DIR} )

ADD_LIBRARY(ITCGADatabase ${LIBI_SOURCES})
TARGET_LINK_LIBRARIES(ITCGADatabase visitcommon)
SET(INSTALLTARGETS ITCGADatabase)

IF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    ADD_LIBRARY(MTCGADatabase ${LIBM_SOURCES})
    TARGET_LINK_LIBRARIES(MTCGADatabase visitcommon avtdbatts avtdatabase_ser )
    ADD_TARGET_DEFINITIONS(MTCGADatabase MDSERVER)
    SET(INSTALLTARGETS ${INSTALLTARGETS} MTCGADatabase)
ENDIF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)

ADD_LIBRARY(ETCGADatabase_ser ${LIBE_SOURCES})
TARGET_LINK_LIBRARIES(ETCGADatabase_ser visitcommon avtpipeline_ser avtdatabase_ser )
ADD_TARGET_DEFINITIONS(ETCGADatabase_ser ENGINE)
SET(INSTALLTARGETS ${INSTALLTARGETS} ETCGADatabase_ser)

IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(ETCGADatabase_par ${LIBE_SOURCES})
    TARGET_LINK_LIBRARIES(ETCGADatabase_par visitcommon avtpipeline_par avtdatabase_par )
    ADD_TARGET_DEFINITIONS(ETCGADatabase_par ENGINE)
    SET(INSTALLTARGETS ${INSTALLTARGETS} ETCGADatabase_par)
ENDIF(VISIT_PARALLEL)

VISIT_INSTALL_DATABASE_PLUGINS(${INSTALLTARGETS})
VISIT_PLUGIN_TARGET_OUTPUT_DIR(databases ${INSTALLTARGETS})
VISIT_PLUGIN_TARGET_FOLDER(databases TCGA ${INSTALLTARGETS})

