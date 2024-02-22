# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

###############################################################################
# file: setup.py
# Purpose: disutils setup for mpicom module.
#
# Programmer: Cyrus Harrison
# Creation: Thu Apr  5 08:42:40 PDT 2012
#
#
# Modifications:
#  Cyrus Harrison, Fri Feb 16 13:41:04 PST 2024
#  Move to use setuptools.
#
###############################################################################

from setuptools import setup

setup (name = 'mpicom',
       description = 'mpicom',
       package_dir = {'mpicom':'py_src'},
       packages=['mpicom'])


