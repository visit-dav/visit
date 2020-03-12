# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_filters_file_ops.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/24/2012
 description:
    unittest test cases for Filters in the file_ops module.

"""

import unittest
import os
from os.path import join as pjoin

from visit_flow import *
from visit_flow.filters import file_ops

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)


class TestFile(unittest.TestCase):
    def setUp(self):
        odir = pjoin("tests","_test_output")
        if not os.path.isdir(odir):
            os.mkdir(odir)
        print("")
    def test_01_workspace_setup(self):
        w = Workspace()
        w.register_filters(file_ops)
        for i in range(10):
            odir = pjoin("tests","_test_output")
            f = open(pjoin(odir,"test.input.%04d.txt" % i),"w")
            f.write("%0d\n" % i)
            f.close()
        fi = w.add_filter("file_name","finput")
        fi["pattern"] = pjoin(odir,"test.input.%04d.txt")
        mv = w.add_filter("file_rename","mv")
        mv["pattern"] = pjoin(odir,"result.%04d.txt")
        w.connect("finput","mv:in")
        print(w.graph)
        sspace = StateSpace({"index":10})
        sgen = StateVectorGenerator(sspace)
        for svec in sgen:
            w.execute(svec)

if __name__ == '__main__':
    unittest.main()

