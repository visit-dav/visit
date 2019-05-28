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
#   Kathleen Bonnell, Fri Dec 10 14:36:52 PST 2010
#   Set ZLIB_LIB to full path.
#
#   Brad Whitlock, Thu Apr 18 14:38:02 PDT 2013
#   Look for zlib on other platforms if VISIT_ZLIB_DIR is set.
#
#   Kathleen Biagas, Wed Jun  5 16:47:21 PDT 2013
#   Always set VTKZLIB_LIB on windows if the vtkzlib target exists.
#
#   Kathleen Biagas, Fri Aug  9 11:01:29 PDT 2013
#   IF this file finds zlib, set HAVE_ZLIB_H. CHECK_INCLUDE_FILES might not 
#   find it, especially on windows.
#
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Removed VISIT_MSVC_VERSION from windows handling.
#
#   Kathleen Biagas, Fri Mar 17 10:29:19 PDT 2017
#   Set HAVE_LIBZ when ZLIB_FOUND.
#
#   Kathleen Biagas, Tue May 28 09:08:56 PDT 2019
#   Since we require zlib from build_visit, no longer need HAVE_ZLIB_H
#
#****************************************************************************/

# Use the ZLIB_DIR hint from the config-site .cmake file 

include(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

if (WIN32)
    if(ZLIB_LIB_NAME)
        SET_UP_THIRD_PARTY(ZLIB lib include ${ZLIB_LIB_NAME})
    else()
        SET_UP_THIRD_PARTY(ZLIB lib include zlib1)
    endif()
    if (ZLIB_FOUND)
        # use full path here, instead of just lib file.
        #set(ZLIB_LIBRARY "${ZLIB_LIB}" CACHE STRING "zlib library" FORCE)
        set(ZLIB_LIBRARY "${ZLIB_LIBRARY_DIR}/${ZLIB_LIB}" CACHE STRING "full path to zlib library" FORCE)
    endif ()
else(WIN32)
    if(VISIT_ZLIB_DIR)
        # We've told VisIt where to look for zlib.
        SET_UP_THIRD_PARTY(ZLIB lib include z)
        if(ZLIB_FOUND)
            # use full path here, instead of just lib file.
            set(ZLIB_LIBRARY "${ZLIB_LIBRARY_DIR}/${ZLIB_LIB}" CACHE STRING "zlib library" FORCE)
        endif()
    endif()
endif (WIN32)

if(ZLIB_FOUND)
    set(HAVE_LIBZ true CACHE BOOL "Have lib z")
else()
    message(FATAL_ERROR "VisIt requires lib z and it could not be found.")
endif()

