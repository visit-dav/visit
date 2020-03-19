# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_engine.py
 author: Cyrus Harrison (cyrush@llnl.gov)
 created: 4/09/2010
 description:
    Unit tests for engine helpers.

"""

import unittest
import os
import sys

from os.path import join as pjoin

from visit_test import *
from visit_utils.common import VisItException, hostname
from visit_utils import engine,common

try:
    import visit
except:
    pass


class TestEngine(unittest.TestCase):
    @visit_test
    def test_open(self):
        if hostname(False) in engine.supported_hosts():
            engine.open()
            self.assertEqual(1,len(visit.GetEngineList()))
            engine.close()
            self.assertEqual(0,len(visit.GetEngineList()))
        else:
            self.assertRaises(VisItException,engine.open)
    @visit_test
    def test_serial(self):
        ename = engine.open(nprocs=1,method="serial")
        self.assertEqual(1,len(visit.GetEngineList()))
        engine.close(ename)
        self.assertEqual(0,len(visit.GetEngineList()))

if __name__ == "__main__":
    unittest.main()