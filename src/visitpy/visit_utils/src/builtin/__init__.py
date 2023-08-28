# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: __init__.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 7/6/2020
 description:
     Init file for 'visit_utils.builtin' module.
"""


###############################################################################
# Modifications:
#    Cyrus Harrison, Wed Feb 24 10:12:20 PST 2021
#    Move PySide logic into visit_utils.builtin
#
#    Mark C. Miller, Mon Jun 28 17:00:28 PDT 2021
#    Add apropos and help override
#
###############################################################################
from .evalfuncs import *
from .writescript import WriteScript
from .convert2to3 import ConvertPy2to3
from .convert2to3 import GetAutoPy2to3
from .convert2to3 import SetAutoPy2to3
from .apropos import *
from .flatten import *

#
# Import PySide2 if it exists.
#
try:
    from . import pyside_hook
    from . import pyside_gui
    from . import pyside_support

    def IsPySideViewerEnabled(): return True

except ImportError:
    def IsPySideViewerEnabled(): return False
    pass

