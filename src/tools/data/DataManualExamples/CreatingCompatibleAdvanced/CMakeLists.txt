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
#
#****************************************************************************

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${SILO_INCLUDE_DIR}
)

LINK_DIRECTORIES(
${SILO_LIBRARY_DIR}
)

# This target depends on all the examples so we make this target if we want
# to build all the examples.
IF(NOT VISIT_DATA_MANUAL_EXAMPLES)
    ADD_CUSTOM_TARGET(all_adv_examples)
ENDIF(NOT VISIT_DATA_MANUAL_EXAMPLES)

FUNCTION(ADD_SILO_EXAMPLE target)
    IF(SILO_FOUND)
        ADD_EXECUTABLE(adv_example_${target} ${ARGN})
        TARGET_LINK_LIBRARIES(adv_example_${target} ${SILO_LIB} stdc++ m)
        SET_TARGET_PROPERTIES(adv_example_${target} PROPERTIES
            OUTPUT_NAME ${target}
            RUNTIME_OUTPUT_DIRECTORY ${VISIT_BINARY_DIR}/tools/DataManualExamples/CreatingCompatibleAdvanced
            FOLDER tools/DataManualExamples/CreatingCompatibleAdvanced
        )
        IF(NOT VISIT_DATA_MANUAL_EXAMPLES)
            SET_TARGET_PROPERTIES(adv_example_${target} PROPERTIES
                EXCLUDE_FROM_ALL TRUE
            )
            ADD_DEPENDENCIES(all_adv_examples adv_example_${target})
        ENDIF(NOT VISIT_DATA_MANUAL_EXAMPLES)
    ENDIF(SILO_FOUND)
ENDFUNCTION(ADD_SILO_EXAMPLE)

##
## Silo examples
##
ADD_SILO_EXAMPLE(dataextents dataextents.c)
ADD_SILO_EXAMPLE(ghostzonesinfile ghostzonesinfile.c)
ADD_SILO_EXAMPLE(mixedmaterials mixedmaterials.c)
ADD_SILO_EXAMPLE(spatialextents spatialextents.c)
ADD_SILO_EXAMPLE(vectorvar vectorvar.c)

##
## Silo fortran examples
##
IF(VISIT_FORTRAN)
    ADD_SILO_EXAMPLE(fdataextents fdataextents.f)
    ADD_SILO_EXAMPLE(fghostzonesinfile fghostzonesinfile.f)
    ADD_SILO_EXAMPLE(fmixedmaterials fmixedmaterials.f)
    #ADD_SILO_EXAMPLE(fmultimat fmultimat.f)
    ADD_SILO_EXAMPLE(fspatialextents fspatialextents.f)
    ADD_SILO_EXAMPLE(fvectorvar fvectorvar.f)
ENDIF(VISIT_FORTRAN)
