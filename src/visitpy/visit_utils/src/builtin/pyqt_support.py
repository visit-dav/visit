# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

###############################################################################
# file: pyqt_support.py
# Purpose: Shapes pyqt support for the visit module api.
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

using_pyqt = False
__internal_qapp__ = None
try:
    from PyQt4.QtGui import QApplication
    import pyqt_gui
    using_pyqt = True
except ImportError:
    pass

__all__ = ["LaunchPyViewer","GetRenderWindow","GetRenderWindowIds","GetUIWindow","GetPlotWindow","GetOperatorWindow", "GetOtherWindow", "GetOtherWindowNames"]
__pyqt_viewer_instance__ = None

# this is a function that polls for keyboard input,
# when it sees one it quits the

def GetPyQtViewerInstance():
    global __internal_qapp__
    global __pyqt_viewer_instance__
    if __pyqt_viewer_instance__ is None:

        __internal_qapp__ = QApplication.instance()
        if __internal_qapp__ is None : __internal_qapp__=QApplication(sys.argv)

        __pyqt_viewer_instance__ = pyqt_gui.PyQtGUI(sys.argv)

    return __pyqt_viewer_instance__

def LaunchPyViewer(args):
    global __internal_qapp__
    global __pyqt_viewer_instance__

    if args is None: 
        args = sys.argv
        args.append("-pyuiembedded")

    if __pyqt_viewer_instance__ is None:

        __internal_qapp__ = QApplication.instance()
        if __internal_qapp__ is None : 
            __internal_qapp__ = QApplication(sys.argv)

        __pyqt_viewer_instance__ = pyqt_gui.PyQtGUI(args)

    return __pyqt_viewer_instance__

def IsPyQtViewerEnabled():
    res = False
    if using_pyqt:
        inst = GetPyQtViewerInstance()
        if not inst is None:
            res = True
    return res

def GetRenderWindow(i):
    if using_pyqt:
        # this will return None, unless properly inited
        inst = GetPyQtViewerInstance()
        if not inst is None:
            return inst.GetRenderWindow(i)
    else:
        return None

def GetRenderWindowIds():
    if using_pyqt:
        # this will return None, unless properly inited
        inst = GetPyQtViewerInstance()
        if not inst is None:
            return inst.GetRenderWindowIDs()
    else:
        return None

def GetUIWindow():
    if using_pyqt:
        # this will return None, unless properly inited
        inst = GetPyQtViewerInstance()
        if not inst is None:
            return inst.GetUIWindow()
    else:
        return None

def GetPlotWindow(name):
    if using_pyqt:
        # this will return None, unless properly inited
        inst = GetPyQtViewerInstance()
        if not inst is None:
            return inst.GetPlotWindow(name)
    else:
        return None

def GetOperatorWindow(name):
    if using_pyqt:
        # this will return None, unless properly inited
        inst = GetPyQtViewerInstance()
        if not inst is None:
            return inst.GetOperatorWindow(name)
    else:
        return None

def GetOtherWindow(name):
    if using_pyqt:
        # this will return None, unless properly inited
        inst = GetPyQtViewerInstance()
        if not inst is None:
            return inst.GetOtherWindow(name)
    else:
        return None

def GetOtherWindowNames():
    if using_pyqt:
        # this will return None, unless properly inited
        inst = GetPyQtViewerInstance()
        if not inst is None:
            return inst.GetOtherWindowNames()
    else:
        return None

