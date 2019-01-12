#!/usr/bin/env python
#
# file: setup.py
# author: Cyrus Harrison <cyrush@llnl.gov>
#
# distutils setup script for the 'masony' module.
#

import os
import sys
import setup_tests

from os.path import join as pjoin

from distutils.core import setup


#
# Support running tests w/ visit's cli.
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

setup(name='masonry',
      version='0.0',
      author = 'Cyrus Harrison',
      author_email = 'cyrush@llnl.gov',
      description='Masonry',
      package_dir = {'masonry':'src'},
      packages=['masonry'],
      package_data= { "masonry": [ pjoin("html","*.html"),
                                   pjoin("html","css","*"),
                                   pjoin("html","js","*"),
                                   pjoin("html","fonts","*")]},
      cmdclass = { 'test': setup_tests.ExecuteTests})

if using_visit:
    sys.exit(0)