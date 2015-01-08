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


from common import VisItException, require_visit


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
        print "[Query: %s(%s)]" % (qname,str(args))
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
        print "[PythonQuery]"
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
