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
#   Kevin Griffin, Thu Jan  4 12:45:28 PST 2018
#   Changed the linked directory lib/qwt.framework/Headers to the actual path
#   it was linked to. This fixes the make install symlink error. 
#
#   Kathleen Biagas, Thu Feb  8 08:30:19 PST 2018
#   Set QWT_LIBRARY to full path, for use in target_link_libraries.
#
#*****************************************************************************

if(NOT EXISTS ${VISIT_QWT_DIR})
    message(FATAL_ERROR "Qwt installation directory is not specified or does not exist")
endif()

include(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

if(APPLE)
    if(VISIT_STATIC)
        SET_UP_THIRD_PARTY(QWT lib include qwt)
    else()
        SET_UP_THIRD_PARTY(QWT lib lib/qwt.framework/Versions/Current/Headers qwt)
    endif()
else()
    SET_UP_THIRD_PARTY(QWT lib include qwt)
endif()

if(NOT QWT_FOUND)
    message(FATAL_ERROR "Qwt installation could not be used.")
endif()

SET(QWT_LIBRARY ${QWT_LIBRARY_DIR}/${QWT_LIB} CACHE FILEPATH "full path to qwt library" FORCE)
