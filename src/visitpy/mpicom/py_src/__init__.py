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
#    Cyrus Harrison, Thu Feb 18 16:06:46 PST 2021
#    Change the way the compiled lib import works to support Python 3.
#
###############################################################################

# import the serial stub module
from . import mpistub

# try to import the compiled module
# (this will only exist if visit was built with mpi support)
try:
    from . import mpicom
except ImportError:
    pass


