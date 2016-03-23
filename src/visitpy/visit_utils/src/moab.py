#!/usr/bin/env python
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
#*****************************************************************************
"""
 file: moab.py
 description:
    Provides a python command for msub submission.
"""

import os
from visit_utils.common import *

class MsubError(Exception):
    def __init__(self,emsg):
        self.msg = "<msub error:> " + emsg

def msub(cmd,rmin,nnodes=1,mach=None,part="pbatch",bank="bdivp",rdir=None,obase=None,depend=None):
    if mach is None:
        mach = hostname(False)
    # create output file name
    if obase is None:
        ctoks = cmd.split()
        sname = os.path.split(ctoks[0])[1]
        obase = sname
    ofile = "out.moab.%s.%s.%s.txt" % (obase,hostname(),timestamp())
    xcmd = "msub -o %s -l nodes=%d -l walltime=%s:00 " % (ofile,nnodes, str(rmin))
    if not rdir is None:
        xcmd += " -d %s " % (os.path.abspath(rdir))
    if not depend is None:
        xcmd += "-l depend=%s " % depend
    xcmd += "-q %s -A %s %s" % (part,bank,cmd)
    ret,out = sexe(xcmd,ret_output=True,echo=True)
    if ret == 0:
        jid = int(out.split(" ")[-1].strip())
        return jid, ofile
    else:
        raise MsubError(out)