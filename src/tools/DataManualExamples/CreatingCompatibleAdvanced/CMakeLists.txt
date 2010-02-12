#*****************************************************************************
#
# Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-400142
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# Modifications:
#   
#****************************************************************************/

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
)

# This target depends on all the examples so we make this target if we want
# to build all the examples.
IF(NOT VISIT_DATA_MANUAL_EXAMPLES)
    ADD_CUSTOM_TARGET(all_adv_examples)
ENDIF(NOT VISIT_DATA_MANUAL_EXAMPLES)

FUNCTION(ADD_SILO_EXAMPLE target)
    IF(SILO_FOUND)
        ADD_EXECUTABLE(adv_example_${target} ${ARGN})
        TARGET_LINK_LIBRARIES(adv_example_${target} ${SILO_LIB})

        FOREACH(libdir ${SILO_LIBRARY_DIR})
            SET(siloL ${siloL} -L${libdir})
        ENDFOREACH(libdir)
        ADD_TARGET_LINK_FLAGS(adv_example_${target} ${siloL})

        FOREACH(inc ${SILO_INCLUDE_DIR})
            SET(siloinc ${siloinc} -I${inc})
        ENDFOREACH(inc)
        ADD_TARGET_COMPILE_FLAGS(adv_example_${target} ${siloinc})

        SET_TARGET_PROPERTIES(adv_example_${target} PROPERTIES
            OUTPUT_NAME ${target}
            RUNTIME_OUTPUT_DIRECTORY ${VISIT_BINARY_DIR}/tools/DataManualExamples/CreatingCompatibleAdvanced
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
    ADD_SILO_EXAMPLE(fmultimat fmultimat.f)
    ADD_SILO_EXAMPLE(fspatialextents fspatialextents.f)
    ADD_SILO_EXAMPLE(fvectorvar fvectorvar.f)
ENDIF(VISIT_FORTRAN)
