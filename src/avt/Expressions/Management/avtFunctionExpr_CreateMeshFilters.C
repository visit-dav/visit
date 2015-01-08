/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <avtCoordinateExtremaExpression.h>
#include <avtCylindricalRadiusExpression.h>
#include <avtDataIdExpression.h>
#include <avtExpressionComponentMacro.h>
#include <avtFindExternalExpression.h>
#include <avtNeighborEvaluatorExpression.h>
#include <avtSurfaceNormalExpression.h>
#include <avtEdgeNormalExpression.h>
#include <avtZoneTypeLabelExpression.h>
#include <avtZoneTypeRankExpression.h>

#include <string>

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
//   Hank Childs, Mon Jun 28 06:43:20 PDT 2010
//   Add avtCoordinateExtrema.
//
//   Hank Childs, Thu Jul  8 08:11:06 PDT 2010
//   Change coordinate extrema to include polar coordinates.
//   
//   Hank Childs, Fri Feb  4 14:00:21 PST 2011
//   Add external_node and external_cell.
//
//   Jeremy Meredith, Mon Mar 10 14:35:18 EDT 2014
//   Added edge normal expressions.
//
// ****************************************************************************

avtExpressionFilter *
avtFunctionExpr::CreateMeshFilters(const std::string &functionName) const
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
    else if (functionName == "edge_normal" || 
        functionName == "point_edge_normal")
    {
        avtEdgeNormalExpression *ff = new avtEdgeNormalExpression;
        ff->DoPointNormals(true);
        return ff;
    }
    else if (functionName == "cell_edge_normal")
    {
        avtEdgeNormalExpression *ff = new avtEdgeNormalExpression;
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
    else if (functionName == "zonetype" || functionName == "zonetype_label")
    {
        return new avtZoneTypeLabelExpression();
    }
    else if (functionName == "zonetype_rank")
    {
        return new avtZoneTypeRankExpression();
    }
    else if (functionName == "min_coord")
    {
        avtCoordinateExtremaExpression *ce = new avtCoordinateExtremaExpression;
        ce->SetGetMinimum(true);
        return ce;
    }
    else if (functionName == "max_coord")
    {
        avtCoordinateExtremaExpression *ce = new avtCoordinateExtremaExpression;
        ce->SetGetMinimum(false);
        return ce;
    }
    else if (functionName == "external_node" || functionName == "external_cell")
    {
        avtFindExternalExpression *e = new avtFindExternalExpression();
        if (functionName == "external_cell")
            e->SetDoCells(true);
        else
            e->SetDoCells(false);
        return e;
    }

    return 0;
}
