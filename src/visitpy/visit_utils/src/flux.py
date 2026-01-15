# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
"""
 file: flux.py
 description:
    Provides a python command for flux submission.
"""

import os
from visit_utils.common import *

class FluxBatchError(Exception):
    def __init__(self,emsg):
        self.msg = "<flux batch error:> " + emsg

def flux_batch(cmd,rmin,nnodes=1,mach=None,part=None,bank=None,rdir=None,obase=None,depend=None):
    if mach is None:
        mach = hostname(False)
    # create output file name
    if obase is None:
        ctoks = cmd.split()
        sname = os.path.split(ctoks[0])[1]
        obase = sname
    ofile = "out.flux.batch.%s.%s.%s.txt" % (obase,hostname(),timestamp())
    xcmd  = "flux batch --output %s -N %d -t %s" % (ofile,nnodes, str(rmin))
    if not rdir is None:
        xcmd += " -d %s " % (os.path.abspath(rdir))
    if not depend is None:
        xcmd += " -after:%s " % depend
    if not part is None:
        xcmd += " -q %s" % part
    if not bank is None:
        xcmd += " -B %s" % bank
    xcmd += " %s" % cmd
    ret,out = sexe(xcmd,ret_output=True,echo=True)
    if ret == 0:
        jid = out.strip()
        return jid, ofile
    else:
        raise FluxBatchError(out)
