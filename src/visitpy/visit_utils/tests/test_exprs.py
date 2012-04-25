#*****************************************************************************
#
# Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
 file: test_exprs.py
 author: Cyrus Harrison (cyrush@llnl.gov)
 created: 4/09/2010
 description:
    Unit tests for expression helpers.

"""

import unittest
import os
import sys

from os.path import join as pjoin

from visit_test import *
from visit_utils import exprs

try:
    import visit
except:
    pass

db_dir = pjoin(os.path.split(__file__)[0],"_data")
db = pjoin(db_dir,"multi_rect2d.silo")
vpe_file = pjoin(db_dir,"visit_sq.vpe")


class TestExprs(unittest.TestCase):
    @visit_test
    def test_clear(self):
        exprs.clear()
        self.assertEqual(0,len(visit.Expressions()))
    @visit_test
    def test_define(self):
        exprs.clear()
        exprs.define("test_expr","a + b")
        vexprs = visit.Expressions()
        self.assertEqual(1,len(vexprs))
        self.assertEqual("test_expr",vexprs [0][0])
        self.assertEqual("a + b",vexprs [0][1])
        exprs.define("test_expr","b*b")
        vexprs = visit.Expressions()
        self.assertEqual("test_expr",vexprs [0][0])
        self.assertEqual("b*b",vexprs [0][1])
    @visit_test
    def test_python(self):
        visit.OpenDatabase(db)
        visit.AddPlot("Pseudocolor","d")
        visit.DrawPlots()
        exprs.clear()
        exprs.define_python("test_vpe",file=vpe_file,args=["d"])
        visit.Query("Max")
        r1 =  visit.GetQueryOutputValue()
        visit.ChangeActivePlotsVar("test_vpe")
        visit.Query("Max")
        r2 = visit.GetQueryOutputValue()
        visit.ChangeActivePlotsVar("d")
        self.assertTrue(r1 *r1 - r2 < 1e-2)
        exprs.clear()
        visit.DeleteAllPlots()
        visit.CloseDatabase(db)
    @visit_test
    def test_exists(self):
        exprs.clear()
        exprs.define("test_expr","a + b")
        self.assertTrue(exprs.exists("test_expr"))
    @visit_test
    def test_delete(self):
        exprs.clear()
        exprs.define("test_expr","a + b")
        self.assertTrue(exprs.exists("test_expr"))
        exprs.delete("test_expr")
        self.assertFalse(exprs.exists("test_expr"))
    @visit_test
    def test_echo(self):
        exprs.clear()
        exprs.define("test_expr","a + b",echo=True)
        self.assertTrue(exprs.exists("test_expr"))
    @visit_test
    def tearDown(self):
        visit.CloseComputeEngine()

if __name__ == "__main__":
    unittest.main()