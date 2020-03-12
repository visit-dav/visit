# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_parser.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 8/29/2012
 description:
    unittest test cases for parser front-end.

"""

import unittest
from visit_flow import *

from decorators import ply_test

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)

class TestParser(unittest.TestCase):
    @ply_test
    def test_01_simple_expr(self):
        Parser.init()
        stmts = Parser.parse("vel_mag = sqrt(vx^2 + vy^2 + vz^2)")
        print("")
        for s in stmts:
            print(s)

if __name__ == '__main__':
    unittest.main()

