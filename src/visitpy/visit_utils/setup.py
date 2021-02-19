# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: setup.py
 author: Cyrus Harrison <cyrush@llnl.gov>

 distutils setup script for the 'visit_utils' module.

"""
import sys
__system_bytecode_setting = sys.dont_write_bytecode
sys.dont_write_bytecode = True
from distutils.core import setup
import sys
import setup_tests


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

setup(name='visit_utils',
      version='0.1',
      author = 'Cyrus Harrison',
      author_email = 'cyrush@llnl.gov',
      description='VisIt Utilities Module',
      package_dir = {'visit_utils':'src'},
      packages=['visit_utils',
                'visit_utils.builtin',
                'visit_utils.qannote',
                'visit_utils.qplot'],
      cmdclass = { 'test': setup_tests.ExecuteTests})

if using_visit:
    sys.exit(0)
