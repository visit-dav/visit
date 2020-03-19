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

def test_txt_items(x,y,wrap,hz,vz="top",txt=None,fsize=22):
    rect = Rect( {"x":x,"y":y,
                  "width":wrap,"height":200,
                   "color":(0,255,0,255)})
    rect2 = Rect( {"x":x-5,"y":y-5,
                   "width":10,"height":10,
                   "color":(255,255,0,255)})
    if txt is None:
        txt = "Testing word wrap capability with a long sentence."
    txt  = Text( {"text": txt,
                  "x": x,
                  "y": y,
                  "color": (255,255,255,255),
                  "vert_align":vz,
                  "horz_align":hz,
                  "wrap":wrap,
                  "font/name": "Times New Roman",
                  "font/bold": True,
                  "font/size": fsize})
    return [rect,rect2,txt]

class TestWrap(unittest.TestCase):
    def setUp(self):
        pass
    @pyside_test
    def test_000_wrap_left(self):
        ofile = out_path("test.wrap.000.png")
        Canvas.render(test_txt_items(400,200,100,"left"),(600,600),ofile)
        self.assertTrue(os.path.isfile(ofile))
    @pyside_test
    def test_001_wrap_right(self):
        ofile = out_path("test.wrap.001.png")
        Canvas.render(test_txt_items(400,200,100,"right"),(600,600),ofile)
        self.assertTrue(os.path.isfile(ofile))
    @pyside_test
    def test_002_wrap_center(self):
        ofile = out_path("test.wrap.002.png")
        Canvas.render(test_txt_items(400,200,100,"center"),(600,600),ofile)
        self.assertTrue(os.path.isfile(ofile))
    @pyside_test
    def test_003_wrap_vz_center(self):
        ofile = out_path("test.wrap.003.png")
        Canvas.render(test_txt_items(400,200,100,"left","center"),(600,600),ofile)
        self.assertTrue(os.path.isfile(ofile))
    @pyside_test
    def test_004_wrap_vz_bottom(self):
        ofile = out_path("test.wrap.004.png")
        Canvas.render(test_txt_items(400,200,100,"left","bottom"),(600,600),ofile)
        self.assertTrue(os.path.isfile(ofile))
    @pyside_test
    def test_005_wrap_left(self):
        ofile = out_path("test.wrap.005.png")
        Canvas.render(test_txt_items(400,200,100,"left",fsize=12),(600,600),ofile)
        self.assertTrue(os.path.isfile(ofile))
    @pyside_test
    def test_006_wrap_right(self):
        ofile = out_path("test.wrap.006.png")
        Canvas.render(test_txt_items(400,200,100,"right",fsize=12),(600,600),ofile)
        self.assertTrue(os.path.isfile(ofile))
    @pyside_test
    def test_007_wrap_center(self):
        ofile = out_path("test.wrap.007.png")
        Canvas.render(test_txt_items(400,200,100,"center",fsize=12),(600,600),ofile)
        self.assertTrue(os.path.isfile(ofile))
    @pyside_test
    def test_008_wrap_vz_center(self):
        ofile = out_path("test.wrap.008.png")
        Canvas.render(test_txt_items(400,200,100,"left","center",fsize=12),(600,600),ofile)
        self.assertTrue(os.path.isfile(ofile))
    @pyside_test
    def test_009_wrap_vz_bottom(self):
        ofile = out_path("test.wrap.009.png")
        Canvas.render(test_txt_items(400,200,100,"left","bottom",fsize=12),(600,600),ofile)
        self.assertTrue(os.path.isfile(ofile))
    @pyside_test
    def test_010_wrap_w_nlines(self):
        txt = "Testing word wrap capability with a long sentence.\nWith some new lines for good measure.\nFinal."
        ofile = out_path("test.wrap.010.png")
        Canvas.render(test_txt_items(200,200,200,"left","top",txt=txt,fsize=12),(600,600),ofile)
        self.assertTrue(os.path.isfile(ofile))
    def tearDown(self):
        pass


if __name__ == '__main__':
    unittest.main()
