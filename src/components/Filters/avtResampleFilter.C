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
//                             avtResampleFilter.C                           //
// ************************************************************************* //

#include <float.h>

#include <avtResampleFilter.h>

#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>

#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtImagePartition.h>
#include <avtParallel.h>
#include <avtPointExtractor.h>
#include <avtRay.h>
#include <avtResampleSelection.h>
#include <avtSamplePointExtractor.h>
#include <avtSamplePointCommunicator.h>
#include <avtSourceFromAVTDataset.h>
#include <avtWorldSpaceToImageSpaceTransform.h>

#include <DebugStream.h>


//
// Function Prototypes
//

static vtkRectilinearGrid  *CreateGrid(const double *, int, int, int,
                                   int, int, int, int, bool);
static void                 CreateViewFromBounds(avtViewInfo &, const double *,
                                             double [3]);
static vtkDataArray        *GetCoordinates(float, float, int, int, int);


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
//  Modifications:
//
//    Mark C. Miller, Tue Sep 13 20:09:49 PDT 2005
//    Initialized selection id
//
//    Hank Childs, Fri Jun  1 16:17:51 PDT 2007
//    Initialized cellCenteredOutput.
//
// ****************************************************************************

avtResampleFilter::avtResampleFilter(const AttributeGroup *a)
{
    atts = *(ResampleAttributes*)a;
    primaryVariable = NULL;
    selID = -1;
    cellCenteredOutput = false;
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
//    Mark C. Miller, Tue Sep 13 20:09:49 PDT 2005
//    Permitted poly data to pass through
//
//    Hank Childs, Thu Oct  6 10:53:17 PDT 2005
//    Do not allow rectilinear grids to bypass resampling.  Only for poly
//    data now. ['6676]
//
// ****************************************************************************

bool
avtResampleFilter::InputNeedsNoResampling(void)
{
    avtDataTree_p inDT = GetInputDataTree();

    //
    // permit VTK_POLY_DATA to pass through unchanged
    //
#if 0
    int n = 0;
    vtkDataSet **in_dss = inDT->GetAllLeaves(n);
    if (n && in_dss && in_dss[0] && in_dss[0]->GetDataObjectType() == VTK_POLY_DATA)
        return true;
#endif

    return false;
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
//    Hank Childs, Sat Jan 29 10:56:58 PST 2005
//    Added support for using bounds from attributes.  Also set up arbitrator
//    if necessary.
//
//    Hank Childs, Sun Mar 13 10:07:07 PST 2005
//    Fix memory leak.
//
//    Mark C. Miller, Tue Sep 13 20:09:49 PDT 2005
//    Added test for if data selection has already been applied 
//
//    Mark C. Miller, Thu Sep 15 11:30:18 PDT 2005
//    Modified where data selection bypass is done and added matching
//    collective calls
//
//    Hank Childs, Sun Oct  2 12:02:50 PDT 2005
//    Add support for distributed resampling.
//
//    Hank Childs, Mon Feb 20 15:22:57 PST 2006
//    Automatically use kernel-based sampling when dealing with point meshes.
//
//    Hank Childs, Fri Mar  3 12:27:11 PST 2006
//    Don't allow the resampler to "send cells", because there is no
//    second pass.
//
//    Hank Childs, Mon May 22 15:14:04 PDT 2006
//    Fix UMR.
//
//    Hank Childs, Thu May 31 15:56:04 PDT 2007
//    Add support for vector variables.  Also add support for cell-centered
//    data.
//
//    Hank Childs, Thu Aug 30 09:24:11 PDT 2007
//    Fix problem where parallel resamples could not get values less than
//    the default value.
//
//    Hank Childs, Tue Jan 15 21:25:01 PST 2008
//    No longer set up the sample point arbitrator, since we have insufficient
//    information when this method is first called.
//
//    Hank Childs, Wed Jan 16 16:29:20 PST 2008
//    Allow for data set with no variables to get pushed through, for the case
//    where a mesh plot is resampled.
//
// ****************************************************************************

void
avtResampleFilter::ResampleInput(void)
{
    int  i, j, k;

    bool is3D = true;

    avtDataset_p output = GetTypedOutput();
    double bounds[6] = { 0, 0, 0, 0, 0, 0 };
    if (atts.GetUseBounds())
    {
        bounds[0] = atts.GetMinX();
        bounds[1] = atts.GetMaxX();
        bounds[2] = atts.GetMinY();
        bounds[3] = atts.GetMaxY();
        bounds[4] = atts.GetMinZ();
        bounds[5] = atts.GetMaxZ();
    }
    else
    {
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
    }
    if (fabs(bounds[4]) < 1e-100 && fabs(bounds[5]) < 1e-100)
    {
        is3D = false;
        bounds[5] += 0.1;
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
    if (GetInput()->GetInfo().GetAttributes().ValidActiveVariable())
    {
        GetDataExtents(range);
        output->GetInfo().GetAttributes().GetEffectiveDataExtents()->Set(range);
    }

    avtViewInfo view;
    double scale[3];
    CreateViewFromBounds(view, bounds, scale);

    //
    // What we want the width, height, and depth to be depends on the
    // attributes.
    //
    int width, height, depth;
    GetDimensions(width, height, depth, bounds, is3D);

    //
    // If there are no variables, then just create the mesh and exit.
    //
    bool thereAreNoVariables = 
          (GetInput()->GetInfo().GetAttributes().GetNumberOfVariables() <= 0);
    if (thereAreNoVariables)
    {
        if (PAR_Rank() == 0)
        {
            vtkRectilinearGrid *rg = CreateGrid(bounds, width, height, depth,
                                      0, width, 0, height, cellCenteredOutput);
            avtDataTree_p tree = new avtDataTree(rg, 0);
            rg->Delete();
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

        return;
    }

    //
    // World space is a right-handed coordinate system.  Image space (as used
    // in the sample point extractor) is a left-handed coordinate system.
    // This is because large X is at the right and large Y is at the top.
    // The z-buffer has the closest points at z=0, so Z is going away from the
    // screen ===> left handed coordinate system.  If we reflect across X,
    // then this will account for the difference between the coordinate 
    // systems.
    //
    scale[0] *= -1.;

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

    bool doKernel = 
        (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0);
    avtSamplePointExtractor extractor(width, height, depth);
    extractor.SendCellsMode(false);
    extractor.Set3DMode(is3D);
    extractor.SetInput(trans.GetOutput());
    if (doKernel)
        extractor.SetKernelBasedSampling(true);
    avtSamplePoints_p samples = extractor.GetTypedOutput();

    //
    // If the selection this filter exists to create has already been handled,
    // or if there are no pieces for this processor to process, then we can skip
    // execution. But, take care to emmulate the same collective
    // calls other processors may make before returning.
    //
    if (GetInput()->GetInfo().GetAttributes().GetSelectionApplied(selID))
    {
        debug1 << "Bypassing Resample operator because database plugin "
                  "claims to have applied the selection already" << endl;

        SetOutputDataTree(GetInputDataTree());

        // we can save a lot of time if we know everyone can bypass
        if (UnifyMaximumValue(0) == 0)
            return;

        // here is some dummied up code to match collective calls below
        int effectiveVars = samples->GetNumberOfRealVariables();
        float *ptrtmp = new float[width*height*depth];
        for (int jj = 0; jj < width*height*depth; jj++)
            ptrtmp[jj] = -FLT_MAX;
        for (i = 0 ; i < effectiveVars ; i++)
            Collect(ptrtmp, width*height*depth);
        delete [] ptrtmp;
        return;
    }
    else
    {
        UnifyMaximumValue(1);
    }

    //
    //
    // PROBLEM SIZED WORK OCCURS BEYOND THIS POINT
    // If you add (or remove) collective calls below this point, make sure to
    // put matching sequence into bypass code above
    //
    //

    avtSamplePointCommunicator communicator;
    avtImagePartition partition(width, height, PAR_Size(), PAR_Rank());
    communicator.SetImagePartition(&partition);
    bool doDistributedResample = false;
#ifdef PARALLEL
    doDistributedResample = atts.GetDistributedResample();
#endif

    if (doDistributedResample)
    {
        partition.SetShouldProduceOverlaps(true);
        avtDataObject_p dob;
        CopyTo(dob, samples);
        communicator.SetInput(dob);
        samples = communicator.GetTypedOutput();
    }

    // Always set up an arbitrator, even if user selected random.
    bool arbLessThan = !atts.GetUseArbitrator() || atts.GetArbitratorLessThan();
    std::string arbName = atts.GetArbitratorVarName();
    if (arbName == "default")
        arbName = primaryVariable;
    extractor.SetUpArbitrator(arbName, arbLessThan);

    //
    // Since this is Execute, forcing an update is okay...
    //
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
    int width_start  = 0;
    int width_end    = width;
    int height_start = 0;
    int height_end   = height;
    if (doDistributedResample)
    {
        partition.GetThisPartition(width_start, width_end, height_start, 
                                   height_end);
        width_end += 1;
        height_end += 1;
    }

    //
    // If we have more processors than domains, we have to handle that
    // gracefully.  Communicate how many variables there are so that those
    // that don't have data can play well.
    //
    int realVars  = samples->GetNumberOfRealVariables();
    int numArrays = realVars;
    if (doKernel)
        numArrays++;
    vtkDataArray **vars = new vtkDataArray*[numArrays];
    for (i = 0 ; i < numArrays ; i++)
    {
        vars[i] = vtkFloatArray::New();
        if (doKernel && (i == numArrays-1))
            vars[i]->SetNumberOfComponents(1);
        else
        {
            vars[i]->SetNumberOfComponents(samples->GetVariableSize(i));
            vars[i]->SetName(samples->GetVariableName(i).c_str());
        }
    }

    if (doKernel)
        samples->GetVolume()->SetUseKernel(true);

    avtImagePartition *ip = NULL;
    if (doDistributedResample)
        ip = &partition;

    // We want all uncovered regions to get the default value.  That is
    // what the first argument of GetVariables is for.  But if the
    // default value is large, then it will screw up the collect call below,
    // which uses MPI_MAX for an all reduce.  So give uncovered regions very
    // small values now (-FLT_MAX) and then replace them later.
    float defaultPlaceholder = -FLT_MAX;
    samples->GetVolume()->GetVariables(defaultPlaceholder, vars, 
                                       numArrays, ip);

    if (!doDistributedResample)
    {
        //
        // Collect will perform the parallel collection.  Does nothing in
        // serial.  This will only be valid on processor 0.
        //
        for (i = 0 ; i < numArrays ; i++)
        {
            float *ptr = (float *) vars[i]->GetVoidPointer(0);
            Collect(ptr, vars[i]->GetNumberOfComponents()*width*height*depth);
        }
    }
    
    // Now replace the -FLT_MAX's with the default value.  (See comment above.)
    for (i = 0 ; i < numArrays ; i++)
    {
        float *ptr = (float *) vars[i]->GetVoidPointer(0);
        int numTups = width*height*depth*vars[i]->GetNumberOfComponents();
        for (j = 0 ; j < numTups ; j++)
            ptr[j] = (ptr[j] == defaultPlaceholder 
                             ? atts.GetDefaultVal() 
                             : ptr[j]);
    }
   
    bool iHaveData = false;
    if (doDistributedResample)
        iHaveData = true;
    if (PAR_Rank() == 0)
        iHaveData = true;
    if (height_end > height)
        iHaveData = false;
    if (iHaveData)
    {
        vtkRectilinearGrid *rg = CreateGrid(bounds, width, height, depth,
                                        width_start, width_end, height_start,
                                        height_end, cellCenteredOutput);

        if (doKernel)
        {
            float min_weight = avtPointExtractor::GetMinimumWeightCutoff();
            vtkDataArray *weights = vars[numArrays-1];
            int numVals = weights->GetNumberOfTuples();
            for (i = 0 ; i < realVars ; i++)
            {
                for (j = 0 ; j < vars[i]->GetNumberOfComponents() ; j++)
                {
                    for (k = 0 ; k < numVals ; k++)
                    {
                        float weight = weights->GetTuple1(k);
                        if (weight <= min_weight)
                            vars[i]->SetComponent(k, j, atts.GetDefaultVal());
                        else
                            vars[i]->SetComponent(k, j, 
                                         vars[i]->GetComponent(k, j) / weight);
                    }
                }
            }
        }

        //
        // Attach these variables to our rectilinear grid.
        //
        for (i = 0 ; i < realVars ; i++)
        {
            const char *varname = vars[i]->GetName();
            if (strcmp(varname, primaryVariable) == 0)
            {
                if (vars[i]->GetNumberOfComponents() == 3)
                    if (cellCenteredOutput)
                        rg->GetCellData()->SetVectors(vars[i]);
                    else
                        rg->GetPointData()->SetVectors(vars[i]);
                else if (vars[i]->GetNumberOfComponents() == 1)
                    if (cellCenteredOutput)
                        rg->GetCellData()->SetScalars(vars[i]);
                    else
                        rg->GetPointData()->SetScalars(vars[i]);
                else
                    if (cellCenteredOutput)
                        rg->GetCellData()->AddArray(vars[i]);
                    else
                        rg->GetPointData()->AddArray(vars[i]);
            }
            else
                if (cellCenteredOutput)
                    rg->GetCellData()->AddArray(vars[i]);
                else
                    rg->GetPointData()->AddArray(vars[i]);
        }

        avtDataTree_p tree = new avtDataTree(rg, 0);
        rg->Delete();
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

    for (i = 0 ; i < numArrays ; i++)
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
//      is3D         Whether or not we should do 3D resampling.
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
//    Hank Childs, Sat Apr 29 15:15:05 PDT 2006
//    Add support for 2D.
//
//    Hank Childs, Fri May 26 10:00:01 PDT 2006
//    Do not allow for 0 dimensions.
//
//    Hank Childs, Tue Sep  5 15:39:58 PDT 2006
//    Check for degenerate input.
//
// ****************************************************************************

void
avtResampleFilter::GetDimensions(int &width, int &height, int &depth,
                                 const double *bounds, bool is3D)
{
    if (atts.GetUseTargetVal())
    {
        // If we have a 2D data set, this is an easy test, so calculate the
        // sizes and return early.
        if (!is3D)
        {
            if (bounds[1] > bounds[0] && bounds[3] > bounds[2])
            {
                double amtX = bounds[1] - bounds[0];
                double amtY = bounds[3] - bounds[2];
                double ratio = amtY / amtX;
                double target = (double) atts.GetTargetVal();
                target /= (ratio + 1.);
                target = floor(sqrt(target)) + 1;
                width = (int) (target);
                height = (int) (target*ratio);
                depth = 1;
                return;
            }
        }

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
        width  = (width <= 1 ? 2 : width);
        height = (int)(amountInX * ratioY);
        height = (height <= 1 ? 2 : height);
        depth  = (int)(amountInX * ratioZ);
        depth  = (depth <= 1 ? 2 : depth);

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
    if (width <= 0 || height <= 0 || depth < 0)
    {
        EXCEPTION1(VisItException, "The grid to resample on is degenerate."
                  "Make sure that the number of samples in each direction "
                  "is positive.");
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
//  Modifications:
//
//    Hank Childs, Fri Feb 24 11:40:57 PST 2006
//    Tell the output that its topological dimension is the same as its
//    spatial dimension (ie lines and points get promoted using kernel
//    scheme).
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//    Since the resample filter creates a new mesh by taking into account
//    any inherent transforms, we clear this value for our new output mesh.
//
// ****************************************************************************

void
avtResampleFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(
                  GetInput()->GetInfo().GetAttributes().GetSpatialDimension());
    GetOutput()->GetInfo().GetAttributes().
                                         SetRectilinearGridHasTransform(false);
}


// ****************************************************************************
//  Method: CreateGrid
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
//      minX      The minimum X index for this processor.
//      maxX      The maximum Y index for this processor.
//      minY      The minimum X index for this processor.
//      maxY      The maximum Y index for this processor.
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
//    Hank Childs, Fri Sep 30 10:50:24 PDT 2005
//    Add support for distributed resampling.
//
//    Hank Childs, Fri Jun  1 16:17:51 PDT 2007
//    Add support for cell-centered data.
//
// ****************************************************************************

vtkRectilinearGrid *
CreateGrid(const double *bounds, int numX, int numY, int numZ, int minX,
           int maxX, int minY, int maxY, bool cellCenteredOutput)
{
    vtkDataArray *xc = NULL;
    vtkDataArray *yc = NULL;
    vtkDataArray *zc = NULL;

    float width  = bounds[1] - bounds[0];
    float height = bounds[3] - bounds[2];
    float depth  = bounds[5] - bounds[4];

    int numX2 = (cellCenteredOutput ? numX+1 : numX);
    int maxX2 = (cellCenteredOutput ? maxX+1 : maxX);
    xc = GetCoordinates(bounds[0], width, numX2, minX, maxX2);
    int numY2 = (cellCenteredOutput ? numY+1 : numY);
    int maxY2 = (cellCenteredOutput ? maxY+1 : maxY);
    yc = GetCoordinates(bounds[2], height, numY2, minY, maxY2);
    int numZ2 = (cellCenteredOutput ? numZ+1 : numZ);
    zc = GetCoordinates(bounds[4], depth, numZ2, 0, numZ2);
      
    vtkRectilinearGrid *rv = vtkRectilinearGrid::New();
    if (cellCenteredOutput)
        rv->SetDimensions(maxX-minX+1, maxY-minY+1, numZ+1);
    else
        rv->SetDimensions(maxX-minX, maxY-minY, numZ);
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
//  Returns:     A vtkDataArray element for the coordinate.
//
//  Programmer:  Hank Childs
//  Creation:    March 26, 2001
//
//  Modifications:
//    Kathleen Bonnell, Mon Nov 19 15:31:36 PST 2001
//    Changes in VTK 4.0 API require use of vtkDataArray /vtkFloatArray
//    in place of vtkScalars. 
//
//    Hank Childs, Fri Sep 30 10:50:24 PDT 2005
//    Add support for distributed resampling.
//
// ****************************************************************************

vtkDataArray *
GetCoordinates(float start, float length, int numEls, int myStart, int myStop)
{
    vtkFloatArray *rv = vtkFloatArray::New();

    //
    // Make sure we don't have any degenerate cases here.
    //
    if (length <= 0. || numEls <= 1 || myStart >= myStop)
    {
        rv->SetNumberOfValues(1);
        rv->SetValue(0, start);
        return rv;
    }

    int realNumEls = myStop - myStart;
    rv->SetNumberOfValues(realNumEls);
    float offset = length / (numEls-1);
    for (int i = myStart ; i < myStop ; i++)
    {
        rv->SetValue(i-myStart, start + i*offset);
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
//    Hank Childs, Sat Jan 29 11:01:59 PST 2005
//    If we are going to use an arbitrator, make sure to request the variable.
//
//    Hank Childs, Sun Mar 13 10:00:01 PST 2005
//    Tell filters upstream that we have rectilinear optimizations.
//
//    Hank Childs, Tue Jun  7 14:04:39 PDT 2005
//    Turn off ghost data, since ghost data created upstream will not be
//    pertinent after resampling.
//
//    Mark C. Miller, Tue Sep 13 20:09:49 PDT 2005
//    Added support for resample data selection
//
// ****************************************************************************

avtPipelineSpecification_p
avtResampleFilter::PerformRestriction(avtPipelineSpecification_p oldspec)
{
    //
    // Best copy constructor we have??
    //
    avtPipelineSpecification_p spec = new avtPipelineSpecification(oldspec,
                                              oldspec->GetDataSpecification());

    //
    // First tell the file format reader that we are going to be doing a
    // resample selection.
    //
    avtResampleSelection *sel = new avtResampleSelection;
    int counts[3];
    counts[0] = atts.GetWidth();
    counts[1] = atts.GetHeight();
    counts[2] = atts.GetDepth();
    sel->SetCounts(counts);
    double starts[3];
    starts[0] = atts.GetMinX();
    starts[1] = atts.GetMinY();
    starts[2] = atts.GetMinZ();
    sel->SetStarts(starts);
    double stops[3];
    stops[0] = atts.GetMaxX();
    stops[1] = atts.GetMaxY();
    stops[2] = atts.GetMaxZ();
    sel->SetStops(stops);
    selID = spec->GetDataSpecification()->AddDataSelection(sel);

    spec->NoDynamicLoadBalancing();
    spec->SetHaveRectilinearMeshOptimizations(true);
    spec->GetDataSpecification()->SetDesiredGhostDataType(NO_GHOST_DATA);
    if (atts.GetUseArbitrator())
    {
        if (atts.GetArbitratorVarName() != "default")
            spec->GetDataSpecification()->
                     AddSecondaryVariable(atts.GetArbitratorVarName().c_str());
    }
    if (primaryVariable != NULL)
    {
        delete [] primaryVariable;
    }
    const char *pv = spec->GetDataSpecification()->GetVariable();
    primaryVariable = new char[strlen(pv)+1];
    strcpy(primaryVariable, pv);
    return spec;
}


// ****************************************************************************
//  Method:  avtResampleFilter::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************

bool
avtResampleFilter::FilterUnderstandsTransformedRectMesh()
{
    // Resampling has been extended to understand these meshes.
    return true;
}
