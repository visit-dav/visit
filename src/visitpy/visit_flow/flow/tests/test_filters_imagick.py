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
 file: test_filters_imagick_ops.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/24/2012
 description:
    unittest test cases for Filters in the imagick module.

"""

import unittest
import os
from os.path import join as pjoin

from flow import *
from flow.filters import imagick, file_ops

from decorators import imagick_test

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)


class TestImagick(unittest.TestCase):
    def setUp(self):
        odir = pjoin("tests","_test_output")
        if not os.path.isdir(odir):
            os.mkdir(odir)
        print ""
    @imagick_test
    def test_01_workspace_setup(self):
        odir = pjoin("tests","_test_output")
        w = Workspace()
        w.register_filters(imagick)
        w.register_filters(file_ops)
        ctx = w.add_context("imagick","root")
        ctx.set_working_dir(odir)
        ng = ctx.add_filter("gen","gen0")
        ng["color"] = "blue"
        ng["width"] = 320
        ng["height"] = 240
        ng = ctx.add_filter("gen","gen1")
        ng["color"] = "green"
        ng["width"] = 320
        ng["height"] = 240

        nc = ctx.add_filter("crop","crop0")
        nc["width"] = 100
        nc["height"] = 200

        no = ctx.add_filter("over","over0")
        no["x_offset"] = 10
        no["y_offset"] = 20

        na = ctx.add_filter("append","append0")
        na["direction"] = "vz"

        w.connect("gen0","crop0:in")

        w.connect("crop0","over0:over")
        w.connect("gen1","over0:under")

        w.connect("over0","append0:in_a")
        w.connect("over0","append0:in_b")

        nm = ctx.add_filter("file_rename","mv0")
        nm["pattern"] = pjoin(odir,"filter.graph.result.%s.png")
        w.connect("append0","mv0:in")

        self.assertEqual(w.graph.has_node("mv0"), True)
        w.execute()

if __name__ == '__main__':
    unittest.main()

