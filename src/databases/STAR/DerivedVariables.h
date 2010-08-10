/*****************************************************************************
*
* Copyright (c) 2010, University of New Hampshire Computer Science Department
* All rights reserved.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  File:        DerivedVariables.h                                          //
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _DERIVED_VARIABLES_H_
#define _DERIVED_VARIABLES_H_

/* ========================================================================= */
/**
 *      A global table of derived variables defines each
 *      derived variable by name and an expression.  @See
 *      addDerivedVariables().
 **/

struct DerivedVariable {
    string name;
    string longName;
    string expression;
    Expression::ExprType expressionType;
} DerivedVariableTable[] = {
    { 
        "vms", 
        "Magnetosonic Speed",
        "sqrt( (21.89*sqrt((bx*bx+by*by+bz*bz)/rr))^2 + "
                "(31.68*sqrt(pp/rr))^2 )",
        Expression::ScalarMeshVar
    }, {
        "Mvms", 
        "Magnetosonic Mach Number",
        "(sqrt(vx*vx+vy*vy+vz*vz))/" 
        "(sqrt((21.89*sqrt((bx*bx+by*by+bz*bz)/rr))^2+(31.68*sqrt(pp/rr))^2))",
        Expression::ScalarMeshVar
    }, {
        "beta", 
        "Plasma Beta",
        "2.5133*pp/if( gt(1.0e-15, sqrt(bx*bx+by*by+bz*bz)), "
            "1.0e-15, sqrt(bx*bx+by*by+bz*bz) )",
        Expression::ScalarMeshVar
    }, {
        "temp", 
        "Plasma Temperature",
        "72429.0*pp/rr",
        Expression::ScalarMeshVar
    }, {
        "tkev", 
        "Plasma Temperature in keV",
        "72429.0*pp/rr/11600.0e3",
        Expression::ScalarMeshVar
    }, {
        "cs", 
        "Sound Speed",
        "31.68*sqrt(pp/rr)",
        Expression::ScalarMeshVar
    }, {
        "Mcs", 
        "Sound Mach Number",
        "sqrt(vx*vx+vy*vy+vz*vz)/(31.68*sqrt(pp/rr))",
        Expression::ScalarMeshVar
    }, {
        "vd", 
        "Ion-Electron Drift Speed",
        "(6241.0*sqrt(xjx*xjx+xjy*xjy+xjz*xjz)/rr)/"
                                "(21.89*sqrt((bx*bx+by*by+bz*bz)/rr))",
        Expression::ScalarMeshVar
    }, {
        "va", 
        "Alfven Speed",
        "21.89*sqrt( (bx*bx+by*by+bz*bz)/if( gt(1.0e-12,rr), 1.0e-12, rr) )",
        Expression::ScalarMeshVar
    }, {
        "Mva", 
        "Alfven Mach Number",
        "sqrt(vx*vx+vy*vy+vz*vz)/"
            "(21.89*sqrt((bx*bx+by*by+bz*bz)/if(gt(1.0e-12,rr),1.0e-12,rr)))",
        Expression::ScalarMeshVar
    }, {
        "vdva", 
        "Ion-Electron Drift Speed Normalized To Alfven Speed",
        "((6241.0*sqrt(xjx*xjx+xjy*xjy+xjz*xjz)/rr)/(21.89*sqrt((bx*bx+by*by+bz*bz)/rr)))/"
            "(21.89*sqrt( (bx*bx+by*by+bz*bz)/if(gt(1.0e-12,rr),1.0e-12,rr)))",
        Expression::ScalarMeshVar
    }, {
        "ent", 
        "Specific Entropy", 
        "72429.0*pp/((if(gt(1.0e-12,rr),1.0e-12,rr))^1.6667)",
        Expression::ScalarMeshVar
    }, {
        "jcb", 
        "Magnetic Force Density", 
        "{(xjy*bz-xjz*by),(xjz*bx-xjx*bz),(xjx*by-xjy*bx)}",
        Expression::VectorMeshVar
    }, {
        "jpar", 
        "Parallel Current Density", 
        "(xjx*bx+xjy*by+xjz*bz)/sqrt(bx*bx+by*by+bz*bz)",
        Expression::ScalarMeshVar
    }, {
        "pfl", 
        "Parallel Velocity", 
        "{(((bx*bx+by*by+bz*bz)*vx-(bx*vx+by*vy+bz*vz)*bx)*7.9577e-4),"
        "(((bx*bx+by*by+bz*bz)*vy-(bx*vx+by*vy+bz*vz)*by)*7.9577e-4),"
        "(((bx*bx+by*by+bz*bz)*vz-(bx*vx+by*vy+bz*vz)*bz)*7.9577e-4)}",
        Expression::VectorMeshVar
    }, {
        "e", 
        "Electric Field", 
        "{(by*vz-bz*vy)*0.001,(bz*vx-bx*vz)*0.001,(bx*vy-by*vx)*0.001}",
        Expression::VectorMeshVar
    }
};

/* ========================================================================= */

#endif // _DERIVED_VARIABLES_H_
