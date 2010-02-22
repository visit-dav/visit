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


# install the serial stub module.
CONFIGURE_FILE(src/mpistub.py
               ${LIBRARY_OUTPUT_DIRECTORY}/site-packages/mpistub.py @ONLY)

INSTALL(FILES src/mpistub.py
              DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/site-packages/
              PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
)

# if we have a parallel build - create the mpicom python module.
IF(VISIT_PARALLEL)
    SET(MPICOM_SOURCES
        src/mpicom.C
        src/Buffer.C
        )
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/
                        ${PYTHON_INCLUDE_PATH}
                       )

    LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

    # Create the mpicom
    PYTHON_ADD_MODULE(mpicom
                     ${MPICOM_SOURCES})
    SET_TARGET_PROPERTIES(mpicom PROPERTIES PREFIX "")
    IF(VISIT_PARALLEL_CXXFLAGS)
        SET(PAR_COMPILE_FLAGS "")
        FOREACH (X ${VISIT_PARALLEL_CXXFLAGS})
            SET(PAR_COMPILE_FLAGS "${PAR_COMPILE_FLAGS} ${X}")
        ENDFOREACH (X ${VISIT_PARALLEL_CXXFLAGS})
            SET_TARGET_PROPERTIES(mpicom PROPERTIES
                                  COMPILE_FLAGS ${PAR_COMPILE_FLAGS}
                                  )
    ENDIF(VISIT_PARALLEL_CXXFLAGS)
    IF(VISIT_PARALLEL_LINKER_FLAGS)
        SET(PAR_LINK_FLAGS "")
        FOREACH (X ${VISIT_PARALLEL_LINKER_FLAGS})
            SET(PAR_LINK_FLAGS "${PAR_LINK_FLAGS} ${X}")
        ENDFOREACH (X ${VISIT_PARALLEL_LINKER_FLAGS})
        SET_TARGET_PROPERTIES(mpicom PROPERTIES
                              LINK_FLAGS ${PAR_LINK_FLAGS}
                              )
    ENDIF(VISIT_PARALLEL_LINKER_FLAGS)

    TARGET_LINK_LIBRARIES(mpicom
                          ${PYTHON_LIBRARIES}
                          ${VISIT_PARALLEL_LIBS}
                          )
    VISIT_INSTALL_TARGETS(mpicom)
ENDIF(VISIT_PARALLEL)

