// ************************************************************************* //
//                              avtContourFilter.C                           //
// ************************************************************************* //

#include <avtContourFilter.h>

#include <limits.h>
#include <float.h>
#include <vector>

#include <vtkCellDataToPointData.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItContourFilter.h>
#include <vtkVisItScalarTree.h>

#include <avtExtents.h>
#include <avtIntervalTree.h>
#include <avtMetaData.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidLimitsException.h>
#include <NoDefaultVariableException.h>
#include <TimingsManager.h>


using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtContourFilter constructor
//
//  Arguments:
//      a       The contour operator attributes.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Aug 18 15:59:28 PDT 2000
//    Initialized cd2pd.
//
//    Jeremy Meredith, Tue Sep 19 22:26:16 PDT 2000
//    Made the avtContourFilter initialize from the raw levels and
//    create its own vtkContourFilter.
//
//    Kathleen Bonnell, Wed Feb 28 17:06:54 PST 2001 
//    Removed creation and filling of vtkContourFilter cf as it is now
//    not needed until ExecuteDomainTree method.
//
//    Hank Childs, Mon Apr 23 15:49:59 PDT 2001
//    Initialize contour grid.
//
//    Hank Childs, Sun Jun 17 18:33:47 PDT 2001
//    Changed argument to be attributes.  Stole code for setting attributes
//    from avtContourPlot.
//
//    Brad Whitlock, Tue May 20 13:42:25 PST 2003
//    I made it use the updated ContourOpAttributes.
//
// ****************************************************************************

avtContourFilter::avtContourFilter(const ContourOpAttributes &a)
{
    atts   = a;
    cf     = vtkVisItContourFilter::New();
    cd2pd  = vtkCellDataToPointData::New();
    stillNeedExtents = true;
    shouldCreateLabels = true;

    logFlag = (atts.GetScaling() == ContourOpAttributes::Linear ?
               false : true);

    percentFlag = (atts.GetContourMethod() ==
                   ContourOpAttributes::Percent ? true : false);

    if (atts.GetContourMethod() == ContourOpAttributes::Level)
    {
        nLevels = atts.GetContourNLevels();
        isoValues.clear();
    }
    else if (atts.GetContourMethod() == ContourOpAttributes::Value)
    {
        isoValues = atts.GetContourValue();
        nLevels = isoValues.size();
        stillNeedExtents = false;
    }
    else // Percent
    {
        isoValues = atts.GetContourPercent();
        nLevels = isoValues.size();
    }
 
    // We need to specify that we want a secondary variable as soon as
    // possible.
    if (strcmp(atts.GetVariable().c_str(), "default") != 0)
    {
        SetActiveVariable(atts.GetVariable().c_str());
    }
}


// ****************************************************************************
//  Method: avtContourFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Aug 18 15:53:57 PDT 2000
//    Added deletion of cd2pd.
//
//    Kathleen Bonnell, Fri Feb 16 13:28:57 PST 2001 
//    Added test for cf equal NULL, as cf only created now in Execute method, 
//    and this object may be destructed before Execute method is called. 
//
//    Hank Childs, Mon Apr 23 15:49:59 PDT 2001
//    Destruct contour grid.
//
// ****************************************************************************

avtContourFilter::~avtContourFilter()
{
    if (cf != NULL)
    {
        cf->Delete();
        cf = NULL;
    }
    if (cd2pd != NULL)
    {
        cd2pd->Delete();
        cd2pd = NULL;
    }
}


// ****************************************************************************
//  Method: avtContourFilter::PerformRestriction
//
//  Purpose:
//      Restrict the data processed by looking at the data extents.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Apr 17 09:22:40 PDT 2002
//    Account for secondary variables.
//
//    Jeremy Meredith, Tue Aug 13 14:34:55 PDT 2002
//    Made it tell the dataspec that we need valid face connectivity.
//
//    Brad Whitlock, Tue May 20 13:43:11 PST 2003
//    Made it use the updated ContourOpAttributes.
//
// ****************************************************************************

