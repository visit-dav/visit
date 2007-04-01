  // ************************************************************************* //
//                             avtResampleFilter.C                           //
// ************************************************************************* //

#include <float.h>

#include <avtResampleFilter.h>

#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>

#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtParallel.h>
#include <avtSamplePointExtractor.h>
#include <avtSourceFromAVTDataset.h>
#include <avtWorldSpaceToImageSpaceTransform.h>

#include <DebugStream.h>


//
// Function Prototypes
//

vtkRectilinearGrid     *CreateGrid(const double *, int, int, int);
void                    CreateViewFromBounds(avtViewInfo &, const double *,
                                             double [3]);
vtkDataArray           *GetCoordinates(float, float, int);


#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

// ****************************************************************************
//  Method: avtResampleFilter constructor
//
//  Arguments:
//      atts    The attributes the filter should use.
//
//  Programmer: Hank Childs 
//  Creation:   March 26, 2001
//
// ****************************************************************************

avtResampleFilter::avtResampleFilter(const AttributeGroup *a)
{
    atts = *(ResampleAttributes*)a;
    primaryVariable = NULL;
}


// ****************************************************************************
//  Method: avtResampleFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   November 16, 2001
//
// ****************************************************************************

avtResampleFilter::~avtResampleFilter()
{
    if (primaryVariable != NULL)
    {
        delete [] primaryVariable;
        primaryVariable = NULL;
    }
}


// ****************************************************************************
//  Method:  avtResampleFilter::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Hank Childs
//  Creation:    March 26, 2001
//
// ****************************************************************************

avtFilter *
avtResampleFilter::Create(const AttributeGroup *atts)
{
    return new avtResampleFilter(atts);
}


// ****************************************************************************
//  Method: avtResampleFilter::Execute
//
//  Purpose:
//      Resamples the input into a rectilinear mesh output.
//
//  Returns:       The output rectilinear grid.
//
//  Programmer: Hank Childs 
//  Creation:   March 26, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Jun  6 13:18:42 PDT 2001
//    Removed domain list argument.
//
// ****************************************************************************

void
avtResampleFilter::Execute(void)
{
    if (InputNeedsNoResampling())
    {
        debug5 << "Bypassing resample" << endl;
        BypassResample();
    }
    else
    {
        debug5 << "Resampling input" << endl;
        ResampleInput();
    }
}


// ****************************************************************************
//  Method: avtResampleFilter::InputNeedsNoResampling
//
//  Purpose:
//      Determines if it is worthwhile to resample the input.  It is only
//      worthwhile if, (1) the input is truly a rectilinear grid, (2) its
//      coordinates are evenly spaced in all dimensions, and (3) the number
//      of nodes it has are somewhat near the desired number of nodes.
//
//  Programmer: Hank Childs
//  Creation:   April 6, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001
//    Reflect changes in avtDataSet, that data is stored as single 
//    avtDataTree, instead of multiple avtDomainTrees.  Treat 'domains'
//    as first level children in input tree.  avtDomain now called 
//    avtDataRepresentation.
//
//    Hank Childs, Mon Nov 19 14:47:10 PST 2001
//    Hooked back up resample bypass.
//
//    Kathleen Bonnell, Tue Nov 20 08:09:45 PST 2001 
//    Use vtkDataArray in place of vtkScalars for rgrid coordinates,
//    to match VTK 4.0 API. 
//
// ****************************************************************************

