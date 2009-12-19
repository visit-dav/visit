#*****************************************************************************
#
# Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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

# Use the TEEM_DIR and SLIVR_DIR hints from the config-site .cmake file 
# Except on windows, where it is part of the repo.
IF (WIN32)
    SET(TEEM_DIR  ${VISIT_WINDOWS_DIR}/ThirdParty/teem)
    SET(SLIVR_DIR ${VISIT_WINDOWS_DIR}/ThirdParty/slivr)
ENDIF (WIN32)

# Use the normal FIND_PATH and FIND_LIBRARY so that
# SLIVR vars are available in gui

FIND_PATH(TEEM_INCLUDE_DIR "teem/nrrd.h"
    ${TEEM_DIR}/include
    DOC "Path to nrrd.h"
)

FIND_PATH(SLIVR_INCLUDE_DIR "slivr/VolumeRenderer.h"
    ${SLIVR_DIR}/include
    DOC "Path to VolumeRenderer.h"
)

FIND_LIBRARY(SLIVRG_LIBRARY NAMES slivrG
    PATHS ${SLIVR_DIR}/lib ${SLIVR_DIR}/lib/${VISIT_MSVC_VERSION}
    DOC "Path to the SLIVRG library"
)
FIND_LIBRARY(SLIVRV_LIBRARY NAMES slivrV
    PATHS ${SLIVR_DIR}/lib ${SLIVR_DIR}/lib/${VISIT_MSVC_VERSION}
    DOC "Path to the SLIVRV library"
)

SET(SLIVR_LIBRARY_DIR ${TEEM_DIR}/lib ${SLIVR_DIR}/lib)

IF(SLIVR_INCLUDE_DIR AND TEEM_INCLUDE_DIR)
    SET(SLIVR_FOUND 1)
    SET(SLIVR_GUI_IMPL    QvisCMap2Display.C QvisCMap2Widget.C)
    SET(SLIVR_GUI_HDR     QvisCMap2Display.h QvisCMap2Widget.h)
    SET(SLIVR_LIBRARY_DIR ${TEEM_DIR}/lib ${SLIVR_DIR}/lib)
    SET(SLIVR_INCLUDE_DIR ${TEEM_INCLUDE_DIR} ${SLIVR_INCLUDE_DIR})
    SET(SLIVR_LIBS        slivrV slivrG nrrd air biff hest ${GLEW_LIBS})
ELSE(SLIVR_INCLUDE_DIR AND TEEM_INCLUDE_DIR)
    SET(SLIVR_FOUND 0)
    SET(SLIVR_GUI_IMPL "")
    SET(SLIVR_GUI_HDR  "")
    SET(SLIVR_LIBRARY_DIR  "")
    SET(SLIVR_INCLUDE_DIR  "")
    SET(SLIVR_LIBS     "")
ENDIF(SLIVR_INCLUDE_DIR AND TEEM_INCLUDE_DIR)
