# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: visit_test.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 4/9/2010
 description:
    Provides a decorator that allows us to skip visit related tests,
    when the module is used outside of visit.

"""

import sys

def visit_test(fn):
    """
    Decorator that skips tests that require visit if
    we aren't running in the cli.
    """
    def run_fn(*args):
        if "visit" in list(sys.modules.keys()):
            return fn(*args)
        else:
            print("[VisIt module not found, skipping test that requires VisIt]")
        return None
    return run_fn


def pyside_test(fn):
    """
    Decorator that skips tests that require visit if
    we aren't running in the cli.
    """
    def run_fn(*args):
        if "PySide2.QtCore" in list(sys.modules.keys()):
            return fn(*args)
        else:
            print("[PySide not found, skipping test that requires PySide]")
        return None
    return run_fn

