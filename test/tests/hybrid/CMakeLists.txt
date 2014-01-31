#*****************************************************************************
#
# Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
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
#
#*****************************************************************************

SET(PY_TESTS
    cleanzonesonly
    clonecopy
    clonefirstref
    conditional
    conn_cmfe
    coord_expr
    curve_anim
    curve_expressions
    cylindrical_radius
    ddf
    defvar_anim
    expr2engine
    expr_cmfe
    expressions
    exprList
    field_operators
    image_proc
    keyframe
    lineout
    locktime
    locus
    math_expr
    matsel
    matvf
    mesh_quality
    mir_cache
    mir
    missingdata
    multivar
    pos_cmfe
    py_expr_script_00
    py_exprs
    replace
    selections
    sil
    simplify_mixed
    specmf
    symm
    time_cmfe
    time_deriv_cmfe
    time_iteration
    timelock
    val4mat
    )

ADD_VISIT_PY_TESTS(Hybrid "${PY_TESTS}")
