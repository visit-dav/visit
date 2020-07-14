# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

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

from visit_flow import *
from visit_flow.filters import imagick, file_ops

from decorators import imagick_test

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)


class TestImagick(unittest.TestCase):
    def setUp(self):
        odir = pjoin("tests","_test_output")
        if not os.path.isdir(odir):
            os.mkdir(odir)
        print("")
    @imagick_test
    def test_01_workspace_setup(self):
        odir = pjoin("tests","_test_output")
        w = Workspace()
        w.register_filters(imagick)
        w.register_filters(file_ops)
        ctx = w.add_context("imagick","root")
        ctx.set_working_dir(odir)
        ng = ctx.add_filter("fill","gen0")
        ng["color"] = "blue"
        ng["width"] = 320
        ng["height"] = 240
        ng = ctx.add_filter("fill","gen1")
        ng["color"] = "green"
        ng["width"] = 320
        ng["height"] = 240

        nc = ctx.add_filter("crop","crop0")
        nc["width"] = 100
        nc["height"] = 200

        no = ctx.add_filter("over","over0")
        no["x"] = 10
        no["y"] = 20

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

