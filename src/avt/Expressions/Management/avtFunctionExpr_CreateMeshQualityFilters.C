// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtExprNode.h>
#include <avtCornerAngle.h>
#include <avtMinCornerArea.h>
#include <avtMinSinCorner.h>
#include <avtEdgeLength.h>
#include <avtFacePlanarity.h>
#include <avtNeighborExpression.h>
#include <avtNodeDegreeExpression.h>
#include <avtRevolvedSurfaceArea.h>
#include <avtRevolvedVolume.h>
#include <avtSideVolume.h>
#include <avtVMetricArea.h>
#include <avtVMetricAspectGamma.h>
#include <avtVMetricAspectRatio.h>
#include <avtVMetricCondition.h>
#include <avtVMetricDiagonalRatio.h>
#include <avtVMetricMinDiagonal.h>
#include <avtVMetricMaxDiagonal.h>
#include <avtVMetricDimension.h>
#include <avtVMetricJacobian.h>
#include <avtVMetricMaximumAngle.h>
#include <avtVMetricMinimumAngle.h>
#include <avtVMetricOddy.h>
#include <avtVMetricRelativeSize.h>
#include <avtVMetricScaledJacobian.h>
#include <avtVMetricShape.h>
#include <avtVMetricShapeAndSize.h>
#include <avtVMetricShear.h>
#include <avtVMetricSkew.h>
#include <avtVMetricStretch.h>
#include <avtVMetricTaper.h>
#include <avtVMetricVolume.h>
#include <avtVMetricWarpage.h>

#include <string>

// ****************************************************************************
// Method: avtFunctionExpr::CreateMeshQualityFilters
//
// Purpose: 
//   Creates mesh quality filters.
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
//    Mark C. Miller, Wed Jun  3 14:50:50 PDT 2009
//    Added face_planarity expressions.
//
//    Hank Childs, Fri Feb  4 14:00:21 PST 2011
//    Place external_node in misc.  It belongs better there.
//
//    Matthew Wheeler, Mon 20 May 12:00:00 GMT 2013
//    Added min_corner_area and min_sin_corner expressions.
//
// ****************************************************************************

avtExpressionFilter *
avtFunctionExpr::CreateMeshQualityFilters(const std::string &functionName) const
{
    avtExpressionFilter *f = 0;

    if (functionName == "neighbor")
        f = new avtNeighborExpression();
    else if (functionName == "node_degree")
        f = new avtNodeDegreeExpression();
    // Begin Verdict Metrics
    else if (functionName == "area")
        f = new avtVMetricArea();
    else if (functionName == "aspect")
        f = new avtVMetricAspectRatio();
    else if (functionName == "skew")
        f = new avtVMetricSkew();
    else if (functionName == "taper")
        f = new avtVMetricTaper();
    else if (functionName == "volume")
        f = new avtVMetricVolume();
    else if (functionName == "volume2")
    {
        avtVMetricVolume *vol = new avtVMetricVolume();
        vol->UseVerdictHex(false);
        return vol;
    }
    else if (functionName == "min_corner_angle")
    {
        avtCornerAngle *ca = new avtCornerAngle();
        ca->SetTakeMin(true);
        return ca;
    }
    else if (functionName == "max_corner_angle")
    {
        avtCornerAngle *ca = new avtCornerAngle();
        ca->SetTakeMin(false);
        return ca;
    }
    else if (functionName == "min_edge_length")
    {
        avtEdgeLength *el = new avtEdgeLength();
        el->SetTakeMin(true);
        return el;
    }
    else if (functionName == "max_edge_length")
    {
        avtEdgeLength *el = new avtEdgeLength();
        el->SetTakeMin(false);
        return el;
    }
    else if (functionName == "min_side_volume")
    {
        avtSideVolume *sv = new avtSideVolume();
        sv->SetTakeMin(true);
        return sv;
    }
    else if (functionName == "max_side_volume")
    {
        avtSideVolume *sv = new avtSideVolume();
        sv->SetTakeMin(false);
        return sv;
    }
    else if (functionName == "stretch")
        f = new avtVMetricStretch();
    else if (functionName == "diagonal_ratio")
        f = new avtVMetricDiagonalRatio();
    else if (functionName == "max_diagonal")
        f = new avtVMetricMaxDiagonal();
    else if (functionName == "min_diagonal")
        f = new avtVMetricMinDiagonal();
    else if (functionName == "dimension")
        f = new avtVMetricDimension();
    else if (functionName == "oddy")
        f = new avtVMetricOddy();
    else if (functionName == "condition")
        f = new avtVMetricCondition();
    else if (functionName == "jacobian")
        f = new avtVMetricJacobian();
    else if (functionName == "scaled_jacobian")
        f = new avtVMetricScaledJacobian();
    else if (functionName == "shear")
        f = new avtVMetricShear();
    else if (functionName == "shape")
        f = new avtVMetricShape();
    else if (functionName == "relative_size")
        f = new avtVMetricRelativeSize();
    else if (functionName == "shape_and_size")
        f = new avtVMetricShapeAndSize();
    else if (functionName == "aspect_gamma")
        f = new avtVMetricAspectGamma();
    else if (functionName == "warpage")
        f = new avtVMetricWarpage();
    else if (functionName == "maximum_angle")
        f = new avtVMetricMaximumAngle();
    else if (functionName == "minimum_angle")
        f = new avtVMetricMinimumAngle();
    else if (functionName == "revolved_volume")
        f = new avtRevolvedVolume;
    else if (functionName == "revolved_surface_area")
        f = new avtRevolvedSurfaceArea;
    else if (functionName == "face_planarity")
    {
        avtFacePlanarity *fp = new avtFacePlanarity();
        return fp;
    }
    else if (functionName == "relative_face_planarity")
    {
        avtFacePlanarity *fp = new avtFacePlanarity();
        fp->SetTakeRelative(true);
        return fp;
    }

    else if (functionName == "min_corner_area")
    {
        avtMinCornerArea *ca = new avtMinCornerArea();
        ca->SetOrderCCW(true);
        return ca;
    }
    else if (functionName == "min_sin_corner")
    {
        avtMinSinCorner *ca  = new avtMinSinCorner();
        ca->SetOrderCCW(true);
        return ca;
    }
    else if (functionName == "min_sin_corner_cw")
    {
        avtMinSinCorner *ca  = new avtMinSinCorner();
        ca->SetOrderCCW(false);
        return ca;
    }

    return f;
}