bool
avtResampleFilter::InputNeedsNoResampling(void)
{
    //
    // If a specific set of dimensions was requested, then this is not going
    // to work.
    //
    if (!atts.GetUseTargetVal())
    {
        debug5 << "Must resample input because it is not using a target value"
               << endl;
        return false;
    }

    //
    // If there is more than one domain, GetSingleLeaf will return NULL.
    //
    avtDataTree_p inDT = GetInputDataTree();
    vtkDataSet *in_ds = inDT->GetSingleLeaf();
    if (in_ds == NULL)
    {
        debug5 << "Must resample the input because there are multiple domains."
               << endl;
        return false;
    }

    if (in_ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        debug5 << "Must resample input because the input is not rectilinear."
               << endl;
        return false;
    }

    vtkRectilinearGrid *rg = (vtkRectilinearGrid *) in_ds;
    vtkDataArray *x = rg->GetXCoordinates();
    vtkDataArray *y = rg->GetYCoordinates();
    vtkDataArray *z = rg->GetZCoordinates();

    double diff = x->GetComponent(1, 0) - x->GetComponent(0, 0);
    int  i;

    int nX = x->GetNumberOfTuples();
    int nY = y->GetNumberOfTuples();
    int nZ = z->GetNumberOfTuples();

    for (i = 0 ; i < nX-1 ; i++)
    {
        double thisDiff = x->GetComponent(i+1, 0) - x->GetComponent(i, 0);
        if (thisDiff < 0.9*diff || thisDiff > 1.1*diff)
        {
            debug5 << "Must resample input because the voxels are not cubes"
                   << endl;
            return false;
        }
    }

    for (i = 0 ; i < nY-1 ; i++)
    {
        double thisDiff = y->GetComponent(i+1, 0) - y->GetComponent(i, 0);
        if (thisDiff < 0.9*diff || thisDiff > 1.1*diff)
        {
            debug5 << "Must resample input because the voxels are not cubes"
                   << endl;
            return false;
        }
    }

    for (i = 0 ; i < nZ-1 ; i++)
    {
        double thisDiff = z->GetComponent(i+1, 0) - z->GetComponent(i, 0);
        if (thisDiff < 0.9*diff || thisDiff > 1.1*diff)
        {
            debug5 << "Must resample input because the voxels are not cubes"
                   << endl;
            return false;
        }
    }

    int numVals = nX*nY*nZ;
    if (numVals > 2*atts.GetTargetVal())
    {
        debug5 << "Must resample input because there are too many voxels ("
               << numVals << ")" << endl;
        return false;
    }
    if (numVals*2 < atts.GetTargetVal())
    {
        debug5 << "Must resample input because there are too few voxels ("
               << numVals << ")" << endl;
        return false;
    }

    return true;
}


// ****************************************************************************
//  Method: avtResampleFilter::BypassResample
//
//  Purpose:
//      Bypasses the resampling.  This will assign the output to be the same
//      as the input.
//
//  Programmer: Hank Childs
//  Creation:   April 6, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001
//    Reflect that input and output stored as single avtDataTree instead
//    of multiple avtDomainTrees.
//
// ****************************************************************************

void
avtResampleFilter::BypassResample(void)
{
    SetOutputDataTree(GetInputDataTree());
}


// ****************************************************************************
//  Method: avtResampleFilter::ResampleInput
//
//  Purpose:
//      Resamples the input.
//
//  Programmer: Hank Childs
//  Creation:   April 6, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Apr  5 15:20:40 PDT 2001
//    Scale the dataset back so we can resample with different x and y bounds,
//    also make all of the cells be approximately squares.
//
//    Hank Childs, Fri Apr  6 17:41:30 PDT 2001
//    Pulled code out of Execute and put it in this routine since the input
//    does not always need to be resampled.
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001
//    Reflect that input and output stored as single avtDataTree instead
//    of multiple avtDomainTrees.
//
//    Kathleen Bonnell, Mon Apr 23 13:26:00 PDT 2001
//    Forced the output of this filter to be an avtDataTree with the same
//    number of children as the input, in order for it to work correctly 
//    with domain lists elsewhere in the pipeline.
//  
//    Hank Childs, Tue Apr 24 16:54:51 PDT 2001
//    Send the data extents down before resampling.
//
//    Hank Childs, Mon Jun 18 08:04:17 PDT 2001
//    Work in parallel.
//
//    Hank Childs, Tue Sep  4 15:12:40 PDT 2001
//    Reflect new extent interface.
//
//    Hank Childs, Tue Nov 13 13:09:46 PST 2001
//    Use effective extents if they are available.
//
//    Hank Childs, Wed Nov 14 16:42:17 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Wed Nov 28 12:46:49 PST 2001 
//    Do not send down a NULL data tree when we have no data, send a dummy
//    instead.
//
//    Hank Childs, Tue Feb  5 17:23:31 PST 2002
//    Add support for processors that do not receive data when run in parallel.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Brad Whitlock, Thu Apr 4 14:49:01 PST 2002
//    Changed CopyTo so it is an inline template function.
//
//    Hank Childs, Mon Jul  7 22:31:00 PDT 2003
//    Copy over whether or not an error occurred in resampling.
//
//    Hank Childs, Wed Jul 23 15:33:14 PDT 2003
//    Make accomodations for not sampling avt and vtk variables.
//
//    Hank Childs, Fri Aug  8 15:34:27 PDT 2003
//    Make sure that we can get good variable names on the root processor.
//
// ****************************************************************************

