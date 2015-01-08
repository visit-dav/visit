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
 file: engine.py
 author: Cyrus Harrison (cyrush@llnl.gov)
 created: 4/9/2010
 description:
    Provides open/close methods that can be used to easily launch
    visit engines.

"""


import sys
import os
import math
from host_profile import *

from common import VisItException, hostname, require_visit


try:
    import visit
except:
    pass

__all__ = [ "open","close","supported_hosts"]

def supported_hosts():
    """ Returns a list of the names of supported hosts. """
    res = hosts().keys()
    res.sort()
    return res

def open(**kwargs):
    """ Launch VisIt compute engine on the current host. """
    args = {"ppn":1,"part":None,"bank":None,"rtime":None,"vdir":None}
    if not kwargs.has_key("method"):
        hname = hostname(False)
        if not hosts().has_key(hname):
            raise VisItException("Unsupported host: '%s'" % hname)
        host = hosts(args["vdir"])[hname]
        # prep args for launch
        args["host"] = host.name
        args.update(host.defaults)
        args.update(host.params)
        args["method"] = host.launch_method(args["part"])
    elif kwargs["method"] == "slurm":
        args["host"] = hostname(False)
        if os.environ.has_key("SLURM_JOB_NUM_NODES"):
            nnodes = int(os.environ["SLURM_JOB_NUM_NODES"])
            ppn    = int(os.environ["SLURM_CPUS_ON_NODE"])
            nprocs = ppn * nnodes
        args["nprocs"]   = nprocs
        args["ppn"]      = ppn
        kwargs["method"] = "srun"
    else:
        args["host"] = hostname(False)
    args.update(kwargs)
    return launch(**args)

@require_visit
def close(ename=None):
    """ Closes VisIt's Compute Engine. """
    if ename is None:
        if visit.CloseComputeEngine() != 1:
            raise VisItException("Failed to close compute engine.")
    else:
        if visit.CloseComputeEngine(ename) != 1:
            raise VisItException("Failed to close compute engine '%s'." % ename)

@require_visit
def launch(host,nprocs,ppn,method,part,bank,rtime,vdir,extra_args=None):
    """ Launch helper. """
    msg = "[%s: opening engine" % host
    if not part is None:
        msg+= " on %s" % part
    msg +="]"
    print  msg
    nnodes = int(math.ceil(float(nprocs) / float(ppn)))
    ehost = host
    if host == hostname(False):
        ehost = "localhost"
    args = ["-l", method]
    if not method == "serial":
        args.extend(["-np",str(nprocs)])
    if not vdir is None:
        args.extend(["-dir",vdir])
    if not part is None:
        args.extend(["-nn",str(nnodes),"-p", part])
    if not extra_args is None:
        args.extend(extra_args)
    if method == "msub/srun":
        rtime = str(rtime)
        if rtime.count(":") == 0: rtime += ":00"
        args.extend(["-b", bank,"-t", rtime])
    if visit.OpenComputeEngine(ehost,args) != 1:
        raise VisItException("Failed to open compute engine on %s." % host)
    return visit.GetEngineList()[0]
