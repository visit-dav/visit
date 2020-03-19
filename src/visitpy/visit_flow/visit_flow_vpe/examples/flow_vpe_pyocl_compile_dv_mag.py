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
    ctx.add_filter("decompose","dvdx",{"index":0})
    ctx.add_filter("decompose","dvdy",{"index":1})
    ctx.add_filter("decompose","dvdz",{"index":2})
    ctx.add_filter("grad","dv")
    ctx.add_filter("mult","vx_sq")
    ctx.add_filter("mult","vy_sq")
    ctx.add_filter("mult","vz_sq")
    ctx.add_filter("add","v_add_1")
    ctx.add_filter("add","v_add")
    ctx.add_filter("sqrt","v_sqrt")
    ctx.connect(":vy","dv:in")
    ctx.connect(":dims","dv:dims")
    ctx.connect(":x","dv:x")
    ctx.connect(":y","dv:y")
    ctx.connect(":z","dv:z")
    ctx.connect("dv","dvdx:in")
    ctx.connect("dv","dvdy:in")
    ctx.connect("dv","dvdz:in")
    ctx.connect("dvdx","vx_sq:in_a")
    ctx.connect("dvdx","vx_sq:in_b")
    ctx.connect("dvdy","vy_sq:in_a")
    ctx.connect("dvdy","vy_sq:in_b")
    ctx.connect("dvdz","vz_sq:in_a")
    ctx.connect("dvdz","vz_sq:in_b")
    ctx.connect("vx_sq","v_add_1:in_a")
    ctx.connect("vy_sq","v_add_1:in_b")
    ctx.connect("v_add_1","v_add:in_a")
    ctx.connect("vz_sq","v_add:in_b")
    ctx.connect("v_add","v_sqrt:in")
    return w


