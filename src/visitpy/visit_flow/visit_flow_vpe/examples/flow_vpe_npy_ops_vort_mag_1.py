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
from flow.filters import npy_ops


def setup_workspace():
    w = Workspace()
    w.register_filters(npy_ops)
    w.add_filter("compose","coord_comp_1")
    w.add_filter("compose","coord_comp_2")
    w.add_filter("compose","vgt_comp_1")
    w.add_filter("compose","vgt_comp_2")
    w.add_filter("grad","dvx")
    w.add_filter("grad","dvy")
    w.add_filter("grad","dvz")
    w.add_filter("decompose","vort_x_decomp_1",{"index":7})
    w.add_filter("decompose","vort_x_decomp_2",{"index":5})
    w.add_filter("sub","vort_x_sub")
    w.add_filter("decompose","vort_y_decomp_1",{"index":2})
    w.add_filter("decompose","vort_y_decomp_2",{"index":6})
    w.add_filter("sub","vort_y_sub")
    w.add_filter("decompose","vort_z_decomp_1",{"index":3})
    w.add_filter("decompose","vort_z_decomp_2",{"index":1})
    w.add_filter("sub","vort_z_sub")
    w.add_filter("mult","vort_x_sq")
    w.add_filter("mult","vort_y_sq")
    w.add_filter("mult","vort_z_sq")
    w.add_filter("add","vort_mag_add_1")
    w.add_filter("add","vort_mag_add_2")
    w.add_filter("sqrt","vort_mag_sqrt")
    # coord_comp_2 = { :x, :y,:z_c}
    w.connect(":x","coord_comp_1:in_a")
    w.connect(":y","coord_comp_1:in_b")
    w.connect("coord_comp_1","coord_comp_2:in_a")
    w.connect(":z","coord_comp_2:in_b")
    w.connect("coord_comp_2","dvx:coords")
    w.connect(":dims","dvx:dims")
    w.connect(":vx","dvx:in")
    w.connect("coord_comp_2","dvy:coords")
    w.connect(":dims","dvy:dims")
    w.connect(":vy","dvy:in")
    w.connect("coord_comp_2","dvz:coords")
    w.connect(":dims","dvz:dims")
    w.connect(":vz","dvz:in")
    # vgt_comp_2 = { dvx,dvy,dvz}
    w.connect("dvx","vgt_comp_1:in_a")
    w.connect("dvy","vgt_comp_1:in_b")
    w.connect("vgt_comp_1","vgt_comp_2:in_a")
    w.connect("dvz","vgt_comp_2:in_b")
    #vort_x_sub =  vgt_comp_2[7] - vgt_comp_2[5]
    w.connect("vgt_comp_2","vort_x_decomp_1:in")
    w.connect("vgt_comp_2","vort_x_decomp_2:in")
    w.connect("vort_x_decomp_1","vort_x_sub:in_a")
    w.connect("vort_x_decomp_2","vort_x_sub:in_b")
    #vort_y_sub =  vgt_comp_2[2] - vgt_comp_2[6]
    w.connect("vgt_comp_2","vort_y_decomp_1:in")
    w.connect("vgt_comp_2","vort_y_decomp_2:in")
    w.connect("vort_y_decomp_1","vort_y_sub:in_a")
    w.connect("vort_y_decomp_2","vort_y_sub:in_b")
    #vort_z_sub =  vgt_comp_2[3] - vgt_comp_2[1]
    w.connect("vgt_comp_2","vort_z_decomp_1:in")
    w.connect("vgt_comp_2","vort_z_decomp_2:in")
    w.connect("vort_z_decomp_1","vort_z_sub:in_a")
    w.connect("vort_z_decomp_2","vort_z_sub:in_b")
    #
    w.connect("vort_x_sub","vort_x_sq:in_a")
    w.connect("vort_x_sub","vort_x_sq:in_b")
    #
    w.connect("vort_y_sub","vort_y_sq:in_a")
    w.connect("vort_y_sub","vort_y_sq:in_b")
    #
    w.connect("vort_z_sub","vort_z_sq:in_a")
    w.connect("vort_z_sub","vort_z_sq:in_b")
    w.connect("vort_x_sq","vort_mag_add_1:in_a")
    w.connect("vort_y_sq","vort_mag_add_1:in_b")
    w.connect("vort_mag_add_1","vort_mag_add_2:in_a")
    w.connect("vort_z_sq","vort_mag_add_2:in_b")
    #
    w.connect("vort_mag_add_2","vort_mag_sqrt:in")
    return w