avtPipelineSpecification_p
avtContourFilter::PerformRestriction(avtPipelineSpecification_p in_spec)
{
    avtPipelineSpecification_p spec = new avtPipelineSpecification(in_spec);

    spec->GetDataSpecification()->SetNeedValidFaceConnectivity(true);

    int  i, j;

    if (atts.GetContourMethod() == ContourOpAttributes::Level ||
        atts.GetContourMethod() == ContourOpAttributes::Percent)
    {
        //
        // These two methods require that we know what the extents are.  We
        // must do static load balancing if we don't know what those extents
        // are.
        //
        double extents[6]; // 6 is just in case.
        const char *varname = NULL;
        if (atts.GetVariable() != "default")
        {
            varname = atts.GetVariable().c_str();
        }
        if (TryDataExtents(extents, varname))
        {
            SetIsoValues(extents[0], extents[1]);
            stillNeedExtents = false;
        }
        else
        {
            spec->NoDynamicLoadBalancing();
            return spec;
        }
    }

    //
    // Get the interval tree of data extents.
    //
    avtIntervalTree *it = GetMetaData()->GetDataExtents();
    if (it == NULL)
    {
        debug5 << "Cannot use interval tree for contour filter, no "
               << "interval tree exists." << endl;
        return spec;
    }

    //
    // Calculate the domains list for each of the values in the contour filter
    // and take the union of those lists as we go.
    //
    vector<bool> useList;
    for (i = 0 ; i < isoValues.size(); i++)
    {
        //
        // The interval tree solves linear equations.  A contour is the linear
        // equations 1x = val.
        //
        float eqn[1] = { 1. };
        float val = isoValues[i];

        vector<int> list;
        it->GetDomainsList(eqn, val, list);

        //
        // list just has the domains for one contour.  Union this with
        // our running list.
        //
        for (j = 0 ; j < list.size() ; j++)
        {
            //
            // Make sure our array is big enough.
            //
            while (list[j] >= useList.size())
            {
                useList.push_back(false);
            }
            useList[list[j]] = true;
        }
    }

    //
    // Use list only has the domains we are interested in as booleans (this was
    // a convenient way to keep track of it without having to determine how big
    // the list is).  Now convert that to a vector of ints to meet our normal
    // interface.
    //
    vector<int> list;
    for (i = 0 ; i < useList.size() ; i++)
    {
        if (useList[i])
        {
            list.push_back(i);
        }
    }

    spec->GetDataSpecification()->GetRestriction()->RestrictDomains(list);

    return spec;
}


// ****************************************************************************
//  Method: avtContourFilter::PreExecute
//
//  Purpose:
//      Sees if we still need to set the extents.  We know that we can call
//      GetDataExtents (which requires static load balancing), since if we
//      weren't able to get the extents when performing the restriction, we
//      disabled load balancing.
//
//  Programmer: Hank Childs
//  Creation:   June 17, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 16:12:00 PDT 2001
//    Reflect changes in interface for avtDataAttributes.
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001 
//    Added calls to CreateLabels and SetLabels. 
//
//    Hank Childs, Wed Apr 17 09:01:33 PDT 2002
//    Added call to base class' PreExecute.
//
//    Hank Childs, Wed Aug 28 16:41:20 PDT 2002
//    Check to make sure that we really have a variable to work with.
//
//    Hank Childs, Thu Aug 29 08:12:22 PDT 2002
//    Fix a bug from last night.  If the variable was not 'default' the test
//    was incorrect.
//
// ****************************************************************************

