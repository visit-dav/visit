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
#   Kathleen Biagas, Thu March 7, 2019
#   Added sphinx_launcher (python executable) for Windows.
#   Removed configure_file in favor of passing options via arguments to
#   sphinx build
#
#****************************************************************************/
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindSphinx.cmake)

if(NOT DEFINED SPHINX_THEME)
    set(SPHINX_THEME sphinx_rtd_theme)
endif()
 
if(NOT DEFINED SPHINX_THEME_DIR)
    set(SPHINX_THEME_DIR _themes)
endif()
 
# configured documentation tools and intermediate build results
#set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")
 
# Sphinx cache with pickled ReST documents
#set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")
 
# HTML output directory
set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")
 
if(WIN32)
    set(sphinx_launcher ${PYTHON_EXECUTABLE}) 
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/conf.py"
    "${CMAKE_CURRENT_BINARY_DIR}/conf.py"
    @ONLY)
 
add_custom_target(user_manuals ALL
    ${sphinx_launcher}
    ${SPHINX_EXECUTABLE}
        -a -q -b html
        -D html_theme_path=${SPHINX_THEME_DIR}
        -D html_theme=${SPHINX_THEME}
        -D version="${VISIT_VERSION_MAJOR}.${VISIT_VERSION_MINOR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${SPHINX_HTML_DIR}"
    COMMENT "Building Sphinx-HTML documentation for GUI and CLI")

if(WIN32)
    unset(sphinx_launcher)
endif()

