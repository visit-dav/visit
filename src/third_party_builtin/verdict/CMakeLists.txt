SET(VERDICT_SOURCES
    V_EdgeMetric.C
    V_GaussIntegration.C
    V_HexMetric.C
    V_KnifeMetric.C
    V_PyramidMetric.C
    V_QuadMetric.C
    V_TetMetric.C
    V_TriMetric.C
    V_WedgeMetric.C
    VerdictVector.C
)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

ADD_DEFINITIONS(-DVERDICT_EXPORTS)

ADD_LIBRARY(visit_verdict ${VERDICT_SOURCES})

VISIT_INSTALL_TARGETS(visit_verdict)
