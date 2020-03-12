# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: pyocl_batch.py
 author: Cyrus Harrison <cyrush@llnl.gov>

 created: 9/1/2012
 description:
    Provides flow filters that execute PyOpenCLBatch operations.

"""
import logging
#logging.basicConfig(level=logging.INFO)
logging.basicConfig(level=logging.ERROR)


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
    log.info(msg,"filters.pyocl_batch")

def err(msg):
    log.error(msg,"filters.pyocl_batch")


class PyOpenCLBatchContext(Context):
    context_type = "pyocl_batch"
    def start(self,platform_id, device_id):
        pyocl_env.Manager.select_device(platform_id,device_id)
        pyocl_env.Manager.clear_events()
        pyocl_env.Pool.reset()
    def set_output_shape(self,shape):
        self.out_shape = shape
    def execute_kernel(self,kernel_source,inputs,out_dim=None):
        buffers = []
        buffers.extend(inputs)
        vshape = self.__find_valid_shape(inputs)
        if out_dim is None:
            out_shape = vshape
        else:
            if out_dim == 1:
                out_shape = (vshape[0],)
            else:
                out_shape = (vshape[0],out_dim)
        msg  = "Execute Kernel:\n"
        msg += kernel_source
        info(msg)
        info("Execute Kernel: out_shape = " + str(out_shape))
        dest_buf = pyocl_env.Pool.request_buffer(out_shape,npy.float32)
        buffers.append(dest_buf)
        evnt = pyocl_env.Manager.dispatch_kernel(kernel_source,
                                                 out_shape,
                                                 buffers)
        return dest_buf
    def events_summary(self):
        return pyocl_env.Manager.events_summary()
    def __find_valid_shape(self,inputs):
        for ipt in inputs:
            return ipt.out_shape
        return self.out_shape


class PyOpenCLBatchBaseFilter(Filter):
    def execute(self):
        inputs = [self.input(v) for v in self.input_ports]
        kcall, out_dim = pyocl_kernels.create_stub(self,inputs)
        return self.context.execute_kernel(kcall,inputs,out_dim)

def PyOpenCLBatchKernel(kernel_info):
    k = kernel_info
    cname = "PyOpenCLBatch" +  k["name"][0].upper() + k["name"][1:]
    cdct =  {"filter_type":k["name"],
             "input_ports": k["input_ports"],
             "default_params": {},
             "output_port":  True,
             "in_types": k["in_types"],
             "out_type": k["out_type"],
             "kernel_source": k["src"]}
    res = type(cname,(PyOpenCLBatchBaseFilter,),cdct)
    return res


class PyOpenCLBatchSource(Filter):
    # overrides standard RegistrySource
    filter_type    = "<registry_source>"
    input_ports    = []
    default_params = {}
    output_port    = True
    def execute(self):
        # fetch data from registry
        # the instance name determines the reg entry_key
        key  = self.name[self.name.rfind(":"):]
        data = self.context.registry_fetch(key)
        buf = pyocl_env.Pool.request_buffer(data.shape,data.dtype)
        buf.write(data)
        return buf

class PyOpenCLBatchConst(Filter):
    filter_type    = "const"
    default_params = {"value":0.0}
    input_ports    = []
    output_port    = True
    def execute(self):
        p = self.params
        kernel_source =  """
            __kernel void kmain(__global float *o)
            {
              o[get_global_id(0)] =  %s;
            }
            """ % str(p.value)
        return self.context.execute_kernel(kernel_source,[])

class PyOpenCLBatchDecompose(Filter):
    filter_type    = "decompose"
    input_ports    = ["in"]
    default_params = {"index":0}
    output_port    = True
    def execute(self):
        p = self.params
        a = self.input("in")
        inputs = [a]
        dim = a.shape[1]
        idx = p.index
        kernel_source =  """
            __kernel void kmain(__global const float *v,
                                __global float *o)
            {
              int gid = get_global_id(0);
              o[gid] =  v[gid*%d+%d];
            }
            """ % (dim,idx)
        return self.context.execute_kernel(kernel_source,inputs,1)

filters = [PyOpenCLBatchSource,
           PyOpenCLBatchDecompose,
           PyOpenCLBatchConst]

for k,v in list(pyocl_kernels.Kernels.items()):
  filters.append(PyOpenCLBatchKernel(v))


contexts = [PyOpenCLBatchContext]

