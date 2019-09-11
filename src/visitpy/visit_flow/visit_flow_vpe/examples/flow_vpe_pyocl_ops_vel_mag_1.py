# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: flow_vpe_pyocl_ops_vel_mag_1.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/28/2012


"""


from flow import *
from flow.filters import pyocl_ops


def setup_workspace():
    w = Workspace()
    w.register_filters(pyocl_ops)
    ctx = w.add_context("pyocl_ops","root")
    ctx.start()
    ctx.add_filter("mult","vel_x_sq")
    ctx.add_filter("mult","vel_y_sq")
    ctx.add_filter("mult","vel_z_sq")
    ctx.add_filter("add","vel_add_1")
    ctx.add_filter("add","vel_add")
    ctx.add_filter("sqrt","vel_sqrt")
    ctx.connect(":vx","vel_x_sq:in_a")
    ctx.connect(":vx","vel_x_sq:in_b")
    ctx.connect(":vy","vel_y_sq:in_a")
    ctx.connect(":vy","vel_y_sq:in_b")
    ctx.connect(":vz","vel_z_sq:in_a")
    ctx.connect(":vz","vel_z_sq:in_b")
    ctx.connect("vel_x_sq","vel_add_1:in_a")
    ctx.connect("vel_y_sq","vel_add_1:in_b")
    ctx.connect("vel_add_1","vel_add:in_a")
    ctx.connect("vel_z_sq","vel_add:in_b")
    ctx.connect("vel_add","vel_sqrt:in")
    return w


