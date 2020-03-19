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

class TestBasic(unittest.TestCase):
    def setUp(self):
        txt =  Text( {"text": "Text Overlay!",
                         "x": 100,
                         "y": 200,
                         "color": (255,255,255,255),
                         "vert_align":"center",
                         "horz_align":"center",
                         "font/name": "Times New Roman",
                         "font/size": 22})
        img = Image( {"image":pjoin(data_dir,"blue.box.png"),
                      "x": 130, "y": 180})
        arr = Arrow( {"x0": 10, "y0":10,
                      "x1":100,"y1":175,"tip_len":20})
        rect = Rect( {"x":400,"y":400,
                      "width":100,"height":200,
                      "color":(0,255,0,255)})
        box  = Rect( {"x":200,"y":200,
                      "width":100,"height":100,
                       "color":(0,255,0,255),"outline":True})
        self.items = [img,txt,arr,rect,box]
    @pyside_test
    def test_00_basic(self):
        test_output = out_path("test.basic.00.png")
        Canvas.render(self.items,(600,600),test_output)
    @pyside_test
    def test_01_basic(self):
        test_output = out_path("test.basic.01.png")
        bg = Image( {"image":pjoin(data_dir,"black.bg.png")})
        items = [bg]
        items.extend(self.items)
        Canvas.render(items,bg.size(),test_output)
    @pyside_test
    def test_02_view(self):
        test_output = out_path("test.basic.02.png")
        bg = Image( {"image":pjoin(data_dir,"black.bg.png"),
                         "x":-10,"y":-10})
        items = [bg]
        items.extend(self.items)
        sz = bg.size()
        Canvas.render(items,sz,test_output,(-10,-10,sz[0],sz[1]))
    @pyside_test
    def test_03_textbox(self):
        test_output = out_path("test.basic.03.png")
        bg = Image( {"image":pjoin(data_dir,"black.bg.png"),
                         "x":-10,"y":-10})
        items = [bg]
        txt = "Testing text box with wrap capability with a long sentence.\nWith some new lines for good measure.\nFinal."
        items.append(TextBox({"x":200,"y":200,
                              "width":300,"height":200,
                              "font/size":20,
                              "text":txt}))
        sz = bg.size()
        Canvas.render(items,sz,test_output,(-10,-10,sz[0],sz[1]))
    def tearDown(self):
        pass


if __name__ == '__main__':
    unittest.main()
