##*****************************************************************************
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
    import PySide.QtCore
except:
    pass


output_dir  = pjoin(os.path.split(__file__)[0],"_output")
data_dir    = pjoin(os.path.split(__file__)[0],"_data")

def out_path(fname):
    odir = pjoin(output_dir,"qannote")
    if not os.path.isdir(odir):
        os.mkdir(odir)
    return pjoin(odir,fname)

def test_txt_items(x,y,wrap,hz,vz="top",txt=None,fsize=22):
    rect = Rect( {"x":x,"y":y,
                  "w":wrap,"h":200,
                   "color":(0,255,0,255)})
    rect2 = Rect( {"x":x-5,"y":y-5,
                   "w":10,"h":10,
                   "color":(255,255,0,255)})
    if txt is None:
        txt = "Testing word wrap capability with a long sentence."
    txt  = Text( {"txt": txt,
                  "x": x,
                  "y": y,
                  "color": (255,255,255,255),
                  "vz":vz,
                  "hz":hz,
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
