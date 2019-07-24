# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

###############################################################################
# file: setup.py
# Purpose: disutils setup for pyavt module.
#
# Programmer: Cyrus Harrison
# Creation: Thu Apr  5 08:42:40 PDT 2012
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


setup (name = 'pyavt',
       description = 'pyavt',
       package_dir = {'pyavt':'py_src'},
       packages=['pyavt'],
       package_data= { "pyavt": ["templates/*.py"]},
       cmdclass={'install_egg_info': SkipEggInfo})


