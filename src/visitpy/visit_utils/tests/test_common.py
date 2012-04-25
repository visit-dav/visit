#*****************************************************************************
#
# Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

