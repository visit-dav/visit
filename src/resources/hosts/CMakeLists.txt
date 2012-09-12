#*****************************************************************************
#
# Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
#
#
# Modifications:
#   Brad Whitlock, Thu Jun 14 15:28:13 PDT 2012
#   Install customlauncher files. Remove installation of all the other stuff
#   because it will be handled by wholesale installation of the hosts directory
#   into resources/hosts.
#
#   Kathleen Biagas, Wed Sep 12 14:34:04 PDT 2012
#   Added config, guiconfig and visitrc files, use OPTIONAL tag in case
#   files don't exist.
#
#*****************************************************************************

## provide proper install structure windows vs other:
IF(WIN32)
    SET(LPROF_DEST ${VISIT_INSTALLED_VERSION_BIN}/hosts)
    SET(BIN_DEST   ${VISIT_INSTALLED_VERSION_BIN})
    SET(CONFIG_DEST ${VISIT_INSTALLED_VERSION_BIN})
ELSE(WIN32)
    SET(LPROF_DEST ${VISIT_VERSION}/.visit/hosts)
    SET(BIN_DEST   ${VISIT_VERSION}/bin) # The internal bin directory
    SET(CONFIG_DEST ${VISIT_VERSION}/.visit)
ENDIF(WIN32)

# Install local host profiles if specified
IF (VISIT_INSTALL_PROFILES_TO_HOSTS)
    FILE(GLOB LPROF_FILES
         "${CMAKE_CURRENT_SOURCE_DIR}/${VISIT_INSTALL_PROFILES_TO_HOSTS}/*.xml")
    INSTALL(FILES ${LPROF_FILES}
            DESTINATION ${LPROF_DEST}
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
    )
    INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${VISIT_INSTALL_PROFILES_TO_HOSTS}/customlauncher"
            DESTINATION ${BIN_DEST}
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
            OPTIONAL 
    ) 
    INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${VISIT_INSTALL_PROFILES_TO_HOSTS}/config"
            DESTINATION ${CONFIG_DEST}
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
            OPTIONAL 
    ) 
    INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${VISIT_INSTALL_PROFILES_TO_HOSTS}/guiconfig"
            DESTINATION ${CONFIG_DEST}
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
            OPTIONAL 
    ) 
    INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${VISIT_INSTALL_PROFILES_TO_HOSTS}/visitrc"
            DESTINATION ${CONFIG_DEST}
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
            OPTIONAL 
    ) 
ENDIF (VISIT_INSTALL_PROFILES_TO_HOSTS)
