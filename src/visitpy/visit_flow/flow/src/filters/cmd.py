# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: cmd.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 10/14/2010
 description:
    Provides a flow filter that executes shell commands.

"""

from ..core import Filter, sexe

class ShellCommand(Filter):
    filter_type    = "cmd"
    input_ports    = ["in"]
    default_params = {"cmd": None, "obase":None}
    output_port    = True
    def execute(self):
        obase = self.name + "_%s_cmd_output"
        if not self.params.obase is None:
            obase = self.params.obase
        cmd = "%s %s " % (self.params.cmd,self.input("in"))
        cmd = cmd.replace("{index}",str(self.state_vector.index()))
        if obase.count("%s") >0:
            ofname = obase % str(self.state_vector.idx)
        else:
            ofname = obase
        cmd = cmd + ofname
        sexe(cmd)
        return ofname

filters = [ShellCommand]
