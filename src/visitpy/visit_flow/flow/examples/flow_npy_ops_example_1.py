# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: flow_npy_ops_example_1.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/25/2012
 description:
    flow example demonstrating use of flow.filters.npy_ops.

"""

import numpy as npy
from flow import *
from flow.filters import *

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)

def main():
    w = Workspace()
    w.register_filters(npy_ops)
    v_a = npy.array(list(range(10)),dtype=npy.double)
    v_b = npy.array(list(range(10)),dtype=npy.double)
    w.registry_add(":src_a",v_a)
    w.registry_add(":src_b",v_b)
    w.add_filter("add","f1")
    w.add_filter("sub","f2")
    w.add_filter("mult","f3")
    w.add_filter("mult","f4")
    w.add_filter("add","f5")
    # f1 = src_a + src_b
    w.connect(":src_a","f1:in_a")
    w.connect(":src_b","f1:in_b")
    # f2 = src_b - src_a
    w.connect(":src_b","f2:in_a")
    w.connect(":src_a","f2:in_b")
    # f3 = f1^2
    w.connect("f1","f3:in_a")
    w.connect("f1","f3:in_b")
    # f4 = f2^2
    w.connect("f2","f4:in_a")
    w.connect("f2","f4:in_b")
    # f5 = f4 + f3
    w.connect("f3","f5:in_a")
    w.connect("f4","f5:in_b")
    print("Setting up Workspace")
    print("v_a: ", v_a)
    print("v_b: ", v_a)
    print("Executing: (v_a + v_b)^2 + (v_a - v_b)^2")
    print(w.execute())

if __name__ == "__main__":
    main()

