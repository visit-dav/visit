# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

###############################################################################
# file: pyside_support.py
# Purpose: Shapes pyside support for the visit module api.
#
# Programmer: Cyrus Harrison & Harinarayan Krishnan
# Creation: Tue May  8 08:45:23 PDT 2012
#
#
# Modifications:
#
#
###############################################################################

import sys
import os
from threading import Event, Thread

using_pyside = False

try:
    from PySide2.QtWidgets import QApplication
    import pyside_hook
    import pyside_gui
    using_pyside = True
except ImportError:
    pass

__all__ = ["LaunchPyViewer","SetupTimer","GetRenderWindow","GetRenderWindowIds","GetUIWindow","GetPlotWindow","GetOperatorWindow", "GetOtherWindow", "GetOtherWindowNames", "GetTimeSliderWindow", "GetSourceManagerWindow", "GetPlotWindows", "GetOperatorWindows", "GetOtherWindows"]
__pyside_viewer_instance__ = None

# this is a function that polls for keyboard input,
# when it sees one it quits the
class ProcessCLIInput(Thread):
    instance = None
    def __init__(self, interval):
        # make sure we actually have a qapp
        if QApplication.instance() is None:
            QApplication(sys.argv)
        Thread.__init__(self)
        self.interval = interval
        self.event = Event()
        self.qtapp = QApplication.instance()
    def run(self):
        while not self.event.is_set():
            try:
                self.event.wait(self.interval)
                if not self.event.is_set():
                    if os.name == 'posix' or os.name == 'mac' :
                        import select
                        try:
                            i,o,e = select.select([sys.stdin],[],[],0.0001)
                            for s in i:
                                if s == sys.stdin: self.qtapp.exit(0)
                        except:
                            if sys.stdin.closed : break
                            pass
                    else:
                        import msvcrt
                        if msvcrt.kbhit(): self.qtapp.exit(0)
            except:
                if sys.stdin.closed : break
                pass

def GetPySideViewerInstance():
    global __pyside_viewer_instance__
    if __pyside_viewer_instance__ is None:
        __pyside_viewer_instance__ = pyside_gui.PySideGUI.instance()
    return __pyside_viewer_instance__

def SetupTimer():
    if using_pyside:
        if ProcessCLIInput.instance is None:
            ProcessCLIInput.instance = ProcessCLIInput(0.001)
            ProcessCLIInput.instance.start()

def LaunchPyViewer(args):
    global __pyside_viewer_instance__

    SetupTimer()
    pyside_hook.SetHook()

    if args is None: 
        args = sys.argv
        args.append("-pyuiembedded")

    if __pyside_viewer_instance__ is None: 
        __pyside_viewer_instance__ = pyside_gui.PySideGUI.instance(args)

    return __pyside_viewer_instance__


def IsPySideViewerEnabled():
    res = False
    if using_pyside:
        inst = GetPySideViewerInstance()
        if not inst is None:
            res = True
    return res

def GetRenderWindow(i):
    if using_pyside:
        # this will return None, unless properly inited
        inst = GetPySideViewerInstance()
        if not inst is None:
            return inst.GetRenderWindow(i)
    return None

def GetRenderWindowIds():
    if using_pyside:
        # this will return None, unless properly inited
        inst = GetPySideViewerInstance()
        if not inst is None:
            return inst.GetRenderWindowIDs()
    return None

def GetUIWindow():
    if using_pyside:
        # this will return None, unless properly inited
        inst = GetPySideViewerInstance()
        if not inst is None:
            return inst.GetUIWindow()
    return None

def GetPlotWindow(name):
    if using_pyside:
        # this will return None, unless properly inited
        inst = GetPySideViewerInstance()
        if not inst is None:
            return inst.GetPlotWindow(name)
    return None

def GetOperatorWindow(name):
    if using_pyside:
        # this will return None, unless properly inited
        inst = GetPySideViewerInstance()
        if not inst is None:
            return inst.GetOperatorWindow(name)
    return None

def GetOtherWindow(name):
    if using_pyside:
        # this will return None, unless properly inited
        inst = GetPySideViewerInstance()
        if not inst is None:
            return inst.GetOtherWindow(name)
    return None

def GetOtherWindowNames():
    if using_pyside:
        # this will return None, unless properly inited
        inst = GetPySideViewerInstance()
        if not inst is None:
            return inst.GetOtherWindowNames()
    return None

def GetTimeSliderWindow():
    if using_pyside:
        # this will return None, unless properly inited
        inst = GetPySideViewerInstance()
        if not inst is None:
            return inst.GetTimeSliderWindow()
    return None

def GetSourceManagerWindow():
    if using_pyside:
        # this will return None, unless properly inited
        inst = GetPySideViewerInstance()
        if not inst is None:
            return inst.GetSourceManagerWindow()
    return None

def GetPlotWindows():
    if using_pyside:
        # this will return None, unless properly inited
        inst = GetPySideViewerInstance()
        if not inst is None:
            return inst.GetPlotWindows()
    return None

def GetOperatorWindows():
    if using_pyside:
        # this will return None, unless properly inited
        inst = GetPySideViewerInstance()
        if not inst is None:
            return inst.GetOperatorWindows()
    return None

def GetOtherWindows():
    if using_pyside:
        # this will return None, unless properly inited
        inst = GetPySideViewerInstance()
        if not inst is None:
            return inst.GetOtherWindows()
    return None

