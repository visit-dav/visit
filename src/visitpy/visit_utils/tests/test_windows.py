# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: test_windows.py
 author: Cyrus Harrison (cyrush@llnl.gov)
 created: 7/2/2010
 description:
    Unit tests for Window instances and window management.

"""

import unittest
import os
import sys

from os.path import join as pjoin

from visit_test import *
from visit_utils import Window, WindowManager, PropertyTree
from visit_utils.common import VisItException

try:
    import visit
except:
    pass

output_dir  = pjoin(os.path.split(__file__)[0],"_output")
db_dir = pjoin(os.path.split(__file__)[0],"_data")
db = pjoin(db_dir,"multi_rect2d.silo")


class SimpleWindow(Window):
    def __init__(self):
        Window.__init__(self)
        visit.OpenDatabase(db)
        visit.AddPlot("Pseudocolor","d")
        visit.DrawPlots()
    def set_view(self):
        pass


class TestWindow(unittest.TestCase):
    def setUp(self):
        if not os.path.exists(output_dir):
            os.mkdir(output_dir)
    def cleanup_windows(self):
        WindowManager.cleanup_windows()
        visit.CloseDatabase(db)
        visit.CloseComputeEngine()
    @visit_test
    def test_bookkeeping(self):
        # note: this will only pass if you start with a single window 
        # (use visit -noconfig)
        # create three windows and verify their ids
        w1 = Window()
        self.assertEqual(w1.window_id,1)
        w2 = Window()
        self.assertEqual(w2.window_id,2)
        w3 = Window()
        self.assertEqual(w3.window_id,3)
        self.assertEqual(3,len(WindowManager.registered_windows()))
        # remove a window
        w2.remove()
        # make sure we have the proper # of windows
        self.assertEqual(2,len(WindowManager.registered_windows()))
        # make sure the window ids make sense
        rwins =  WindowManager.registered_windows()
        rwids =  [ w.window_id for w in rwins ]
        self.assertTrue(1 in rwids)
        self.assertTrue(3 in rwids)
        # create a new window, visit should give it the id '2'
        w4 = Window()
        self.assertEqual(w4.window_id,2)
        w5 = Window()
        self.assertEqual(w5.window_id,4)
        self.assertEqual(4,len(WindowManager.registered_windows()))
        w1.remove()
        w6 = Window()
        # create a new window, visit should give it the id '1'
        self.assertEqual(w6.window_id,1)
        self.assertEqual(4,len(WindowManager.registered_windows()))
        w3.remove()
        w4.remove()
        w6.remove()
        self.assertEqual(1,len(WindowManager.registered_windows()))
        w5.remove()
        # new window id should be 4
        w7 = Window()
        self.assertEqual(w7.window_id,4)
        # make sure last id is set back to 1
        wf = Window()
        w7.remove()
        self.assertEqual(wf.window_id,1)
        wf.remove()
        self.assertEqual(0,len(WindowManager.registered_windows()))
    @visit_test
    def test_creation(self):
        w = Window()
        self.assertEqual(w.window_id,1)
        w.activate()
        sw = SimpleWindow()
        self.assertEqual(sw.window_id,2)
        sw.render(obase=pjoin(output_dir,"test.window.render.0"),res=[200,200])
        self.assertTrue(os.path.isfile(pjoin(output_dir,"test.window.render.0.png")))
        sw.export("VTK",obase=pjoin(output_dir,"test.window.export.0"))
        self.assertTrue(os.path.isfile(pjoin(output_dir,"test.window.export.0.visit")))
        self.cleanup_windows()
    @visit_test
    def test_render_resize(self):
        # NOTE -- VIEWER IS CRASHING HERE, NOT SURE WHY:
        sw1 = SimpleWindow()
        sw1.render(obase=pjoin(output_dir,"test.window.render.1"),res=[50,50],ores=[200,200])
        self.assertTrue(os.path.isfile(pjoin(output_dir,"test.window.render.1.png")))
        sw2 = SimpleWindow()
        sw2.render(obase=pjoin(output_dir,"test.window.render.2"),res=[800,800],ores=[200,200])
        self.assertTrue(os.path.isfile(pjoin(output_dir,"test.window.render.2.png")))
        self.cleanup_windows()
    @visit_test
    def test_bad_file_save(self):
        sw = SimpleWindow()
        self.assertRaises(VisItException,sw.render,"/bad/path/to/a/file",[200,200])
        self.cleanup_windows()

if __name__ == "__main__":
    unittest.main()

