# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

###############################################################################
# file: setup.py
# Purpose: python setup for the visit module.
#
# Programmer: Cyrus Harrison
# Creation: Wed Jul 16 07:52:20 PDT 2025
#
###############################################################################

from setuptools import setup

setup (name = 'visit',
       description = 'visit',
       package_dir = {'visit':'py_src'},
       packages=['visit'])


