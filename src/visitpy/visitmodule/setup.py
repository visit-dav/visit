# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

###############################################################################
# file: setup.py
# Purpose: disutils setup for the visit module.
#
# Programmer: Cyrus Harrison
# Creation: Tue Apr  3 13:40:18 PDT
#
#
# Modifications:
#  Cyrus Harrison, Fri Feb 16 13:41:04 PST 2024
#  Move to use setuptools.
#
###############################################################################

from setuptools import setup

setup (name = 'visit',
       description = 'visit',
       package_dir = {'visit':'py_src'},
       packages=['visit'])


