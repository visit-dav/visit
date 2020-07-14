# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_flow_vpe_npy_ops.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/28/2012
 description:
    unittest test cases for flow vpe w/ npy_ops module.

"""

import os
import unittest
import visit

from os.path import join as pjoin

from flow import *
from flow.filters import npy_ops

from visit_flow_vpe import *

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)


tests_dir    = os.path.split(__file__)[0]
examples_dir = pjoin(tests_dir,"..","examples")

class TestFlowVPE(unittest.TestCase):
    def setUp(self):
        self.data_path = pjoin(tests_dir,"_data","rect2d.silo")
        visit.OpenDatabase(self.data_path)
        print("")
    def setup_workspace(self,file):
        define_flow_vpe("flow",pjoin(examples_dir,file),"pyocl_ops",0,0)
        visit.AddPlot("Pseudocolor","flow")
        visit.DrawPlots()
        visit.DefineScalarExpression("check","flow - ((d + p)^2.0 + (d-p)^2.0)")
        visit.AddPlot("Pseudocolor","check")
        visit.DrawPlots()
    def test_01_npy_ops(self):
        self.setup_workspace("flow_vpe_npy_ops_example_1.py")
        # the total sum of all scalar vals of 'check' should equal zero.
        res = 1e8
        if visit.Query("Variable Sum"):
            res = visit.GetQueryOutputValue()
        self.assertTrue(res < 1.0e-8)
    def test_02_pyocl_ops(self):
        self.setup_workspace("flow_vpe_pyocl_ops_example_1.py")
        # the total sum of all scalar vals of 'check' should equal zero.
        res = 1e8
        if visit.Query("Variable Sum"):
            res = visit.GetQueryOutputValue()
        self.assertTrue(res < 1.0e-8)
    def test_03_pyocl_compile(self):
        self.setup_workspace("flow_vpe_pyocl_compile_example_1.py")
        # the total sum of all scalar vals of 'check' should equal zero.
        res = 1e8
        if visit.Query("Variable Sum"):
            res = visit.GetQueryOutputValue()
        self.assertTrue(res < 1.0e-8)
    def tearDown(self):
        # clean up
        visit.DeleteAllPlots()
        visit.CloseDatabase(self.data_path)
        visit.CloseComputeEngine()


if __name__ == '__main__':
    unittest.main()

