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

class TestMultiProgBar(unittest.TestCase):
    @pyside_test
    def test_01_mpb(self):
        w = 600;
        h = 1000;
        items = [ Rect({"x":0,"y":0,
                        "w":w,"h":h,
                        "color":(0,0,0,255)})]
        y = 10
        for v in range(0,600,30):
            mbp = MultiProgressBar({"x":10,"y":y,
                                    "w":500,"h":20,
                                    "bgcolor":(20,20,20,255),
                                    "segment/ranges": [250,150,100],
                                    "segment/labels": ["Go","Caution","Stop"],
                                    "segment/colors": [ (0,193,0,255),
                                                        (235,195,0,255),
                                                        (195,0,0,255) ],
                                    "position": v})
            items.append(mbp)
            y += 55
        test_output = out_path("test.multi.prog.bar.00.png")
        Canvas.render(items,(w,h),test_output)
    @pyside_test
    def test_02_mpb(self):
        w = 600;
        h = 1000;
        items = [ Rect({"x":0,"y":0,
                        "w":w,"h":h,
                        "color":(0,0,0,255)})]
        y = 10
        for v in range(0,600,30):
            mbp = MultiProgressBar({"x":10,"y":y,
                                    "w":500,"h":20,
                                    "bgcolor":(20,20,20,255),
                                    "force_labels" : True,
                                    "segment/ranges": [250,150,100],
                                    "segment/labels": ["Go","Caution","Stop"],
                                    "segment/colors": [ (0,193,0,255),
                                                        (235,195,0,255),
                                                        (195,0,0,255) ],
                                    "position": v})
            items.append(mbp)
            y += 55
        test_output = out_path("test.multi.prog.bar.01.png")
        Canvas.render(items,(w,h),test_output)


if __name__ == '__main__':
    unittest.main()
