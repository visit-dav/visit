# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

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
        for i in range(10):
            crv = ult.Curve(name="random_crv_%03d" % i)
            for j in range(100):
                crv.add_sample(j,random.random() * i)
            crvs.append(crv)
        self.assertEqual(10,len(crvs))
        self.assertEqual(len(crvs[0]),100)
    @visit_test
    def test_plot_direct(self):
        w = Window()
        ult.plot(db)
        obase = pjoin(output_dir,"test.ult.plot.direct.render")
        w.render(obase=obase,res=[200,200])
        self.assertTrue(os.path.isfile(obase + ".png"))
        WindowManager.cleanup_windows()
        visit.DeleteAllPlots()
        visit.CloseDatabase(db)
    @visit_test
    def tearDown(self):
        visit.CloseComputeEngine()

if __name__ == "__main__":
    unittest.main()
