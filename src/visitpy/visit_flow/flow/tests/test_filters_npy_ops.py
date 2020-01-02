# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_filters_npy_ops.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/24/2012
 description:
    unittest test cases for Filters in the npy_ops module.

"""

import unittest
try:
    import numpy as npy
except:
    pass

from visit_flow import *
from visit_flow.filters import npy_ops

from decorators import numpy_test

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)


class TestNPyOps(unittest.TestCase):
    def setUp(self):
        print("")
    @numpy_test
    def test_01_simple_workspace(self):
        w = Workspace()
        w.register_filters(npy_ops)
        v_a = npy.array(list(range(10)),dtype=npy.double)
        v_b = npy.array(list(range(10)),dtype=npy.double)
        print("")
        w.registry_add(":src_a",v_a)
        w.registry_add(":src_b",v_b)
        w.add_filter("add","f1")
        w.add_filter("sub","f2")
        w.add_filter("mult","f3")
        w.add_filter("mult","f4")
        w.add_filter("add","f5")
        # f1 = src_a + src_b
        w.connect(":src_a","f1:in_a")
        w.connect(":src_b","f1:in_b")
        # f2 = src_b - src_a
        w.connect(":src_b","f2:in_a")
        w.connect(":src_a","f2:in_b")
        # f3 = f1^2
        w.connect("f1","f3:in_a")
        w.connect("f1","f3:in_b")
        # f4 = f2^2
        w.connect("f2","f4:in_a")
        w.connect("f2","f4:in_b")
        # f5 = f4 + f3
        w.connect("f3","f5:in_a")
        w.connect("f4","f5:in_b")
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
    @numpy_test
    def test_02_more_ops(self):
        w = Workspace()
        w.register_filters(npy_ops)
        v_a = npy.array(list(range(10)),dtype=npy.double)
        v_b = npy.array(list(range(10)),dtype=npy.double)
        v_p = npy.array([2]*10,dtype=npy.double)
        w.registry_add(":src_a",v_a)
        w.registry_add(":src_b",v_b)
        w.registry_add(":src_p",v_p)
        w.add_filter("add","f1")
        w.add_filter("sub","f2")
        w.add_filter("pow","f3")
        w.add_filter("pow","f4")
        w.add_filter("add","f5")
        # f1 = src_a + src_b
        w.connect(":src_a","f1:in_a")
        w.connect(":src_b","f1:in_b")
        # f2 = src_b - src_a
        w.connect(":src_b","f2:in_a")
        w.connect(":src_a","f2:in_b")
        # f3 = f1^2
        w.connect("f1","f3:in_a")
        w.connect(":src_p","f3:in_b")
        # f4 = f2^2
        w.connect("f2","f4:in_a")
        w.connect(":src_p","f4:in_b")
        # f5 = f4 + f3
        w.connect("f3","f5:in_a")
        w.connect("f4","f5:in_b")
        print("")
        act_res = w.execute()
        # get output and test
        test_res = npy.power((v_a + v_b),2.0)+ npy.power((v_a - v_b),2.0)
        dsum = npy.sum(act_res - test_res)
        print("Filter Graph Result: %s" % str(act_res))
        print("Test Result:         %s" % str(test_res))
        print("Difference:          %s" % str(dsum))
        self.assertTrue(dsum < 1e-6)


if __name__ == '__main__':
    unittest.main()

