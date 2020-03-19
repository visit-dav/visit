# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: npy_ops.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 12/09/2011
 description:
    Provides flow filters that execute numpy operations.

"""

# Guarded import of numpy
found_numpy = False
try:
    import numpy as npy
    found_numpy = True
except ImportError:
    pass

from ..core import Filter


class NPyAdd(Filter):
    filter_type    = "add"
    input_ports    = ["in_a","in_b"]
    default_params = {}
    output_port    = True
    def execute(self):
        return self.input("in_a") + self.input("in_b")

class NPySub(Filter):
    filter_type    = "sub"
    input_ports    = ["in_a","in_b"]
    default_params = {}
    output_port    = True
    def execute(self):
        return self.input("in_a") - self.input("in_b")

class NPyMult(Filter):
    filter_type    = "mult"
    input_ports    = ["in_a","in_b"]
    default_params = {}
    output_port    = True
    def execute(self):
        return self.input("in_a") * self.input("in_b")

class NPyPow(Filter):
    filter_type    = "pow"
    input_ports    = ["in_a","in_b"]
    default_params = {}
    output_port    = True
    def execute(self):
        return npy.power(self.input("in_a"),self.input("in_b"))

class NPySin(Filter):
    filter_type    = "sin"
    input_ports    = ["in"]
    default_params = {}
    output_port    = True
    def execute(self):
        return npy.sin(self.input("in"))

class NPyCos(Filter):
    filter_type    = "cos"
    input_ports    = ["in"]
    default_params = {}
    output_port    = True
    def execute(self):
        return npy.cos(self.input("in"))

class NPySqrt(Filter):
    filter_type    = "sqrt"
    input_ports    = ["in"]
    default_params = {}
    output_port    = True
    def execute(self):
        return npy.sqrt(self.input("in"))

class NPyCLConst(Filter):
    filter_type    = "const"
    default_params = {"value":0}
    input_ports    = []
    output_port    = True
    def execute(self):
        p = self.params
        return p.value

class NPyArrayCompose(Filter):
    filter_type    = "compose"
    input_ports    = ["in_a","in_b"]
    default_params = {}
    output_port    = True
    def execute(self):
        a = self.input("in_a")
        b = self.input("in_b")
        if len(a.shape) == 1:
            a = a.reshape(a.shape[0],1)
        if len(b.shape) == 1:
            b = b.reshape(a.shape[0],1)
        res = npy.hstack((a,b))
        return res

class NPyArrayDecompose(Filter):
    filter_type    = "decompose"
    input_ports    = ["in"]
    default_params = {"index":0}
    output_port    = True
    def execute(self):
        p = self.params
        a = self.input("in")
        return npy.array(a[:,p.index])

class NPyGrad(Filter):
    filter_type    = "grad"
    input_ports    = ["in","coords","dims"]
    default_params = {}
    output_port    = True
    def execute(self):
        coords = self.input("coords")
        vals   = self.input("in")
        d      = self.input("dims")
        lvals  = vals.shape[0]
        res = npy.zeros((vals.shape[0],3),dtype=vals.dtype)
        di = d[0]-1;
        dj = d[1]-1;
        dk = d[2]-1;
        for gid  in range(lvals):
            zi  = gid % di
            zj  = (gid / di) % dj
            zk  = (gid / di) / dj
            pi0 = zi + zj*(di+1) + zk*(di+1)*(dj+1)
            pi1 = zi + 1 + (zj+1)*(di+1) + (zk+1)*(di+1)*(dj+1)
            c0  = coords[pi0,:]
            c1  = coords[pi1,:]
            v   = vals[gid]
            vv  = npy.array([v, v, v, v, v, v],dtype=vals.dtype)
            if zi > 0:     vv[0] = vals[gid - 1]
            if zi < di -1: vv[1] = vals[gid + 1]
            if zj > 0:     vv[2] = vals[gid - di]
            if zj < dj -1: vv[3] = vals[gid + di]
            if zk > 0:     vv[4] = vals[gid - (di*dj)]
            if zk < dk-1 : vv[5] = vals[gid + (di*dj)]
            dvdx = (vv[1] - vv[0]) / (c1[0] - c0[0])
            dvdy = (vv[3] - vv[2]) / (c1[1] - c0[1])
            dvdz = (vv[5] - vv[4]) / (c1[2] - c0[2])
            if (zi != 0) and ( zi != di-1): dvdx *=.5
            if (zj != 0) and ( zj != dj-1): dvdy *=.5
            if (zk != 0) and ( zk != dk-1): dvdz *=.5
            res[gid,0] =  dvdx
            res[gid,1] =  dvdy
            res[gid,2] =  dvdz
        return res


filters = [NPyAdd,
           NPySub,
           NPyMult,
           NPyPow,
           NPySin,
           NPyCos,
           NPySqrt,
           NPyCLConst,
           NPyArrayCompose,
           NPyArrayDecompose,
           NPyGrad]

