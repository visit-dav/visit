#*****************************************************************************
#
# Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
#   Kathleen Biagas, Fri Nov 18 12:00:18 MST 2011
#   Added plugin name to VISIT_PLUGIN_TARGET_FOLDER args, so that plugins
#   are grouped by name in VisualStudio for ease of building/debugging
#   individual plugins.
#
#   Kathleen Biagas, Tue Nov 22 14:37:17 MST 2011
#   Remove VISIT_PLUGIN_TARGET_PREFIX in favor of VISIT_PLUGIN_TARGET_RTOD
#   which sets runtime output directory, which works with all versions of
#   Visual Studio, while the target prefix version didn't work with 2010.
#
#   Kathleen Biagas, Wed Jan 8 16:05:27 PST 2014
#   Fix non-msvc-ide filename path. Patch provided by John Cary.
#
#   Kathleen Biagas, Fri Oct 31 11:18:54 PDT 2014
#   Move VISIT_PLUGIN_TARGET_RTOD to VisItMacros.cmake, so it can be used
#   by PluginVsInstall.cmake (since it needs no re-write).
#
#****************************************************************************/


MACRO(VISIT_INSTALL_PLUGINS type)
    IF(NOT VISIT_STATIC)
        IF(VISIT_RPATH_RELATIVE_TO_EXECUTABLE_PATH)
            SET_TARGET_PROPERTIES(${ARGN} PROPERTIES INSTALL_RPATH "$ORIGIN/../../lib")
        ENDIF(VISIT_RPATH_RELATIVE_TO_EXECUTABLE_PATH)
        IF(NOT WIN32)
            INSTALL(TARGETS ${ARGN}
                LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/${type}
                RUNTIME DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/${type}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE 
                            WORLD_READ             WORLD_EXECUTE
                CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
            )
        ELSE(NOT WIN32)
            # ${BUILD_TYPE} refers to the configuration option chosen (Release,
            # Debug, etc). It is a var that will be given a value during compile
            # not configure, so the dollar sign must be escaped in the string
            # below.  Then during install, ${BUILD_TYPE} will be expanded.
            FOREACH(target ${ARGN})
                IF(MSVC_IDE)
                  SET(filename "${VISIT_BINARY_DIR}/exe/\${BUILD_TYPE}/${type}/${target}.dll")
                  INSTALL(FILES ${filename}
                    DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/${type}
                    COMPONENT RUNTIME
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                                GROUP_READ GROUP_WRITE GROUP_EXECUTE 
                                WORLD_READ             WORLD_EXECUTE
                    CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                  )
                ELSE()  # For no IDE, installed straight into exe
                  SET(filename "${VISIT_BINARY_DIR}/exe/${type}/${target}.dll")
                  INSTALL(FILES ${filename}
                    DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/${type}
                    COMPONENT RUNTIME
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                                GROUP_READ GROUP_WRITE GROUP_EXECUTE 
                                WORLD_READ             WORLD_EXECUTE
                  )
                ENDIF()
            ENDFOREACH(target)
        ENDIF(NOT WIN32)
    ENDIF(NOT VISIT_STATIC)
ENDMACRO(VISIT_INSTALL_PLUGINS)

MACRO(VISIT_INSTALL_DATABASE_PLUGINS)
    VISIT_INSTALL_PLUGINS(databases ${ARGN})
ENDMACRO(VISIT_INSTALL_DATABASE_PLUGINS)

MACRO(VISIT_INSTALL_OPERATOR_PLUGINS)
    VISIT_INSTALL_PLUGINS(operators ${ARGN})
ENDMACRO(VISIT_INSTALL_OPERATOR_PLUGINS)

MACRO(VISIT_INSTALL_PLOT_PLUGINS)
    VISIT_INSTALL_PLUGINS(plots ${ARGN})
ENDMACRO(VISIT_INSTALL_PLOT_PLUGINS)

MACRO(VISIT_PLUGIN_TARGET_FOLDER type pname) 
    SET_TARGET_PROPERTIES(${ARGN} PROPERTIES FOLDER "plugins/${type}/${pname}")
ENDMACRO(VISIT_PLUGIN_TARGET_FOLDER)

