# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 File: generator.py
 Authors: Cyrus Harrison, <cyrush@llnl.gov>
         Maysam Moussalem <maysam@tacc.utexas.edu>

 Description:
  Takes a list of expressions, parses them and builds a data flow
  network specification.

 Usage:
   >>> from generator import *
   >>> print Generator.parse_network("vx = a(2,3) + b^3 + 4 * var")

"""
import sys
import os
from .parser import *

from ..core import log

def info(msg):
    log.info(msg,"flow.parser")

class Generator(object):
    @classmethod
    def parse_network(cls,txt,ctx=None):
        filters = []
        vmaps   = {}
        stmts = Parser.parse(txt)
        cls.__create_network(stmts,filters,vmaps)
        cls.__vmaps_sub(filters,vmaps)
        filters = cls.__cse_elim(filters)
        if not ctx is None:
            cls.__setup_context(filters,ctx,vmaps)
        return filters
    @classmethod
    def __create_network(cls,stmts,filters,vmaps=None,count=None):
        if vmaps is None: vmaps = {}
        if count is None: count = [0]
        res = []
        for expr in stmts:
            if isinstance(expr,FuncCall):
                args = cls.__create_network(expr.args,filters,vmaps,count)
                fname = "f%d" % count[0]
                msg ="Generator: create_network func_call:"
                msg += "%s = %s( %s, %s)" % (fname, expr.name,str(args),str(expr.params))
                info(msg)
                filters.append([fname, FuncCall(expr.name, args, expr.params)])
                res.append(Identifier(fname))
                count[0]+=1
            if isinstance(expr,Assignment):
                res = cls.__create_network([expr.value],filters,vmaps,count)
                msg ="Generator: create_network assignment:"
                msg += "%s := %s"  % (expr.name,res[0])
                info(msg)
                vmaps[expr.name] = res[0].name
            if isinstance(expr,Identifier):
                if expr.name in list(vmaps.keys()):
                    iname =  vmaps[expr.name]
                else:
                    iname = ":" + expr.name
                res.append(Identifier(iname))
            if isinstance(expr,list):
                rvals = cls.__create_network(expr,filters,vmaps,count)
                res.extend(rvals)
            if isinstance(expr,Constant):
                fname  = "c%d" % count[0]
                params = {"value":expr.value}
                msg ="Generator: create_network constant:"
                msg += "%s = const([],%s)"  % (fname,str(params))
                info(msg)
                filters.append([fname, FuncCall("const", [], params)])
                res.append(Identifier(fname))
                count[0]+=1
        return res
    @classmethod
    def __vmaps_sub(cls,filters,vmaps):
        vmaps_inv = dict([[v,k] for k,v in list(vmaps.items())])
        for f in filters:
            fname = f[0]
            # check for nick name
            if fname in list(vmaps_inv.keys()):
                fname = vmaps_inv[fname]
                f[0] = fname
            fcall = f[1]
            for arg in fcall.args:
                if isinstance(arg,Identifier):
                    if arg.name in list(vmaps_inv.keys()):
                        arg.name = vmaps_inv[arg.name]
    @classmethod
    def __cse_elim(cls,filters):
        rfilters = []
        fmaps    = {}
        rmaps    = {}
        for f in filters:
            fname = f[0]
            fcall = str(f[1])
            if not fcall in list(fmaps.keys()):
                fmaps[fcall] = fname
                rfilters.append(f)
            else:
                rmaps[fname] = fmaps[fcall]
        for f in rfilters:
            fcall = f[1]
            for arg in fcall.args:
                if arg.name in list(rmaps.keys()):
                    arg.name = rmaps[arg.name]
        return rfilters
    @classmethod
    def __setup_context(cls,filters,ctx,vmaps):
        for f in filters:
            fname = f[0]
            fcall = f[1]
            msg   = "Generator setup_context: "
            msg  += "ctx.add_filter(%s,%s,%s)" % (str(fcall.name),str(fname),str(fcall.params))
            info(msg)
            ctx.add_filter(fcall.name,fname,fcall.params)
            idx = 0
            for arg in fcall.args:
                if isinstance(arg,Identifier):
                    aname = arg.name
                    msg   = "Generator setup_context: "
                    msg  += "ctx.connect(%s,(%s,%d))\n" % (aname,str(fname),idx)
                    info(msg)
                    ctx.connect(aname,(fname,idx))
                    idx+=1
