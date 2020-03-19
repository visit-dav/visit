# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: pyocl_ops.py
 author: Cyrus Harrison <cyrush@llnl.gov>
         Maysam Moussalem <maysam@tacc.utexas.edu>
 created: 3/24/2012
 description:
    Provides flow filters that execute PyOpenCLOps operations.

"""

# Guarded import of pyopencl
found_pyopencl = False
try:
    import numpy as npy
    import pyopencl as cl
    found_pyopencl = True
except ImportError:
    pass

from ..core import Filter, Context, log
from . import pyocl_env
from . import pyocl_kernels

def info(msg):
    log.info(msg,"filters.pyocl_ops")

class PyOpenCLOpsContext(Context):
    context_type = "pyocl_ops"
    def start(self,platform_id, device_id):
        pyocl_env.Manager.select_device(platform_id,device_id)
        pyocl_env.Manager.clear_events()
        pyocl_env.Pool.reset()
    def set_output_shape(self,shape):
        self.out_shape = shape
    def execute_kernel(self,kernel_source,inputs,out_dim=None):
        ctx = pyocl_env.Manager.context()
        msg  = "Execute Kernel:\n"
        msg += kernel_source
        info(msg)
        buffers = []
        vshape = self.__find_valid_shape(inputs)
        if out_dim is None:
            out_shape = vshape
        else:
            out_shape = (vshape[0],out_dim)
        for ipt in inputs:
            if not isinstance(ipt, npy.ndarray): # const case
                ibf    = npy.zeros(out_shape,dtype=npy.float32)
                ibf[:] = ipt
            else:
                ibf = ipt
            buf = pyocl_env.Pool.request_buffer(ibf.shape,ibf.dtype)
            buf.write(ibf)
            buffers.append(buf)
        dest_buf = pyocl_env.Pool.request_buffer(out_shape,npy.float32)
        buffers.append(dest_buf)
        pyocl_env.Manager.dispatch_kernel(kernel_source,
                                          out_shape,
                                          buffers)
        r= dest_buf.read()
        for b in buffers:
            b.release()
        pyocl_env.Pool.reclaim()
        return r
    def events_summary(self):
        return pyocl_env.Manager.events_summary()
    def __find_valid_shape(self,inputs):
        for ipt in inputs:
            if isinstance(ipt, npy.ndarray):
                return ipt.shape
        return None

class PyOpenCLOpsConst(Filter):
    filter_type    = "const"
    default_params = {"value":0}
    input_ports    = []
    output_port    = True
    def execute(self):
        p = self.params
        return p.value

class PyOpenCLOpsDecompose(Filter):
    filter_type    = "decompose"
    input_ports    = ["in"]
    default_params = {"index":0}
    output_port    = True
    def execute(self):
        p = self.params
        a = self.input("in")
        return npy.array(a[:,p.index])

class PyOpenCLOpsBaseFilter(Filter):
    def execute(self):
        inputs = [self.input(v) for v in self.input_ports]
        kcall, out_dim = pyocl_kernels.create_stub(self,inputs)
        return self.context.execute_kernel(kcall,inputs,out_dim)

def PyOpenCLOpsKernel(kernel_info):
    k     = kernel_info
    cname = "PyOpenCLBatch" +  k["name"][0].upper() + k["name"][1:]
    cdct  =  {"filter_type":k["name"],
             "input_ports": k["input_ports"],
             "default_params": {},
             "output_port":  True,
             "in_types": k["in_types"],
             "out_type": k["out_type"],
             "kernel_source": k["src"]}
    res   = type(cname,(PyOpenCLOpsBaseFilter,),cdct)
    return res

filters = [PyOpenCLOpsConst,
           PyOpenCLOpsDecompose]

for k,v in list(pyocl_kernels.Kernels.items()):
   filters.append(PyOpenCLOpsKernel(v))

contexts = [PyOpenCLOpsContext]

