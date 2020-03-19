# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

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
        props = list(p.properties().keys())
        self.assertEqual(props,['deeper/test/path', 'there/now','here'])
    def test_09_prop_update(self):
        p = PropertyTree()
        p.here = 3.0
        p.there.now = 4.0
        p.deeper.test.path = 5.0
        props = list(p.properties().keys())
        p2 = PropertyTree()
        p2.value = True
        p2.last  = False
        p2.update(p)
        print(p2)
        props = list(p2.properties().keys())
        self.assertEqual(props,['deeper/test/path', 'there/now','last','value','here'])
        self.assertEqual(p2.value,True)
        self.assertEqual(p2.last,False)
        self.assertEqual(p2.here,3.0)
        self.assertEqual(p2.there.now,4.0)
        self.assertEqual(p2.deeper.test.path,5.0)

if __name__ == '__main__':
    unittest.main()

