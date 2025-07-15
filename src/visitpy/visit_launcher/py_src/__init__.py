"""Main init for the pyside_visithook module."""

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
# Modifications:
#   Brad Whitlock, Tue May  1 16:49:40 PDT 2012
#   Add writescript.
#
#   Cyrus Harrison, Wed May  2 14:07:07 PDT 2012
#   Avoid creating pyc files when importing the visit module
#
#   Cyrus Harrison, Wed Feb 24 10:12:20 PST 2021
#   Moved PySide logic into visit_utils.builtin
#
#   Mark C. Miller, Thu Mar 31 17:33:22 PDT 2022
#   Restructured this file a bit putting a docstring on the first line.
#   This makes help(visit) after import visit do something more reasonable.
###############################################################################

# The frontend is responsible for loading the actual visit module.
# The dont_write_bytecode setting effects imports that follow.
import sys
__system_bytecode_setting = sys.dont_write_bytecode
sys.dont_write_bytecode = True                      # effects imports to follow

from .frontend import *
from visit_utils.builtin import *
sys.dont_write_bytecode = __system_bytecode_setting # reset to initial value
