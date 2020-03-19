# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

###############################################################################
# file: __init__.py
# Purpose: Main init for the pyside_visithook module.
#
# Programmer: Cyrus Harrison
# Creation: Tue Apr  3 13:40:18 PDT
#
#
# Modifications:
#   Brad Whitlock, Tue May  1 16:49:40 PDT 2012
#   Add writescript.
#
#   Cyrus Harrison, Wed May  2 14:07:07 PDT 2012
#   Avoid creating pyc files when importing the visit module
#
###############################################################################
import sys
__system_bytecode_setting = sys.dont_write_bytecode
sys.dont_write_bytecode = True

#
# note: the frontend is responsible for loading the actual visit module
#

from .frontend import *
from .evalfuncs import *
from .writescript import *

try:
    import PySide2
    import pyside_hook
    import pyside_gui
    from . import pyside_support

    class KeyPressEater(PySide2.QtCore.QObject):
        def eventFilter(self, obj, event):
            if event.type() == PySide2.QtCore.QEvent.KeyPress:
                return True
            elif event.type() == PySide2.QtCore.QEvent.MouseButtonPress:
                return True
            elif event.type() == PySide2.QtCore.QEvent.MouseButtonRelease:
                return True
            elif event.type() == PySide2.QtCore.QEvent.MouseButtonDblClick:
                return True
            elif event.type() == PySide2.QtCore.QEvent.MouseMove:
                return True
            else:
                return PySide2.QtCore.QObject.eventFilter(self, obj, event)

    def __VisIt_PySide_Idle_Hook__():
      a = KeyPressEater()
      app = PySide2.QtCore.QEventLoop();
      PySide2.QtCore.QCoreApplication.instance().installEventFilter(a)
      app.processEvents(PySide2.QtCore.QEventLoop.ProcessEventsFlag.ExcludeUserInputEvents);
      PySide2.QtCore.QCoreApplication.instance().removeEventFilter(a)

    def IsPySideViewerEnabled(): return True
except ImportError:
    def IsPySideViewerEnabled(): return False
    pass

sys.dont_write_bytecode = __system_bytecode_setting










