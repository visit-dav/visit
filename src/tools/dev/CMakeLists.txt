#*****************************************************************************
#
# Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
#****************************************************************************/
IF(VISIT_BUILD_DIAGNOSTICS)
    ADD_SUBDIRECTORY(diagnostics) # dev
ENDIF()

IF(VISIT_DBIO_ONLY OR VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)
ELSE(VISIT_DBIO_ONLY OR VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)
    IF(NOT WIN32)
        ADD_SUBDIRECTORY(clipeditor) # dev
    ENDIF(NOT WIN32)
    ADD_SUBDIRECTORY(protocol) # dev
    ADD_SUBDIRECTORY(xml) # dev
    ADD_SUBDIRECTORY(xmledit) # dev
ENDIF(VISIT_DBIO_ONLY OR VISIT_ENGINE_ONLY OR VISIT_SERVER_COMPONENTS_ONLY)

#CMake add subdirectory forces this action to go last
#todo: find way for this function to execute as last
#step of make install process
#ADD_CUSTOM_TARGET(osxfixup ALL
#     COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/osxfixup/osxfixup.py \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION} @executable_path/.. 3)
#ADD_DEPENDENCIES(osxfixup install)

# todo: replace below with lines above
IF(APPLE)
    ADD_SUBDIRECTORY(osxfixup)
ENDIF(APPLE)

# These do not need to be built:
# avt_do_add
# doxygenate
# escan
# vtk_debug_parser
