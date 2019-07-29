# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_common.py
 author: Cyrus Harrison (cyrush@llnl.gov)
 created: 4/9/2010
 description:
    Unit tests for common helpers.

"""

import unittest
import os
import sys

from os.path import join as pjoin

from visit_utils import common

output_dir  = pjoin(os.path.split(__file__)[0],"_output")
data_dir    = pjoin(os.path.split(__file__)[0],"_data")

class TestCommon(unittest.TestCase):
    def setUp(self):
        if not os.path.exists(output_dir):
            os.mkdir(output_dir)
    def test_sexe(self):
        ofile = pjoin(output_dir,"_sexe_test")
        if os.path.exists(ofile):
            os.remove(ofile)
        common.sexe("touch %s" % ofile)
        self.assertTrue(os.path.isfile(ofile))
    def test_hostname(self):
        res = common.hostname()
        self.assertTrue(len(res) > 0)
    def test_lsearch(self):
        res = common.lsearch(dir(),"self")
        self.assertTrue(len(res) > 0)
    def test_define_module(self):
        mymodule = common.define_module("mymodule","x=10.0")
        self.assertEqual(mymodule.x,10.0)
    def test_define_global_module(self):
        common.define_module("mymodule","x=10.0",globals())
        self.assertEqual(mymodule.x,10.0)
    def test_load_params(self):
        pfile = pjoin(data_dir,"params.test.in")
        p = common.load_params(pfile)
        self.assertEqual(p.tree.x,10.0)
    def test_load_params_root(self):
        pfile = pjoin(data_dir,"params.test.in")
        p = common.load_params(pfile)
        root = p.root
        self.assertTrue(root.has_property("tree"))
        self.assertEqual(root.tree.x,10.0)


if __name__ == '__main__':
    unittest.main()

