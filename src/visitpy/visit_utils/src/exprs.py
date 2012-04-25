#*****************************************************************************
#
# Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
 file: exprs.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 4/9/2010
 description:
    Provides helper methods for creating and managing VisIt Expressions.

 examples:
    from visit_utils import *
    exprs.define("my_expr","a+b")
    exprs.define("my_vec","{a,b,0}",etype="vector")
    exprs.exists("my_expr")
    exprs.delete("my_expr")
    exprs.clear()

"""

from common import VisItException, require_visit

try:
    import visit
except:
    pass

@require_visit
def define(ename,edef,etype ='scalar',echo = False):
    """
    Defines a new expression.
    If expression with given name already exists the new
    definition will replace it.
    """
    etype = etype.lower()
    if not etype in ["scalar","vector","curve","array","tensor"]:
        raise VisItException("Unsupported expression type: %s" % etype)
    delete(ename)
    if exists(ename):
        raise VisItException("Cannot redefine database expression: %s" % ename)
    if etype == "scalar":
        visit.DefineScalarExpression(ename,edef)
    elif etype == "vector":
        visit.DefineVectorExpression(ename,edef)
    elif etype == "curve":
        visit.DefineCurveExpression(ename,edef)
    elif etype == "array":
        visit.DefineArrayExpression(ename,edef)
    elif etype == "tensor":
        visit.DefineTensorExpression(ename,edef)
    if echo:
        print "[Expression: %s = '%s']" % (ename, edef)

@require_visit
def define_python(ename,args,source=None,file=None,etype ='scalar',echo = False):
    """
    Defines a new python expression.
    If expression with given name already exists the new
    definition will replace it.
    """
    etype = etype.lower()
    if not etype in ["scalar","vector","curve","array","tensor"]:
        raise VisItException("Unsupported expression type: %s" % etype)
    delete(ename)
    if exists(ename):
        raise VisItException("Cannot redefine database expression: %s" % ename)
    kwargs ={"name":ename,"args": args,"type":etype}
    if not source is None:
        kwargs["source"] = source
    if not file is None:
        kwargs["file"] = file
    visit.DefinePythonExpression(**kwargs)
    if echo:
        print "[Created Python Expression: %s ]" % ename


@require_visit
def exists(ename):
    """
    Checks if an expression with the given name exists.
    """
    expr_names = [ expr[0] for expr in visit.Expressions()]
    if ename in expr_names:
        return True
    else:
        return False

@require_visit
def delete(ename):
    """
    Deletes the expression with given name if it exists.
    """
    if exists(ename):
        try:
            visit.DeleteExpression(ename)
        except:
            # must be a database expression
            pass

@require_visit
def clear():
    """
    Deletes all existing expressions.
    """
    expr_names = [ expr[0] for expr in visit.Expressions()]
    for ename in expr_names:
        try:
            visit.DeleteExpression(ename)
        except:
            # must be a database expression
            pass
