# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_filters_cmd.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/24/2012
 description:
    unittest test cases for Filters in the cmd module.

"""

import unittest
import os
from os.path import join as pjoin

from visit_flow import *
from visit_flow.filters import cmd, file_ops

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)

class TestCmd(unittest.TestCase):
    def setUp(self):
        odir = pjoin("tests","_test_output")
        if not os.path.isdir(odir):
            os.mkdir(odir)
        print("")
    def test_01_workspace_setup(self):
        odir = pjoin("tests","_test_output")
        w = Workspace()
        w.register_filters(file_ops)
        w.register_filters(cmd)
        for i in range(10):
            f = open(pjoin(odir,"cmd.test.input.%04d.txt" % i),"w")
            f.write("%0d\n" % i)
            f.close()
        fi = w.add_filter("file_name","finput")
        fi["pattern"] = pjoin(odir,"cmd.test.input.%04d.txt")
        shell_move = w.add_filter("cmd","shell_move")
        shell_move["cmd"]   = "mv "
        shell_move["obase"] = pjoin(odir,"cmd_mv")
        w.connect("finput","shell_move:in")
        print(w.graph)
        sspace = StateSpace({"index":10})
        sgen = StateVectorGenerator(sspace)
        w.execute(sgen)

if __name__ == '__main__':
    unittest.main()

