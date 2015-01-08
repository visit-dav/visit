#*****************************************************************************
#
# Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#*****************************************************************************
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
import pyocl_env
import pyocl_kernels

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

for k,v in pyocl_kernels.Kernels.items():
   filters.append(PyOpenCLOpsKernel(v))

contexts = [PyOpenCLOpsContext]

