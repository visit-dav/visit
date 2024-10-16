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
    """ Launch VisIt compute engine on the current host.

    Example usage:
    
    Launch engine with 36 MPI tasks using default options for this host:

      engine.open(nprocs=36)

    Launch engine with 36 MPI tasks using a specific partition:

      engine.open(nprocs=36, part="pbatch")

    Launch engine with 36 MPI tasks, ask for 60 minute time limit:

      engine.open(nprocs=36, rtime=60)

    If you already have a slurm batch allocation, you can use:

      engine.open(method="slurm")

    This reads the SLURM_NPROCS or (SLURM_JOB_NUM_NODES and
    SLURM_CPUS_ON_NODE) env vars and uses these values to launch
    with srun.

    If you already have a lsf batch allocation, you can use:

      engine.open(method="lsf")

    This reads the LSB_DJOB_NUMPROC env var and uses it
    the to launch with mpirun.

    """
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
        if "SLURM_NPROCS" in os.environ:
            nprocs = int(os.environ["SLURM_NPROCS"])
        elif "SLURM_JOB_NUM_NODES" in os.environ:
            nnodes = int(os.environ["SLURM_JOB_NUM_NODES"])
            ppn    = int(os.environ["SLURM_CPUS_ON_NODE"])
            args["ppn"]      = ppn
            nprocs = ppn * nnodes
        else:
            raise VisItException("engine.open(method='slurm') requires "
                                 "SLURM_NPROCS OR (SLURM_JOB_NUM_NODES and SLURM_CPUS_ON_NODE) env vars")
        args["nprocs"]   = nprocs
        kwargs["method"] = "srun"
    elif kwargs["method"] == "lsf":
        args["host"] = hostname(False)
        if "LSB_DJOB_NUMPROC" in os.environ:
            nprocs = int(os.environ["LSB_DJOB_NUMPROC"])
        else:
            raise VisItException("engine.open(method='lsf') requires "
                                 "LSB_DJOB_NUMPROC env vars")
        args["nprocs"]   = nprocs
        kwargs["method"] = "mpirun"
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
