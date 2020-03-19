# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

###############################################################################
# file: setup.py
# Purpose: disutils setup for pyside_hook module.
#
# Programmer: Cyrus Harrison
# Creation: Tue Apr  3 13:40:18 PDT
#
#
# Modifications:
#
#
###############################################################################

import sys
from distutils.core import setup
from distutils.command.install_egg_info import install_egg_info

# disable install_egg_info
class SkipEggInfo(install_egg_info):
    def run(self):
        pass


setup (name = 'pyside_hook',
       description = 'pyside_hook',
       package_dir = {'pyside_hook':'py_src'},
       packages=['pyside_hook'],
       cmdclass={'install_egg_info': SkipEggInfo})


