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

class TestMultiProgBar(unittest.TestCase):
    @pyside_test
    def test_01_mpb(self):
        w = 600;
        h = 1000;
        items = [ Rect({"x":0,
                        "y":0,
                        "width":w,
                        "height":h,
                        "color":(0,0,0,255)})]
        y = 10
        for v in range(0,100,5):
            mbp = MultiProgressBar({"x":10,"y":y,
                                    "width":500,
                                    "height":20,
                                    "bg_color":(20,20,20,255),
                                    "segment/ranges": [.5,
                                                       .2,
                                                       .3],
                                    "segment/labels": ["Go","Caution","Stop"],
                                    "segment/colors": [ (0,193,0,255),
                                                        (235,195,0,255),
                                                        (195,0,0,255) ],
                                    "position": v / 100.0})
            items.append(mbp)
            y += 55
        test_output = out_path("test.multi.prog.bar.00.png")
        Canvas.render(items,(w,h),test_output)
    @pyside_test
    def test_02_mpb(self):
        w = 600;
        h = 1000;
        items = [ Rect({"x":0,
                        "y":0,
                        "width":w,
                        "height":h,
                        "color":(0,0,0,255)})]
        y = 10
        for v in range(0,100,5):
            mbp = MultiProgressBar({"x":10,"y":y,
                                    "width":500,
                                    "height":20,
                                    "bg_color":(20,20,20,255),
                                    "force_labels" : True,
                                    "segment/ranges": [.5,
                                                       .2,
                                                       .3],
                                    "segment/labels": ["Go","Caution","Stop"],
                                    "segment/colors": [ (0,193,0,255),
                                                        (235,195,0,255),
                                                        (195,0,0,255) ],
                                    "position": v / 100.0})
            items.append(mbp)
            y += 55
        test_output = out_path("test.multi.prog.bar.01.png")
        Canvas.render(items,(w,h),test_output)
    @pyside_test
    def test_03_mpb(self):
        w = 600;
        h = 1000;
        items = [ Rect({"x":0,
                        "y":0,
                        "width":w,"height":h,
                        "color":(0,0,0,255)})]
        y = 10
        for v in range(0,100,5):
            mbp = MultiProgressBar({"x":100,
                                    "y":y,
                                    "width":400,
                                    "height":20,
                                    "bg_color":(20,20,20,255),
                                    "force_labels" : True,
                                    "segment/ranges": [.25,
                                                       .25,
                                                       .5],
                                    "segment/labels": ["Go","Caution","Stop"],
                                    "segment/colors": [ (0,193,0,255),
                                                        (235,195,0,255),
                                                        (195,0,0,255) ],
                                    "position": v / 100.0})
            items.append(mbp)
            y += 55
        test_output = out_path("test.multi.prog.bar.02.png")
        Canvas.render(items,(w,h),test_output)


if __name__ == '__main__':
    unittest.main()
