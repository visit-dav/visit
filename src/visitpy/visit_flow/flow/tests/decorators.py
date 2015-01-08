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
#*****************************************************************************
"""
 file: decorators.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/26/2012
 description:
    Provides decorators that skip numpy & pyopencl related tests
    if these modules are not available.

"""
from flow.core import sexe

def skip_warning(name):
    print "[%s not found: Skipping dependant test.]" % name

def numpy_test(fn):
    """
    Decorator that skips tests that require numpy.
    """
    def run_fn(*args):
        try:
            import numpy
        except ImportError:
            skip_warning("numpy")
            return None
        return fn(*args)
    return run_fn

def pyocl_test(fn):
    """
    Decorator that skips tests that require pyopencl.
    """
    def run_fn(*args):
        try:
            import pyopencl
        except ImportError:
            skip_warning("pyopencl")
            return None
        return fn(*args)
    return run_fn

def imagick_test(fn):
    """
    Decorator that skips tests that require ImageMagick tools.
    """
    def run_fn(*args):
        # check for convert, composite
        r1 = sexe("which convert",  ret_output=True)
        r2 = sexe("which composite",ret_output=True)
        if r1[0] != 0 or r2[0]!=0:
            skip_warning("ImageMagick")
            return None
        return fn(*args)
    return run_fn
