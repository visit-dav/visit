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
 file: test_property_tree.py
 author: Cyrus Harrison (cyrush@llnl.gov)
 created: 4/15/2010
 description:
    Unit tests for the PropertyTree class.

"""

import unittest
import pickle
from visit_utils import PropertyTree

class TestPropertyTree(unittest.TestCase):
    def test_01_simple_access(self):
        p = PropertyTree()
        p.add_property("testing/a/number",0.0)
        p.add_property("testing/a/boolean",False)
        self.assertEqual(p["testing/a/number"], 0.0)
        p["testing/a/number"] = 10.0
        self.assertEqual(p["testing/a/number"], 10.0)
        self.assertEqual(p.testing.a.number, 10.0)
        self.assertEqual(p.testing.a["number"], 10.0)

    def test_02_simple_init(self):
        p = PropertyTree(init={"path/to/val_0":0,
                               "path/to/val_1":1.0})
        self.assertEqual(p["path/to/val_1"],1.0)
        p["path/to/val_1"] = 1000.0
        self.assertEqual(p["path/to/val_0"],0)
        self.assertEqual(p["path/to/val_1"],1000.0)

    def test_03_add_remove_property(self):
        p = PropertyTree()
        self.assert_(p.has_property("testing/string_value") == False)
        p.add_property("testing/string_value",None)
        p.add_property("testing/float_value",0.0)
        p["testing/float_value"] = 10.0
        sval = "here is a string ..."
        p["testing/string_value"] = sval
        self.assertEqual(p["testing/string_value"], sval)
        p.remove_property("testing/string_value")
        self.assert_(p.has_property("testing/string_value") == False)

    def test_05_freeform_add(self):
        p = PropertyTree()
        p.here = 3.0
        p.there.now = 4.0
        p.deeper.test.path = 5.0
        self.assertEqual(p.here,3.0)
        self.assertEqual(p.there.now,4.0)
        self.assertEqual(p.deeper.test.path,5.0)

    def test_06_locking(self):
        p = PropertyTree()
        p.here = 3.0
        p.there.now = 4.0
        p.deeper.test.path = 5.0
        p.lock()
        self.assertEqual(p.here,3.0)
        self.assertEqual(p.there.now,4.0)
        self.assertEqual(p.deeper.test.path,5.0)
        self.assertRaises(AttributeError,p.__getitem__,"bad")
        self.assertRaises(AttributeError,p.__getitem__,"bad/deeper/path")
        self.assertRaises(AttributeError,p.__getattr__,"bad")
        self.assertRaises(AttributeError,p.there.__getattr__,"bad")
        self.assertEqual(p.deeper.test._locked,True)
    def test_07_pickle(self):
        p = PropertyTree()
        p.here = 3.0
        p.there.now = 4.0
        p.deeper.test.path = 5.0
        pstr = pickle.dumps(p)
        pres = pickle.loads(pstr)
        self.assertEqual(pres.here,3.0)
        self.assertEqual(pres.there.now,4.0)
        self.assertEqual(pres.deeper.test.path ,5.0)
    def test_08_prop_list(self):
        p = PropertyTree()
        p.here = 3.0
        p.there.now = 4.0
        p.deeper.test.path  = 5.0
        props = p.properties().keys()
        self.assertEqual(props,['deeper/test/path', 'there/now','here'])
    def test_09_prop_update(self):
        p = PropertyTree()
        p.here = 3.0
        p.there.now = 4.0
        p.deeper.test.path = 5.0
        props = p.properties().keys()
        p2 = PropertyTree()
        p2.value = True
        p2.last  = False
        p2.update(p)
        print p2
        props = p2.properties().keys()
        self.assertEqual(props,['deeper/test/path', 'there/now','last','value','here'])
        self.assertEqual(p2.value,True)
        self.assertEqual(p2.last,False)
        self.assertEqual(p2.here,3.0)
        self.assertEqual(p2.there.now,4.0)
        self.assertEqual(p2.deeper.test.path,5.0)

if __name__ == '__main__':
    unittest.main()

