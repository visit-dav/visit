// ************************************************************************* //
//                             avtSurfaceFilter.C                            //
// ************************************************************************* //

#include <avtSurfaceFilter.h>

#include <math.h>

#include <vtkCellDataToPointData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>
#include <vtkPointSet.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetWriter.h>
#include <vtkSurfaceFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkUniqueFeatureEdges.h>
#include <vtkGeometryFilter.h>

#include <avtDatasetExaminer.h>

#include <InvalidDimensionsException.h>
#include <InvalidLimitsException.h>
#include <DebugStream.h>


// ****************************************************************************
//  Method: avtSurfaceFilter constructor
//
//  Arguments:
//      a       The attributed group with which to set the atts. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 05, 2001.
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Oct  2 17:34:53 PDT 2001
//    Intialize new members.
//
// ****************************************************************************

avtSurfaceFilter::avtSurfaceFilter(const AttributeGroup *a)
{
    atts = *(SurfaceAttributes*)a;
    filter       = vtkSurfaceFilter::New();
    cd2pd        = vtkCellDataToPointData::New();
    geoFilter    = vtkGeometryFilter::New();
    appendFilter = vtkAppendPolyData::New();
    edgesFilter  = vtkUniqueFeatureEdges::New();

    edgesFilter->ManifoldEdgesOn();
    edgesFilter->NonManifoldEdgesOff();
    edgesFilter->FeatureEdgesOff();
    edgesFilter->BoundaryEdgesOff();

    appendFilter->UserManagedInputsOn();
    appendFilter->SetNumberOfInputs(2);

    stillNeedExtents = true;
    min = -1;
    max = -1;
    Ms = 1.;
    Bs = 0.;
}


// ****************************************************************************
//  Method: avtSurfaceFilter destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 05, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Oct  2 17:34:53 PDT 2001
//    Delete new members.
//
// ****************************************************************************

avtSurfaceFilter::~avtSurfaceFilter()
{
    cd2pd->Delete();
    cd2pd = NULL;

    filter->Delete();
    filter = NULL;

    geoFilter->Delete();
    geoFilter = NULL;

    appendFilter->Delete();
    appendFilter = NULL;

    edgesFilter->Delete();
    edgesFilter = NULL;
}


// ****************************************************************************
//  Method:  avtSurfaceFilter::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 05, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jun 12 13:54:14 PDT 2001
//    Changed return value to avtFilter.
//
// ****************************************************************************

avtFilter *
avtSurfaceFilter::Create(const AttributeGroup *atts)
{
    return new avtSurfaceFilter(atts);
}


// ****************************************************************************
//  Method: avtSurfaceFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtSurfaceFilter with the given
//      parameters would result in an equivalent avtSurfaceFilter.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 05, 2001 
//
//  Modifications:
//
// ****************************************************************************

bool
avtSurfaceFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(SurfaceAttributes*)a);
}


// ****************************************************************************
//  Method: avtSurfaceFilter::ExecuteData
//
//  Purpose:
//      Maps this input 2d dataset to a 3d dataset by setting z coordinates
//      to a scaled version of the point/cell data.
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label as a string.
//
//  Returns:      The output dataset. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 05, 2001 
//
//  Modifications:
//
//    Hank Childs, Tue Mar 20 08:58:14 PST 2001
//    Use input's centering rather than guessing.
//
//    Kathleen Bonnell, Tue Mar 20 09:48:07 PST 2001
//    Modified so that output (outUG) is created with 'New'.  Changed so
//    that scalar range and spatial extents are calculated from input 
//    avtDataset (input) instead of the input vtkDataset (inDS), so that 
//    values are correct across multiple domains.
//
//    Hank Childs, Sun Mar 25 11:44:12 PST 2001
//    Account for new interface with data object information.
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001 
//    Implemented skew scaling. 
//
//    Kathleen Bonnell, Tue Apr 10 11:46:11 PDT 2001 
//    Renamed method as ExecuteData. 
//
//    Kathleen Bonnell, Wed Jun 13 11:46:02 PDT 2001
//    Preserve ghost-cell information if present in input dataset.
//
//    Hank Childs, Fri Sep  7 18:56:02 PDT 2001
//    Use doubles instead of floats.
//
//    Kathleen Bonnell, Tue Oct  2 17:34:53 PDT 2001 
//    Moved calculation of scale factors to method CalculateScaleValues. 
//    Added filters to extract edges after surface is constructed, needed
//    for rendering wireframe correctly.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray 
//    and vtkFloatArray instead.
//
// ****************************************************************************

