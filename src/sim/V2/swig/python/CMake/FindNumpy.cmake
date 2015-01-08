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
#*****************************************************************************
# FindNumpy
#
# Check if numpy is installed and configure c-api includes
#
# This module defines
#  NUMPY_FOUND, set TRUE if numpy and c-api are available
#  NUMPY_INCLUDE_DIR, where to find c-api headers
#  NUMPY_VERSION, numpy release version
set(_TMP_PY_OUTPUT)
set(_TMP_PY_RETURN)
exec_program("${PYTHON_EXECUTABLE}"
  ARGS "-c 'import numpy; print numpy.get_include()'"
  OUTPUT_VARIABLE _TMP_PY_OUTPUT
  RETURN_VALUE _TMP_PY_RETURN)
set(NUMPY_INCLUDE_FOUND FALSE)
if(NOT _TMP_PY_RETURN AND EXISTS "${_TMP_PY_OUTPUT}")
  set(NUMPY_INCLUDE_FOUND TRUE)
else()
  set(_TMP_PY_OUTPUT)
endif()
set(NUMPY_INCLUDE_DIR "${_TMP_PY_OUTPUT}")

set(_TMP_PY_OUTPUT)
set(_TMP_PY_RETURN)
exec_program("${PYTHON_EXECUTABLE}"
  ARGS "-c 'import numpy; print numpy.version.version'"
  OUTPUT_VARIABLE _TMP_PY_OUTPUT
  RETURN_VALUE _TMP_PY_RETURN)
set(NUMPY_VERSION_FOUND FALSE)
if(NOT _TMP_PY_RETURN)
  set(NUMPY_VERSION_FOUND TRUE)
else()
  set(_TMP_PY_OUTPUT)
endif()
set(NUMPY_VERSION "${_TMP_PY_OUTPUT}")

#set(NUMPY_INCLUDE_DIR "${_TMP_PY_OUTPUT}" CACHE PATH "Numpy C API headers")
#mark_as_advanced(NUMPY_INCLUDE_DIR)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NUMPY DEFAULT_MSG NUMPY_INCLUDE_FOUND NUMPY_VERSION_FOUND)
