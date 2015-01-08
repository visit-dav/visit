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
 file: test_state_control.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/24/2012
 description:
    unittest test cases for state control.

"""

import unittest
from flow import *

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)


class TestStateControl(unittest.TestCase):
    def setUp(self):
        print ""
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
            print svec

if __name__ == '__main__':
    unittest.main()



