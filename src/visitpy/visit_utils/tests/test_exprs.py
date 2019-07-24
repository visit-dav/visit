# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

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