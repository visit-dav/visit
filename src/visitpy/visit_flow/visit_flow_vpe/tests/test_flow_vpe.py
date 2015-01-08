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
        print ""
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