vtkDataSet *
avtSurfaceFilter::ExecuteData(vtkDataSet *inDS, int, std::string)
{
    vtkDataArray *inScalars = NULL;
    vtkFloatArray *outScalars  = vtkFloatArray::New();
    outScalars->SetNumberOfComponents(1);
    vtkDataSet *outDS;
  
    if (GetInput()->GetInfo().GetAttributes().GetCentering() == AVT_ZONECENT)
    {
        //
        // The input is zone-centered, but this filter needs
        // node-centered data, so put it through a filter.
        //
        cd2pd->SetInput(inDS);

        // 
        // Prepare the output for the filter.
        // 
        switch (inDS->GetDataObjectType())
        {
            case VTK_RECTILINEAR_GRID :
                outDS = vtkRectilinearGrid::New(); break;
            case VTK_STRUCTURED_GRID :
                outDS = vtkStructuredGrid::New(); break;
            case VTK_UNSTRUCTURED_GRID :
                outDS = vtkUnstructuredGrid::New(); break;
            case VTK_POLY_DATA :
                outDS = vtkPolyData::New(); break;
            default : 
                debug5 << "Filter not set up to handle that input type! ("
                       << inDS->GetDataObjectType() << ").  Returning NULL"
                       << endl;
                return NULL;
        }
        cd2pd->SetOutput(outDS);
        cd2pd->Update();

        inScalars = cd2pd->GetOutput()->GetPointData()->GetScalars();
    }
    else 
    {
        inScalars = inDS->GetPointData()->GetScalars();
        outDS = inDS;
    }

    // Convert the scalars point data based on the scaling factors

    double zVal;
    int numScalars = inScalars->GetNumberOfTuples();
    outScalars->SetNumberOfTuples(numScalars);

    for (int i = 0; i < numScalars; i++)
    {
        // calculate  and store zVals
        zVal = inScalars->GetTuple1(i);
         
        if (atts.GetScaling() == SurfaceAttributes::Log)
        {
            // min & max may have been set by user to be pos values
            // but individual data values are not guaranteed to fall
            // within that range, so check for non-positive data values
            // or log won't work.   
            if (zVal <= 0)
            {
                // The current minimum is the log of the original minimum,
                // so reverse the process to set the value. 
                zVal = pow(10.0, min);
            }
            zVal = log10(zVal);
        }
        else if (atts.GetScaling() == SurfaceAttributes::Skew)
        {
             zVal = SkewTheValue(zVal); 
        }
        zVal = Ms * zVal + Bs;
        outScalars->SetValue(i, zVal);
    }

    vtkUnstructuredGrid *outUG = vtkUnstructuredGrid::New(); 

    // call the vtk filter to create the output dataset.
    filter->SetInput(outDS);
    filter->SetinScalars(outScalars);
    filter->SetOutput(outUG);
    filter->Update();

    // We want to preserve ghost-cell information, if present.
    vtkDataArray *ghosts = inDS->GetCellData()->GetArray("vtkGhostLevels");
    if (ghosts)
    {
        outUG->GetCellData()->AddArray(ghosts);
        // remove ghost-point data possibly set by cd2pd filter.
        outUG->GetPointData()->RemoveArray("vtkGhostLevels");
    }

    outScalars->Delete();


    // now we want to extract the edges for correct wireframe rendering.
    
    geoFilter->SetInput(outUG);
    edgesFilter->SetInput(geoFilter->GetOutput());

    appendFilter->SetInputByNumber(0, geoFilter->GetOutput());
    appendFilter->SetInputByNumber(1, edgesFilter->GetOutput());
 
    vtkPolyData *outPolys = vtkPolyData::New();
    appendFilter->SetOutput(outPolys);
    appendFilter->Update();

    outUG->Delete();
    return (vtkDataSet*) outPolys;
}


// ****************************************************************************
//  Method: avtSurfaceFilter::SkewTheValue
//
//  Purpose:
//      Takes the value and skews it according to the skewfactor. 
//
//  Arguments:
//      val   The value to be skewed.
//
//  Returns:
//      The value after the skew function has been applied to it.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 29, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Oct  2 17:34:53 PDT 2001
//    Removed arguments that can be accessed directly in this method.
//
//    Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002  
//    Handle max==min to avoid divide by zero errors. 
//
// ****************************************************************************

double
avtSurfaceFilter::SkewTheValue(const double val) 
{
    double skew = atts.GetSkewFactor();
    if (skew <= 0. || skew == 1.)
    {
        return val;
    }

    double range = (max == min ) ? 1. : max - min ;
    double rangeInverse = 1. / range;
    double logSkew = log(skew);
    double k = range / (skew -1.);
    double v2 = (val - min) * rangeInverse;
    double v = k * (exp(v2 * logSkew) -1.) + min;
    return v;
}


// ****************************************************************************
//  Method: avtSurfaceFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Reflect how the surface filter changes a data object.
//
//  Programmer: Hank Childs
//  Creation:   June 12, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Aug 20 17:03:17 PDT 2001
//    Remove original spatial extents since they no longer apply.
//
//    Hank Childs, Tue Sep  4 16:14:49 PDT 2001
//    Reflect new interface for avtDataAttributes.
//
// ****************************************************************************

void
avtSurfaceFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();
    GetOutput()->GetInfo().GetAttributes().SetSpatialDimension(3);
    GetOutput()->GetInfo().GetAttributes().SetCentering(AVT_NODECENT);
}


