#*****************************************************************************
#
# Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#*****************************************************************************
"""
 file: test_filter_graph.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/24/2012
 description:
    unittest test cases for filter graph setup.

"""

import unittest
from flow import *

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
        print ""
    def test_01_simple_setup(self):
        n = self.g.add_node("src","database")
        self.assertEqual(n.name,"database")
        self.assertEqual(self.g.has_node("database"),True)
        self.assertEqual(len(self.g.nodes.values()),1)
        self.g.remove_node("database")
        self.assertEqual(self.g.has_node("database"),False)
        self.assertEqual(len(self.g.nodes.values()),0)
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
        print eplan
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
        print eplan.traversals
        #self.assertEqual(traversals,eplan.traversals)
        #self.assertEqual(untouched,eplan.untouched)

if __name__ == '__main__':
    unittest.main()