void
avtContourFilter::PreExecute(void)
{
    avtDatasetToDatasetFilter::PreExecute();

    if (atts.GetVariable() == "default" && 
        GetInput()->GetInfo().GetAttributes().GetVariableName() == "<unknown>")
    {
        //
        // Somehow the variable we asked for didn't make it down far enough.
        // This often happens when we are doing a plot that doesn't have a
        // variable (say a mesh plot) and then we apply the isosurface
        // operator.
        //
        EXCEPTION1(NoDefaultVariableException, "Isosurface");
    }

    if (stillNeedExtents)
    {
        double extents[6]; // just in case
        const char *varname = NULL;
        if (atts.GetVariable() != "default")
        {
            varname = atts.GetVariable().c_str();
        }
        GetDataExtents(extents, varname);
 
        //
        // If we are dealing with the default variable, set the values for our
        // output.
        //
        if (varname == NULL)
        {
            GetOutput()->GetInfo().GetAttributes()
                                      .GetEffectiveDataExtents()->Set(extents);
        }
        SetIsoValues(extents[0], extents[1]);
    }

    debug5 << "About to execute contour filter.  " << isoValues.size()
           << " isovalues are: ";
    for (int i = 0 ; i < isoValues.size() ; i++)
    {
        debug5 << isoValues[i] << ", ";
    }
    debug5 << endl;
    CreateLabels();
    GetOutput()->GetInfo().GetAttributes().SetLabels(isoLabels);
}


// ****************************************************************************
//  Method: avtContourFilter::ExecuteDataTree
//
//  Purpose:
//      Sends the specified input and output through the contour filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      domain     The domain number.
//
//  Returns:       The output dataset.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Aug 18 15:53:57 PDT 2000
//    Added cd2pd to convert zone-centered data to node-centered data so the
//    contour filter would still work in those cases.
//
//    Jeremy Meredith, Thu Sep 28 12:45:16 PDT 2000
//    Made this create a new vtk dataset.
//
//    Hank Childs, Fri Oct 27 10:23:52 PDT 2000
//    Added argument for domain number to match inherited interface.
//
//    Kathleen Bonnell, Fri Feb 16 13:28:57 PST 2001 
//    Renamed this method ExecuteDomainTree and made it return an
//    avtDomainTree_p to reflect new inheritance.  Feed levels to
//    the vtk filter one-by-one.
//
//    Kathleen Bonnell, Thu Mar  1 10:01:44 PST 2001
//    Added call to InvalidateOperation if levels are emtpy and
//    made it return NULL.
//
//    Hank Childs, Tue Mar 20 08:40:24 PST 2001
//    Use object information to determine centering instead of guessing.
//
//    Hank Childs, Sun Mar 25 12:24:17 PST 2001
//    Account for new data object information interface.
//
//    Kathleen Bonnell, Tue Apr 10 11:35:39 PDT 2001 
//    Renamed method ExecuteDataTree. 
//
//    Hank Childs, Thu Apr 12 16:59:54 PDT 2001
//    Made use of scalar tree if there was more than one isolevel.  Also use
//    a contour grid for unstructured grids.
//
//    Kathleen Bonnell, Tue Jun 12 14:34:02 PDT 2001
//    Added preservation of ghost-cell data, if present.  Forced toBeContoured
//    to be created from New() if cd2pd filter used. 
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001 
//    Added string argument to match new interface, but this filter
//    does not need the arg as it will set the labels itself.
//
//    Hank Childs, Wed Apr 17 15:11:27 PDT 2002
//    Re-worked routine to account for multiple variables.  Also removed some
//    hacks to get around VTK bugs.
//    
//    Jeremy Meredith, Thu Jul 11 13:36:31 PDT 2002
//    Added code to not interpolate avtOriginalCellNumbers to the nodes.
//
//    Hank Childs, Sun Aug  4 18:50:21 PDT 2002
//    Better handling of cell variables -- only interpolate the cell variable
//    we are going to contour by and no others.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    Use NewInstance instead of MakeObject to match new vtk api. 
//
//    Hank Childs, Mon May 12 20:01:50 PDT 2003
//    Make sure an iso-level can actually contribute triangles before
//    contouring.
//
//    Hank Childs, Fri Jul 25 22:13:58 PDT 2003
//    Made use of scalar tree and faster VisIt contouring module.
//
// ****************************************************************************

