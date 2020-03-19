# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_parser.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 8/29/2012
 description:
    unittest test cases for parser front-end.

"""

import unittest
try:
    import numpy as npy
except:
    pass

from visit_flow import *
from visit_flow.filters import npy_ops

from decorators import numpy_test, ply_test
# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)

class TestParser(unittest.TestCase):
    @ply_test
    def test_01_simple_expr_gen(self):
        filters = Generator.parse_network("vel_mag = sqrt(vx^2 + vy^2 + vz^2)")
        print("")
        for f in filters:
            print(f)
        self.assertTrue(True)
    @ply_test
    def test_02_simple_expr_gen_context(self):
        w = Workspace()
        w.register_filters(npy_ops)
        v_a = npy.array(list(range(10)),dtype=npy.double)
        v_b = npy.array(list(range(10)),dtype=npy.double)
        c_2 = 2.0
        w.registry_add(":v_a",v_a)
        w.registry_add(":v_b",v_b)
        w.registry_add(":c_2",c_2)
        print("")
        expr = "res = (v_a + v_b)^c_2 + (v_a  - v_b)^c_2"
        print("test_expr: " + expr)
        filters = Generator.parse_network(expr,w)
        print("")
        print(w.graph)
        print(w.execution_plan())
        act_res = w.execute()
        # get output and test
        test_res = npy.power((v_a + v_b),2.0)+ npy.power((v_a - v_b),2.0)
        dsum = npy.sum(act_res - test_res)
        print("Filter Graph Result: %s" % str(act_res))
        print("Test Result:         %s" % str(test_res))
        print("Difference:          %s" % str(dsum))
        self.assertTrue(dsum < 1e-6)
    @ply_test
    def test_03_simple_expr_gen_from_workspace(self):
        w = Workspace()
        w.register_filters(npy_ops)
        v_a = npy.array(list(range(10)),dtype=npy.double)
        v_b = npy.array(list(range(10)),dtype=npy.double)
        w.registry_add(":v_a",v_a)
        w.registry_add(":v_b",v_b)
        print("")
        expr = "res = (v_a + v_b)^2 + (v_a  - v_b)^2"
        print("test_expr: " + expr)
        w.setup_expression_network(expr)
        print("")
        print(w.graph)
        print(w.execution_plan())
        act_res = w.execute()
        # get output and test
        test_res = npy.power((v_a + v_b),2.0)+ npy.power((v_a - v_b),2.0)
        dsum = npy.sum(act_res - test_res)
        print("Filter Graph Result: %s" % str(act_res))
        print("Test Result:         %s" % str(test_res))
        print("Difference:          %s" % str(dsum))
        self.assertTrue(dsum < 1e-6)
    @ply_test
    def test_04_simple_decomp_gen_context(self):
        w = Workspace()
        w.register_filters(npy_ops)
        v_a = npy.array(list(range(10)),dtype=npy.double)
        v_b = npy.ndarray((10,2),dtype=npy.double)
        c_2 = 2.0
        w.registry_add(":v_a",v_a)
        w.registry_add(":v_b",v_b)
        w.registry_add(":c_2",c_2)
        v_b[:,0] = list(range(10))
        v_b[:,1] = list(range(10,20))
        expr = "res = (v_a + v_b[0])^c_2 + (v_a  - v_b[1])^c_2"
        print("test_expr: " + expr)
        w.setup_expression_network(expr)
        print("")
        print(w.graph)
        print(w.execution_plan())
        act_res = w.execute()
        test_res = npy.power((v_a + v_b[:,0]),2.0)+ npy.power((v_a - v_b[:,1]),2.0)
        dsum = npy.sum(act_res - test_res)
        print("Filter Graph Result: %s" % str(act_res))
        print("Test Result:         %s" % str(test_res))
        print("Difference:          %s" % str(dsum))
        self.assertTrue(dsum < 1e-6)

if __name__ == '__main__':
    unittest.main()

