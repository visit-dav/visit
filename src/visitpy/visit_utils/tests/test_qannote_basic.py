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

class TestBasic(unittest.TestCase):
    def setUp(self):
        txt =  Text( {"txt": "Text Overlay!",
                         "x": 100,
                         "y": 200,
                         "color": (255,255,255,255),
                         "vz":"center",
                         "hz":"center",
                         "font/name": "Times New Roman",
                         "font/size": 22})
        img = Image( {"image":pjoin(data_dir,"blue.box.png"),
                      "x": 130, "y": 180})
        arr = Arrow( {"x0": 10, "y0":10,
                      "x1":100,"y1":175,"tip_len":20})
        rect = Rect( {"x":400,"y":400,
                      "w":100,"h":200,
                      "color":(0,255,0,255)})
        box  = Rect( {"x":200,"y":200,
                      "w":100,"h":100,
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
                              "w":300,"h":200,
                              "font/size":20,
                              "txt":txt}))
        sz = bg.size()
        Canvas.render(items,sz,test_output,(-10,-10,sz[0],sz[1]))
    def tearDown(self):
        pass


if __name__ == '__main__':
    unittest.main()
