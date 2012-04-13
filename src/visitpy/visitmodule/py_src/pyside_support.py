#
#
#

import sys
import os
from threading import Event, Thread

using_pyside = False

try:
    from PySide.QtGui import QApplication
    import pyside_viewer
    using_pyside = True
except ImportError:
    pass

__all__ = ["SetupTimer","GetRenderWindow","GetRenderWindowIds"]

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
            self.event.wait(self.interval)
            if not self.event.is_set():
                if os.name == 'posix' or os.name == 'mac' :
                    import select
                    i,o,e = select.select([sys.stdin],[],[],0.0001)
                    for s in i:
                        if s == sys.stdin:
                            self.qtapp.exit(0)
                else:
                    import msvcrt
                    if msvcrt.kbhit():
                        self.qtapp.exit(0)

def SetupTimer():
    if ProcessCLIInput.instance is None:
        ProcessCLIInput.instance = ProcessCLIInput(0.001)
        ProcessCLIInput.instance.start()

def IsPySideViewerEnabled():
    res = False
    if using_pyside:
        inst = pyside_viewer.PySideViewer.instance()
        if not inst is None:
            res = True
    return res

def GetRenderWindow(i):
    if using_pyside:
        # this will return None, unless properly inited
        inst = pyside_viewer.PySideViewer.instance()
        if not inst is None:
            return inst.GetRenderWindow(i)
    else:
        return None

def GetRenderWindowIds():
    if using_pyside:
        # this will return None, unless properly inited
        inst = pyside_viewer.PySideViewer.instance()
        if not inst is None:
            return inst.GetRenderWindowIDs()
    else:
        return None

