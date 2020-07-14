# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: decorators.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/26/2012
 description:
    Provides decorators that skip numpy & pyopencl related tests
    if these modules are not available.

"""
from visit_flow.core import sexe

def skip_warning(name):
    print("[%s not found: Skipping dependant test.]" % name)

def ply_test(fn):
    """
    Decorator that skips tests that require ply.
    """
    def run_fn(*args):
        try:
            import ply
        except ImportError:
            skip_warning("ply")
            return None
        return fn(*args)
    return run_fn

def numpy_test(fn):
    """
    Decorator that skips tests that require numpy.
    """
    def run_fn(*args):
        try:
            import numpy
        except ImportError:
            skip_warning("numpy")
            return None
        return fn(*args)
    return run_fn

def pyocl_test(fn):
    """
    Decorator that skips tests that require pyopencl.
    """
    def run_fn(*args):
        try:
            import pyopencl
        except ImportError:
            skip_warning("pyopencl")
            return None
        return fn(*args)
    return run_fn

def imagick_test(fn):
    """
    Decorator that skips tests that require ImageMagick tools.
    """
    def run_fn(*args):
        # check for convert, composite
        r1 = sexe("which convert",  ret_output=True)
        r2 = sexe("which composite",ret_output=True)
        if r1[0] != 0 or r2[0]!=0:
            skip_warning("ImageMagick")
            return None
        return fn(*args)
    return run_fn
