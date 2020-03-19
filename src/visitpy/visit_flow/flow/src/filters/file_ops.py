# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: file_ops.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 10/14/2010
 description:
    Provides flow filters that file system commands.

"""

from ..core import Filter, PropertyTree, sexe

def gen_ofname(node):
    p = node.params
    pattern = p.pattern
    offset  = p.offset
    if pattern.find("%") >0:
        ofname = pattern % (node.state_vector.index() + offset)
    else:
        ofname = pattern
    return ofname

class FileName(Filter):
    filter_type    = "file_name"
    input_ports    = []
    default_params = {"pattern":None, "offset":0}
    output_port    = True
    def execute(self):
        return gen_ofname(self)

class FileRename(Filter):
    filter_type    = "file_rename"
    input_ports    = ["in"]
    default_params =  {"pattern":None, "offset":0}
    output_port    = True
    def execute(self):
        ofname = gen_ofname(self)
        cmd = "mv %s %s" % (self.input("in"),ofname)
        sexe(cmd)
        return ofname

class FileCopy(Filter):
    filter_type    = "file_copy"
    input_ports    = ["in"]
    default_params = {"pattern":None, "offset":0}
    output_port    = True
    def execute(self):
        ofname = gen_ofname(self)
        cmd = "cp %s %s" % (self.input("in"),ofname)
        sexe(cmd)
        return ofname


filters = [FileName, FileRename, FileCopy]
