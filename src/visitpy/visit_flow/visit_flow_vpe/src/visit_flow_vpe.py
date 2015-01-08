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
 file: visit_flow_expr.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 12/09/2011
 description:
    Provides a function that defines a VisIt Python Expression
    to execute a flow workspace.

"""

import visit
import os
from flow import *
import flow.filters

from os.path import join as pjoin

def vpe_path():
    return os.path.split(os.path.abspath(__file__))[0]

def escape_src(src):
    # make sure the source will survive visits expr parser
    src = src.replace('"','\\"')
    src = src.replace('\n','\\n')
    src = src.replace(' ','\\s')
    return src

def define_flow_vpe(ename,expr,
                    filter_set,
                    platform_id,
                    device_id):
    # get proper vpe path ...
    fvpe = pjoin(vpe_path(),"visit_flow_exec.vpe")
    args = []
    # keep a path for the old way
    if expr.endswith(".py") and os.path.isfile(expr):
        filter_set = "src"
        expr = open(expr).read()
        w = Workspace.load_workspace_script(src=expr)
    else:
        if os.path.isfile(expr):
            expr = open(expr).read()
        w = Workspace()
        w.register_filters(flow.filters.module(filter_set))
        ctx = w.add_context(filter_set,"root")
        w.setup_expression_network(expr,ctx)
    # get root vars & use as expr args
    evars = w.filter_names()
    evars = [ evar[1:] for evar in evars if evar[0] == ":" and evar != ":dims"]
    #print "evars: " , evars
    args.extend(evars)
    expr_escaped = escape_src(expr)
    args.extend(['"'+ filter_set +  '"','"' + expr_escaped+ '"'])
    args.extend(['"%d"' % platform_id, '"%d"' % device_id])
    visit.DefinePythonExpression(ename,file=fvpe,args=args)

def vpe_timing_info(ttag,wcomps,wtot):
    res = ""
    itot  = 0
    ctx_ste = ttag["total"]["ste"]
    ctx_qte = ttag["total"]["qte"]
    ctx_ste_diff = wtot.get_elapsed() - ctx_ste
    ctx_ste_diff_perc = str(round(100.0*ctx_ste_diff/wtot.get_elapsed(),2)) + " (%)"
    ctx_qte_diff = wtot.get_elapsed() - ctx_qte
    ctx_qte_diff_perc = str(round(100.0*ctx_qte_diff/wtot.get_elapsed(),2)) + " (%)"
    res +=  "\n::TimingInfo\n"
    for wcomp in wcomps:
        itot += wcomp.get_elapsed()
        res += "::TimingInfo  %s\n" % wcomp
    res +=  "::TimingInfo  %s\n" % wtot
    res +="\n"
    res += "::TimingInfo  wtot - (all_wcomps) = %s\n" % repr(wtot.get_elapsed() - itot)
    res += "::TimingInfo  ctx_ste = %s\n" % repr(ctx_ste)
    res +="::TimingInfo  wtot - ctx_ste = %s\n" % repr(ctx_ste_diff)
    res +="::TimingInfo  ctx_ste_diff/wtot = %s\n" % ctx_ste_diff_perc
    res +="\n"
    res +="::TimingInfo  ctx_qte = %s\n" % repr(ctx_qte)
    res +="::TimingInfo  wtot - ctx_qte = %s\n" % repr(ctx_qte_diff)
    res +="::TimingInfo  ctx_qte_diff/wtot = %s\n" % ctx_qte_diff_perc
    return res

__all__ = [ "define_flow_vpe","vpe_timing_info"]

