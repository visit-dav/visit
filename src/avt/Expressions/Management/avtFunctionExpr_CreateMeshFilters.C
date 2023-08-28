// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtExprNode.h>

#include <avtCoordinateExtremaExpression.h>
#include <avtCylindricalRadiusExpression.h>
#include <avtDataIdExpression.h>
#include <avtGhostZoneIdExpression.h>
#include <avtExpressionComponentMacro.h>
#include <avtFindExternalExpression.h>
#include <avtNeighborEvaluatorExpression.h>
#include <avtSurfaceNormalExpression.h>
#include <avtEdgeNormalExpression.h>
#include <avtZoneCentersExpression.h>
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
//   Chris Laganella, Mon Jan 31 15:43:30 EST 2022
//   Add logical node / zone id expressions.
// 
//   Justin Privitera, Wed Mar 30 12:47:07 PDT 2022
//   Added ghost_zoneid expression.
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
    else if (functionName == "ghost_zoneid")
    {
        avtGhostZoneIdExpression *ff = new avtGhostZoneIdExpression;
        ff->CreateGhostZoneIds();
        return ff;
    }
    else if (functionName == "zoneid")
    {
        avtDataIdExpression *ff = new avtDataIdExpression;
        ff->CreateZoneIds();
        ff->CreateLocalNumbering();
        return ff;
    }
    else if (functionName == "logical_zoneid")
    {
        avtDataIdExpression *ff = new avtDataIdExpression;
        ff->CreateZoneIds();
        ff->CreateLocalNumbering();
        ff->CreateIJK();
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
    else if (functionName == "logical_nodeid")
    {
        avtDataIdExpression *ff = new avtDataIdExpression;
        ff->CreateNodeIds();
        ff->CreateLocalNumbering();
        ff->CreateIJK();
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
    else if (functionName == "zone_centers")
    {
        avtZoneCentersExpression *e = new avtZoneCentersExpression();
        return e;
    }
    else if (functionName == "node_domain")
    {
        avtDataIdExpression *e = new avtDataIdExpression();
        e->CreateNodeIds();
        e->CreateDomainIds();
        e->CreateLocalNumbering();
        return e;
    }
    else if (functionName == "zone_domain")
    {
        avtDataIdExpression *e = new avtDataIdExpression();
        e->CreateZoneIds();
        e->CreateDomainIds();
        e->CreateLocalNumbering();
        return e;
    }

    return 0;
}
