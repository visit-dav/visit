# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: vpe_flow_npy_ops_example_1.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/28/2012
 description:
    vpe flow example demonstrating use of flow.filters.npy_ops.

"""


from flow import *
from flow.filters import pyocl_compile


def setup_workspace():
    w = Workspace()
    w.register_filters(pyocl_compile)
    ctx = w.add_context("pyocl_compile","root")
    ctx.start()
    ctx.add_filter("decompose","dudx",{"index":0})
    ctx.add_filter("decompose","dudy",{"index":1})
    ctx.add_filter("decompose","dudz",{"index":2})
    ctx.add_filter("grad","du")
    ctx.add_filter("mult","vx_sq")
    ctx.add_filter("mult","vy_sq")
    ctx.add_filter("mult","vz_sq")
    ctx.add_filter("add","v_add_1")
    ctx.add_filter("add","v_add")
    ctx.add_filter("sqrt","v_sqrt")
    ctx.connect(":vx","du:in")
    ctx.connect(":dims","du:dims")
    ctx.connect(":x","du:x")
    ctx.connect(":y","du:y")
    ctx.connect(":z","du:z")
    ctx.connect("du","dudx:in")
    ctx.connect("du","dudy:in")
    ctx.connect("du","dudz:in")
    ctx.connect("dudx","vx_sq:in_a")
    ctx.connect("dudx","vx_sq:in_b")
    ctx.connect("dudy","vy_sq:in_a")
    ctx.connect("dudy","vy_sq:in_b")
    ctx.connect("dudz","vz_sq:in_a")
    ctx.connect("dudz","vz_sq:in_b")
    ctx.connect("vx_sq","v_add_1:in_a")
    ctx.connect("vy_sq","v_add_1:in_b")
    ctx.connect("v_add_1","v_add:in_a")
    ctx.connect("vz_sq","v_add:in_b")
    ctx.connect("v_add","v_sqrt:in")
    return w


