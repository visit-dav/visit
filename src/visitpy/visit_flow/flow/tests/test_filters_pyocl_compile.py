# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_filters_pyocl_compile.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/24/2012
 description:
    unittest test cases for Filters in the pyocl_compile module.

"""

import unittest
try:
    import numpy as npy
except:
    pass

from visit_flow import *
from visit_flow.filters import pyocl_compile

from decorators import pyocl_test

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)


class TestPyOpenCLCompile(unittest.TestCase):
    def setUp(self):
        print("")
    @pyocl_test
    def test_01_simple_workspace(self):
        w = Workspace()
        w.register_filters(pyocl_compile)
        v_a = npy.array(list(range(10)),dtype=npy.float32)
        v_b = npy.array(list(range(10)),dtype=npy.float32)
        ctx  = w.add_context("pyocl_compile","root")
        # add our sources to the registry
        ctx.registry_add(":src_a",v_a)
        ctx.registry_add(":src_b",v_b)
        ctx.start(0,0)
        ctx.add_filter("add","f1")
        ctx.add_filter("sub","f2")
        ctx.add_filter("mult","f3")
        ctx.add_filter("mult","f4")
        ctx.add_filter("add","f5")
        # f1 = src_a + src_b
        ctx.connect(":src_a","f1:in_a")
        ctx.connect(":src_b","f1:in_b")
        # f2 = src_b - src_a
        ctx.connect(":src_b","f2:in_a")
        ctx.connect(":src_a","f2:in_b")
        # f3 = f1^2
        ctx.connect("f1","f3:in_a")
        ctx.connect("f1","f3:in_b")
        # f4 = f2^2
        ctx.connect("f2","f4:in_a")
        ctx.connect("f2","f4:in_b")
        # f5 = f4 + f3
        ctx.connect("f3","f5:in_a")
        ctx.connect("f4","f5:in_b")
        ctx.set_output_shape(v_a.shape)
        print(w.graph)
        print(w.execution_plan())
        w.execute()
        print("Generated Kernel\n%s" % ctx.compile())
        act_res  = ctx.run()
        test_res = npy.power((v_a + v_b),2.0)+ npy.power((v_a - v_b),2.0)
        dsum = npy.sum(act_res - test_res)
        print("Filter Graph Result: %s" % str(act_res))
        print("Test Result:         %s" % str(test_res))
        print("Difference:          %s" % str(dsum))
        self.assertTrue(dsum < 1e-6)


if __name__ == '__main__':
    unittest.main()

