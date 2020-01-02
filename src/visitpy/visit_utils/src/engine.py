# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

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
from .host_profile import *

from .common import VisItException, hostname, require_visit


try:
    import visit
except:
    pass

__all__ = [ "open","close","supported_hosts"]

def supported_hosts():
    """ Returns a list of the names of supported hosts. """
    res = list(hosts().keys())
    res.sort()
    return res

def open(**kwargs):
    """ Launch VisIt compute engine on the current host. """
    args = {"ppn":1,"part":None,"bank":None,"rtime":None,"vdir":None}
    if "method" not in kwargs:
        hname = hostname(False)
        # when the visit module is imported (vs used in the CLI), 
        # VISITHOME won't be set, allow user to pass vdir argument 
        # here to locate the host profiles
        vdir = None
        if "vdir" in kwargs:
            vdir = kwargs["vdir"]
        if hname not in hosts(vdir=vdir):
            raise VisItException("Unsupported host: '%s'" % hname)
        host = hosts(vdir=vdir)[hname]
        # prep args for launch
        args["host"] = host.name
        args.update(host.defaults)
        args.update(host.params)
        args["method"] = host.launch_method(args["part"])
    elif kwargs["method"] == "slurm":
        args["host"] = hostname(False)
        if "SLURM_JOB_NUM_NODES" in os.environ:
            nnodes = int(os.environ["SLURM_JOB_NUM_NODES"])
            ppn    = int(os.environ["SLURM_CPUS_ON_NODE"])
            nprocs = ppn * nnodes
        else:
            raise VisItException("engine.open(method='slurm') requires "
                                 "SLURM_JOB_NUM_NODES and SLURM_CPUS_ON_NODE env vars")
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
    print(msg)
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
        if rtime.count(":") == 0:
            rtime += ":00"
        args.extend(["-b", bank,"-t", rtime])
    if visit.OpenComputeEngine(ehost,args) != 1:
        raise VisItException("Failed to open compute engine on %s." % host)
    return visit.GetEngineList()[0]
