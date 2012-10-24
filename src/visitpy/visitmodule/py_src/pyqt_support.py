###############################################################################
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
#
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

