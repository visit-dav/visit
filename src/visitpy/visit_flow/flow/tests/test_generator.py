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

from flow import *
from flow.filters import npy_ops

from decorators import numpy_test
# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)

class TestParser(unittest.TestCase):
    def test_01_simple_expr_gen(self):
        filters = Generator.parse_network("vel_mag = sqrt(vx^2 + vy^2 + vz^2)")
        print ""
        for f in filters:
            print f
        self.assertTrue(True)
    def test_02_simple_expr_gen_context(self):
        w = Workspace()
        w.register_filters(npy_ops)
        v_a = npy.array(range(10),dtype=npy.double)
        v_b = npy.array(range(10),dtype=npy.double)
        c_2 = 2.0
        w.registry_add(":v_a",v_a)
        w.registry_add(":v_b",v_b)
        w.registry_add(":c_2",c_2)
        print ""
        expr = "res = (v_a + v_b)^c_2 + (v_a  - v_b)^c_2"
        print "test_expr: " + expr
        filters = Generator.parse_network(expr,w)
        print ""
        print w.graph
        print w.execution_plan()
        act_res = w.execute()
        # get output and test
        test_res = npy.power((v_a + v_b),2.0)+ npy.power((v_a - v_b),2.0)
        dsum = npy.sum(act_res - test_res)
        print "Filter Graph Result: %s" % str(act_res)
        print "Test Result:         %s" % str(test_res)
        print "Difference:          %s" % str(dsum)
        self.assertTrue(dsum < 1e-6)
    def test_03_simple_expr_gen_from_workspace(self):
        w = Workspace()
        w.register_filters(npy_ops)
        v_a = npy.array(range(10),dtype=npy.double)
        v_b = npy.array(range(10),dtype=npy.double)
        w.registry_add(":v_a",v_a)
        w.registry_add(":v_b",v_b)
        print ""
        expr = "res = (v_a + v_b)^2 + (v_a  - v_b)^2"
        print "test_expr: " + expr
        w.setup_expression_network(expr)
        print ""
        print w.graph
        print w.execution_plan()
        act_res = w.execute()
        # get output and test
        test_res = npy.power((v_a + v_b),2.0)+ npy.power((v_a - v_b),2.0)
        dsum = npy.sum(act_res - test_res)
        print "Filter Graph Result: %s" % str(act_res)
        print "Test Result:         %s" % str(test_res)
        print "Difference:          %s" % str(dsum)
        self.assertTrue(dsum < 1e-6)
    def test_04_simple_decomp_gen_context(self):
        w = Workspace()
        w.register_filters(npy_ops)
        v_a = npy.array(range(10),dtype=npy.double)
        v_b = npy.ndarray((10,2),dtype=npy.double)
        c_2 = 2.0
        w.registry_add(":v_a",v_a)
        w.registry_add(":v_b",v_b)
        w.registry_add(":c_2",c_2)
        v_b[:,0] = range(10)
        v_b[:,1] = range(10,20)
        expr = "res = (v_a + v_b[0])^c_2 + (v_a  - v_b[1])^c_2"
        print "test_expr: " + expr
        w.setup_expression_network(expr)
        print ""
        print w.graph
        print w.execution_plan()
        act_res = w.execute()
        test_res = npy.power((v_a + v_b[:,0]),2.0)+ npy.power((v_a - v_b[:,1]),2.0)
        dsum = npy.sum(act_res - test_res)
        print "Filter Graph Result: %s" % str(act_res)
        print "Test Result:         %s" % str(test_res)
        print "Difference:          %s" % str(dsum)
        self.assertTrue(dsum < 1e-6)

if __name__ == '__main__':
    unittest.main()

