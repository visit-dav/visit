# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_state_control.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/24/2012
 description:
    unittest test cases for state control.

"""

import unittest
from visit_flow import *

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)


class TestStateControl(unittest.TestCase):
    def setUp(self):
        print("")
    def test_01_full_walk(self):
        space = StateSpace({"a":2,"b":3})
        sgen = StateVectorGenerator(space)
        self.assertEqual(len(sgen),6)
        res = [str(svec) for svec in sgen]
        res_exp = "['[0]:[0, 0]', '[1]:[1, 0]', '[2]:[0, 1]', '[3]:[1, 1]', '[4]:[0, 2]', '[5]:[1, 2]']"
        self.assertEqual(str(res),res_exp)
    def test_02_padded_idx(self):
        space = StateSpace({"a":2,"b":3})
        sgen = StateVectorGenerator(space,"padded_index")
        self.assertEqual(len(sgen),3)
        res = [str(svec) for svec in sgen]
        res_exp = "['[0]:[0, 0]', '[1]:[1, 1]', '[2]:[1, 2]']"
        self.assertEqual(str(res),res_exp)
    def test_03_new_full_walk(self):
        space = StateSpace({"a":3,"b":2,"c":3})
        sgen = StateVectorGenerator(space)
        for svec in sgen:
            print(svec)

if __name__ == '__main__':
    unittest.main()



