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
 file: vpe_flow_pyocl_ops_example_1.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/28/2012
 description:
    vpe flow example demonstrating use of flow.filters.pyocl_ops.

"""


from flow import *
from flow.filters import pyocl_ops


def setup_workspace():
    w = Workspace()
    w.register_filters(pyocl_ops)
    ctx  = w.add_context("pyocl_ops","root")
    ctx.start()
    ctx.add_filter("decompose","dudx",{"index":0})
    ctx.add_filter("decompose","dudy",{"index":1})
    ctx.add_filter("decompose","dudz",{"index":2})
    ctx.add_filter("decompose","dvdx",{"index":0})
    ctx.add_filter("decompose","dvdy",{"index":1})
    ctx.add_filter("decompose","dvdz",{"index":2})
    ctx.add_filter("decompose","dwdx",{"index":0})
    ctx.add_filter("decompose","dwdy",{"index":1})
    ctx.add_filter("decompose","dwdz",{"index":2})
    ctx.add_filter("grad","du")
    ctx.add_filter("grad","dv")
    ctx.add_filter("grad","dw")
    ctx.add_filter("sub","vort_x")
    ctx.add_filter("sub","vort_y")
    ctx.add_filter("sub","vort_z")
    ctx.add_filter("mult","vort_x_sq")
    ctx.add_filter("mult","vort_y_sq")
    ctx.add_filter("mult","vort_z_sq")
    ctx.add_filter("add","vort_add_1")
    ctx.add_filter("add","vort_add")
    ctx.add_filter("sqrt","vort_sqrt")
    ctx.connect(":x","du:x")
    ctx.connect(":y","du:y")
    ctx.connect(":z","du:z")
    ctx.connect(":dims","du:dims")
    ctx.connect(":vx","du:in")
    ctx.connect(":x","dv:x")
    ctx.connect(":y","dv:y")
    ctx.connect(":z","dv:z")
    ctx.connect(":dims","dv:dims")
    ctx.connect(":vy","dv:in")
    ctx.connect(":x","dw:x")
    ctx.connect(":y","dw:y")
    ctx.connect(":z","dw:z")
    ctx.connect(":dims","dw:dims")
    ctx.connect(":vz","dw:in")
    # ctx.connect("du","dudx:in")
    ctx.connect("du","dudy:in")
    ctx.connect("du","dudz:in")
    ctx.connect("dv","dvdx:in")
    # ctx.connect("dv","dvdy:in")
    ctx.connect("dv","dvdz:in")
    ctx.connect("dw","dwdx:in")
    ctx.connect("dw","dwdy:in")
    # ctx.connect("dw","dwdz:in")
    ctx.connect("dwdy","vort_x:in_a")
    ctx.connect("dvdz","vort_x:in_b")
    ctx.connect("dudz","vort_y:in_a")
    ctx.connect("dwdx","vort_y:in_b")
    ctx.connect("dvdx","vort_z:in_a")
    ctx.connect("dudy","vort_z:in_b")
    ctx.connect("vort_x","vort_x_sq:in_a")
    ctx.connect("vort_x","vort_x_sq:in_b")
    ctx.connect("vort_y","vort_y_sq:in_a")
    ctx.connect("vort_y","vort_y_sq:in_b")
    ctx.connect("vort_z","vort_z_sq:in_a")
    ctx.connect("vort_z","vort_z_sq:in_b")
    ctx.connect("vort_x_sq","vort_add_1:in_a")
    ctx.connect("vort_y_sq","vort_add_1:in_b")
    ctx.connect("vort_add_1","vort_add:in_a")
    ctx.connect("vort_z_sq","vort_add:in_b")
    ctx.connect("vort_add","vort_sqrt:in")
    return w


