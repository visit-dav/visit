###############################################################################
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
#
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
    from PySide.QtGui import QApplication
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

