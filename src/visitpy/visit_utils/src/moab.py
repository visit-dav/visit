#!/usr/bin/env python
# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

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