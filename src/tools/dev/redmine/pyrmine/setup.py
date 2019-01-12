#!/usr/bin/env python
#
# file: setup.py
# author: cdh
#
# distutils setup script for the pyredmine module.
#

from distutils.core import setup
import setup_tests

setup(name='pyrmine',
      version='0.1',
      author = 'Cyrus Harrison',
      author_email = 'cyrush@llnl.gov',
      description='Python Module for (limited) Redmine interaction.',
      package_dir = {'pyrmine':'src'},
      packages=['pyrmine'],
      cmdclass = { 'test': setup_tests.ExecuteTests})