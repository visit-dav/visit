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
# Notes:
#   This directory may later contain additional subdirectories that provide
#   useful content. For example, we could have a "logos" directory that contains
#   VisIt project logos or commonly used Lab logos. We could have some VTK
#   datasets such as globes, etc. 
#
# Modifications:
#   Brad Whitlock, Thu Sep  6 11:50:09 PDT 2012
#   Add hosts to the directories that get installed to resources.
#
#   Kathleen Biagas, Wed Sep 12 11:57:06 PDT 2012
#   Add CMakeLists.txt and cmake-generated files to exclusion lists.
#
#*****************************************************************************

# Do a little extra in the hosts directory.
ADD_SUBDIRECTORY(hosts)

# Directories that we want installed wholesale to the "resources" directory.
SET(RESOURCE_DIRECTORIES colortables hosts movietemplates translations usage)

# Copy the resources directory into the out of source build
FOREACH(ITEM ${RESOURCE_DIRECTORIES})
    GET_FILENAME_COMPONENT(ITM_RP ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}
                           REALPATH)
    FILE(COPY ${ITM_RP}
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
         PATTERN ".svn" EXCLUDE
         PATTERN "CMakeLists.txt" EXCLUDE)
ENDFOREACH(ITEM)

# Install these directories in their entirety to the installed "resources" 
# directory.
INSTALL(DIRECTORY ${RESOURCE_DIRECTORIES}
    DESTINATION ${VISIT_INSTALLED_VERSION_RESOURCES}
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE 
                     GROUP_READ GROUP_WRITE 
                     WORLD_READ
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                          GROUP_READ GROUP_WRITE GROUP_EXECUTE 
                          WORLD_READ             WORLD_EXECUTE
    PATTERN ".svn" EXCLUDE
    PATTERN "CMakeLists.txt" EXCLUDE
    PATTERN "CMakeFiles" EXCLUDE
    PATTERN "Makefile" EXCLUDE
    PATTERN "cmake_install.cmake" EXCLUDE
)
