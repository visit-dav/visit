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
 file: vpe_flow_npy_ops_example_1.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/28/2012
 description:
    vpe flow example demonstrating use of flow.filters.npy_ops.

"""


from flow import *
from flow.filters import npy_ops


def setup_workspace():
    w = Workspace()
    w.register_filters(npy_ops)
    w.add_filter("compose","coord_comp_1")
    w.add_filter("compose","coords")
    w.add_filter("decompose","dudx",{"index":0})
    w.add_filter("decompose","dudy",{"index":1})
    w.add_filter("decompose","dudz",{"index":2})
    w.add_filter("decompose","dvdx",{"index":0})
    w.add_filter("decompose","dvdy",{"index":1})
    w.add_filter("decompose","dvdz",{"index":2})
    w.add_filter("decompose","dwdx",{"index":0})
    w.add_filter("decompose","dwdy",{"index":1})
    w.add_filter("decompose","dwdz",{"index":2})
    w.add_filter("grad","du")
    w.add_filter("grad","dv")
    w.add_filter("grad","dw")
    w.add_filter("sub","vort_x")
    w.add_filter("sub","vort_y")
    w.add_filter("sub","vort_z")
    w.add_filter("mult","vort_x_sq")
    w.add_filter("mult","vort_y_sq")
    w.add_filter("mult","vort_z_sq")
    w.add_filter("add","vort_add_1")
    w.add_filter("add","vort_add")
    w.add_filter("sqrt","vort_sqrt")
    w.connect(":x","coord_comp_1:in_a")
    w.connect(":y","coord_comp_1:in_b")
    w.connect("coord_comp_1","coords:in_a")
    w.connect(":z","coords:in_b")
    w.connect("coords","du:coords")
    w.connect(":dims","du:dims")
    w.connect(":vx","du:in")
    w.connect("coords","dv:coords")
    w.connect(":dims","dv:dims")
    w.connect(":vy","dv:in")
    w.connect("coords","dw:coords")
    w.connect(":dims","dw:dims")
    w.connect(":vz","dw:in")
    # w.connect("du","dudx:in")
    w.connect("du","dudy:in")
    w.connect("du","dudz:in")
    w.connect("dv","dvdx:in")
    # w.connect("dv","dvdy:in")
    w.connect("dv","dvdz:in")
    w.connect("dw","dwdx:in")
    w.connect("dw","dwdy:in")
    # w.connect("dw","dwdz:in")
    w.connect("dwdy","vort_x:in_a")
    w.connect("dvdz","vort_x:in_b")
    w.connect("dudz","vort_y:in_a")
    w.connect("dwdx","vort_y:in_b")
    w.connect("dvdx","vort_z:in_a")
    w.connect("dudy","vort_z:in_b")
    w.connect("vort_x","vort_x_sq:in_a")
    w.connect("vort_x","vort_x_sq:in_b")
    w.connect("vort_y","vort_y_sq:in_a")
    w.connect("vort_y","vort_y_sq:in_b")
    w.connect("vort_z","vort_z_sq:in_a")
    w.connect("vort_z","vort_z_sq:in_b")
    w.connect("vort_x_sq","vort_add_1:in_a")
    w.connect("vort_y_sq","vort_add_1:in_b")
    w.connect("vort_add_1","vort_add:in_a")
    w.connect("vort_z_sq","vort_add:in_b")
    w.connect("vort_add","vort_sqrt:in")
    return w


