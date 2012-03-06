#*****************************************************************************
#
# Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
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
#  Cyrus Harrison, Thu May 12 15:46:45 PDT 2011
#  Remove COMPONENT RUNTIME statement from INSTALL command to avoid
#  cpack problems.
#
#****************************************************************************/

INCLUDE_DIRECTORIES(
${VISIT_SOURCE_DIR}/sim/V2/lib
${PYTHON_INCLUDE_PATH}
)

PYTHON_ADD_MODULE(pysimV2
    simV2_wrap.c
    simV2_custom.c
)
# We pick "pysimV2" as the target name because "simV2" is already taken by
# libsimV2. We then reset its output name to _simV2 since that's what SWIG
# wants us to call it.
SET_TARGET_PROPERTIES(pysimV2 PROPERTIES PREFIX "" OUTPUT_NAME _simV2)
TARGET_LINK_LIBRARIES(pysimV2 
    simV2
    ${PYTHON_LIBRARIES}
)
IF(NOT APPLE)
    SET_TARGET_PROPERTIES(pysimV2 PROPERTIES INSTALL_RPATH "$ORIGIN")
ENDIF(NOT APPLE)

VISIT_INSTALL_TARGETS(pysimV2)
INSTALL(FILES simV2.py
        DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_WRITE GROUP_EXECUTE 
                    WORLD_READ             WORLD_EXECUTE
       )

