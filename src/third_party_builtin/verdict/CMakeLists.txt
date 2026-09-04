set(VERDICT_SOURCES
    V_EdgeMetric.C
    V_GaussIntegration.C
    V_HexMetric.C
    V_KnifeMetric.C
    V_PyramidMetric.C
    V_QuadMetric.C
    V_TetMetric.C
    V_TriMetric.C
    V_WedgeMetric.C
    VerdictVector.C)

set(VERDICT_HEADERS
    V_GaussIntegration.h
    v_vector.h
    verdict.h
    verdict_defines.h
    VerdictVector.h)

visit_add_library(
    NAME     visit_verdict
    SOURCES  ${VERDICT_SOURCES}
    HEADERS  ${VERDICT_HEADERS}
    INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
             $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/third_party_builtin/verdict>
    DEFINES  VERDICT_EXPORTS
    FOLDER   thirdparty_builtin/visit_verdict)