void
avtResampleFilter::ResampleInput(void)
{
    int  i, j;

    avtDataset_p output = GetTypedOutput();
    double bounds[6];
    avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
    avtExtents *exts = datts.GetEffectiveSpatialExtents();
    if (exts->HasExtents())
    {
        exts->CopyTo(bounds);
    }
    else
    {
        GetSpatialExtents(bounds);
    }

    debug4 << "Resampling over space: " << bounds[0] << ", " << bounds[1]
           << ": " << bounds[2] << ", " << bounds[3] << ": " << bounds[4]
           << ", " << bounds[5] << endl;
    
    //
    // Our resampling leaves some invalid values in the data range.  The
    // easiest way to bypass this is to get the data range from the input and
    // pass it along (since resampling does not change it in theory).
    //
    double range[2];
    GetDataExtents(range);
    output->GetInfo().GetAttributes().GetEffectiveDataExtents()->Set(range);

    avtViewInfo view;
    double scale[3];
    CreateViewFromBounds(view, bounds, scale);

    //
    // We don't want an Update to go all the way up the pipeline, so make
    // a terminating source corresponding to our input.
    //
    avtDataset_p ds;
    avtDataObject_p dObj = GetInput();
    CopyTo(ds, dObj);
    avtSourceFromAVTDataset termsrc(ds);

    //
    // The sample point extractor expects everything to be in image space.
    //
    avtWorldSpaceToImageSpaceTransform trans(view, scale);
    trans.SetInput(termsrc.GetOutput());

    //
    // What we want the width, height, and depth to be depends on the
    // attributes.
    //
    int width, height, depth;
    GetDimensions(width, height, depth, bounds);

    avtSamplePointExtractor extractor(width, height, depth);
    extractor.SetInput(trans.GetOutput());
    
    //
    // Since this is Execute, forcing an update is okay...
    //
    avtSamplePoints_p samples = extractor.GetTypedOutput();
    samples->Update(GetGeneralPipelineSpecification());

    if (samples->GetInfo().GetValidity().HasErrorOccurred())
    {
        GetOutput()->GetInfo().GetValidity().ErrorOccurred();
        GetOutput()->GetInfo().GetValidity().SetErrorMessage(
                          samples->GetInfo().GetValidity().GetErrorMessage());
    }

    //
    // Create a rectilinear dataset that is stretched according to the 
    // original bounds.
    //
    vtkRectilinearGrid *rg = CreateGrid(bounds, width, height, depth);
    VarList vl;
    vl.nvars = -1;
    avtDatasetExaminer::GetVariableList(ds, vl);

    //
    // This is hack-ish.  The sample point extractor throws out variables
    // that have "avt" or "vtk" in them.  The returned sample points don't
    // have variable names, so we need to match up the variables from the
    // input dataset.
    //
    int myVars = 0;
    for (i = 0 ; i < vl.nvars ; i++)
        if ((strstr(vl.varnames[i].c_str(), "vtk") == NULL) &&
            (strstr(vl.varnames[i].c_str(), "avt") == NULL))
            myVars++;

    //
    // If we have more processors than domains, we have to handle that
    // gracefully.  Communicate how many variables there are so that those
    // that don't have data can play well.
    //
    int effectiveVars = UnifyMaximumValue(myVars);
    vtkDataArray **vars = new vtkDataArray*[effectiveVars];
    for (i = 0 ; i < effectiveVars ; i++)
    {
        vars[i] = vtkFloatArray::New();
    }
    if (myVars <= 0)
    {
        //
        // No variables -- this is probably because we didn't get any domains
        // to process on this processor.
        //
        for (i = 0 ; i < effectiveVars ; i++)
        {
            int numVals = width*height*depth;
            vars[i]->SetNumberOfTuples(numVals);
            for (j = 0 ; j < numVals ; j++)
            {
                vars[i]->SetTuple1(j, atts.GetDefaultVal());
            }
        }
    }
    else
    {
        samples->GetVolume()->GetVariables(atts.GetDefaultVal(), vars);
    }

    //
    // Collect will perform the parallel collection.  Does nothing in serial.
    // This will only be valid on processor 0.
    //
    bool iHaveData = false;
    for (i = 0 ; i < effectiveVars ; i++)
    {
        float *ptr = (float *) vars[i]->GetVoidPointer(0);
        iHaveData = Collect(ptr, width*height*depth);
    }

    std::vector<std::string> varnames;
    for (i = 0 ; i < vl.nvars ; i++)
    {
        const char *varname = vl.varnames[i].c_str();
        if ((strstr(varname, "vtk") == NULL) &&
            (strstr(varname, "avt") == NULL))
           varnames.push_back(varname);
    }
    GetListToRootProc(varnames, effectiveVars);

    if (iHaveData)
    {
        //
        // Attach this variable to our rectilinear grid.
        //
        int varsSeen = 0;
        for (i = 0 ; i < effectiveVars ; i++)
        {
            const char *varname = varnames[i].c_str();
            vars[varsSeen]->SetName(varname);
            if (strcmp(varname, primaryVariable) == 0)
                rg->GetPointData()->SetScalars(vars[varsSeen]);
            else
                rg->GetPointData()->AddArray(vars[varsSeen]);
            varsSeen++;
        }

        avtDataTree_p tree = new avtDataTree(rg, 0);
        SetOutputDataTree(tree);
    }
    else
    {
        //
        // Putting in a NULL data tree can lead to seg faults, etc.
        //
        avtDataTree_p dummy = new avtDataTree();
        SetOutputDataTree(dummy);
    }

    for (i = 0 ; i < effectiveVars ; i++)
    {
        vars[i]->Delete();
    }
    delete [] vars;
}


