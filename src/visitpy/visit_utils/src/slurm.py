# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
"""
 file: slurm.py
 description:
    Provides a python command for sbatch submission.
"""

import os
from visit_utils.common import *

class SbatchError(Exception):
    def __init__(self,emsg):
        self.msg = "<sbatch error:> " + emsg

def sbatch(cmd,rmin,nnodes=1,mach=None,part=None,bank=None,rdir=None,obase=None,depend=None):
    if mach is None:
        mach = hostname(False)
    # create output file name
    if obase is None:
        ctoks = cmd.split()
        sname = os.path.split(ctoks[0])[1]
        obase = sname
    ofile = "out.sbatch.%s.%s.%s.txt" % (obase,hostname(),timestamp())
    xcmd  = "sbatch -o %s -N %d -t %s" % (ofile,nnodes, str(rmin))
    if not rdir is None:
        xcmd += " -d %s " % (os.path.abspath(rdir))
    if not depend is None:
        xcmd += " -after:%s " % depend
    if not part is None:
        xcmd += " -p %s" % part
    if not bank is None:
        xcmd += " -A %s" % bank
    xcmd += " %s" % cmd
    ret,out = sexe(xcmd,ret_output=True,echo=True)
    if ret == 0:
        jid = int(out.split(" ")[-1].strip())
        return jid, ofile
    else:
        raise SbatchError(out)