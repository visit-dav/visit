# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

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

from .common import VisItException, require_visit

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
        print("[Expression: %s = '%s']" % (ename, edef))

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
        print("[Created Python Expression: %s ]" % ename)


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
