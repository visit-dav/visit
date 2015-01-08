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
 file: test_ult.py
 author: Cyrus Harrison (cyrush@llnl.gov)
 created: 4/18/2014
 description:
    Unit tests for curve helpers.

"""

import unittest
import os
import sys
import random

from os.path import join as pjoin

from visit_test import *
from visit_utils import ult
from visit_utils import Window, WindowManager


try:
    import visit
except:
    pass

db_dir = pjoin(os.path.split(__file__)[0],"_data")
db = pjoin(db_dir,"random.ult")
output_dir  = pjoin(os.path.split(__file__)[0],"_output")

class TestUlt(unittest.TestCase):
    def test_gen(self):
        crvs = []
        for i in xrange(10):
            crv = ult.Curve(name="random_crv_%03d" % i)
            for j in xrange(100):
                crv.add_sample(j,random.random() * i)
            crvs.append(crv)
        self.assertEqual(10,len(crvs))
        self.assertEqual(len(crvs[0]),100)
    @visit_test
    def test_plot_direct(self):
        w = Window()
        ult.plot(db)
        ofile = pjoin(output_dir,"test.ult.plot.direct.render.png")
        w.render(obase=ofile,res=[200,200])
        self.assertTrue(os.path.isfile(ofile))
        WindowManager.cleanup_windows()
        visit.DeleteAllPlots()
        visit.CloseDatabase(db)
    @visit_test
    def tearDown(self):
        visit.CloseComputeEngine()

if __name__ == "__main__":
    unittest.main()