// ****************************************************************************
//  Method: avtSurfaceFilter::VerifyInput
//
//  Purpose:
//      Verifies that the input is 2D data, throws an exception if not. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 05, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 16:14:49 PDT 2001
//    Reflect new interface for avtDataAttributes.
//
// ****************************************************************************

void
avtSurfaceFilter::VerifyInput(void)
{
    if  (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() != 2)
    {
        EXCEPTION2(InvalidDimensionsException, "Surface", "2D");
    }
}


// ****************************************************************************
//  Method: avtSurfaceFilter::PerformRestriction
//
//  Purpose:
//    Disable dynamic load balancing if data extents cannot be retrieved now.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 2, 2001 
//
//  Modifications:
//
//    Hank Childs, Fri Mar 15 18:25:20 PST 2002
//    Account for dataset examiner.
//
// ****************************************************************************

avtPipelineSpecification_p
avtSurfaceFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    double dataExtents[2];
    double spatialExtents[6];
    if (TryDataExtents(dataExtents))
    {
        avtDataset_p input = GetTypedInput();
        avtDatasetExaminer::GetSpatialExtents(input, spatialExtents);
        CalculateScaleValues(dataExtents, spatialExtents);
        stillNeedExtents = false;
    }
    else
    {
        spec->NoDynamicLoadBalancing();
    }

    return spec;
}


// ****************************************************************************
//  Method: avtSurfaceFilter::PreExecute
//
//  Purpose:
//      Sees if we still need to set the extents.  We know that we can call
//      GetDataExtents (which requires static load balancing), since if we
//      weren't able to get the extents when performing the restriction, we
//      disabled load balancing.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 2, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Mar 15 18:25:20 PST 2002
//    Account for dataset examiner.
//
// ****************************************************************************

void
avtSurfaceFilter::PreExecute(void)
{
    if (stillNeedExtents)
    {
        double dataExtents[2];
        double spatialExtents[6];
        GetDataExtents(dataExtents);
        avtDataset_p input = GetTypedInput();
        avtDatasetExaminer::GetSpatialExtents(input, spatialExtents);

        CalculateScaleValues(dataExtents, spatialExtents);
    }
}

// ****************************************************************************
//  Method: avtSurfaceFilter::CalculateScaleValues
//
//  Purpose:
//    Calculates scaling factors (taken mostly from po_surf.c MeshTV)
//
//  Arguments:
//    de        The data extents.
//    se        The spatial extents.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 2, 2001
//
//  Modifications:
//    Kathleen Bonnell, Thu Oct 11 12:45:30 PDT 2001
//    Use min, max associated with scaling.
//
//    Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002  
//    Only one type of min/max (not scaling and coloring).
//    Added test for min > max. 
//
// ****************************************************************************

void
avtSurfaceFilter::CalculateScaleValues(double *de, double *se)
{
    double dX, dY, dZ, dXY;

    min = ( atts.GetMinFlag() ? atts.GetMin() : de[0] );
    max = ( atts.GetMaxFlag() ? atts.GetMax() : de[1] );

    if (min >= max && atts.GetMinFlag() && atts.GetMaxFlag())
    {
        EXCEPTION1(InvalidLimitsException, false); 
    }
    else if (min > max && atts.GetMinFlag())
    {
        max = min;
    }
    else if (min > max && atts.GetMaxFlag())
    {
        min = max;
    }

    if ( atts.GetScaling() == SurfaceAttributes::Log )
    {
        min = log10(min);
        max = log10(max);
    }

    dX = se[1] - se[0];
    dY = se[3] - se[2];
    dZ = max - min;
    dXY = (dX > dY ? dX : dY);

    if ( 0. == dZ )
    {
        Ms = 1.;
        Bs = 0.;
    }
    else
    {
        Ms = dXY / dZ;
        Bs = - (min * dXY) / dZ;
    }
}


// ****************************************************************************
//  Method: avtSurfaceFilter::ReleaseData
//
//  Purpose:
//      Releases all problem size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
// ****************************************************************************

void
avtSurfaceFilter::ReleaseData(void)
{
    avtStreamer::ReleaseData();

    filter->SetInput(NULL);
    filter->SetOutput(NULL);
    filter->SetinScalars(NULL);
    cd2pd->SetInput(NULL);
    cd2pd->SetOutput(NULL);
    geoFilter->SetInput(NULL);
    geoFilter->SetOutput(NULL);
    geoFilter->SetLocator(NULL);
    int nInputs = appendFilter->GetNumberOfInputs();
    for (int i = nInputs-1 ; i >= 0 ; i--)
    {
        vtkPolyData *pd = appendFilter->GetInput(i);
        appendFilter->RemoveInput(pd);
    }
    appendFilter->SetOutput(NULL);
    edgesFilter->SetInput(NULL);
    edgesFilter->SetOutput(NULL);
    edgesFilter->SetLocator(NULL);
}


