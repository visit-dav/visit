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
    w.add_filter("add","f1")
    w.add_filter("sub","f2")
    w.add_filter("mult","f3")
    w.add_filter("mult","f4")
    w.add_filter("add","f5")
    # f1 = src_a + src_b
    w.connect(":d","f1:in_a")
    w.connect(":p","f1:in_b")
    # f2 = src_b - src_a
    w.connect(":d","f2:in_a")
    w.connect(":p","f2:in_b")
    # f3 = f1^2
    w.connect("f1","f3:in_a")
    w.connect("f1","f3:in_b")
    # f4 = f2^2
    w.connect("f2","f4:in_a")
    w.connect("f2","f4:in_b")
    # f5 = f4 + f3
    w.connect("f3","f5:in_a")
    w.connect("f4","f5:in_b")
    return w

