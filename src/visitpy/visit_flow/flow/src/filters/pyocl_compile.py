# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: pyocl_compile.py
 author: Cyrus Harrison <cyrush@llnl.gov>
         Maysam Moussalem <maysam@cs.utexas.edu>
 created: 3/24/2012
 description:
    Provides flow filters that compile and execute PyOpenCL operations.

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
    log.info(msg,"filters.pyocl_compile")

class PyOpenCLCompileContext(Context):
    context_type = "pyocl_compile"
    def start(self,platform_id, device_id):
        pyocl_env.Manager.select_device(platform_id,device_id)
        pyocl_env.Manager.clear_events()
        pyocl_env.Pool.reset()
        self.kernels = {}
        self.stmts   = []
        self.inputs  = []
        self.out_shape = None
    def set_device_id(self,dev_id):
        pyocl_env.Manager.set_device_id(dev_id)
    def bind_data(self,obj):
        idx = len(self.inputs)
        self.inputs.append(obj)
        base = "in_%04d" %idx
        return ("%s_fetch" % base, base)
    def set_output_shape(self,shape):
        self.out_shape = shape
    def add_decompose(self,var,out_type = "float"):
        idx = len(self.stmts)
        res_name  = "_auto_res_%04d" % idx
        stmt = "%s %s = %s;" % (out_type,res_name,var)
        self.stmts.append(stmt)
        return (res_name,None)
    def add_call(self,kernel_name,
                      kernel_source,
                      args,
                      in_types = None,
                      out_type = "float"):
        idx = len(self.stmts)
        if not kernel_name in list(self.kernels.keys()):
            self.kernels[kernel_name] = kernel_source
        res_name  = "_auto_res_%04d" % idx
        stmt = "%s(" % kernel_name
        if in_types is None:
            in_types = ["fetch"]*len(args)
        for idx in range(len(args)):
            arg = args[idx]
            if in_types[idx] == "fetch":
                stmt += "%s," % arg[0]
            else:
                stmt += "%s," % arg[1]
        stmt = stmt[:-1] + ")"
        stmt = "%s %s = %s;" % (out_type,res_name,stmt)
        self.stmts.append(stmt)
        return (res_name,None)
    def compile(self):
        act = pyocl_env.PyOpenCLHostTimer("auto_kgen",0)
        act.start()
        res = ""
        for kern in list(self.kernels.values()):
            res += kern
        ident = "            "
        args_ident = "                               "
        res += "\n%s__kernel void kmain(" % ident
        for idx in range(len(self.inputs)):
            if self.inputs[idx].dtype == npy.int32:
                itype = "int  "
            else:
                itype = "float"
            iname = "in_%04d" % idx
            res  += "__global const %s *%s,\n%s " % (itype,iname,args_ident)
        res += " __global float *out)\n"
        res += "%s{\n" % ident
        res += "%s int gid = get_global_id(0);\n" % ident
        for idx in range(len(self.inputs)):
            if self.inputs[idx].dtype == npy.int32:
                itype = "int  "
            else:
                itype = "float"
            iname = "in_%04d" % idx
            res += "%s %s %s_fetch = %s[gid];\n" % (ident,itype,iname,iname)
        for stmt in self.stmts:
            res += "%s %s\n" % (ident,stmt)
        res += "%s out[gid] = _auto_res_%04d;\n" % (ident,len(self.stmts)-1)
        res += "%s}\n" % ident
        act.stop()
        pyocl_env.Manager.add_host_event(act)
        return res
    def run(self):
        # run in context
        kernel_source = self.compile()
        return self.execute_kernel(kernel_source,self.inputs)
    def execute_kernel(self,kernel_source,inputs):
        msg  = "Execute Kernel:\n"
        msg += kernel_source
        info(msg)
        buffers = []
        for ipt in inputs:
            buf = pyocl_env.Pool.request_buffer(ipt.shape,ipt.dtype)
            buf.write(ipt)
            buffers.append(buf)
        dest_buf = pyocl_env.Pool.request_buffer(self.out_shape,npy.float32)
        buffers.append(dest_buf)
        pyocl_env.Manager.dispatch_kernel(kernel_source,
                                          self.out_shape,
                                          buffers)
        return dest_buf.read()
    def events_summary(self):
        return pyocl_env.Manager.events_summary()

class PyOpenCLCompileSource(Filter):
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
        # bind var into the context
        var_name = self.context.bind_data(data)
        return var_name

class PyOpenCLCompileConst(Filter):
    filter_type    = "const"
    default_params = {"value":0}
    input_ports    = []
    output_port    = True
    def execute(self):
        p = self.params
        return ("%s" % str(p.value), None)

class PyOpenCLCompileDecompose(Filter):
    filter_type    = "decompose"
    input_ports    = ["in"]
    default_params = {"index":0}
    output_port    = True
    def execute(self):
        p = self.params
        a = self.input("in")
        res = ("%s.s%d" % (a[0],p.index), None)
        self.context.add_decompose(res[0])
        return ("%s.s%d" % (a[0],p.index), None)


class PyOpenCLCompileBaseFilter(Filter):
    def execute(self):
        args = [self.input(v) for v in self.input_ports]
        return self.context.add_call("k" + self.filter_type,
                                     self.kernel_source,
                                     args,
                                     self.in_types,
                                     self.out_type)

def PyOpenCLCompileKernel(kernel_info):
    k     = kernel_info
    cname = "PyOpenCLCompile" +  k["name"][0].upper() + k["name"][1:]
    cdct  =  {"filter_type":k["name"],
             "input_ports": k["input_ports"],
             "default_params": {},
             "output_port":  True,
             "in_types": k["in_types"],
             "out_type": k["out_type"],
             "kernel_source": k["src"]}
    res  = type(cname,(PyOpenCLCompileBaseFilter,),cdct)
    return res


filters = [PyOpenCLCompileSource,
           PyOpenCLCompileConst,
           PyOpenCLCompileDecompose]

for k,v in list(pyocl_kernels.Kernels.items()):
    filters.append(PyOpenCLCompileKernel(v))

contexts = [PyOpenCLCompileContext]