// ****************************************************************************
//  Method: avtResampleFilter::GetDimensions
//
//  Purpose:
//      Determines what the dimensions should be from the attributes.
//
//  Arguments:
//      width        The desired width.
//      height       The desired height.
//      depth        The desired depth.
//      bounds       The bounds of the dataset.
//
//  Programmer: Hank Childs
//  Creation:   April 5, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 15:12:40 PDT 2001
//    Made a cast to remove compiler warnings with g++.
//
//    Hank Childs, Tue Feb  5 09:42:22 PST 2002
//    Do not blow up memory if we get faked up bounds.  Changed bounds to
//    double.
//
//    Jeremy Meredith, Thu Oct  2 12:52:42 PDT 2003
//    Added ability to preferentially choose power-of-two sized dimensions.
//
//    Eric Brugger, Tue Jul 27 08:48:58 PDT 2004
//    Add several casts to fix compile errors.
//
// ****************************************************************************

void
avtResampleFilter::GetDimensions(int &width, int &height, int &depth,
                                 const double *bounds)
{
    if (atts.GetUseTargetVal())
    {
        double ratioX = 1.;
        double ratioY = 1.;
        double ratioZ = 1.;

        if (bounds[0] != +DBL_MAX && bounds[1] != -DBL_MAX &&
            bounds[2] != +DBL_MAX && bounds[3] != -DBL_MAX &&
            bounds[4] != +DBL_MAX && bounds[5] != -DBL_MAX)
        {
            //
            // Classic algebra problem -- we know the volume of the cube and 
            // the ratio of the sides, but not the actual length of any of the 
            // sides.  Start off by determining the ratios and the put
            // everything in terms of the number of sample points in the width.
            // Once that is solved, everything falls out.
            //
            float X = bounds[1] - bounds[0];
            float Y = bounds[3] - bounds[2];
            float Z = bounds[5] - bounds[4];

            ratioX = 1.;
            ratioY = Y / X;
            ratioZ = Z / X;
        }

        double multiplier = ratioX * ratioY * ratioZ;
        double tmp = atts.GetTargetVal() / multiplier;
        double amountInX = pow(tmp, 0.3333333);
        width  = (int)(amountInX * ratioX);
        height = (int)(amountInX * ratioY);
        depth  = (int)(amountInX * ratioZ);

        if (atts.GetPrefersPowersOfTwo())
        {
            int w[2], h[2], d[2];
            w[1] = MAX(int(pow(2.0,1+int(log(double(width -1))/log(2.0)))),2);
            h[1] = MAX(int(pow(2.0,1+int(log(double(height-1))/log(2.0)))),2);
            d[1] = MAX(int(pow(2.0,1+int(log(double(depth -1))/log(2.0)))),2);
            w[0] = w[1]/2;
            h[0] = h[1]/2;
            d[0] = d[1]/2;

            if (width - w[0] < w[1] - width)
                width = w[0];
            else
                width = w[1];

            if (height - h[0] < h[1] - height)
                height = h[0];
            else
                height = h[1];

            if (depth - d[0] < d[1] - depth)
                depth = d[0];
            else
                depth = d[1];
        }
    }
    else
    {
        width  = atts.GetWidth();
        height = atts.GetHeight();
        depth  = atts.GetDepth();
    }
    debug5 << "Resampling onto grid of dimensions: " << width << ", "
           << height << ", " << depth << endl;
}