avtDataTree_p 
avtContourFilter::ExecuteDataTree(vtkDataSet *in_ds, int domain, string label)
{
    int   i;
    char *contourVar = (activeVariable != NULL ? activeVariable 
                                               : pipelineVariable);
    if (isoValues.empty())
    {
        debug3 << "No levels to calculate! " << endl;
        GetOutput()->GetInfo().GetValidity().InvalidateOperation();
        return NULL;
    }

    //
    //  Creating these from ::New to pass through the cd2pd filter
    //  is a HACK - WORK-AROUND for a vtk Update/Whole-Extents error
    //  message encountered when cd2pd->Update() is called.  Occurs 
    //  only when Contour is used on multi-block data.
    // 
    vtkDataSet *toBeContoured = (vtkDataSet *)in_ds->NewInstance();

    //
    // Contouring only works on nodal variables -- make sure that we have
    // a nodal variable before progressing.
    //
    vtkDataArray *cellVar = in_ds->GetCellData()->GetArray(contourVar);
    if (cellVar != NULL)
    {
        //
        // The input is zone-centered, but the contour filter needs
        // node-centered data, so put it through a filter.
        //
        vtkDataSet *new_in_ds = (vtkDataSet *) in_ds->NewInstance();
        new_in_ds->CopyStructure(in_ds);
        new_in_ds->GetCellData()->AddArray(cellVar);
        cd2pd->SetInput(new_in_ds);
        cd2pd->SetOutput(toBeContoured);
        cd2pd->Update();
        for (i = 0 ; i < in_ds->GetPointData()->GetNumberOfArrays() ; i++)
        {
            vtkDataArray *arr = in_ds->GetPointData()->GetArray(i);
            toBeContoured->GetPointData()->AddArray(arr);
        }
        for (i = 0 ; i < in_ds->GetCellData()->GetNumberOfArrays() ; i++)
        {
            vtkDataArray *arr = in_ds->GetCellData()->GetArray(i);
            if (strcmp(arr->GetName(), contourVar) != 0)
            {
                toBeContoured->GetCellData()->AddArray(arr);
            }
        }
        new_in_ds->Delete();
    }
    else
    {
        toBeContoured->ShallowCopy(in_ds);
    }
    toBeContoured->GetPointData()->SetActiveScalars(contourVar);

    vtkVisItScalarTree *tree = vtkVisItScalarTree::New();
    tree->SetDataSet(toBeContoured);
    int id0 = visitTimer->StartTimer();
    tree->BuildTree();
    visitTimer->StopTimer(id0, "Building scalar tree");

    //
    // Do the actual contouring.  Split each isolevel into its own dataset.
    //
    vtkDataSet **out_ds = new vtkDataSet*[isoValues.size()];
    cf->SetInput(toBeContoured);
    vtkPolyData *output = cf->GetOutput();
    for (i = 0 ; i < isoValues.size() ; i++)
    {
        std::vector<int> list;
        int id1 = visitTimer->StartTimer();
        tree->GetCellList(isoValues[i], list);
        visitTimer->StopTimer(id1, "Getting cell list");
        int id2 = visitTimer->StartTimer();
        cf->SetIsovalue(isoValues[i]);
        cf->SetCellList(&(list[0]), list.size());

        output->Update();
        if (output->GetNumberOfCells() == 0)
            out_ds[i] = NULL;
        else
        {
            out_ds[i] = vtkPolyData::New();
            out_ds[i]->ShallowCopy(output);
        }
        visitTimer->StopTimer(id2, "Calculating isosurface");
    }

    //
    // Create the output in an AVT friendly fashion.
    //
    avtDataTree_p outDT = NULL;
    if (shouldCreateLabels)
    {   
        outDT = new avtDataTree(isoValues.size(), out_ds, domain, isoLabels);
    }
    else
    {   
        outDT = new avtDataTree(isoValues.size(), out_ds, domain, label);
    }

    //
    // Clean up memory.
    //
    for (i = 0; i < isoValues.size(); i++)
    {
        if (out_ds[i] != NULL) 
        {
            out_ds[i]->Delete();
        }
    }
    delete [] out_ds;
    toBeContoured->Delete();
    tree->Delete();

    return outDT;
}


