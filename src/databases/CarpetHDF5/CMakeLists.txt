# DO NOT EDIT THIS FILE! THIS FILE IS AUTOMATICALLY GENERATED BY xml2cmake
PROJECT(CarpetHDF5_database)

ADD_DATABASE_CODE_GEN_TARGETS(CarpetHDF5)

SET(COMMON_SOURCES
CarpetHDF5PluginInfo.C
CarpetHDF5CommonPluginInfo.C
)

SET(LIBI_SOURCES
CarpetHDF5PluginInfo.C
)

SET(LIBM_SOURCES
CarpetHDF5MDServerPluginInfo.C
${COMMON_SOURCES}
avtCarpetHDF5FileFormat.C
H5Index.C
)

SET(LIBE_SOURCES
CarpetHDF5EnginePluginInfo.C
${COMMON_SOURCES}
avtCarpetHDF5FileFormat.C
H5Index.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${HDF5_INCLUDE_DIR}
${VISIT_DATABASE_INCLUDES}
)

LINK_DIRECTORIES(${VISIT_LIBRARY_DIR} ${HDF5_LIBRARY_DIR} )

ADD_LIBRARY(ICarpetHDF5Database ${LIBI_SOURCES})
TARGET_LINK_LIBRARIES(ICarpetHDF5Database visitcommon)
SET(INSTALLTARGETS ICarpetHDF5Database)

IF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    ADD_LIBRARY(MCarpetHDF5Database ${LIBM_SOURCES})
    TARGET_LINK_LIBRARIES(MCarpetHDF5Database visitcommon avtdbatts avtdatabase_ser ${HDF5_LIB} )
    ADD_TARGET_DEFINITIONS(MCarpetHDF5Database MDSERVER)
    SET(INSTALLTARGETS ${INSTALLTARGETS} MCarpetHDF5Database)
ENDIF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)

ADD_LIBRARY(ECarpetHDF5Database_ser ${LIBE_SOURCES})
TARGET_LINK_LIBRARIES(ECarpetHDF5Database_ser visitcommon avtpipeline_ser avtdatabase_ser ${HDF5_LIB} )
ADD_TARGET_DEFINITIONS(ECarpetHDF5Database_ser ENGINE)
SET(INSTALLTARGETS ${INSTALLTARGETS} ECarpetHDF5Database_ser)

IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(ECarpetHDF5Database_par ${LIBE_SOURCES})
    TARGET_LINK_LIBRARIES(ECarpetHDF5Database_par visitcommon avtpipeline_par avtdatabase_par ${HDF5_LIB} )
    ADD_TARGET_DEFINITIONS(ECarpetHDF5Database_par ENGINE)
    SET(INSTALLTARGETS ${INSTALLTARGETS} ECarpetHDF5Database_par)
ENDIF(VISIT_PARALLEL)

VISIT_INSTALL_DATABASE_PLUGINS(${INSTALLTARGETS})
VISIT_PLUGIN_TARGET_OUTPUT_DIR(databases ${INSTALLTARGETS})
VISIT_PLUGIN_TARGET_FOLDER(databases CarpetHDF5 ${INSTALLTARGETS})