// ****************************************************************************
//  Function: CreateViewFromBounds
//
//  Purpose:
//      Creates a view from bounds.
//
//  Arguments:
//      view    The view to set.
//      bounds  The bounds to use.
//      scale   The proportions of the X-Y trick we play to get around VTK.
//
//  Programmer: Hank Childs
//  Creation:   March 26, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Mar 30 17:03:39 PST 2001
//    Determine how matrix should be scaled.
//
//    Hank Childs, Tue Feb  5 09:42:22 PST 2002
//    Made bounds be a double.
//
// ****************************************************************************

void
CreateViewFromBounds(avtViewInfo &view, const double *bounds, double scale[3])
{
    //
    // Put the camera one unit away from the bounding box in the z direction.
    //
    view.camera[0] = (bounds[0]+bounds[1])/2.;
    view.camera[1] = (bounds[2]+bounds[3])/2.;
    view.camera[2] = bounds[4] - 1.;

    //
    // Put the focus in the middle of the bounding box.
    //
    view.focus[0]  = (bounds[0]+bounds[1])/2.;
    view.focus[1]  = (bounds[2]+bounds[3])/2.;
    view.focus[2]  = (bounds[4]+bounds[5])/2.;

    view.viewUp[0] = 0.;
    view.viewUp[1] = 1.;
    view.viewUp[2] = 0.;

    //
    // Make the parallel scale be the larger of the difference in x and y.
    //
    view.orthographic = true;
    view.setScale     = true;
    float width  = bounds[1] - bounds[0];
    float height = bounds[3] - bounds[2];
    view.parallelScale = (width > height ? width/2. : height/2.);

    //
    // We put the camera one unit away from the bounding box, so the near
    // plane should also be one.  The far should be however far it needs
    // to be to cover the bounding box.
    //
    view.nearPlane = 1.;
    view.farPlane  = 1. + (bounds[5] - bounds[4]);

    //
    // Our module that calculates the transform uses VTK and VTK wants the
    // viewport to be square.  We can scale it after the fact.  Do that here.
    //
    scale[0] = 1.;
    scale[1] = 1.;
    scale[2] = 1.;
    if (width > height)
    {
        scale[1] = width/height;
    }
    else
    {
        scale[0] = height/width;
    }
}


