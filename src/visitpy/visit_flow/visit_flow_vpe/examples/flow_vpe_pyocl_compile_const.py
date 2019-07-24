# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: flow_vpe_pyocl_compile_const.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/28/2012


"""


from flow import *
from flow.filters import pyocl_compile

def setup_workspace():
    w = Workspace()
    w.register_filters(pyocl_compile)
    ctx = w.add_context("pyocl_compile","root")
    ctx.start()
    ctx.add_filter("mult","f0")
    ctx.add_filter("const","half",{"value":.5})
    ctx.connect(":vx","f0:in_a")
    ctx.connect("half","f0:in_b")
    return w


