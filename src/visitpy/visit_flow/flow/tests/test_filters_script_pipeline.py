# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_filters_spipeline.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 2/4/2013
 description:
    unittest test cases for Filters in the spipeline module.

"""

import unittest
try:
    import numpy as npy
except:
    pass

from visit_flow import *
from visit_flow.filters import script_pipeline

from decorators import numpy_test

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)

class TestSPipeline(unittest.TestCase):
    def setUp(self):
        print("")
    @numpy_test
    def test_01_simple_spipeline(self):
        sdef = {}
        sdef["scripts"] = {"add":{"source":"r=a+b\nsetout(r)",
                                 "vars":["a","b"]},
                          "sub":{"source":"r=a-b\nsetout(r)",
                                 "vars":["a","b"]},
                          "mult":{"source":"r=a*b\nsetout(r)",
                                  "vars":["a","b"]}}
        w = Workspace()
        script_pipeline.register_scripts(sdef["scripts"])
        w.register_filters(script_pipeline)
        v_a = npy.array(list(range(10)),dtype=npy.float32)
        v_b = npy.array(list(range(10)),dtype=npy.float32)
        w.registry_add(":src_a",v_a)
        w.registry_add(":src_b",v_b)
        w.add_filter("add","f1")
        w.add_filter("sub","f2")
        w.add_filter("mult","f3")
        w.add_filter("mult","f4")
        w.add_filter("add","f5")
        w.connect(":src_a","f1:a")
        w.connect(":src_b","f1:b")
        # f1 = src_a + src_b
        w.connect(":src_a","f1:a")
        w.connect(":src_b","f1:b")
        # f2 = src_b - src_a
        w.connect(":src_b","f2:a")
        w.connect(":src_a","f2:b")
        # f3 = f1^2
        w.connect("f1","f3:a")
        w.connect("f1","f3:b")
        # f4 = f2^2
        w.connect("f2","f4:a")
        w.connect("f2","f4:b")
        # f5 = f4 + f3
        w.connect("f3","f5:a")
        w.connect("f4","f5:b")
        act_res = w.execute()
        test_res = npy.power((v_a + v_b),2.0)+ npy.power((v_a - v_b),2.0)
        dsum = npy.sum(act_res - test_res)
        test_res = npy.power((v_a + v_b),2.0)+ npy.power((v_a - v_b),2.0)
        dsum = npy.sum(act_res - test_res)
        print("Filter Graph Result: %s" % str(act_res))
        print("Test Result:         %s" % str(test_res))
        print("Difference:          %s" % str(dsum))
        self.assertTrue(dsum < 1e-6)
    @numpy_test
    def test_02_simple_from_dict(self):
        sdef = {}
        sdef["scripts"] = {"add":{"source":"r=a+b\nsetout(r)",
                                 "vars":["a","b"]},
                          "sub":{"source":"r=a-b\nsetout(r)",
                                 "vars":["a","b"]},
                          "mult":{"source":"r=a*b\nsetout(r)",
                                  "vars":["a","b"]}}
        sdef["nodes"] = {"f1":{"type":"add"},
                         "f2":{"type":"sub"},
                         "f3":{"type":"mult"},
                         "f4":{"type":"mult"},
                         "f5":{"type":"add"}}
        sdef["connections"] = [{"from":":src_a","to":"f1","port":"a"},
                               {"from":":src_b","to":"f1","port":"b"},
                               {"from":":src_b","to":"f2","port":"a"},
                               {"from":":src_a","to":"f2","port":"b"},
                               {"from":"f1","to":"f3","port":"a"},
                               {"from":"f1","to":"f3","port":"b"},
                               {"from":"f2","to":"f4","port":"a"},
                               {"from":"f2","to":"f4","port":"b"},
                               {"from":"f3","to":"f5","port":"a"},
                               {"from":"f4","to":"f5","port":"b"}]
        w = Workspace()
        script_pipeline.register_scripts(sdef["scripts"])
        w.register_filters(script_pipeline)
        v_a = npy.array(list(range(10)),dtype=npy.float32)
        v_b = npy.array(list(range(10)),dtype=npy.float32)
        w.registry_add(":src_a",v_a)
        w.registry_add(":src_b",v_b)
        w.load_dict(sdef)
        act_res = w.execute()
        test_res = npy.power((v_a + v_b),2.0)+ npy.power((v_a - v_b),2.0)
        dsum = npy.sum(act_res - test_res)
        test_res = npy.power((v_a + v_b),2.0)+ npy.power((v_a - v_b),2.0)
        dsum = npy.sum(act_res - test_res)
        print("Filter Graph Result: %s" % str(act_res))
        print("Test Result:         %s" % str(test_res))
        print("Difference:          %s" % str(dsum))
        self.assertTrue(dsum < 1e-6)


if __name__ == '__main__':
    unittest.main()