// ****************************************************************************
//  Method: avtResampleFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Indicates the zones no longer correspond to the original problem.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

void
avtResampleFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
}


// ****************************************************************************
//  Method: avtResampleFilter::CreateGrid
//
//  Purpose:
//      Creates a rectilinear grid that makes sense for the bounds of the
//      original dataset.
//
//  Arguments:
//      bounds    The bounds of the original dataset.
//      numX      The number of samples in X.
//      numY      The number of samples in Y.
//      numZ      The number of samples in Z.
//
//  Programmer:   Hank Childs
//  Creation:     March 26, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Nov 19 15:31:36 PST 2001
//    Use vtkDataArray instead of vtkScalars to match VTK 4.0 API.
//
//    Hank Childs, Tue Feb  5 09:49:34 PST 2002
//    Use double for bounds.
//
// ****************************************************************************

vtkRectilinearGrid *
CreateGrid(const double *bounds, int numX, int numY, int numZ)
{
    vtkDataArray *xc = NULL;
    vtkDataArray *yc = NULL;
    vtkDataArray *zc = NULL;

    float width  = bounds[1] - bounds[0];
    float height = bounds[3] - bounds[2];
    float depth  = bounds[5] - bounds[4];

    xc = GetCoordinates(bounds[0], width, numX);
    yc = GetCoordinates(bounds[2], height, numY);
    zc = GetCoordinates(bounds[4], depth, numZ);
      
    vtkRectilinearGrid *rv = vtkRectilinearGrid::New();
    rv->SetDimensions(numX, numY, numZ);
    rv->SetXCoordinates(xc);
    xc->Delete();
    rv->SetYCoordinates(yc);
    yc->Delete();
    rv->SetZCoordinates(zc);
    zc->Delete();

    return rv;
}


// ****************************************************************************
//  Function: GetCoordinates
//
//  Purpose:
//      Creates a coordinates array based on specifications.
//
//  Arguments:
//      start    The start of the coordinates array.
//      length   The length of the coordinates array.
//      numEls   The number of elements in the coordinates array.
//
//  Returns:     A vtkScalars element for the coordinate.
//
//  Programmer:  Hank Childs
//  Creation:    March 26, 2001
//
//  Modifications:
//    Kathleen Bonnell, Mon Nov 19 15:31:36 PST 2001
//    Changes in VTK 4.0 API require use of vtkDataArray /vtkFloatArray
//    in place of vtkScalars. 
//
// ****************************************************************************

vtkDataArray *
GetCoordinates(float start, float length, int numEls)
{
    vtkFloatArray *rv = vtkFloatArray::New();

    //
    // Make sure we don't have any degenerate cases here.
    //
    if (length <= 0. || numEls <= 1)
    {
        rv->SetNumberOfValues(1);
        rv->SetValue(0, start);
        return rv;
    }

    rv->SetNumberOfValues(numEls);
    float offset = length / (numEls-1);
    for (int i = 0 ; i < numEls ; i++)
    {
        rv->SetValue(i, start + i*offset);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtResampleFilter::PerformRestriction
//
//  Purpose:
//      Indicates that we cannot do dynamic load balancing with this filter.
//
//  Programmer: Hank Childs
//  Creation:   June 17, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Nov 16 08:51:59 PST 2001
//    Capture what the primary variable is.
//
// ****************************************************************************

avtPipelineSpecification_p
avtResampleFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    spec->NoDynamicLoadBalancing();
    if (primaryVariable != NULL)
    {
        delete [] primaryVariable;
    }
    const char *pv = spec->GetDataSpecification()->GetVariable();
    primaryVariable = new char[strlen(pv)+1];
    strcpy(primaryVariable, pv);
    return spec;
}


