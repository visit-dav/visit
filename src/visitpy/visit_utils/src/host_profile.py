# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: host_profile.py
 author: Cyrus Harrison (cyrush@llnl.gov)
 created: 3/2/2012
 description:
    Provides host profile infrastructure for engine.py

"""


import sys
import os
import math
import glob

import xml.etree.ElementTree as et
from os.path import join as pjoin


from .common import VisItException, hostname, require_visit


try:
    import visit
except:
    pass

__all__ = ["hosts"]

__hosts = {}

class HostProfile(object):
    def __init__(self,name,params,partitions,defaults):
        self.name = name
        self.params = params
        self.partitions = partitions
        self.defaults = defaults
    def launch_method(self,part):
        if part not in self.partitions:
            raise VisItException("Unsupported partition '%s' on host %s" % (part,self.name))
        return self.partitions[part]
    @classmethod
    def load(cls,xml_file):
        tree = et.parse(xml_file)
        root = tree.getroot()
        # get host name:
        host = cls.__fetch_xml_field(root,"host")
        if host.count("."):
            host = host[:host.find(".")]
        # we will want to get the first instance of ppn
        ppn = None
        params = {"vdir": cls.__fetch_xml_field(root,"directory")}
        parts  = {}
        defs   = {}
        for lp in root.findall("Object"):
            if lp.attrib["name"] == "LaunchProfile":
                pname  = cls.__fetch_xml_field(lp,"profileName")
                nprocs = cls.__fetch_xml_field(lp,"numProcessors")
                par    = cls.__fetch_xml_field(lp,"parallel")
                if par and ppn  is None:
                    nnset =  cls.__fetch_xml_field(lp,"numNodesSet")
                    if nnset:
                        nn  = cls.__fetch_xml_field(lp,"numNodes")
                        ppn = nprocs / nn
                if par and len(list(defs.keys())) == 0:
                    nnset =  cls.__fetch_xml_field(lp,"numNodesSet")
                    nn    = cls.__fetch_xml_field(lp,"numNodes")
                    defs["nprocs"] = nprocs
                    if cls.__fetch_xml_field(lp,"partitionSet"):
                        defs["part"] = cls.__fetch_xml_field(lp,"partition")
                    if cls.__fetch_xml_field(lp,"bankSet"):
                        defs["bank"] = cls.__fetch_xml_field(lp,"bank")
                    if cls.__fetch_xml_field(lp,"timeLimitSet"):
                        defs["rtime"] = cls.__fetch_xml_field(lp,"timeLimit")
                if not par:
                    parts[pname] = "serial"
                else:
                    if cls.__fetch_xml_field(lp,"partitionSet"):
                        parts[cls.__fetch_xml_field(lp,"partition")] = cls.__fetch_xml_field(lp,"launchMethod")
                    else:
                        parts[pname] = cls.__fetch_xml_field(lp,"launchMethod")
        if not ppn is None:
            params["ppn"] = ppn
        return HostProfile(host, params, parts, defs)
    @classmethod
    def __fetch_xml_field(cls,n,fname):
        for f in n.findall("Field"):
            if f.attrib["name"] == fname:
                ftype = f.attrib["type"]
                if  ftype == "int":
                    return int(f.text)
                elif ftype == "bool":
                    return f.text.lower().strip() == "true"
                elif ftype == "stringVector":
                    return f.text.split()
                else:
                    return f.text


def hosts(vdir=None,legacy=False,reload=False):
    global __hosts
    if len(list(__hosts.keys())) == 0 or reload:
        __hosts.clear()
        if legacy:
            llnl_open, llnl_closed = legacy_vutils_engine_profiles()
            __hosts.update(llnl_open)
            __hosts.update(llnl_closed)
        else:
            if vdir is None:
                if "VISITHOME" not in os.environ:
                    raise VisItException("hosts() requires VISITHOME env var or explicit vdir argument")
                vdir = os.environ["VISITHOME"]
            hpf_files = glob.glob(pjoin(vdir,".visit","hosts","*.xml"))
            for hpf_file in hpf_files:
                try:
                    hpf = HostProfile.load(hpf_file)
                    __hosts[hpf.name] = hpf
                except:
                    print("[warning: Could not load host profile: '%s']" % hpf_file)
    return __hosts