// ****************************************************************************
//  Method: avtContourFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Indicates that the topological dimension of the output is not the same
//      as the input.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 16:12:00 PDT 2001
//    Reflect changes in interface for avtDataAttributes.
//
//    Hank Childs, Thu Oct 10 13:05:49 PDT 2002 
//    Do not assume that output is node-centered.
//
// ****************************************************************************

void
avtContourFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    avtDataAttributes &inAtts  = GetInput()->GetInfo().GetAttributes();
   
    outAtts.SetTopologicalDimension(inAtts.GetTopologicalDimension()-1);
    if (atts.GetVariable() == "default")
    {
        outAtts.SetCentering(AVT_NODECENT);
    }
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
}


// ****************************************************************************
//  Method: avtContourFilter::SetIsoValues
//
//  Purpose:
//      Creates IsoValues.
//
//  Arguments:
//    min      The variable's minimum value.
//    max      The variable's maximum value.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     March 1, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 28 17:18:05 PST 2001
//    Added arguments, and passed them to CreateNIsoValues and 
//    CreatePercentValues.
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001 
//    Revised check for invalid min/max to test artificial limits
//    set by user if applicable.
//
//    Hank Childs, Tue Apr 10 10:51:43 PDT 2001
//    Relaxed constraint of lo != hi for people who use nlevels == 1.
//
//    Hank Childs, Sun Jun 17 18:42:00 PDT 2001
//    Moved function from avtContourPlot.
//
//    Hank Childs, Sun Jun 24 19:48:46 PDT 2001
//    When there is an error, clear the isoValues if they are *not* empty.
//
// ****************************************************************************

void
avtContourFilter::SetIsoValues(double min, double max)
{
    double lo = atts.GetMinFlag() ? atts.GetMin() : min;
    double hi = atts.GetMaxFlag() ? atts.GetMax() : max;

    if (lo > hi || lo == DBL_MAX || hi == DBL_MAX)
    {
        debug1 << "Min & Max invalid, (" << min << ", " << max
               << ") returning empty isoValues." << endl;

        if (!isoValues.empty())
            isoValues.clear();
        return;
    }

    if (isoValues.empty())
    {
        CreateNIsoValues(min, max);
    }
    else if ( percentFlag )
    {
        CreatePercentValues(min, max);
    }

    // else isoValues are user-specified, so nothing to do.
}


// ****************************************************************************
//  Method: avtContourFilter::CreatePercentValues
//
//  Purpose:
//    Creates N isoValues between min & max. 
//
//  Arguments
//    mn     The variables minimum value.
//    mx     The variables maximum value.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 20, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Mar  2 11:34:46 PST 2001
//    Moved test for min equal max to SetIsoValues method.
//
//    Kathleen Bonnell, Tue Mar 27 15:27:50 PST 2001 
//    Added arguments, and test for valid min/max before using log scale.  
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001 
//    Revised to use artificial limits if user specified. 
//
//    Kathleen Bonnell, Wed Apr 25 14:28:22 PDT 2001 
//    Reflect change in InvalidLimitsException signature. 
//
//    Hank Childs, Sun Jun 17 18:42:00 PDT 2001
//    Moved function from avtContourPlot.
//
// ****************************************************************************

void
avtContourFilter::CreatePercentValues(double mn, double mx)
{
    double min, max;
    // 
    // should we be using user-defined limits?
    // 
    min = atts.GetMinFlag() ? atts.GetMin() : mn;
    max = atts.GetMaxFlag() ? atts.GetMax() : mx;

    if ( logFlag )
    {
       if (min <= 0. || max <= 0.)
       {
           // if mn, mx from the var extents, user needs to specify 
           // limits > 0 in order to use log scaling.
           EXCEPTION1(InvalidLimitsException, true);
       }
       min = log10(min);
       max = log10(max);
    }

    double delta = 0.01 * (max - min);

    if ( !logFlag )
    {
       for (int i = 0; i < nLevels; i++)
           isoValues[i] = min + (isoValues[i] * delta) ;
    }
    else 
    {
        for (int i = 0; i < nLevels; i++)
           isoValues[i] = pow( 10., min + (isoValues[i] * delta)) ;
    }
}



