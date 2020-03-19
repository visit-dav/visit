# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_status.py
 author: Cyrus Harrison (cyrush@llnl.gov)
 created: 8/4/2011
 description:
    Unit tests for status functionality.

"""

import unittest
import os
import sys

from os.path import join as pjoin
from visit_utils import common, status

output_dir  = pjoin(os.path.split(__file__)[0],"_output")
data_dir    = pjoin(os.path.split(__file__)[0],"_data")

class TestLog(unittest.TestCase):
    def setUp(self):
        if not os.path.exists(output_dir):
            os.mkdir(output_dir)
    def test_log_0(self):
        lfile = pjoin(output_dir,"log.out")
        status.open_file(lfile)
        self.assertTrue(os.path.isfile(lfile))
        print("")
        status.info("Message 1 l0\nMessage 1 l1\nMessage 1 l3")
        status.info("Message 2",center=True)
        status.info("Message 3",lvl=0)
        status.info("Message 4",ident="    ")


if __name__ == '__main__':
    unittest.main()


