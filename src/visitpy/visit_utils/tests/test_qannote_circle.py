# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 author: Cyrus Harrison (cyrush@llnl.gov)
 description:
      Tests for qannote module.

"""


import unittest
import os
from os.path import join as pjoin

from visit_test import *
from visit_utils.qannote import *

try:
    import PySide2.QtCore
except:
    pass


output_dir  = pjoin(os.path.split(__file__)[0],"_output")
data_dir    = pjoin(os.path.split(__file__)[0],"_data")

def out_path(fname):
    if not os.path.isdir(output_dir):
        os.mkdir(output_dir)
    odir = pjoin(output_dir,"qannote")
    if not os.path.isdir(odir):
        os.mkdir(odir)
    return pjoin(odir,fname)

class TestCircle(unittest.TestCase):
    @pyside_test
    def setUp(self):
        bg = Rect({"x":0,"y":0,
                   "width":500,"height":250,
                   "color":(30,30,30,255)})

        hz = Line({"x0":0,"x1":500,
                   "y0":125,"y1":125,
                   "width":2,"color":(0,255,0,255)})

        vz = Line({"x0":250,"x1":250,
                   "y0":0,"y1":250,
                   "width":2,"color":(0,255,0,255)})

        circ = Circle({"x":250,
                       "y":125,
                       "radius":125,
                       "outline":True,
                       "color":(255,0,0,255)})
        self.items = [bg,hz,vz,circ]
    @pyside_test
    def test_01_circle(self):
        test_output = out_path("test.circle.00.png")
        Canvas.render(self.items,(500,250),test_output)
    def tearDown(self):
        pass


if __name__ == '__main__':
    unittest.main()
