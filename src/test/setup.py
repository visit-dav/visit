#!/usr/bin/env python
#
# file: setup.py
# author: Cyrus Harrison <cyrush@llnl.gov>
#
# distutils gen + setup script for the 'visit_testing' module.
#
# Modifications:
#    Cyrus Harrison, Wed Sep  7 11:34:36 PDT 2022
#    Refactor to use real source dir.
#
# ----------------------------------------------------------------------------


import sys
import os
import shutil

from os.path import join  as pjoin
from distutils.core import setup


#
# Support run w/ visit's cli.
#
using_visit = False
try:
    # the command line string passed to cli
    # will confuse distutils, so modify
    # sys.argv to only have args passed after
    # '-s setup.py'
    args = Argv()
    sys.argv = [__file__]
    sys.argv.extend(args)
    using_visit = True
except:
    pass


setup(name='visit_testing',
      version='0.1',
      author = 'VisIt Team',
      description='VisIt Testing Module',
      package_dir = {'visit_testing':'py_src'},
      packages=['visit_testing'],
      package_data={'visit_testing': ["nobaseline.pnm",
                                      "notext.txt",
                                      pjoin("report_templates","css","*"),
                                      pjoin("report_templates","js","*"),
                                      pjoin("report_templates","index.html")]})

if using_visit:
    sys.exit(0)