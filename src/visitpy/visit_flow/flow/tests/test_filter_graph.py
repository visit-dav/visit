# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_filter_graph.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/24/2012
 description:
    unittest test cases for filter graph setup.

"""

import unittest
from visit_flow import *

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)

class FSource(Filter):
    filter_type    = "src"
    input_ports    = []
    default_params = {"param":0}
    output_port    = True

class FA(Filter):
    filter_type    = "a"
    input_ports    = ["input"]
    default_params = {"param":0}
    output_port    = True

class FB(Filter):
    filter_type    = "b"
    input_ports    = ["input"]
    default_params = {"param":0}
    output_port    = True

class FC(Filter):
    filter_type    = "c"
    input_ports    = ["input"]
    default_params = {"param":0}
    output_port    = True

class FD(Filter):
    filter_type    = "d"
    input_ports    = ["input_a","input_b"]
    default_params = {"param":0}
    output_port    = True

class FSink(Filter):
    filter_type    = "snk"
    input_ports    = ["input"]
    default_params = {"param":0}
    output_port    = False


class TestFilterGraph(unittest.TestCase):
    def setUp(self):
        g = FilterGraph()
        g.register_filter(FSource)
        g.register_filter(FA)
        g.register_filter(FB)
        g.register_filter(FC)
        g.register_filter(FD)
        g.register_filter(FSink)
        self.g = g
        print("")
    def test_01_simple_setup(self):
        n = self.g.add_node("src","database")
        self.assertEqual(n.name,"database")
        self.assertEqual(self.g.has_node("database"),True)
        self.assertEqual(len(list(self.g.nodes.values())),1)
        self.g.remove_node("database")
        self.assertEqual(self.g.has_node("database"),False)
        self.assertEqual(len(list(self.g.nodes.values())),0)
        self.assertRaises(UnknownFilterNodeError, self.g.remove_node, "src")
    def test_02_simple_traversal(self):
        self.g.add_node("src","database")
        self.g.add_node("a","a0")
        self.g.add_node("b","b0")
        self.g.add_node("c","c0")
        self.g.add_node("d","d0")
        self.g.add_node("snk","fileout")
        self.g.add_node("snk","!dangle!")
        # "database:bundle/path","a0:input"
        self.g.connect("database","a0","input")
        self.g.connect("a0","b0","input")
        self.g.connect("a0","c0","input")
        self.g.connect("b0","d0","input_a")
        self.g.connect("c0","d0","input_b")
        self.g.connect("d0","fileout","input")
        eplan = ExecutionPlan(self.g)
        print(eplan)
        traversals = [[('database', 1), ('a0', 2), ('c0', 1), ('b0', 1), ('d0', 1), ('fileout', 1)]]
        #untouched = ["!dangle!"]
        self.assertEqual(traversals,eplan.traversals)
        #self.assertEqual(untouched,eplan.untouched)
    def test_03_multi_out(self):
        self.g.add_node("src","db")
        self.g.add_node("a","a0")
        self.g.add_node("d","d0")
        self.g.add_node("snk","out0")
        # "database:bundle/path","a0:input"
        self.g.connect("db","a0","input")
        self.g.connect("a0","d0","input_a")
        self.g.connect("db","d0","input_b")
        self.g.connect("d0","out0","input")
        eplan = ExecutionPlan(self.g)
        print(eplan.traversals)
        #self.assertEqual(traversals,eplan.traversals)
        #self.assertEqual(untouched,eplan.untouched)

if __name__ == '__main__':
    unittest.main()

