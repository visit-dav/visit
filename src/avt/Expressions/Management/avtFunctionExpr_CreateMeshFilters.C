/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
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
#include <avtExprNode.h>

#include <avtCylindricalRadiusExpression.h>
#include <avtDataIdExpression.h>
#include <avtExpressionComponentMacro.h>
#include <avtNeighborEvaluatorExpression.h>
#include <avtSurfaceNormalExpression.h>
#include <avtZoneTypeExpression.h>

// ****************************************************************************
// Method: avtFunctionExpr::CreateMeshFilters
//
// Purpose: 
//   Creates mesh-related filters.
//
// Arguments:
//   functionName : The name of the expression filter to create.
//
// Returns:    An expression filter or 0 if one could not be created.
//
// Note:       
//
// Programmer: 
// Creation:   Thu May 21 08:55:58 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

avtExpressionFilter *
avtFunctionExpr::CreateMeshFilters(const string &functionName) const
{
    if (functionName == "surface_normal" || 
        functionName == "point_surface_normal")
    {
        avtSurfaceNormalExpression *ff = new avtSurfaceNormalExpression;
        ff->DoPointNormals(true);
        return ff;
    }
    else if (functionName == "cell_surface_normal")
    {
        avtSurfaceNormalExpression *ff = new avtSurfaceNormalExpression;
        ff->DoPointNormals(false);
        return ff;
    }
    else if (functionName == "zoneid")
    {
        avtDataIdExpression *ff = new avtDataIdExpression;
        ff->CreateZoneIds();
        ff->CreateLocalNumbering();
        return ff;
    }
    else if (functionName == "global_zoneid")
    {
        avtDataIdExpression *ff = new avtDataIdExpression;
        ff->CreateZoneIds();
        ff->CreateGlobalNumbering();
        return ff;
    }
    else if (functionName == "nodeid")
    {
        avtDataIdExpression *ff = new avtDataIdExpression;
        ff->CreateNodeIds();
        ff->CreateLocalNumbering();
        return ff;
    }
    else if (functionName == "global_nodeid")
    {
        avtDataIdExpression *ff = new avtDataIdExpression;
        ff->CreateNodeIds();
        ff->CreateGlobalNumbering();
        return ff;
    }
    else if (functionName == "biggest_neighbor")
    {
        avtNeighborEvaluatorExpression *ff = new avtNeighborEvaluatorExpression;
        ff->SetEvaluationType(avtNeighborEvaluatorExpression::BIGGEST_NEIGHBOR);
        return ff;
    }
    else if (functionName == "smallest_neighbor")
    {
        avtNeighborEvaluatorExpression *ff = new avtNeighborEvaluatorExpression;
        ff->SetEvaluationType(avtNeighborEvaluatorExpression::SMALLEST_NEIGHBOR);
        return ff;
    }
    else if (functionName == "neighbor_average")
    {
        avtNeighborEvaluatorExpression *ff = new avtNeighborEvaluatorExpression;
        ff->SetEvaluationType(avtNeighborEvaluatorExpression::AVERAGE_NEIGHBOR);
        return ff;
    }
    else if (functionName == "cylindrical_radius")
    {
        return new avtCylindricalRadiusExpression;
    }
    else if (functionName == "cylindrical_theta")
    {
        avtExpressionComponentMacro *ecm = new avtExpressionComponentMacro;
        ecm->SetMacro("cylindrical", 1);
        return ecm;
    }
    else if (functionName == "polar_radius")
    {
        avtExpressionComponentMacro *ecm = new avtExpressionComponentMacro;
        ecm->SetMacro("polar", 0);
        return ecm;
    }
    else if (functionName == "polar_theta")
    {
        avtExpressionComponentMacro *ecm = new avtExpressionComponentMacro;
        ecm->SetMacro("polar", 1);
        return ecm;
    }
    else if (functionName == "polar_phi")
    {
        avtExpressionComponentMacro *ecm = new avtExpressionComponentMacro;
        ecm->SetMacro("polar", 2);
        return ecm;
    }
    else if (functionName == "zonetype")
        return new avtZoneTypeExpression();

    return 0;
}
