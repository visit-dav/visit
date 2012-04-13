#
#
#

import sys
import os
from threading import Event, Thread
from PySide.QtGui import QApplication

__all__ = ["setup_timer"]

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

def setup_timer():
    if ProcessCLIInput.instance is None:
        ProcessCLIInput.instance = ProcessCLIInput(0.001)
        ProcessCLIInput.instance.start()
