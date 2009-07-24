/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                              avtContourFilter.C                           //
// ************************************************************************* //

#include <avtContourFilter.h>

#include <limits.h>
#include <float.h>
#include <vector>

#include <vtkCellData.h>
#include <vtkVisItCellDataToPointData.h>
#include <vtkDataSet.h>
#include <vtkExecutive.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItContourFilter.h>
#include <vtkVisItScalarTree.h>

#include <avtExtents.h>
#include <avtIntervalTree.h>
#include <avtIsolevelsSelection.h>
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
//    Hank Childs, Sun Mar  6 08:18:53 PST 2005
//    Removed cd2pd.
//
// ****************************************************************************

avtContourFilter::avtContourFilter(const ContourOpAttributes &a)
{
    atts   = a;
    cf     = vtkVisItContourFilter::New();
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
//    Hank Childs, Sun Mar  6 08:18:53 PST 2005
//    Removed cd2pd.
//
// ****************************************************************************

avtContourFilter::~avtContourFilter()
{
    if (cf != NULL)
    {
        cf->Delete();
        cf = NULL;
    }
}


// ****************************************************************************
//  Method: avtContourFilter::ModifyContract
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
//    Hank Childs, Wed Jul 30 09:04:28 PDT 2003
//    Do not claim we need face connectivity if we are in 2D.
//
//    Hank Childs, Mon Mar  1 07:56:53 PST 2004
//    Give a better hint about what variable we are working on.
//
//    Kathleen Bonnell, Thu Mar 11 11:10:07 PST 2004 
//    DataExtents now always have only 2 components. 
//
//    Hank Childs, Wed Aug 11 08:53:57 PDT 2004
//    Make sure that we request ghost zones.
//
//    Hank Childs, Wed Sep 22 16:40:06 PDT 2004
//    Refine our ghost zone request a bit -- only request when we have a zonal
//    quantity.
//
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
//    Hank Childs, Thu Mar  1 16:43:48 PST 2007
//    Improve chances that we will get to use the interval tree.  The old
//    logic did not retrieve the extents from the interval tree (for setting
//    isolevels), even though it was possible.  Also, we were disabling
//    dynamic load balancing even when it was possible.  If the min and max
//    were explicitly set, then we could set a percent or nLevels without
//    needing to know the actual data extents.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Hank Childs, Fri Nov 14 09:05:04 PST 2008
//    Make sure ghost data is not requested if we ultimately want ghost nodes.
//
//    Hank Childs, Mon Jan  5 15:18:09 CST 2009
//    Add a data selection.
//
//    Eric Brugger, Fri Jul 24 10:59:44 PDT 2009
//    Added the variable name to the call to GetMetaData()->GetDataExtents()
//    so that it would get the correct interval tree.  This fix was provided
//    Hank Childs.
//
// ****************************************************************************

avtContract_p
avtContourFilter::ModifyContract(avtContract_p in_contract)
{
    int  i, j;

    avtContract_p contract = new avtContract(in_contract);

    const char *varname = NULL;
    if (atts.GetVariable() != "default")
        varname = atts.GetVariable().c_str();
    else 
        varname = contract->GetDataRequest()->GetVariable();

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 3)
        contract->GetDataRequest()->SetNeedValidFaceConnectivity(true);

    //
    // We will need the ghost zones so that we can interpolate along domain
    // boundaries and get no cracks in our isosurface.
    //
    avtDataAttributes &in_atts = GetInput()->GetInfo().GetAttributes();
    bool skipGhost = false;
    if (in_atts.ValidVariable(varname) && 
        in_atts.GetCentering(varname) == AVT_NODECENT)
        skipGhost = true;
    if (!skipGhost)
        contract->GetDataRequest()->SetDesiredGhostDataType(GHOST_ZONE_DATA);
    else if (contract->GetDataRequest()->GetDesiredGhostDataType() == GHOST_NODE_DATA)
        contract->GetDataRequest()->SetDesiredGhostDataType(NO_GHOST_DATA);

    //
    // Get the interval tree of data extents.
    //
    avtIntervalTree *it = GetMetaData()->GetDataExtents(varname);
    if (it != NULL && it->GetDimension() != 1)
    {
        debug1 << "The interval tree returned for the contour variable "
               << "is not for a scalar.  Internal error?" << endl;
        it = NULL;
    }

    if (atts.GetContourMethod() == ContourOpAttributes::Level ||
        atts.GetContourMethod() == ContourOpAttributes::Percent)
    {
        //
        // These two methods require that we know what the extents are.  We
        // must do static load balancing if we don't know what those extents
        // are.
        //
        double extents[2] = { 0., 0. }; 
        stillNeedExtents = true;
        if (atts.GetMinFlag() && atts.GetMaxFlag())
            stillNeedExtents = false;  // wouldn't use them anyway
        else if (TryDataExtents(extents, varname))
            stillNeedExtents = false;
        else if (it != NULL)
        {
            it->GetExtents(extents);
            stillNeedExtents = false;
        }

        if (!stillNeedExtents)
            SetIsoValues(extents[0], extents[1]);
        else
        {
            contract->NoStreaming();
            return contract;
        }
    }

    //
    // Tell the file format reader that we will be extracting isolevels,
    // in case it can limit its reads to only the domains/elements that
    // cross the isolevel.
    //
    avtIsolevelsSelection *sel = new avtIsolevelsSelection;
    sel->SetVariable(varname);
    sel->SetIsolevels(isoValues);
    contract->GetDataRequest()->AddDataSelection(sel);

    if (it == NULL)
    {
        debug5 << "Cannot use interval tree for contour filter, no "
               << "interval tree exists." << endl;
        return contract;
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
        double eqn[1] = { 1. };
        double val = isoValues[i];

        vector<int> list;
        it->GetElementsList(eqn, val, list);

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
        if (useList[i])
            list.push_back(i);

    contract->GetDataRequest()->GetRestriction()->RestrictDomains(list);

    return contract;
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
//    Kathleen Bonnell, Thu Mar 11 11:10:07 PST 2004 
//    DataExtents now always have only 2 components. 
//
//    Hank Childs, Mon Aug 30 08:42:48 PDT 2004
//    Initialize current_node and nnodes for better progress indicators.
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
        double extents[2]; 
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

    nnodes = GetInputDataTree()->GetNumberOfLeaves();
    current_node = 0;
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
//    Hank Childs, Tue May 11 06:47:24 PDT 2004
//    Fix a bug that assumed that we would always have *something* to contour.
//
//    Hank Childs, Mon Aug 30 08:45:14 PDT 2004
//    Give better progress.
//
//    Hank Childs, Sun Mar  6 08:18:53 PST 2005
//    Instantiate cd2pd here inside this routine rather than using a data
//    member.
//
//    Kathleen Bonnell, Tue May 16 09:57:29 PDT 2006 
//    VTK pipeline changes: no more SetOutput method for filters, instead
//    SetOutputData for the filter's Executive. 
//
//    Gunther H. Weber, Wed May 30 16:41:15 PDT 2007
//    Copy field data when generating isosurfaces. There are still problems
//    when isosurfaces for multiple values are generated, but it seems to work
//    for single isosurfaces.
//
//    Jeremy Meredith, Thu Oct 16 18:21:59 EDT 2008
//    Switch to our own version of the cd2pd filter, since it has
//    optimizations for structured data.
//
//    Hank Childs, Fri Nov 14 09:03:58 PST 2008
//    Remove ghost nodes, as they will make a bad picture.
//
//    Hank Childs, Wed Jan  7 16:03:29 CST 2009
//    Only use a scalar tree if we have multiple isolevels.
//
// ****************************************************************************

avtDataTree_p 
avtContourFilter::ExecuteDataTree(vtkDataSet *in_ds, int domain, string label)
{
    int tt1 = visitTimer->StartTimer();

    int   i;
    char *contourVar = (activeVariable != NULL ? activeVariable 
                                               : pipelineVariable);
    if (isoValues.empty())
    {
        debug3 << "No levels to calculate! " << endl;
        GetOutput()->GetInfo().GetValidity().InvalidateOperation();
        current_node++;
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
    int t3 = visitTimer->StartTimer();
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
        vtkVisItCellDataToPointData *cd2pd = vtkVisItCellDataToPointData::New();
        cd2pd->SetInput(new_in_ds);
        cd2pd->GetExecutive()->SetOutputData(0, toBeContoured);
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
        cd2pd->Delete();
    }
    else
    {
        toBeContoured->ShallowCopy(in_ds);
    }
    toBeContoured->GetPointData()->SetActiveScalars(contourVar);
    visitTimer->StopTimer(t3, "Recentering");

    //
    // The progress is a bit funny.  Recentering and building the scalar
    // tree takes a lot of time.  So we would like for that to be 1/2 of
    // the progress for this stage.  So allocate 1/4 for each.  The "+2"
    // business is to account for a case where there are 0 isolevels,
    // which could lead to a divide-by-0 when calculating progress.
    //
    int nLevels = isoValues.size();
    bool useScalarTree = (nLevels > 1);
    int total = 4*nLevels+2;
    UpdateProgress(current_node*total + nLevels+1, total*nnodes);

    vtkVisItScalarTree *tree = vtkVisItScalarTree::New();
    if (useScalarTree)
    {
         tree->SetDataSet(toBeContoured);
         int id0 = visitTimer->StartTimer();
         tree->BuildTree();
         visitTimer->StopTimer(id0, "Building scalar tree");
    }

    UpdateProgress(current_node*total + 2*nLevels+2, total*nnodes);

    //
    // Do the actual contouring.  Split each isolevel into its own dataset.
    //
    vtkDataSet **out_ds = new vtkDataSet*[isoValues.size()];
    cf->SetInput(toBeContoured);
    vtkPolyData *output = cf->GetOutput();
    for (i = 0 ; i < isoValues.size() ; i++)
    {
        std::vector<int> list;
        if (useScalarTree)
        {
            int id1 = visitTimer->StartTimer();
            tree->GetCellList(isoValues[i], list);
            visitTimer->StopTimer(id1, "Getting cell list");
        }
        int id2 = visitTimer->StartTimer();
        cf->SetIsovalue(isoValues[i]);
        int *list2 = NULL;
        if (useScalarTree)
        {
            int emptylist[1] = { 0 };
            if (list.size() <= 0)
                list2 = emptylist;
            else
                list2 = &(list[0]);
            cf->SetCellList(list2, list.size());
         }

        output->Update();
        if (output->GetNumberOfCells() == 0)
            out_ds[i] = NULL;
        else
        {
            out_ds[i] = vtkPolyData::New();
            out_ds[i]->ShallowCopy(output);
	    out_ds[i]->GetFieldData()->ShallowCopy(in_ds->GetFieldData());
            out_ds[i]->GetPointData()->RemoveArray("avtGhostNodes");
        }
        visitTimer->StopTimer(id2, "Calculating isosurface");
        UpdateProgress(current_node*total + 2*nLevels+2+2*i, total*nnodes);
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

    visitTimer->StopTimer(tt1, "avtContourFilter::ExecuteData");
    current_node++;
    return outDT;
}


// ****************************************************************************
//  Method: avtContourFilter::UpdateDataObjectInfo
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
//    Hank Childs, Thu Feb 26 09:05:34 PST 2004
//    Do a better job of handling multiple variables.
//
//    Kathleen Bonnell, Thu Mar  2 15:05:17 PST 2006
//    Add ZonesSplit.
//
// ****************************************************************************

void
avtContourFilter::UpdateDataObjectInfo(void)
{
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    avtDataAttributes &inAtts  = GetInput()->GetInfo().GetAttributes();
   
    outAtts.SetTopologicalDimension(inAtts.GetTopologicalDimension()-1);

    const char *var_to_modify = NULL;
    if (atts.GetVariable() == "default")
    {
        if (inAtts.ValidActiveVariable())
        {
            var_to_modify = inAtts.GetVariableName().c_str();
        }
    }
    else
    {
        var_to_modify = atts.GetVariable().c_str();
    }
    if (var_to_modify != NULL)
    {
        if (outAtts.ValidVariable(var_to_modify))
            outAtts.SetCentering(AVT_NODECENT, var_to_modify);
    }

    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetValidity().ZonesSplit();
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
//    Kathleen Bonnell, Tue Jan 20 17:38:37 PST 2004 
//    Fix problem with delta when lo > hi.
//
//    Eric Brugger, Mon Apr  5 15:35:27 PDT 2004
//    Change the way the levels are set when the user specifies the number
//    of levels and the minimum and maximum.
//
//    Brad Whitlock, Mon Dec 19 17:17:05 PST 2005
//    I changed the code so it conditionally applies the extrema offset 
//    because applying it when min/max were set made it impossible to get
//    contours that go through the min/max values.
//
// ****************************************************************************

void
avtContourFilter::CreateNIsoValues(double min, double max)
{
    double lo, hi, delta, extremaOffset;
    if (atts.GetMinFlag())
        lo = atts.GetMin();
    else 
        lo = min;
    if (atts.GetMaxFlag())
        hi = atts.GetMax();
    else 
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
    // at the extrema.  This offset is arbitrary and mimicks what MeshTV did,
    // except in the case where the minimum and maximum are specified.
    //
    extremaOffset = (hi - lo) / (nLevels + 1.);
    if(!atts.GetMinFlag())
        lo += extremaOffset;
    if(!atts.GetMaxFlag())
        hi -= extremaOffset;

    if (nLevels <= 1)
    {
        delta = 0.;
    }
    else
    {
        if (lo < hi)
            delta = (hi - lo) / (nLevels - 1.);
        else 
            delta = (lo - hi) / (nLevels - 1.);
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
//    Hank Childs, Fri Mar  4 08:12:25 PST 2005
//    Do not set outputs of filters to NULL, since this will prevent them 
//    from re-executing correctly in DLB-mode.
//
//    Hank Childs, Sun Mar  6 08:18:53 PST 2005
//    Removed cd2pd.
//
//    Hank Childs, Fri Mar 11 07:37:05 PST 2005
//    Fix non-problem size leak introduced with last fix.
//
// ****************************************************************************

void
avtContourFilter::ReleaseData(void)
{
    avtSIMODataTreeIterator::ReleaseData();

    cf->SetInput(NULL);
    vtkPolyData *p = vtkPolyData::New();
    cf->SetOutput(p);
    p->Delete();
}


