# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: flow_pyocl_compile_example_1.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/25/2012
 description:
    flow example demonstrating use of flow.filters.pyocl_compile.

"""

import numpy as npy
from flow import *
from flow.filters import *

# uncomment for detailed exe info
#import logging
#logging.basicConfig(level=logging.INFO)

def main():
    w = Workspace()
    w.register_filters(pyocl_compile)
    v_a = npy.array(list(range(10)),dtype=npy.float32)
    v_b = npy.array(list(range(10)),dtype=npy.float32)
    ctx  = w.add_context("pyocl_compile","root")
    ctx.registry_add(":src_a",v_a)
    ctx.registry_add(":src_b",v_b)
    ctx.start()
    ctx.add_filter("add","f1")
    ctx.add_filter("sub","f2")
    ctx.add_filter("mult","f3")
    ctx.add_filter("mult","f4")
    ctx.add_filter("add","f5")
    # f1 = src_a + src_b
    ctx.connect(":src_a","f1:in_a")
    ctx.connect(":src_b","f1:in_b")
    # f2 = src_b - src_a
    ctx.connect(":src_b","f2:in_a")
    ctx.connect(":src_a","f2:in_b")
    # f3 = f1^2
    ctx.connect("f1","f3:in_a")
    ctx.connect("f1","f3:in_b")
    # f4 = f2^2
    ctx.connect("f2","f4:in_a")
    ctx.connect("f2","f4:in_b")
    # f5 = f4 + f3
    ctx.connect("f3","f5:in_a")
    ctx.connect("f4","f5:in_b")
    print("Setting up Workspace")
    print("v_a: ", v_a)
    print("v_b: ", v_a)
    print("Executing: (v_a + v_b)^2 + (v_a - v_b)^2")
    print("Generated Kernel:")
    w.execute() # creates info needed by context to setup the kernel
    print(ctx.compile())
    print(ctx.run())

if __name__ == "__main__":
    main()

