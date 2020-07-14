# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

###############################################################################
# file: __init__.py
# Purpose: Main init for the mpicom module.
#
# Programmer: Cyrus Harrison
# Creation: Thu Apr  5 08:51:03 PDT 2012
#
#
# Modifications:
#
#
###############################################################################

from . import mpistub
try:
    from mpicom import *
except ImportError:
    pass




