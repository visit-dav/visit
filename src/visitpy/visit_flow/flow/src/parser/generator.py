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
from parser import *

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
                if expr.name in vmaps.keys():
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
        vmaps_inv = dict([[v,k] for k,v in vmaps.items()])
        for f in filters:
            fname = f[0]
            # check for nick name
            if fname in vmaps_inv.keys():
                fname = vmaps_inv[fname]
                f[0] = fname
            fcall = f[1]
            for arg in fcall.args:
                if isinstance(arg,Identifier):
                    if arg.name in vmaps_inv.keys():
                        arg.name = vmaps_inv[arg.name]
    @classmethod
    def __cse_elim(cls,filters):
        rfilters = []
        fmaps    = {}
        rmaps    = {}
        for f in filters:
            fname = f[0]
            fcall = str(f[1])
            if not fcall in fmaps.keys():
                fmaps[fcall] = fname
                rfilters.append(f)
            else:
                rmaps[fname] = fmaps[fcall]
        for f in rfilters:
            fcall = f[1]
            for arg in fcall.args:
                if arg.name in rmaps.keys():
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
