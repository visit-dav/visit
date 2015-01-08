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


from common import VisItException, hostname, require_visit


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
        if not self.partitions.has_key(part):
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
                if par and len(defs.keys()) == 0:
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
    if len(__hosts.keys()) == 0 or reload:
        __hosts.clear()
        if legacy:
            llnl_open, llnl_closed = legacy_vutils_engine_profiles()
            __hosts.update(llnl_open)
            __hosts.update(llnl_closed)
        else:
            if vdir is None:
                vdir = os.environ["VISITHOME"]
            hpf_files = glob.glob(pjoin(vdir,".visit","hosts","*.xml"))
            for hpf_file in hpf_files:
                try:
                    hpf = HostProfile.load(hpf_file)
                    __hosts[hpf.name] = hpf
                except:
                    print "[warning: Could not load host profile: '%s']" % hpf_file
    return __hosts
