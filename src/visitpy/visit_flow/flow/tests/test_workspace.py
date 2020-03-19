# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_workspace.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/28/2012
 description:
    unittest test cases for filter workspace setup.

"""

import unittest
import math
import json
from visit_flow import *

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)


class CalcPower(Filter):
    filter_type    = "pow"
    input_ports    = ["in"]
    default_params = {"power":2.0}
    output_port    = True
    def execute(self):
        v = self.input("in")
        return math.pow(v,self.params.power)

class CalcAdd(Filter):
    filter_type    = "add"
    input_ports    = ["in_a","in_b"]
    default_params = {}
    output_port    = True
    def execute(self):
        a = self.input(0)
        b = self.input(1)
        return a + b

class CalcContext(Context):
    context_type = "calc"
    def calc_setup(self):
        pass


class TestWorkspace(unittest.TestCase):
    def setUp(self):
        w = Workspace()
        w.register_filter(CalcPower)
        w.register_filter(CalcAdd)
        w.register_context(CalcContext)
        self.w = w
        print("")
    def test_01_simple_setup(self):
        ctx = self.w.add_context("calc","root")
        ctx.calc_setup() # check if we get proper context
        ctx.add_registry_source(":src_a",10.0)
        ctx.add_registry_source(":src_b",20.0)
        self.assertEqual([":src_a",":src_b"],ctx.registry_keys())
        ctx.add_filter("add","f1")
        f2 = ctx.add_filter("pow","f2")
        ctx.connect(":src_a","f1:in_a")
        ctx.connect(":src_b","f1:in_b")
        ctx.connect("f1","f2:in")
        r = self.w.execute()
        self.assertEqual(r,math.pow(10.0+20.0,2.0))
        f2["power"] = 3.0
        r = self.w.execute()
        self.assertEqual(r,math.pow(10.0+20.0,3.0))
    def test_02_dict_ser(self):
        ctx = self.w.add_context("calc","root")
        ctx.calc_setup() # check if we get proper context
        ctx.add_registry_source(":src_a",10.0)
        ctx.add_registry_source(":src_b",20.0)
        self.assertEqual([":src_a",":src_b"],ctx.registry_keys())
        ctx.add_filter("add","f1")
        f2 = ctx.add_filter("pow","f2")
        ctx.connect(":src_a","f1:in_a")
        ctx.connect(":src_b","f1:in_b")
        ctx.connect("f1","f2:in")
        print(self.w.to_dict())
    def test_03_dict_load(self):
        w1 = Workspace()
        w1.register_filter(CalcPower)
        w1.register_filter(CalcAdd)
        w1.add_filter("add","f1")
        w1.add_filter("pow","f2")
        w1.connect("f1","f2:in")
        w1_dict = w1.to_dict()
        print("Source Workspace")
        print(json.dumps(w1_dict,indent=1))
        w2 = Workspace()
        w2.register_filter(CalcPower)
        w2.register_filter(CalcAdd)
        w2.load_dict(w1_dict)
        w2_dict = w2.to_dict()
        print("Resulting Workspace")
        print(json.dumps(w2_dict,indent=1))
        self.assertEqual(w1_dict,w2_dict)
    def test_04_dict_init(self):
        r = {"nodes":{"f1":{"type":"add"},
                      "f2":{"type":"pow"}},
            "connections":[ {"from":"f1",
                             "to":"f2",
                             "port":"in"}]
            }
        self.w.load_dict(r)
        w_dict = self.w.to_dict()
        print("Input Dict")
        print(json.dumps(r,indent=1))
        print("Resulting Workspace")
        print(json.dumps(w_dict,indent=1))


if __name__ == '__main__':
    unittest.main()

