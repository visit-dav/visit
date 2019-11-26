# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_query.py
 author: Cyrus Harrison (cyrush@llnl.gov)
 created: 4/09/2010
 description:
    Unit tests for query helpers.

"""

import unittest
import os
import sys

from os.path import join as pjoin

from visit_test import *
from visit_utils import query, python_query

try:
    import visit
except:
    pass


db_dir = pjoin(os.path.dirname(__file__),"_data")
db = pjoin(db_dir,"multi_rect2d.silo")
vpq_file = pjoin(db_dir,"visit_min.vpq")


class TestQuery(unittest.TestCase):
    @visit_test
    def setUp(self):
        visit.OpenDatabase(db)
        visit.AddPlot("Pseudocolor","d")
        visit.DrawPlots()
    @visit_test
    def test_query_01_std(self):
        res_v = query("Min")
        res_s = query("Min",rmode="string").strip()
        self.assertTrue( (res_v - 0.02357020415365696) < 1e-5 )
        self.assertEqual(res_s,"d -- Min = 0.0235702 (zone 44 in domain 5 at coord <0.483333, 0.483333>)")
    @visit_test
    def test_query_02_kwargs(self):
        #add a threshold 
        visit.AddOperator("Isovolume")
        iatts = visit.IsovolumeAttributes()
        iatts.lbound = 0.4
        iatts.ubound = 1e+37
        visit.SetOperatorOptions(iatts)
        visit.DrawPlots()
        res_v = query("Min",use_actual_data=True)
        self.assertTrue( abs(res_v - 0.3778594434261322) < 1e-5 )
        res_v = query("Min",use_actual_data=False)
        self.assertTrue( abs(res_v - 0.02357020415365696) < 1e-5 )
    @visit_test
    def test_query_03_py_query(self):
        res_a = python_query(file=vpq_file,msg_lvl=4)
        res_b = python_query(source=open(vpq_file).read(),msg_lvl=4)
        res_c = python_query(file=vpq_file,rmode="string",msg_lvl=4).strip()
        print(res_a, res_b, res_c)
        self.assertTrue( (res_a - 0.02357020415365696) < 1e-5 )
        self.assertTrue( (res_b - 0.02357020415365696) < 1e-5 )
        self.assertEqual(res_c,"Min = %s" % str(res_a))
    @visit_test
    def tearDown(self):
        visit.DeleteAllPlots()
        visit.CloseDatabase(db)
        visit.CloseComputeEngine()


if __name__ == "__main__":
    unittest.main()

