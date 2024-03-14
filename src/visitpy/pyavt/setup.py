# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

###############################################################################
# file: setup.py
# Purpose: setuptools setup for pyavt module.
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

setup (name = 'pyavt',
       description = 'pyavt',
       package_dir = {'pyavt':'py_src'},
       packages=['pyavt'],
       package_data= { "pyavt": ["templates/*.py"]})

