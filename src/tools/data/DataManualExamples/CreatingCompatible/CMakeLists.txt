# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#   
#   Mark C. Miller, Mon May 19 10:51:53 PDT 2014
#   Added explicit 'm' (math library) for SILO_LIB linkage.
#****************************************************************************

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_BINARY_DIR}/include
${SILO_INCLUDE_DIR}
)

LINK_DIRECTORIES(
${SILO_LIBRARY_DIR}
)

# This target depends on all the examples so we make this target if we want
# to build all the examples.
IF(NOT VISIT_DATA_MANUAL_EXAMPLES)
    ADD_CUSTOM_TARGET(all_examples)
ENDIF(NOT VISIT_DATA_MANUAL_EXAMPLES)

FUNCTION(ADD_EXAMPLE target)
    ADD_EXECUTABLE(example_${target} ${ARGN})
    TARGET_LINK_LIBRARIES(example_${target} m)
    SET_TARGET_PROPERTIES(example_${target} PROPERTIES
        OUTPUT_NAME ${target}
        RUNTIME_OUTPUT_DIRECTORY ${VISIT_BINARY_DIR}/tools/DataManualExamples/CreatingCompatible
        FOLDER tools/DataManualExamples/CreatingCompatible
    )
    IF(NOT VISIT_DATA_MANUAL_EXAMPLES)
        SET_TARGET_PROPERTIES(example_${target} PROPERTIES
            EXCLUDE_FROM_ALL TRUE
        )
        ADD_DEPENDENCIES(all_examples example_${target})
    ENDIF(NOT VISIT_DATA_MANUAL_EXAMPLES)
ENDFUNCTION(ADD_EXAMPLE)

# We add stdc++ because Silo contains C++. We can't use a C++ linker because
# we might be linking Fortran so it's easiest to just link with the C++ lib.
FUNCTION(ADD_SILO_EXAMPLE target)
    IF(SILO_FOUND)
        ADD_EXECUTABLE(example_${target} ${ARGN})
        TARGET_LINK_LIBRARIES(example_${target} ${SILO_LIB} stdc++ m)
        SET_TARGET_PROPERTIES(example_${target} PROPERTIES
            OUTPUT_NAME ${target}
            RUNTIME_OUTPUT_DIRECTORY ${VISIT_BINARY_DIR}/tools/DataManualExamples/CreatingCompatible
            FOLDER tools/DataManualExamples/CreatingCompatible
        )
        IF(NOT VISIT_DATA_MANUAL_EXAMPLES)
            SET_TARGET_PROPERTIES(example_${target} PROPERTIES
                EXCLUDE_FROM_ALL TRUE
            )
            ADD_DEPENDENCIES(all_examples example_${target})
        ENDIF(NOT VISIT_DATA_MANUAL_EXAMPLES)
    ENDIF(SILO_FOUND)
ENDFUNCTION(ADD_SILO_EXAMPLE)

##
## BOV examples
##
ADD_EXAMPLE(bov bov.c)
IF(VISIT_FORTRAN)
    ADD_EXAMPLE(fbov fbov.f)
ENDIF(VISIT_FORTRAN)

##
## Silo examples
##
ADD_SILO_EXAMPLE(basic basic.c)
ADD_SILO_EXAMPLE(curv2d curv2d.c)
ADD_SILO_EXAMPLE(curv3d curv3d.c)
ADD_SILO_EXAMPLE(defvars defvars.c)
ADD_SILO_EXAMPLE(multimesh multimesh.c)
ADD_SILO_EXAMPLE(multivar multivar.c)
ADD_SILO_EXAMPLE(optlist optlist.c)
ADD_SILO_EXAMPLE(point2d point2d.c)
ADD_SILO_EXAMPLE(point3d point3d.c)
ADD_SILO_EXAMPLE(pointvar3d pointvar3d.c)
ADD_SILO_EXAMPLE(quadvar2d quadvar2d.c)
ADD_SILO_EXAMPLE(quadvar3d quadvar3d.c)
ADD_SILO_EXAMPLE(rect2d rect2d.c)
ADD_SILO_EXAMPLE(rect3d rect3d.c)
ADD_SILO_EXAMPLE(time time.c)
ADD_SILO_EXAMPLE(ucd2d ucd2d.c)
ADD_SILO_EXAMPLE(ucd3d ucd3d.c)
ADD_SILO_EXAMPLE(ucdvar2d ucdvar2d.c)

##
## Silo examples
##
IF(VISIT_FORTRAN)
    ADD_SILO_EXAMPLE(fbasic fbasic.f)
    ADD_SILO_EXAMPLE(fcurv2d fcurv2d.f)
    ADD_SILO_EXAMPLE(fcurv3d fcurv3d.f)
    ADD_SILO_EXAMPLE(fdefvars fdefvars.f)
    ADD_SILO_EXAMPLE(fmultimesh fmultimesh.f)
    ADD_SILO_EXAMPLE(fmultivar fmultivar.f)
    ADD_SILO_EXAMPLE(foptlist foptlist.f)
    ADD_SILO_EXAMPLE(fpoint2d fpoint2d.f)
    ADD_SILO_EXAMPLE(fpoint3d fpoint3d.f)
    ADD_SILO_EXAMPLE(fpointvar3d fpointvar3d.f)
    ADD_SILO_EXAMPLE(fquadvar2d fquadvar2d.f)
    ADD_SILO_EXAMPLE(fquadvar3d fquadvar3d.f)
    ADD_SILO_EXAMPLE(frect2d frect2d.f)
    ADD_SILO_EXAMPLE(frect3d frect3d.f)
    ADD_SILO_EXAMPLE(fucd2d fucd2d.f)
    ADD_SILO_EXAMPLE(fucd3d fucd3d.f)
    ADD_SILO_EXAMPLE(fucdvar2d fucdvar2d.f)
ENDIF(VISIT_FORTRAN)

##
## visit_writer examples
##
ADD_EXAMPLE(vwcurv3d  vwcurv3d.c  visit_writer.c)
ADD_EXAMPLE(vwpoint3d vwpoint3d.c visit_writer.c)
ADD_EXAMPLE(vwrect2d  vwrect2d.c  visit_writer.c)
ADD_EXAMPLE(vwrect3d  vwrect3d.c  visit_writer.c)
ADD_EXAMPLE(vwregmesh vwregmesh.c visit_writer.c)
ADD_EXAMPLE(vwucd2d   vwucd2d.c   visit_writer.c)
