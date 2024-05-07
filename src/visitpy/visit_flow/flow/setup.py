# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: setup_tests.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 10/14/2010
 description:
    distutils setup script for the 'flow' module.

"""

import sys
__system_bytecode_setting = sys.dont_write_bytecode
sys.dont_write_bytecode = True
from setuptools import setup
import sys
# TODO: update setup_tests
# and cmdclass = { 'test': setup_tests.ExecuteTests}
#import setup_tests



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

setup(name='visit_flow',
      author       = 'Cyrus Harrison',
      author_email = 'cyrush@llnl.gov',
      description  ='visit_flow: A small, flexible python data flow framework.',
      package_dir  = {'visit_flow':'src'},
      packages=['visit_flow','visit_flow.core','visit_flow.parser','visit_flow.filters'])
      #cmdclass = { 'test': setup_tests.ExecuteTests})

if using_visit:
    sys.exit(0)