// ****************************************************************************
//  Method: avtContourFilter::CreateNIsoValues
//
//  Purpose:
//    Creates N isoValues between min & max. 
//
//  Arguments:
//    min      The variables minimum value.
//    max      The variables maximum value.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 20, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Mar  2 11:34:46 PST 2001
//    Moved test for min equal max to SetIsoValues method.
//
//    Kathleen Bonnell, Wed Mar 28 17:18:05 PST 2001 
//    Added arguments and test for positive min & max values before using log. 
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001 
//    Overhauled this method so that it more closely mimics the MeshTV method
//    of creating isoValues, including using artificial limits as hi/lo value
//    if user has requested them. 
//
//    Hank Childs, Tue Apr 10 10:20:00 PDT 2001
//    More graceful handling of nLevels == 1.
//
//    Kathleen Bonnell, Wed Apr 25 14:28:22 PDT 2001 
//    Reflect change in InvalidLimitsException signature. 
//
//    Hank Childs, Sun Jun 17 18:42:00 PDT 2001
//    Moved function from avtContourPlot.
//
// ****************************************************************************

void
avtContourFilter::CreateNIsoValues(double min, double max)
{
    double lo, hi, delta, extremaOffset;
    lo = min;
    hi = max;
    if (logFlag)
    {
        if (min <= 0.) 
        {
            if (!atts.GetMinFlag() || atts.GetMin() <= 0.)
            {
                EXCEPTION1(InvalidLimitsException, true);
            }
            else 
            {
                lo = atts.GetMin();
            }
        }
        if (max <=0. && (!atts.GetMaxFlag() || atts.GetMax() <= 0.) )
        {
            EXCEPTION1(InvalidLimitsException, true);
        }
        lo = log10(lo);
        hi = log10(hi);

    }

    //
    // If we have to generate the isolevels, then we want them to be offset
    // at the extrema.  This offset is arbitrary and mimicks what MeshTV did.
    //
    extremaOffset = (hi - lo) / (nLevels + 1.);

    if (atts.GetMinFlag())
    {
        // use the artificial min as first isoValue
        lo = atts.GetMin();
        if (logFlag)
            lo = log10(lo);
    }
    else
    {
        lo += extremaOffset;
    }

    if (atts.GetMaxFlag())
    {
        // use the artificial max as last isoValue
        hi = atts.GetMax();
        if (logFlag)
            hi = log10(hi);
    }
    else
    {
        hi -= extremaOffset;
    }

    if (nLevels <= 1)
    {
        delta = 0.;
    }
    else
    {
        delta = (hi - lo) / (nLevels - 1.);
    }

    if (logFlag)
    {
        for (int i = 0; i < nLevels; ++i)
            isoValues.push_back(pow(10., lo + i * delta));
    }
    else 
    {
        for (int i = 0; i < nLevels; ++i)
            isoValues.push_back(lo + i * delta);
    }
}


// ****************************************************************************
//  Method: avtContourFilter::CreateLabels
//
//  Purpose:
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Apr 17 18:33:28 PDT 2002
//    Don't muddy the waters downstream if we aren't making labels.
//
// ****************************************************************************

void
avtContourFilter::CreateLabels()
{
    if (!shouldCreateLabels)
    {
        return;
    }

    char temp[48];

    if (!isoLabels.empty())
    {
        isoLabels.clear();
    }

    for (int i = 0; i < isoValues.size(); i++)
    {
        sprintf(temp, "%# -9.4g", isoValues[i]);
        isoLabels.push_back(temp);
    }
}


// ****************************************************************************
//  Method: avtContourFilter::ReleaseData
//
//  Purpose:
//      Release all problem size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Sep 16 18:26:11 PDT 2002
//    Fix additional memory bloat problems.
//
// ****************************************************************************

void
avtContourFilter::ReleaseData(void)
{
    avtDataTreeStreamer::ReleaseData();

    cf->SetInput(NULL);
    cf->SetOutput(NULL);
    cd2pd->SetInput(NULL);
    cd2pd->SetOutput(NULL);
}


