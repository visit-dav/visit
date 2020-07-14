# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: query.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 4/9/2010
 description:
    Provides helper methods for executing visit queries.

 examples:
    from visit_utils import *
    val  = query("Min")
    vstr = query("Min",rmode="string")

"""


from .common import VisItException, require_visit


try:
    import visit
except:
    pass

#
# Query Helpers
#
@require_visit
def query(qname,args=None,rmode="value",echo = False, msg_lvl = 2,**kwargs):
    """
    Executes a query and returns result based on 'rmode'.
     rmode={'value','string',object'}
     Raises an exception if an error occurs.
    """
    if echo:
        print("[Query: %s(%s)]" % (qname,str(args)))
    prev_lvl = visit.SuppressMessages(msg_lvl)
    if args is None:
        qres = visit.Query(qname,**kwargs)
    else:
        # call w/ args + kwargs
        cargs = [qname]
        cargs.extend(args)
        qres = visit.Query(*cargs,**kwargs)
    if qres == 0:
        visit.SuppressMessages(prev_lvl)
        raise VisItException("Execution of query: '%s' failed." % qname)
    if rmode == "value":
        res =  visit.GetQueryOutputValue()
    elif rmode == "string" or rmode == "text" or rmode == "txt":
        res = visit.GetQueryOutputString()
    elif rmode == "object":
        res =  visit.GetQueryOutputObject()
    else:
        visit.SuppressMessages(prev_lvl)
        raise VisItException("Unsupported query result mode: %s" % rmode)
    visit.SuppressMessages(prev_lvl)
    return res

#
# Query Helpers
#
@require_visit
def python_query(source=None,file=None,vars=None,args=[],rmode="value",echo = False, msg_lvl = 2):
    """
    Executes a Python Query and returns result based on 'rmode'.
     rmode={'value','string',object'}
     Rasies an exception if an error occurs.
    """
    if echo:
        print("[PythonQuery]")
    prev_lvl = visit.SuppressMessages(msg_lvl)
    kwargs = {}
    if not source is None:
        kwargs["source"] = source
    if not file is None:
        kwargs["file"] = file
    if not vars is None:
        kwargs["vars"] = vars
    if len(args) > 0:
        kwargs["args"] = args
    qres = visit.PythonQuery(**kwargs)
    if qres == 0:
        visit.SuppressMessages(prev_lvl)
        raise VisItException("Execution of PythonQuery failed.")
    if rmode == "value":
        res =  visit.GetQueryOutputValue()
    elif rmode == "string":
        res = visit.GetQueryOutputString()
    elif rmode == "object":
        res =  visit.GetQueryOutputObject()
    else:
        visit.SuppressMessages(prev_lvl)
        raise VisItException("Unsupported Python Query result mode: %s" % rmode)
    visit.SuppressMessages(prev_lvl)
    return res
