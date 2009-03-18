/*****************************************************************************
*
* Copyright (c) 2000 - 2008, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                         avtParallelCoordinatesFilter.C                    //
// ************************************************************************* //

#include <avtParallelCoordinatesFilter.h>

#include <PlotInfoAttributes.h>

#include <limits.h>
#include <float.h>
#include <math.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <visitstream.h>

#include <sys/types.h>

#include <vector>
#include <string>

#include <vtkDataSet.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkVisItScalarTree.h>

#include <avtCallback.h>
#include <avtDataAttributes.h>
#include <avtDataRangeSelection.h>
#include <avtDataTree.h>
#include <avtExtents.h>
#include <avtIdentifierSelection.h>
#include <avtIntervalTree.h>
#include <avtMetaData.h>
#include <avtNamedSelection.h>
#include <avtParallel.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidLimitsException.h>
#include <NoDefaultVariableException.h>
#include <InvalidDimensionsException.h>
#include <TimingsManager.h>


static void
UpdateLimitsWithAllHSTimeSteps(int axis,
                              const std::vector<avtHistogramSpecification*> &h,
                              double &varmin, double &varmax);
static void
CheckHistograms(int axis,
                const std::vector<avtHistogramSpecification*> &h);

// ****************************************************************************
//  Method: avtParallelCoordinatesFilter
//
//  Purpose: Constructor for the avtParallelCoordinatesFilter class.
//
//  Arguments:
//      atts    The ParallelCoordinates plot attributes.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
//    Jeremy Meredith, Mon Feb  4 15:46:42 EST 2008
//    Some more distillation and related cleanup.
//
//    Hank Childs, Mon Mar  3 19:42:29 PST 2008
//    Added more data members.
//
//    Jeremy Meredith, Fri Mar  7 14:27:25 EST 2008
//    Allow multiple histogram sets (for multiple time steps).
//
//    Jeremy Meredith, Wed Feb 25 15:25:20 EST 2009
//    Port to trunk.
//
// ****************************************************************************

avtParallelCoordinatesFilter::avtParallelCoordinatesFilter(
                                           ParallelCoordinatesAttributes &atts)
{
    parCoordsAtts       = atts;
    gotHistogramsFromDB = false;
}


// ****************************************************************************
//  Method: ~avtParallelCoordinatesFilter
//
//  Purpose: Destructor for the avtParallelCoordinatesFilter class.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// ****************************************************************************

avtParallelCoordinatesFilter::~avtParallelCoordinatesFilter()
{
    return;
}


// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::ModifyContract
//
//  Purpose: Disable streaming.  We can eventually do an interval tree here.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  7, 2008
//
//  Modifications:
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Hank Childs, Tue Feb 26 15:54:49 PST 2008
//    Try to get a histogram specification directly from the database.
//
//    Hank Childs, Mon Mar  3 19:42:29 PST 2008
//    Try to get a second histogram specification if we have a selection.
//
//    Hank Childs, Wed Mar  5 16:06:41 PST 2008
//    Add some primitive support for time varying data.
//
//    Hank Childs, Wed Mar  5 20:08:38 PST 2008
//    Add support for named selections.
//
//    Jeremy Meredith, Fri Mar  7 11:51:40 EST 2008
//    Changed support for time varying data: it now comes from the user
//    attributes, and it also adds support for strides.
//
//    Jeremy Meredith, Fri Mar  7 14:27:25 EST 2008
//    Allow multiple histogram sets (for multiple time steps).
//
//    Jeremy Meredith, Thu Mar 27 16:36:36 EDT 2008
//    Force it to do a focus histogram, even if no extents were applied,
//    if the user asked for them in the attributes.
//    Also, disable all histogram optimizations if the user
//    requested a full data point rendering mode.
//
//    Jeremy Meredith, Wed Feb 25 15:18:58 EST 2009
//    Port to trunk.
//    Removed named selection support (since it's being done differently).
//    Added missing check to see if we want to draw focus at all.
//
// ****************************************************************************

avtContract_p
avtParallelCoordinatesFilter::ModifyContract(avtContract_p in_contract)
{
    int timer1 = visitTimer->StartTimer();
   
    avtContract_p out_contract = new avtContract(in_contract);
    
    // The extension _Local is to designate that this variable is local
    // to this method.
    bool isArrayVar_Local = false;
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (!atts.ValidVariable(pipelineVariable) ||
         atts.GetVariableType(pipelineVariable) == AVT_ARRAY_VAR)
       isArrayVar_Local = true;

    if (!parCoordsAtts.GetForceFullDataFocus() &&
        GetInput()->GetInfo().GetValidity().GetZonesPreserved() &&
        !isArrayVar_Local)
    {
        // Problems with the multiple time slice route:
        // (1) FIXED!
        //     Right now the startTime and endTime are hardcoded ... I still
        //     need to add infrastructure to figure out what the last 
        //     time slice is.
        // (2) Each time slice is loaded into the same "histograms" array.
        //     ... that array needs to have more entries for each time slice.
        bool doMultipleTimeslices = false;//TODO: parCoordsAtts.GetDoTime();
        int startTime  = 0;//TODO: parCoordsAtts.GetTimeBegin();
        int endTime    = 0;//TODO: parCoordsAtts.GetTimeEnd();
        int strideTime = 1;//TODO: parCoordsAtts.GetTimeStride();
        vector<int>  allTimeIndexes;
        vector<bool> indexesIShouldProcess;
        if (doMultipleTimeslices)
        {
            int nProcs = PAR_Size();
            int numTimeSlices = 1 + (endTime - startTime)/strideTime;
            if (numTimeSlices > PCP_MAX_TIMESTEPS)
            {
                EXCEPTION1(ImproperUseException,
                           "We only support a fixed limit on the number "
                           "of time steps at the moment, and your "
                           "time step selection exceeds it.");
            }
            int amountPer = numTimeSlices / nProcs;
            int oneExtraUntil = numTimeSlices % nProcs;
            int lastSlice = startTime;
            for (int i = 0 ; i < nProcs ; i++)
            {
                int amount = amountPer + (i < oneExtraUntil ? 1 : 0);
                for (int j = 0 ; j < amount ; j++)
                {
                    allTimeIndexes.push_back(lastSlice);
                    indexesIShouldProcess.push_back(i == PAR_Rank());
                    lastSlice += strideTime;
                }
            }
        }
        else
        {
            int ts = in_contract->GetDataRequest()->GetTimestep();
            allTimeIndexes.push_back(ts);
            indexesIShouldProcess.push_back(PAR_Rank() == 0);
        }

        axisCount = parCoordsAtts.GetScalarAxisNames().size();
        histograms.resize(allTimeIndexes.size());
        histogramsForSelectedRegion.resize(allTimeIndexes.size());
        int nctxparts = parCoordsAtts.GetContextNumPartitions();
        int nlineparts = parCoordsAtts.GetLinesNumPartitions();
        stringVector curAxisVarNames = parCoordsAtts.GetScalarAxisNames();
        bool success = true;

        //
        // THIS IS WHERE WE SET UP THE CONTEXT HISTOGRAMS
        //
        for (int t = 0 ; t < allTimeIndexes.size() ; t++)
        {
            if (!indexesIShouldProcess[t])
                continue;

            int timer2 = visitTimer->StartTimer();
            
            histograms[t] = new avtHistogramSpecification[axisCount-1];
            for (int i = 0 ; i < axisCount-1 ; i++)
            {
                histograms[t][i].SpecifyHistogram(
                              allTimeIndexes[t],
                              curAxisVarNames[i],   //variable1
                              nctxparts,            //number of bins 1
                              curAxisVarNames[i+1], //variable 2
                              nctxparts,            //number of bins 2
                              "",//TODO: ConvertNamedSelectionToCondition(),
                              true ,                //condition is exact
                              true                  //use regular binning
                              );
                success = GetMetaData()->GetHistogram(histograms[t] + i);
                if (!success)
                    break; // out of requesting 2D histograms
            }
            
            visitTimer->StopTimer(timer2, "avtParallelCoordinatesFilter::ModifyContract() Compute context histograms");  
            
            if (!success)
                break; // out of iterating over time

            int numVars = parCoordsAtts.GetExtentMinima().size();
            bool haveSelection = false;
            for (int i = 0 ; i < numVars ; i++)
            {
                if (parCoordsAtts.GetExtentMinima()[i] > -1e+36)
                {
                    haveSelection = true;
                    break; // out of checking if we have a selection
                }
                if (parCoordsAtts.GetExtentMaxima()[i] < +1e+36)
                {
                    haveSelection = true;
                    break; // out of checking if we have a selection
                }
            }
            
            if (parCoordsAtts.GetDrawLines() &&
                (haveSelection || !parCoordsAtts.GetDrawLinesOnlyIfExtentsOn()))
            {
                int timer3 = visitTimer->StartTimer();
                  
                histogramsForSelectedRegion[t] = new avtHistogramSpecification[axisCount-1];
                for (int j = 0 ; j < axisCount-1 ; j++)
                {
                    //
                    // THIS IS WHERE WE REQUEST THE FOCUS HISTOGRAM
                    //
                    histogramsForSelectedRegion[t][j].SpecifyHistogram(
                          allTimeIndexes[t],
                          curAxisVarNames[j], nlineparts, 
                          curAxisVarNames[j+1], nlineparts, 
                          ConvertExtentsToCondition(),
                          true , 
                          true 
                        );
                
                    success = GetMetaData()->GetHistogram(histogramsForSelectedRegion[t] + j);
                    if (!success)
                        break; // out of requesting 2D histograms
                }
                visitTimer->StopTimer(timer3, "avtParallelCoordinatesFilter::ModifyContract() Compute focus histograms");    
            }
            if (!success)
                break; // out of iterating over time
        }

        int iHadFailure = (success ? 0 : 1);
        int anyoneHadFailure = UnifyMaximumValue(iHadFailure);
        if (anyoneHadFailure)
        {
            for (int t=0; t<allTimeIndexes.size(); t++)
            {
                delete [] histograms[t];
                delete [] histogramsForSelectedRegion[t];
            }
            histograms.clear();
            histogramsForSelectedRegion.clear();

            gotHistogramsFromDB = false;
        }
        else
        {
            // Turn off the processing of all data 
            // ... send in an empty domains list
            vector<int> domains;
            out_contract->GetDataRequest()->GetRestriction()->RestrictDomains(domains);

            gotHistogramsFromDB = true;
        }
    }
    else
        gotHistogramsFromDB = false;

    out_contract->NoStreaming();
    
    visitTimer->StopTimer(timer1, "avtParallelCoordinatesFilter::ModifyContract()");  

    return out_contract;
}


// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::PreExecute
//
//  Purpose: Computes the data extents of all relevant scalar variables.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    No longer writes scratch file, which is no longer necessary.  Also added
//    support for non-uniform axis spacing.
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now supports all variable axis spacing and axis group conventions.
//
//    Jeremy Meredith, Fri Mar 16 13:50:26 EDT 2007
//    Create colormap labels from both the datacurve/annotation labels and
//    the context labels.  Also initialize the context accumulation bins.
//
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
//    Jeremy Meredith, Mon Feb  4 15:46:42 EST 2008
//    Some more distillation and related cleanup.
//
//    Jeremy Meredith, Wed Feb  6 16:13:17 EST 2008
//    Initialize axisCount to zero right away, just in case we error out.
//
//    Jeremy Meredith, Thu Feb  7 17:46:48 EST 2008
//    Handle array variables.
//
//    Jeremy Meredith, Mon Feb 11 17:47:12 EST 2008
//    More precise support for sendNullOutput since it can get triggered if
//    nprocs > ndomains, so we need to make sure we do the right thing with
//    parallel communication.
//
//    Jeremy Meredith, Fri Feb 15 13:16:46 EST 2008
//    Renamed orderedAxisNames to scalarAxisNames to distinguish these
//    as names of actual scalars instead of just display names.
//
//    Jeremy Meredith, Tue Mar  4 18:17:33 EST 2008
//    Did some renaming and cleanup, changing focus to always be drawn
//    using a histogram.
//
//    Jeremy Meredith, Fri Mar  7 14:27:25 EST 2008
//    Allow multiple histogram sets (for multiple time steps).
//
//    Jeremy Meredith, Fri Mar  7 19:00:20 EST 2008
//    Added primitive support for multiple timesteps to use multiple colors.
//
//    Jeremy Meredith, Thu Mar 27 16:38:18 EDT 2008
//    Allow user to force into the mode using individual data point lines
//    for the focus instead of using a histogram.
//
//    Jeremy Meredith, Wed Feb 25 15:22:58 EST 2009
//    Port to trunk.
//    Removed time support (not added to this plot yet).
//
//    Jeremy Meredith, Wed Mar 18 12:30:52 EDT 2009
//    Check for a zero axis count and report as an error.
//
// *****************************************************************************

void
avtParallelCoordinatesFilter::PreExecute(void)
{
    int timer1 = visitTimer->StartTimer();
          
    avtDatasetToDatasetFilter::PreExecute();

    axisCount = parCoordsAtts.GetScalarAxisNames().size();
    PrepareForArrayVariable();
    if (axisCount == 0)
        EXCEPTION1(ImproperUseException, "Could not determine number of "
                   "variables in plot; wasn't either a collection of "
                   "variables, or an array variable.");


    if (!parCoordsAtts.AttributesAreConsistent())
    {
        debug3 << "PCP/aPAF/PE1: ParallelCoordinates plot attributes are inconsistent."
               << endl;
        
        sendNullOutput = true;
        return;
    }

    sendNullOutput = false;

    ComputeCurrentDataExtentsOverAllDomains();
    InitializeDataTupleInput();
    
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
   
    CreateLabels();

    // We need to create the colormap labels from the focus
    // and context labels.  This ordering must match what's created
    // in the avtParallelCoordinatesPlot, and the intent is that the context
    // is drawn underneath the focus.
    stringVector colorMapLabels;
    bool overTime = false;//TODO: parCoordsAtts.GetDoTime();
    int numTimeSteps = overTime ? PCP_MAX_TIMESTEPS : 1;
    for (int t=0; t<numTimeSteps; t++)
    {
        colorMapLabels.insert(colorMapLabels.end(),
                              contextLabels[t].begin(),contextLabels[t].end());
    }
    for (int t=0; t<numTimeSteps; t++)
    {
        colorMapLabels.insert(colorMapLabels.end(),
                              focusLabels[t].begin(),focusLabels[t].end());
    }
    outAtts.SetLabels(colorMapLabels);
    
    // initialize things needed for drawing the context
    if (parCoordsAtts.GetDrawContext() && histograms.size() == 0)
    {
        InitializeContextHistograms();
    }

    if (parCoordsAtts.GetDrawLines() &&
        (!parCoordsAtts.GetDrawLinesOnlyIfExtentsOn() || extentsApplied) &&
        histogramsForSelectedRegion.size() == 0)
    {
        if (! parCoordsAtts.GetForceFullDataFocus())
            InitializeFocusHistograms();
    }
    
    visitTimer->StopTimer(timer1, "avtParallelCoordinatesFilter::PreExecute()");   
}


// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::PostExecute
//
//  Purpose: Changes the vis window's spatial extents to match the viewport of
//          the plot's curves.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Axis attribute for outside queries now stored after filter is executed.
//
//    Jeremy Meredith, Fri Mar 16 13:50:26 EDT 2007
//    Draw the context, and clean up.
//
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
//    Jeremy Meredith, Wed Feb  6 16:13:34 EST 2008
//    Abort early in the case of earlier errors.
//
//    Jeremy Meredith, Thu Feb  7 17:46:59 EST 2008
//    Handle array variables and bin-defined axis x-positions.
//
//    Jeremy Meredith, Mon Feb 11 17:47:12 EST 2008
//    More precise support for sendNullOutput since it can get triggered if
//    nprocs > ndomains, so we need to make sure we do the right thing with
//    parallel communication.
//
//    Hank Childs, Tue Feb 26 17:03:26 PST 2008
//    Add handling for the case where we get the histograms directly from the
//    database.
//
//    Jeremy Meredith, Tue Mar  4 18:17:33 EST 2008
//    Did some renaming and cleanup, changing focus to always be drawn
//    using a histogram.
//
//    Jeremy Meredith, Wed Mar  5 12:18:28 EST 2008
//    Moved the handling for histograms directly from a DB into
//    ComputeCurrentDataExtentsOverAllDomains so it's handled in PreExecute.
//
//    Jeremy Meredith, Fri Mar  7 19:00:20 EST 2008
//    Added primitive support for multiple timesteps to use multiple colors.
//
//    Hank Childs, Sun Mar  9 11:09:38 PDT 2008
//    Fix assumption that we always have a data selection.
//
//    Jeremy Meredith, Thu Mar 27 16:38:18 EDT 2008
//    Allow user to force into the mode using individual data point lines
//    for the focus instead of using a histogram.
//
//    Jeremy Meredith, Wed Feb 25 15:24:48 EST 2009
//    Port to trunk.
//
// ***************************************************************************

void
avtParallelCoordinatesFilter::PostExecute(void)
{
    int timer1 = visitTimer->StartTimer();
      
    avtDatasetToDatasetFilter::PostExecute();
    if (!sendNullOutput)
    {
        avtDataAttributes &inAtts  = GetInput()->GetInfo().GetAttributes();
        avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
   
        outAtts.GetTrueSpatialExtents()->Clear();
        outAtts.GetCumulativeTrueSpatialExtents()->Clear();

        double spatialExtents[6];

        if (inAtts.GetTrueSpatialExtents()->HasExtents())
        {
            inAtts.GetTrueSpatialExtents()->CopyTo(spatialExtents);

            spatialExtents[0] = axisCount<2 ? 0 : axisXPositions[0];
            spatialExtents[1] = axisCount<2 ? 1 : axisXPositions[axisCount-1];
            spatialExtents[2] = 0.0;
            spatialExtents[3] = 1.0;

            outAtts.GetCumulativeTrueSpatialExtents()->Set(spatialExtents);
        }
        else if (inAtts.GetCumulativeTrueSpatialExtents()->HasExtents())
        {
            inAtts.GetCumulativeTrueSpatialExtents()->CopyTo(spatialExtents);

            spatialExtents[0] = axisCount<2 ? 0 : axisXPositions[0];
            spatialExtents[1] = axisCount<2 ? 1 : axisXPositions[axisCount-1];
            spatialExtents[2] = 0.0;
            spatialExtents[3] = 1.0;

            outAtts.GetCumulativeTrueSpatialExtents()->Set(spatialExtents);
        }

        outAtts.SetXLabel("");
        outAtts.SetYLabel("");

        outAtts.SetXUnits("");
        outAtts.SetYUnits("");
    }

    for (int t=0; t<histograms.size(); t++)
        DrawContext(t);

    if (! parCoordsAtts.GetForceFullDataFocus())
    {
        for (int t=0; t<histogramsForSelectedRegion.size(); t++)
            if (histogramsForSelectedRegion[t] != NULL)
                DrawFocusHistograms(t);
    }

    CleanUpAllHistograms();
    visitTimer->StopTimer(timer1, "avtParallelCoordinatesFilter::PostExecute()");     
}

// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::ExecuteDataTree
//
//  Purpose: Processes one domain chunk of the input mesh into corresponding
//           curves of a ParallelCoordinates plot.
//
//  Arguments:
//      in_ds  :  The input dataset.
//      domain :  The domain number of the input dataset.
//
//  Returns: The output dataset (curves of the ParallelCoordinates plot).
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Mark Blair, Fri Jun  9 14:29:00 PDT 2006
//    Rewrote to eliminate use of the ParallelCoordinatePlot class.  Code
//    adapted from that class was added directly to the filter code.
//
//    Mark Blair, Wed Aug 16 16:46:00 PDT 2006
//    Added check for attribute consistency.
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now supports all variable axis spacing and axis group conventions.
//
//    Jeremy Meredith, Fri Mar 16 13:50:26 EDT 2007
//    Count each tuple in the appropriate pairwise axis bins.
//    Avoid drawing data curves (lines) if we didn't ask for them.
//    Renamed data set labels specific to data curves and axis annotations.
//
//    Jeremy Meredith, Wed Mar 21 18:22:08 EDT 2007
//    Added a new setting which will hide the lines until the extents
//    tool has actually been enabled and thresholded the range of one or
//    more axes.  This allows the "lines" to be used strictly as a "focus".
//
//    Jeremy Meredith, Fri May 18 09:41:30 EDT 2007
//    Only add the individual tuples to the data sets if the lines are
//    actually going to be drawn.  This lets situtions where only the context
//    is drawn scale easily to huge datasets without going into SR mode.
//
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
//    Jeremy Meredith, Mon Feb  4 15:46:42 EST 2008
//    Some more distillation and related cleanup.
//
//    Jeremy Meredith, Thu Feb  7 17:46:59 EST 2008
//    Handle array variables and bin-defined axis x-positions.
//
//    Jeremy Meredith, Fri Feb 15 13:16:46 EST 2008
//    Renamed orderedAxisNames to scalarAxisNames to distinguish these
//    as names of actual scalars instead of just display names.
//
//    Jeremy Meredith, Tue Mar  4 18:17:33 EST 2008
//    Did some renaming and cleanup, changing focus to always be drawn
//    using a histogram.
//
//    Jeremy Meredith, Thu Mar 27 16:38:18 EDT 2008
//    Allow user to force into the mode using individual data point lines
//    for the focus instead of using a histogram.
//
//    Jeremy Meredith, Wed Feb 25 15:26:29 EST 2009
//    Port to trunk.
//
// ****************************************************************************

avtDataTree_p 
avtParallelCoordinatesFilter::ExecuteDataTree(vtkDataSet *in_ds, int domain, string label)
{
    // NOTE: we will never enter this function for the case where we have
    // gotten a histogram specification from the database.

    if (sendNullOutput) return NULL;

    if (in_ds == NULL)
    {
        debug3 << "PCP/aPAF/EDT/1: ParallelCoordinates plot input is NULL." << endl;
        return NULL;
    }

    int timer1 = visitTimer->StartTimer();

    stringVector curAxisVarNames = parCoordsAtts.GetScalarAxisNames();
    
    int cellArrayCount  = in_ds->GetCellData()->GetNumberOfArrays();
    int pointArrayCount = in_ds->GetPointData()->GetNumberOfArrays();
    int cellCount       = in_ds->GetNumberOfCells();
    int pointCount      = in_ds->GetNumberOfPoints();

    bool plotCellData  = (cellArrayCount > 0);
    int plotTupleCount = (plotCellData) ? cellCount : pointCount;

    if (cellArrayCount + pointArrayCount == 0)
    {
        debug3 << "PCP/aPAF/EDT/3: No ParallelCoordinates plot input data arrays found."
               << endl;
        visitTimer->StopTimer(timer1, "avtParallelCoordinatesFilter::ExecuteDataTree()");
        return NULL;
    }

    if (plotTupleCount == 0)
    {
        debug3 << "PCP/aPAF/EDT/4: ParallelCoordinates plot input data array "
               << "is of zero length." << endl;
        visitTimer->StopTimer(timer1, "avtParallelCoordinatesFilter::ExecuteDataTree()");
        return NULL;
    }

    // If the input contains both cell data and point data, then by convention
    // the cell data takes precedence.  In this case, the value of a point
    // variable that is plotted for a given cell, by convention, is the average
    // value of that variable over all vertices of the cell.  (mb)

    const std::string pipeVariableName = pipelineVariable;

    int axisNum;
    int tupleCount, tupleNum, componentCount;
    int cellVertexCount, vertexNum, valueNum;
    bool arrayIsCellData, dataBadOrMissing;
    std::string arrayName;
    vtkDataArray *dataArray;
    vtkIdList *pointIdList;
    float *arrayValues;
    float valueSum;
    
    std::vector<float *> varArrayValues;
    boolVector           varIsCellData;
    intVector            varComponentCounts;

    if (parCoordsAtts.GetForceFullDataFocus())
        InitializeFocusPolyData();

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        if (isArrayVar)
            arrayName = pipelineVariable;
        else
            arrayName = curAxisVarNames[axisNum];

        dataArray = in_ds->GetCellData()->GetArray(arrayName.c_str());
        arrayIsCellData = true;
        tupleCount = cellCount;

        if (dataArray == NULL)
        {
            dataArray = in_ds->GetPointData()->GetArray(arrayName.c_str());
            arrayIsCellData = false;
            tupleCount = pointCount;
        }

        if (dataArray == NULL)
            dataBadOrMissing = true;
        else if (dataArray->GetDataType() != VTK_FLOAT)
            dataBadOrMissing = true;
        else if (dataArray->GetNumberOfTuples() != tupleCount)
            dataBadOrMissing = true;
        else
            dataBadOrMissing = false;
            
        if (dataBadOrMissing)
        {
            debug3 << "PCP/aPAF/EDT/5: ParallelCoordinates plot input data array "
                   << arrayName << " is bad or missing." << endl;
            visitTimer->StopTimer(timer1, "avtParallelCoordinatesFilter::ExecuteDataTree()");
            return NULL;
        }
    
        if (isArrayVar)
            varArrayValues.push_back(((float *)dataArray->GetVoidPointer(0))
                                     + axisNum);
        else
            varArrayValues.push_back((float *)dataArray->GetVoidPointer(0));
        varIsCellData.push_back(arrayIsCellData);
        varComponentCounts.push_back(dataArray->GetNumberOfComponents());
    }
    

    floatVector inputTuple = floatVector(axisCount);
    
    bool drawLines = false;
    if (parCoordsAtts.GetDrawLines() &&
        (!parCoordsAtts.GetDrawLinesOnlyIfExtentsOn() || extentsApplied))
    {
        drawLines = true;
    }
    bool drawContext = parCoordsAtts.GetDrawContext();

    if (plotCellData && (pointArrayCount > 0))
    {
        pointIdList = vtkIdList::New();
    }

    if (plotCellData)
    {
        for (tupleNum = 0; tupleNum < cellCount; tupleNum++)
        {
            for (axisNum = 0; axisNum < axisCount; axisNum++)
            {
                arrayValues     = varArrayValues[axisNum];
                componentCount  = varComponentCounts[axisNum];

                if (varIsCellData[axisNum])
                {
                    inputTuple[axisNum] =
                    arrayValues[tupleNum*componentCount];
                }
                else
                {
                    in_ds->GetCellPoints(tupleNum, pointIdList);
                    cellVertexCount = pointIdList->GetNumberOfIds();

                    valueSum = 0.0;

                    for (vertexNum = 0; vertexNum < cellVertexCount; vertexNum++)
                    {
                        valueNum = pointIdList->GetId(vertexNum)*componentCount;
                        valueSum += arrayValues[valueNum];
                    }

                    if (cellVertexCount == 0) cellVertexCount = 1;

                    inputTuple[axisNum] = valueSum / (float)cellVertexCount;
                }
            }
            
            // TODO: DOESN'T SUPPORT MULTIPLE TIME STEPS; hardcoded to 0
            if (drawLines)
                if (parCoordsAtts.GetForceFullDataFocus())
                    AppendDataTupleFocus(inputTuple);
                else
                    CountDataTupleFocus(0, inputTuple);
            if (drawContext)
                CountDataTupleContext(0, inputTuple);
        }
    }
    else
    {
        for (tupleNum = 0; tupleNum < pointCount; tupleNum++)
        {
            for (axisNum = 0; axisNum < axisCount; axisNum++)
            {
                arrayValues     = varArrayValues[axisNum];
                componentCount  = varComponentCounts[axisNum];

                inputTuple[axisNum] =
                arrayValues[tupleNum*componentCount];
            }
            
            // TODO: DOESN'T SUPPORT MULTIPLE TIME STEPS; hardcoded to 0
            if (drawLines)
                if (parCoordsAtts.GetForceFullDataFocus())
                    AppendDataTupleFocus(inputTuple);
                else
                    CountDataTupleFocus(0, inputTuple);
            if (drawContext)
                CountDataTupleContext(0, inputTuple);
        }
    }

    if (plotCellData && (pointArrayCount > 0))
    {
        pointIdList->Delete();
    }

    if (drawLines && parCoordsAtts.GetForceFullDataFocus())
    {
        DrawFocusPolyLines();

        vtkDataSet **outputDataSets = new vtkDataSet *[1];

        outputDataSets[0] = dataCurvePolyData;

        avtDataTree *outputDataTree =
            new avtDataTree(1, outputDataSets, domain, focusLabels[0]);

        outputDataSets[0]->Delete();
        delete [] outputDataSets;

        visitTimer->StopTimer(timer1, "avtParallelCoordinatesFilter::ExecuteDataTree()");
        return outputDataTree;
    }
    else
    {
        visitTimer->StopTimer(timer1, "avtParallelCoordinatesFilter::ExecuteDataTree()");
        return NULL;
    }

    visitTimer->StopTimer(timer1, "avtParallelCoordinatesFilter::ExecuteDataTree()");
}


// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::UpdateDataObjectInfo
//
//  Purpose: Indicates that the topological dimension of the output is not the
//           same as the input.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::UpdateDataObjectInfo(void)
{
    avtDataAttributes &inAtts  = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    
    outAtts.SetTopologicalDimension(inAtts.GetTopologicalDimension()-1);

    GetOutput()->GetInfo().GetValidity().InvalidateZones();

    outAtts.SetSpatialDimension(2);
}


// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::CreateLabels
//
//  Purpose: Create the labels to be used for the data set arrays.
//           These are also concatenated into colormap labels elsewhere
//           in this filter.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Jeremy Meredith, Fri Mar 16 13:54:09 EDT 2007
//    Also create labels for the context data sets.  Renamed the old labels
//    so it's clear they are specific to the data curves and axes annotations.
//
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
//    Jeremy Meredith, Fri Mar  7 19:00:20 EST 2008
//    Added primitive support for multiple timesteps to use multiple colors.
//
//    Jeremy Meredith, Wed Feb 25 15:28:50 EST 2009
//    Port to trunk.
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::CreateLabels()
{
    char str[100];
    for (int t=0; t<PCP_MAX_TIMESTEPS; t++)
    {
        focusLabels[t].clear();
        contextLabels[t].clear();

        sprintf(str, "Data Curves, time %03d", t);
        focusLabels[t].push_back(str);
        for (int i=0; i<PCP_CTX_BRIGHTNESS_LEVELS; i++)
        {
            sprintf(str, "Data Context level %03d, time %03d", i, t);
            contextLabels[t].push_back(str);
        }
    }
}


// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::ReleaseData
//
//  Purpose: Release all problem-sized data associated with this filter.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::ReleaseData(void)
{
    avtSIMODataTreeIterator::ReleaseData();
}


// ****************************************************************************
//  Method: avtParallelCoordinatesPlot::ComputeCurrentDataExtentsOverAllDomains
//
//  Purpose: Computes extent of each axis's scalar variable.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
//    Jeremy Meredith, Mon Feb  4 15:46:42 EST 2008
//    Some more distillation and related cleanup.
//
//    Jeremy Meredith, Wed Feb  6 16:14:05 EST 2008
//    Resize axisMinima and axisMaxima early, just in case.
//
//    Jeremy Meredith, Thu Feb  7 17:46:59 EST 2008
//    Handle array variables and bin-defined axis x-positions.
//
//    Jeremy Meredith, Fri Feb  8 12:32:36 EST 2008
//    Support unifying extents across all axes.
//
//    Jeremy Meredith, Fri Feb 15 13:16:46 EST 2008
//    Renamed orderedAxisNames to scalarAxisNames to distinguish these
//    as names of actual scalars instead of just display names.
//
//    Jeremy Meredith, Wed Mar  5 12:18:28 EST 2008
//    Moved the handling for histograms directly from a DB from PostExecute
//    into this function so we get the right ranges before drawing.
//
//    Jeremy Meredith, Fri Mar  7 14:27:25 EST 2008
//    Allow multiple histogram sets (for multiple time steps).
//
//    Jeremy Meredith, Mon Mar 10 15:29:32 EDT 2008
//    Support unification of axis extents across all time steps.
//
//    Jeremy Meredith, Wed Feb 25 16:35:47 EST 2009
//    Port to trunk.  Removed named selection stuff since it's
//    done differently now.
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::ComputeCurrentDataExtentsOverAllDomains()
{
    int timer1 = visitTimer->StartTimer();
    
    axisXPositions.resize(axisCount);
    axisMinima.resize(axisCount);
    axisMaxima.resize(axisCount);

    if (isArrayVar)
    {
        avtDataAttributes &inatts = GetInput()->GetInfo().GetAttributes();
        avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();

        // Get the actual extents for the array variable
        int dim = inatts.GetVariableDimension(pipelineVariable);
        const vector<double> &bins = inatts.GetVariableBinRanges(
                                                             pipelineVariable);
        avtExtents *ext = inatts.GetVariableComponentExtents(pipelineVariable);
        if (!ext || !ext->HasExtents())
        {
            debug3 << "PCP/aPAF/CCDEOAD/1: Didn't have array var component "
                   << "extents." << endl;
            sendNullOutput = true;
            return;
        }

        double *varDataExtents = new double[dim*2];
        ext->CopyTo(varDataExtents);

        // If we're unifying the extents across all axes, update
        // with the global limits and store it in the output
        if (parCoordsAtts.GetUnifyAxisExtents())
        {
            double globalMin =  DBL_MAX;
            double globalMax = -DBL_MAX;
            for (int axisNum = 0; axisNum < axisCount; axisNum++)
            {
                if (varDataExtents[2*axisNum + 0] < globalMin)
                    globalMin = varDataExtents[2*axisNum + 0];
                if (varDataExtents[2*axisNum + 1] > globalMax)
                    globalMax = varDataExtents[2*axisNum + 1];
            }
            for (int axisNum = 0; axisNum < axisMinima.size(); axisNum++)
            {
                varDataExtents[axisNum*2 + 0] = globalMin;
                varDataExtents[axisNum*2 + 1] = globalMax;
            }
            outAtts.GetVariableComponentExtents(pipelineVariable)->
                Set(varDataExtents);
        }

        // Set the internal data members we need to have set
        for (int i=0; i<dim; i++)
        {
            if (bins.size() > i)
                axisXPositions[i] = (bins[i]+bins[i+1])/2;
            else
                axisXPositions[i] = i;
            axisMinima[i] = varDataExtents[i*2+0];
            axisMaxima[i] = varDataExtents[i*2+1];
        }

        // probably not needed, since the things that use this information
        // already know to treat array variables specially
        outAtts.SetUseForAxis(0, pipelineVariable);

        delete[] varDataExtents;
    }
    else
    {
        avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();

        // Get the actual extents for each axis scalar variable
        double *varDataExtents = new double[axisCount*2];
        for (int axisNum = 0; axisNum < axisCount; axisNum++)
        {
            string axisVarName = parCoordsAtts.GetScalarAxisNames()[axisNum];
            if (gotHistogramsFromDB)
            {
                varDataExtents[2*axisNum+0] =  DBL_MAX;
                varDataExtents[2*axisNum+1] = -DBL_MAX;
                UpdateLimitsWithAllHSTimeSteps(axisNum,
                                               histograms,
                                               varDataExtents[2*axisNum+0],
                                               varDataExtents[2*axisNum+1]);
                UpdateLimitsWithAllHSTimeSteps(axisNum,
                                               histogramsForSelectedRegion,
                                               varDataExtents[2*axisNum+0],
                                               varDataExtents[2*axisNum+1]);
            }
            else
            {
                GetDataExtents(&(varDataExtents[2*axisNum]), axisVarName.c_str());
            }
        }

        // If we're unifying the extents across all axes, update
        // with the global limits before storing it in the output
        if (parCoordsAtts.GetUnifyAxisExtents())
        {
            double globalMin =  DBL_MAX;
            double globalMax = -DBL_MAX;
            for (int axisNum = 0; axisNum < axisCount; axisNum++)
            {
                if (varDataExtents[2*axisNum + 0] < globalMin)
                    globalMin = varDataExtents[2*axisNum + 0];
                if (varDataExtents[2*axisNum + 1] > globalMax)
                    globalMax = varDataExtents[2*axisNum + 1];
            }
            for (int axisNum = 0; axisNum < axisCount; axisNum++)
            {
                varDataExtents[2*axisNum + 0] = globalMin;
                varDataExtents[2*axisNum + 1] = globalMax;
            }
        }

        // Update the output extents for the variables, and
        // update the internal data members we need to have set
        for (int axisNum = 0; axisNum < axisCount; axisNum++)
        {
            string axisVarName = parCoordsAtts.GetScalarAxisNames()[axisNum];

            outAtts.GetCumulativeTrueDataExtents(axisVarName.c_str())->
                Set(&(varDataExtents[2*axisNum]));
            outAtts.SetUseForAxis(axisNum, axisVarName.c_str());

            axisXPositions[axisNum] = axisNum;
            axisMinima[axisNum] = varDataExtents[2*axisNum + 0];
            axisMaxima[axisNum] = varDataExtents[2*axisNum + 1];
        }
        delete[] varDataExtents;
    }

    if (parCoordsAtts.GetUnifyAxisExtents())
    {
        double globalMin =  DBL_MAX;
        double globalMax = -DBL_MAX;
        for (int axisNum = 0; axisNum < axisMinima.size(); axisNum++)
        {
            double axisMin = axisMinima[axisNum];
            double axisMax = axisMaxima[axisNum];
            if (axisMin < globalMin)
                globalMin = axisMin;
            if (axisMax > globalMax)
                globalMax = axisMax;
        }
        for (int axisNum = 0; axisNum < axisMinima.size(); axisNum++)
        {
            axisMinima[axisNum] = globalMin;
            axisMaxima[axisNum] = globalMax;
        }
    }

    for (int axisNum = 0; axisNum < axisMinima.size(); axisNum++)
    {
        double &axisMinimum = axisMinima[axisNum];
        double &axisMaximum = axisMaxima[axisNum];

        if (fabs(axisMinimum) < 1e-20)
            axisMinimum = 0.0;
        if (fabs(axisMaximum) < 1e-20)
            axisMaximum = 0.0;

        if (fabs(axisMaximum - axisMinimum) < 1e-20)
        {
            double dataAverage = (axisMinimum + axisMaximum) * 0.5;
            axisMinimum = dataAverage - 1e-20;
            axisMaximum = dataAverage + 1e-20;
        }
    }
    
    visitTimer->StopTimer(timer1, "avtParallelCoordinatesFilter::ComputeCurrentDataExtentsOverAllDomains()");
}



// *****************************************************************************
// Method: avtParallelCoordinatesPlot:InitializeDataTupleInput
//
// Purpose: Initializes parallel axis plot for the input of data tuples in
//          n-dimensional space.
//
// Notes: Adapted from more general "parallel coordinate plot" package developed
//        earlier.
//
// Programmer: Mark Blair
// Creation:   Thu Jun  8 17:18:00 PDT 2006
//
// Modifications:
//    Mark Blair, Wed Nov  8 16:01:27 PST 2006
//    Applies extents to input data only when Extents tool is active.
//
//    Mark Blair, Wed Dec 20 17:52:01 PST 2006
//    Added support for non-uniform axis spacing.
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now supports all variable axis spacing and axis group conventions.
//
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
//    Jeremy Meredith, Mon Feb  4 15:46:42 EST 2008
//    Some more distillation and related cleanup.
//
// *****************************************************************************

void
avtParallelCoordinatesFilter::InitializeDataTupleInput()
{
    applySubranges.clear();
    extentsApplied = false;
    for (int axisNum = 0; axisNum < axisCount; axisNum++)
    {
        double axisSpan, axisMinSpan, axisMaxSpan;
        axisSpan = axisMaxima[axisNum] - axisMinima[axisNum];

        axisMinSpan = parCoordsAtts.GetExtentMinima()[axisNum] -
                      axisMinima[axisNum];
        axisMaxSpan = axisMaxima[axisNum] -
                      parCoordsAtts.GetExtentMaxima()[axisNum];

        bool applySubrange = (axisMinSpan/axisSpan > 0.0001) ||
                             (axisMaxSpan/axisSpan > 0.0001);

        applySubranges.push_back(applySubrange);
        extentsApplied |= applySubrange;
    }
}


// *****************************************************************************
// Method: avtParallelCoordinatesFilter::CountDataTupleFocus
//
// Purpose: Takes the n-dimensional data tuple and increments the appropriate
//          bin for each pair of consecutive axes.
//
// Programmer: Jeremy Meredith
// Creation:   March  4, 2008
//
// Modifications:
//    Jeremy Meredith, Wed Mar  5 16:50:53 EST 2008
//    Use double for more arithmetic to increase accuracy.
//
//    Jeremy Meredith, Fri Mar  7 14:27:25 EST 2008
//    Allow multiple histogram sets (for multiple time steps).
//
//    Jeremy Meredith, Fri Mar  7 19:00:20 EST 2008
//    Added primitive support for multiple timesteps to use multiple colors.
//
//    Jeremy Meredith, Wed Feb 25 16:40:56 EST 2009
//    Port to trunk.
//
// *****************************************************************************

void
avtParallelCoordinatesFilter::CountDataTupleFocus(int ts, const floatVector &inputTuple)
{
    for (int axisID = 0; axisID < axisCount; axisID++)
    {
        if (applySubranges[axisID])
        {
            if (inputTuple[axisID] < parCoordsAtts.GetExtentMinima()[axisID])
                return;
            if (inputTuple[axisID] > parCoordsAtts.GetExtentMaxima()[axisID])
                return;
        }
    }
    
    for (int axisID = 0; axisID < axisCount-1; axisID++)
    {
        VISIT_LONG_LONG *counts = histogramsForSelectedRegion[ts][axisID].GetCounts();
        int nparts0 = histogramsForSelectedRegion[ts][axisID].GetNumberOfBins()[0];
        int nparts1 = histogramsForSelectedRegion[ts][axisID].GetNumberOfBins()[1];

        int a0 = axisID;
        int a1 = axisID+1;
        // Normalize the raw values to [0,1]
        double v0 = ((inputTuple[a0] - axisMinima[a0]) / 
                     (axisMaxima[a0] - axisMinima[a0]));
        double v1 = ((inputTuple[a1] - axisMinima[a1]) / 
                     (axisMaxima[a1] - axisMinima[a1]));
        // Convert to [0,nparts]
        int i0 = int(nparts0*v0);
        int i1 = int(nparts1*v1);
        // Clamp to [0,nparts)
        if (i0<0)
            i0=0;
        if (i0>=nparts0)
            i0=nparts0-1;
        if (i1<0)
            i1=0;
        if (i1>=nparts1)
            i1=nparts1-1;
        // Increment the bin
        counts[i0*nparts1+i1]++;
    }
}


// *****************************************************************************
// Method: avtParallelCoordinatesFilter::CountDataTupleContext
//
// Purpose: Takes the n-dimensional data tuple and increments the appropriate
//          bin for each pair of consecutive axes.
//
// Programmer: Jeremy Meredith
// Creation:   March 14, 2007
//
// Modifications:
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
//    Jeremy Meredith, Fri Feb 22 16:46:00 EST 2008
//    Changed to use new histogram structure.
//
//    Jeremy Meredith, Tue Mar  4 11:34:20 EST 2008
//    Get number of partitions from histogram specification to support
//    non-uniform partitioning across axes.
//
//    Jeremy Meredith, Tue Mar  4 18:17:33 EST 2008
//    Did some renaming and cleanup.
//
//    Jeremy Meredith, Wed Mar  5 16:50:53 EST 2008
//    Use double for more arithmetic to increase accuracy.
//
//    Jeremy Meredith, Fri Mar  7 14:27:25 EST 2008
//    Allow multiple histogram sets (for multiple time steps).
//
//    Jeremy Meredith, Fri Mar  7 19:00:20 EST 2008
//    Added primitive support for multiple timesteps to use multiple colors.
//
//    Jeremy Meredith, Wed Feb 25 16:41:11 EST 2009
//    Port to trunk.
//
// *****************************************************************************

void
avtParallelCoordinatesFilter::CountDataTupleContext(int ts, const floatVector &inputTuple)
{
    int axisID;
    for (axisID = 0; axisID < axisCount-1; axisID++)
    {
        VISIT_LONG_LONG *counts = histograms[ts][axisID].GetCounts();
        int nparts0 = histograms[ts][axisID].GetNumberOfBins()[0];
        int nparts1 = histograms[ts][axisID].GetNumberOfBins()[1];

        int a0 = axisID;
        int a1 = axisID+1;
        // Normalize the raw values to [0,1]
        double v0 = ((inputTuple[a0] - axisMinima[a0]) / 
                     (axisMaxima[a0] - axisMinima[a0]));
        double v1 = ((inputTuple[a1] - axisMinima[a1]) / 
                     (axisMaxima[a1] - axisMinima[a1]));
        // Convert to [0,nparts]
        int i0 = int(nparts0*v0);
        int i1 = int(nparts1*v1);
        // Clamp to [0,nparts)
        if (i0<0)
            i0=0;
        if (i0>=nparts0)
            i0=nparts0-1;
        if (i1<0)
            i1=0;
        if (i1>=nparts1)
            i1=nparts1-1;
        // Increment the bin
        counts[i0*nparts1+i1]++;
    }
}


// ****************************************************************************
//  Method:  avtParallelCoordinatesFilter::DrawContext
//
//  Purpose:
//    Draw the parallel axis context bins.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 14, 2007
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
//    Jeremy Meredith, Mon Feb  4 15:46:42 EST 2008
//    Some more distillation and related cleanup.
//
//    Jeremy Meredith, Thu Feb  7 17:48:46 EST 2008
//    Handle array-bin-defined x positions and empty context-bins.
//
//    Jeremy Meredith, Fri Feb 22 16:46:00 EST 2008
//    Changed to use new histogram structure.
//
//    Jeremy Meredith, Tue Mar  4 11:34:20 EST 2008
//    Get number of partitions from histogram specification to support
//    non-uniform partitioning across axes.
//
//    Jeremy Meredith, Tue Mar  4 18:17:33 EST 2008
//    Draw brightness using density instead of count, as this allows
//    for irregular binning schemes.  Added the other necessary support
//    for irregular binning.  Also did some renaming and cleanup,
//    since focus is now also drawn using a histogram.
//
//    Jeremy Meredith, Wed Mar  5 16:50:53 EST 2008
//    Use double for more arithmetic to increase accuracy.
//
//    Jeremy Meredith, Fri Mar  7 14:27:25 EST 2008
//    Allow multiple histogram sets (for multiple time steps).
//
//    Jeremy Meredith, Fri Mar  7 19:00:20 EST 2008
//    Added primitive support for multiple timesteps to use multiple colors.
//
//    Jeremy Meredith, Fri Mar  7 19:46:08 EST 2008
//    Better parallel support.
//
//    Jeremy Meredith, Mon Mar 10 14:56:34 EDT 2008
//    Fix multiple-timestep support.
//
//    Jeremy Meredith, Mon Mar 10 15:29:32 EDT 2008
//    Moved multi-timestep axis extent unification to helper function to
//    accommodate both context and focus extents.
//
//    Jeremy Meredith, Wed Feb 25 16:41:43 EST 2009
//    Port to trunk.
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::DrawContext(int ts)
{
    int timer1 = visitTimer->StartTimer();
    
    CheckHistograms(axisCount, histograms);

#ifdef PARALLEL
    // We assume either all processors have a timestep (i.e. we decomposed
    // this timestep data-parallel), or exactly one does (i.e. we decomposed
    // by one timestep per processor).

    // Figure out what kind of unification we need
    bool i_have_this_histogram = (histograms[ts] != NULL);
    int processors_with_histogram = (i_have_this_histogram ? 1 : 0);
    SumIntAcrossAllProcessors(processors_with_histogram);

    if (processors_with_histogram>1 && processors_with_histogram<PAR_Size())
    {
        EXCEPTION1(ImproperUseException, "More than one processor had a "
                   "histogram for a given timestep, yet not all processors "
                   "had one.");
    }

    bool success = true;
    if (processors_with_histogram > 1)
    {
        // First case: all processors have a histogram so sum
        for (int axis=0; axis<axisCount-1; axis++)
            success &= histograms[ts][axis].SumAcrossAllProcessors();
    }
    else
    {
        // Second case: only one process has it.
        if (PAR_Rank()==0  &&  ! i_have_this_histogram)
        {
            // If the root processor doesn't have it, he'll need to receive it
            histograms[ts] = new avtHistogramSpecification[axisCount-1];
            for (int axis=0; axis<axisCount-1; axis++)
                success &= histograms[ts][axis].GetToRootProcessor(ts*axisCount+axis);
        }
        else if (PAR_Rank()!=0  &&  i_have_this_histogram)
        {
            // If we're the a non-root processor with it, then send it.
            for (int axis=0; axis<axisCount-1; axis++)
                success &= histograms[ts][axis].GetToRootProcessor(ts*axisCount+axis);
        }
    }

    if (!success)
    {
        EXCEPTION1(ImproperUseException,
                   "Error in parallel unification of histograms.");
    }

    if (PAR_Rank() != 0)
    {
        // only process 0 has to draw the context
        return;
    }
#endif

    vtkPolyData        *contextPolyData[PCP_CTX_BRIGHTNESS_LEVELS];
    vtkPoints          *contextPoints[PCP_CTX_BRIGHTNESS_LEVELS];
    vtkCellArray       *contextLines[PCP_CTX_BRIGHTNESS_LEVELS];
    vtkCellArray       *contextVerts[PCP_CTX_BRIGHTNESS_LEVELS];
    vtkCellArray       *contextPolys[PCP_CTX_BRIGHTNESS_LEVELS];

    //
    //  Initialize polygon datasets for the context.
    //
    for (int i=0; i<PCP_CTX_BRIGHTNESS_LEVELS; i++)
    {
        contextPolyData[i] = vtkPolyData::New();

        contextPoints[i] = vtkPoints::New();
        contextPolyData[i]->SetPoints(contextPoints[i]);
        contextPoints[i]->Delete();

        contextLines[i] = vtkCellArray::New();
        contextPolyData[i]->SetLines(contextLines[i]);
        contextLines[i]->Delete();

        contextVerts[i] = vtkCellArray::New();
        contextPolyData[i]->SetVerts(contextVerts[i]);
        contextVerts[i]->Delete();

        contextPolys[i] = vtkCellArray::New();
        contextPolyData[i]->SetPolys(contextPolys[i]);
        contextPolys[i]->Delete();
    }

    for (int axisNum = 0; axisNum < axisCount; axisNum++)
    {
        double varmin =  DBL_MAX;
        double varmax = -DBL_MAX;
        UpdateLimitsWithAllHSTimeSteps(axisNum, histograms,
                                       varmin, varmax);
        UpdateLimitsWithAllHSTimeSteps(axisNum, histogramsForSelectedRegion,
                                       varmin, varmax);
        const vector<double> *bounds;
        if (axisNum==0)
            bounds = &(histograms[ts][axisNum].GetBounds()[0]);
        else
            bounds = &(histograms[ts][axisNum-1].GetBounds()[1]);
        int nparts = bounds->size() - 1;
        for (int part = 0 ; part <= nparts ; part++)
        {
            double val = (*bounds)[part];
            float pt[3];
            pt[0] = axisXPositions[axisNum];
            pt[1] = (val-varmin)/(varmax-varmin);
            pt[2] = 0.0;
            for (int i = 0 ; i < PCP_CTX_BRIGHTNESS_LEVELS ; i++)
            {
                contextPoints[i]->InsertNextPoint(pt);
            }
        }
    }

    double gamma = parCoordsAtts.GetContextGamma();
    if (gamma<.1)
        gamma=.1;
    if (gamma>10)
        gamma=10;

    int axis0index = 0;
    for (int axis = 0; axis < axisCount-1; axis++)
    {
        VISIT_LONG_LONG *counts = histograms[ts][axis].GetCounts();
        int nparts0 = histograms[ts][axis].GetNumberOfBins()[0];
        int nparts1 = histograms[ts][axis].GetNumberOfBins()[1];

        int axis1index = axis0index + nparts0+1;

        // Find the maximum density in the bins
        double maxdensity = 0;
        for (int a=0; a<nparts0; a++)
        {
            double size0 = histograms[ts][axis].GetBounds()[0][a+1] -
                           histograms[ts][axis].GetBounds()[0][a];
            for (int b=0; b<nparts1; b++)
            {
                int count = counts[a*nparts1+b];
                if (count == 0)
                    continue;
                double size1 = histograms[ts][axis].GetBounds()[1][b+1] -
                               histograms[ts][axis].GetBounds()[1][b];
                double area = size0 * size1;
                double density = (area>0) ? count/area : 0;
                if (density > maxdensity)
                    maxdensity = density;
            }
        }

        if (maxdensity == 0)
            continue;

        // Draw each bin as a polygon in the appropriately
        // colored (and layered) context polydata
        for (int a=0; a<nparts0; a++)
        {
            double size0 = histograms[ts][axis].GetBounds()[0][a+1] -
                           histograms[ts][axis].GetBounds()[0][a];
            for (int b=0; b<nparts1; b++)
            {
                double size1 = histograms[ts][axis].GetBounds()[1][b+1] -
                               histograms[ts][axis].GetBounds()[1][b];
                double area = size0 * size1;
                double density = (area>0) ? counts[a*nparts1+b]/area : 0;

                double alpha = density / maxdensity;

#if defined(__GNUC__) && ((__GNUC__ < 3) || (__GNUC__ == 3 && __GNUC_MINOR__ < 2) || (__GNUC__ == 3 && __GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL__ == 0))
                alpha = pow(alpha,1./gamma);
#else
                alpha = powf(alpha,1.f/gamma);
#endif

                int c = int(double(PCP_CTX_BRIGHTNESS_LEVELS-1) * alpha);

                if (c != 0)
                {
                    vtkIdType poly[4];
                    poly[0] = axis0index + a;
                    poly[1] = axis1index + b;
                    poly[2] = poly[1] + 1;
                    poly[3] = poly[0] + 1;
                    contextPolys[c]->InsertNextCell(4, poly);
                }
            }
        }

        axis0index = axis1index;
    }

    // We have to explicitly convert these into base types
    vtkDataSet **outputDataSets = new vtkDataSet*[PCP_CTX_BRIGHTNESS_LEVELS];
    for (int i=0; i<PCP_CTX_BRIGHTNESS_LEVELS; i++)
        outputDataSets[i] = contextPolyData[i];

    // Add them to a data tree
    avtDataTree_p contextTree = new avtDataTree(PCP_CTX_BRIGHTNESS_LEVELS,
                                                outputDataSets,
                                                -1, contextLabels[ts]);
    for (int i=0; i<PCP_CTX_BRIGHTNESS_LEVELS; i++)
        outputDataSets[i]->Delete();
    delete [] outputDataSets;

    // Add the new context after the old ones
    avtDataTree_p oldTree = GetDataTree();
    avtDataTree_p trees[2] = { oldTree, contextTree };
    avtDataTree_p newOutput = new avtDataTree(2, trees);
    SetOutputDataTree(newOutput);
    
    visitTimer->StopTimer(timer1, "avtParallelCoordinatesFilter::DrawContex()");     
}


// ****************************************************************************
//  Method:  avtParallelCoordinatesFilter::DrawFocusHistograms
//
//  Purpose:
//    Draw the focus polylines from their histograms.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2008
//
//  Modifications:
//    Jeremy Meredith, Wed Mar  5 16:50:53 EST 2008
//    Use double for more arithmetic to increase accuracy.
//
//    Jeremy Meredith, Fri Mar  7 14:27:25 EST 2008
//    Allow multiple histogram sets (for multiple time steps).
//
//    Jeremy Meredith, Fri Mar  7 19:00:20 EST 2008
//    Added primitive support for multiple timesteps to use multiple colors.
//
//    Jeremy Meredith, Fri Mar  7 19:46:08 EST 2008
//    Better parallel support.
//
//    Jeremy Meredith, Mon Mar 10 14:56:34 EDT 2008
//    Fix multiple-timestep support.
//
//    Jeremy Meredith, Mon Mar 10 15:29:32 EDT 2008
//    Moved multi-timestep axis extent unification to helper function to
//    accommodate both context and focus extents.
//
//    Jeremy Meredith, Wed Feb 25 16:42:15 EST 2009
//    Port to trunk.
//
// ****************************************************************************
void
avtParallelCoordinatesFilter::DrawFocusHistograms(int ts)
{
    int timer1 = visitTimer->StartTimer();
    
    CheckHistograms(axisCount, histogramsForSelectedRegion);

#ifdef PARALLEL
    // We assume either all processors have a timestep (i.e. we decomposed
    // this timestep data-parallel), or exactly one does (i.e. we decomposed
    // by one timestep per processor).

    // Figure out what kind of unification we need
    bool i_have_this_histogram = (histogramsForSelectedRegion[ts] != NULL);
    int processors_with_histogram = (i_have_this_histogram ? 1 : 0);
    SumIntAcrossAllProcessors(processors_with_histogram);

    if (processors_with_histogram>1 && processors_with_histogram<PAR_Size())
    {
        EXCEPTION1(ImproperUseException, "More than one processor had a "
                   "histogram for a given timestep, yet not all processors "
                   "had one.");
    }

    bool success = true;
    if (processors_with_histogram > 1)
    {
        // First case: all processors have a histogram so sum
        for (int axis=0; axis<axisCount-1; axis++)
            success &= histogramsForSelectedRegion[ts][axis].SumAcrossAllProcessors();
    }
    else
    {
        // Second case: only one process has it.
        if (PAR_Rank()==0  &&  ! i_have_this_histogram)
        {
            // If the root processor doesn't have it, he'll need to receive it
            histogramsForSelectedRegion[ts] = new avtHistogramSpecification[axisCount-1];
            for (int axis=0; axis<axisCount-1; axis++)
                success &= histogramsForSelectedRegion[ts][axis].GetToRootProcessor(ts*axisCount+axis);
        }
        else if (PAR_Rank()!=0  &&  i_have_this_histogram)
        {
            // If we're the a non-root processor with it, then send it.
            for (int axis=0; axis<axisCount-1; axis++)
                success &= histogramsForSelectedRegion[ts][axis].GetToRootProcessor(ts*axisCount+axis);
        }
    }

    if (!success)
    {
        EXCEPTION1(ImproperUseException,
                   "Error in parallel unification of histogramsForSelectedRegion.");
    }

    if (PAR_Rank() != 0)
    {
        // only process 0 has to draw the lines
        return;
    }
#endif

    vtkPolyData        *focusPolyData;
    vtkPoints          *focusPoints;
    vtkCellArray       *focusLines;
    vtkCellArray       *focusVerts;
    vtkCellArray       *focusPolys;

    //
    //  Initialize polygon datasets for the lines.
    //
    focusPolyData = vtkPolyData::New();

    focusPoints = vtkPoints::New();
    focusPolyData->SetPoints(focusPoints);
    focusPoints->Delete();

    focusLines = vtkCellArray::New();
    focusPolyData->SetLines(focusLines);
    focusLines->Delete();

    focusVerts = vtkCellArray::New();
    focusPolyData->SetVerts(focusVerts);
    focusVerts->Delete();

    focusPolys = vtkCellArray::New();
    focusPolyData->SetPolys(focusPolys);
    focusPolys->Delete();

    for (int axisNum = 0; axisNum < axisCount; axisNum++)
    {
        double varmin =  DBL_MAX;
        double varmax = -DBL_MAX;
        UpdateLimitsWithAllHSTimeSteps(axisNum, histograms,
                                       varmin, varmax);
        UpdateLimitsWithAllHSTimeSteps(axisNum, histogramsForSelectedRegion,
                                       varmin, varmax);
        const vector<double> *bounds;
        if (axisNum==0)
            bounds = &(histogramsForSelectedRegion[ts][axisNum].GetBounds()[0]);
        else
            bounds = &(histogramsForSelectedRegion[ts][axisNum-1].GetBounds()[1]);
        int nparts = bounds->size() - 1;
        for (int part = 0 ; part < nparts ; part++)
        {
            double val = ((*bounds)[part] + (*bounds)[part+1]) / 2.0;
            float pt[3];
            pt[0] = axisXPositions[axisNum];
            pt[1] = (val-varmin)/(varmax-varmin);
            pt[2] = 0.0;
            focusPoints->InsertNextPoint(pt);
        }
    }

    float gamma = parCoordsAtts.GetContextGamma();
    if (gamma<.1)
        gamma=.1;
    if (gamma>10)
        gamma=10;

    int axis0index = 0;
    for (int axis = 0; axis < axisCount-1; axis++)
    {
        VISIT_LONG_LONG *counts = histogramsForSelectedRegion[ts][axis].GetCounts();
        int nparts0 = histogramsForSelectedRegion[ts][axis].GetNumberOfBins()[0];
        int nparts1 = histogramsForSelectedRegion[ts][axis].GetNumberOfBins()[1];

        int axis1index = axis0index + nparts0;

        // Draw each bin as a polygon in the appropriately
        // colored (and layered) context polydata
        for (int a=0; a<nparts0; a++)
        {
            for (int b=0; b<nparts1; b++)
            {
                int c = counts[a*nparts1+b];
                if (c != 0)
                {
                    vtkIdType line[2];
                    line[0] = axis0index + a;
                    line[1] = axis1index + b;
                    focusLines->InsertNextCell(2, line);
                }
            }
        }

        axis0index = axis1index;
    }

    // We have to explicitly convert these into base types
    vtkDataSet **outputDataSets = new vtkDataSet*[1];
    outputDataSets[0] = focusPolyData;

    // Add them to a data tree
    avtDataTree_p focusTree = new avtDataTree(1,
                                              outputDataSets,
                                              -1, focusLabels[ts]);
    outputDataSets[0]->Delete();
    delete [] outputDataSets;

    // Add the new context after the old ones (and after the contexts)
    avtDataTree_p oldTree = GetDataTree();
    avtDataTree_p trees[2] = { oldTree, focusTree };
    avtDataTree_p newOutput = new avtDataTree(2, trees);
    SetOutputDataTree(newOutput);
    
    visitTimer->StopTimer(timer1, "avtParallelCoordinatesFilter::DrawFocus()");  
}


// ****************************************************************************
//  Method:  avtParallelCoordinatesFilter::InitializeContextHistograms
//
//  Purpose:
//    Allocate arrays to hold counts of binned data.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 16, 2007
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
//    Jeremy Meredith, Fri Feb 22 16:46:00 EST 2008
//    Changed to use new histogram structure.
//
//    Jeremy Meredith, Tue Mar  4 11:35:53 EST 2008
//    Some cleanup and renaming since histograms are now also used for focus.
//
//    Jeremy Meredith, Fri Mar  7 14:27:25 EST 2008
//    Allow multiple histogram sets (for multiple time steps).
//
//    Jeremy Meredith, Wed Feb 25 16:44:35 EST 2009
//    Port to trunk.
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::InitializeContextHistograms()
{
    int nctxparts = parCoordsAtts.GetContextNumPartitions();
    // TODO: DOESN'T SUPPORT MULTIPLE TIME STEPS
    histograms.resize(1);
    histograms[0] = new avtHistogramSpecification[axisCount-1];
    for (int i=0; i<axisCount-1; i++)
    {
        string name1="name1", name2="name2";
        if (parCoordsAtts.GetVisualAxisNames().size() > 0)
        {
            name1 = parCoordsAtts.GetVisualAxisNames()[i];
            name2 = parCoordsAtts.GetVisualAxisNames()[i+1];
        }
        else if (parCoordsAtts.GetScalarAxisNames().size() > 0)
        {
            name1 = parCoordsAtts.GetScalarAxisNames()[i];
            name2 = parCoordsAtts.GetScalarAxisNames()[i+1];
        }
        // TODO: DOESN'T SUPPORT MULTIPLE TIME STEPS
        histograms[0][i].SpecifyHistogram(-1,
                           name1, nctxparts, axisMinima[i],   axisMaxima[i],
                           name2, nctxparts, axisMinima[i+1], axisMaxima[i+1]);
    }
}

// ****************************************************************************
//  Method:  avtParallelCoordinatesFilter::InitializeFocusHistograms
//
//  Purpose:
//    Initialize the histograms for the focus.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2008
//
//  Modifications:
//    Jeremy Meredith, Fri Mar  7 14:27:25 EST 2008
//    Allow multiple histogram sets (for multiple time steps).
//
//    Jeremy Meredith, Wed Feb 25 16:45:06 EST 2009
//    Port to trunk.
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::InitializeFocusHistograms()
{
    int nlineparts = parCoordsAtts.GetLinesNumPartitions();
    // TODO: DOESN'T SUPPORT MULTIPLE TIME STEPS
    histogramsForSelectedRegion.resize(1);
    histogramsForSelectedRegion[0] = new avtHistogramSpecification[axisCount-1];
    for (int i=0; i<axisCount-1; i++)
    {
        string name1="name1", name2="name2";
        if (parCoordsAtts.GetVisualAxisNames().size() > 0)
        {
            name1 = parCoordsAtts.GetVisualAxisNames()[i];
            name2 = parCoordsAtts.GetVisualAxisNames()[i+1];
        }
        else if (parCoordsAtts.GetScalarAxisNames().size() > 0)
        {
            name1 = parCoordsAtts.GetScalarAxisNames()[i];
            name2 = parCoordsAtts.GetScalarAxisNames()[i+1];
        }
        // TODO: DOESN'T SUPPORT MULTIPLE TIME STEPS
        histogramsForSelectedRegion[0][i].SpecifyHistogram(-1,
                          name1, nlineparts, axisMinima[i],   axisMaxima[i],
                          name2, nlineparts, axisMinima[i+1], axisMaxima[i+1]);
    }
}

// ****************************************************************************
//  Method:  avtParallelCoordinatesFilter::CleanUpAllHistograms
//
//  Purpose:
//    Free data allocated to hold the bins for the context.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 16, 2007
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
//    Jeremy Meredith, Fri Feb 22 16:46:00 EST 2008
//    Changed to use new histogram structure.
//
//    Hank Childs, Mon Mar  3 19:42:29 PST 2008
//    Delete histogramsForSelectedRegion.
//
//    Jeremy Meredith, Fri Mar  7 14:27:25 EST 2008
//    Allow multiple histogram sets (for multiple time steps).
//
//    Jeremy Meredith, Wed Feb 25 16:45:38 EST 2009
//    Port to trunk.
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::CleanUpAllHistograms()
{
    for (int t=0; t<histograms.size(); t++)
        delete [] histograms[t];
    for (int t=0; t<histogramsForSelectedRegion.size(); t++)
        delete [] histogramsForSelectedRegion[t];
    histograms.clear();
    histogramsForSelectedRegion.clear();
}


// ****************************************************************************
//  Method:  avtParallelCoordinatesFilter::PrepareForArrayVariable
//
//  Purpose:
//    Handle the case where our variable is an array variable
//    (as opposed to being constructed from a bunch of scalars).
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  7, 2008
//
// ****************************************************************************
void
avtParallelCoordinatesFilter::PrepareForArrayVariable()
{
    isArrayVar = false;

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (atts.GetVariableType(pipelineVariable) != AVT_ARRAY_VAR)
        return;

    isArrayVar = true;
    axisCount = atts.GetVariableDimension(pipelineVariable);

    parCoordsAtts.GetExtentMinima().resize(axisCount, -1e+37);
    parCoordsAtts.GetExtentMaxima().resize(axisCount, +1e+37);

    return;
}

// ****************************************************************************
//  Method:  avtParallelCoordinatesFilter::ConvertExtentsToCondition
//
//  Purpose:
//   Convert the extents given by the extents tool into a string
//   defining the condition of the histograms
//
//  Arguments:
//    none
//
//  Programmer:  Oliver Ruebel
//  Creation:    March  5, 2008
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 25 16:46:03 EST 2009
//    Port to trunk.  Disabled named selections since they're now
//    done differently.
//
// ****************************************************************************

std::string   
avtParallelCoordinatesFilter::ConvertExtentsToCondition()
{
    std::string condition;
    axisCount = parCoordsAtts.GetScalarAxisNames().size();
    stringVector curAxisVarNames = parCoordsAtts.GetScalarAxisNames();
    int numberOfConditions = 0;
    for (int j = 0 ; j < axisCount ; j++)
    {
        //Convert min max values to string
        std::ostringstream min;
        std::ostringstream max;
        min<<parCoordsAtts.GetExtentMinima()[j];
        max<<parCoordsAtts.GetExtentMaxima()[j];
        
        if (parCoordsAtts.GetExtentMinima()[j] > -1e+36)
        {    
            if (numberOfConditions>0)
            {
                condition.append( "&&" ); //add a condition
            }
            //define condition x>min
            condition.append( "(" );
            condition.append( curAxisVarNames[j] );
            condition.append( ">"  );
            condition.append( min.str()  );
            condition.append( ")"  );
            numberOfConditions++;
        }      
        
        if (parCoordsAtts.GetExtentMaxima()[j] < +1e+36) 
        {
            if (numberOfConditions>0)
            {
                condition.append( "&&" ); //add a condition
            }
            //define condtion x<max
            condition.append( "(" );
            condition.append( curAxisVarNames[j] );
            condition.append( "<"  );
            condition.append( max.str()  );
            condition.append( ")"  ); 
            numberOfConditions++;
        }    
    }
    /*
      // TODO: Named selections are done differently now....
    if (parCoordsAtts.GetApplyNamedSelection())
    {
        string cond2 = ConvertNamedSelectionToCondition();
        condition.append("&&");
        condition.append(cond2);
    }
    */
      
    return condition;
}        

// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::CreateNamedSelection
//
//  Purpose:
//      Creates a named selection from its input.
//
//  Programmer: Hank Childs
//  Creation:   February 9, 2009
//
// ****************************************************************************

avtNamedSelection *
avtParallelCoordinatesFilter::CreateNamedSelection(avtContract_p c, 
                                                   const std::string &selName)
{
    debug1 << "Start avtParallelCoordinatesFilter::CreateNamedSelection" << endl;
    avtNamedSelection *rv = NULL;
    rv = CreateDBAcceleratedNamedSelection(c, selName);
    if (rv != NULL)
        return rv;

    rv = CreateNamedSelectionThroughTraversal(c, selName);
    debug1 << "End avtParallelCoordinatesFilter::CreateNamedSelection" << endl;
    return rv;
}


// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::CreateDBAcceleratedNamedSelection
//
//  Purpose:
//      Creates a named selection by getting it from the database where
//      it uses acceleration.
//
//  Programmer: Hank Childs
//  Creation:   February 9, 2009
//
// ****************************************************************************

avtNamedSelection *
avtParallelCoordinatesFilter::CreateDBAcceleratedNamedSelection(
                                                   avtContract_p c,
                                                   const std::string &selName)
{
    if (! GetInput()->GetInfo().GetValidity().GetZonesPreserved())
    {
        // Zones have been removed upstream, so the direct-to-database query
        // will be invalid.  Give up.
        return NULL;
    }

    int  j;

    std::vector<avtDataSelection *> drs;
    stringVector curAxisVarNames = parCoordsAtts.GetScalarAxisNames();
    for (j = 0 ; j < axisCount ; j++)
    {
        drs.push_back(new avtDataRangeSelection(
                     curAxisVarNames[j],
                     parCoordsAtts.GetExtentMinima()[j],
                     parCoordsAtts.GetExtentMaxima()[j]));
    }
/*
    if (parCoordsAtts.GetApplyNamedSelection())
    {
        vector<double> ids;
        avtCallback::GetNamedSelection(
                            parCoordsAtts.GetApplyNamedSelectionName(), ids);
        if (ids.size()>0)
        {
            avtIdentifierSelection *idsel = new avtIdentifierSelection();
            idsel->SetIdentifiers(ids);
            drs.push_back(idsel);
        }
    }
 */
    avtIdentifierSelection *ids = GetMetaData()->GetIdentifiers(drs);
    avtNamedSelection *rv = NULL;
    if (ids != NULL)
        rv = new avtFloatingPointIdNamedSelection(selName, ids->GetIdentifiers());
    delete ids;

    for (j = 0 ; j < drs.size() ; j++)
        delete drs[j];

    return rv;
}


// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::CreateNamedSelectionThroughTraversal
//
//  Purpose:
//      Creates a named selection from its input by traversing the data.
//
//  Programmer: Hank Childs
//  Creation:   February 9, 2009
//
// ****************************************************************************

avtNamedSelection *
avtParallelCoordinatesFilter::CreateNamedSelectionThroughTraversal(avtContract_p c, 
                                                   const std::string &selName)
{
    int  i;

    // Get the zone number labels loaded up.
    GetInput()->Update(c);

    avtDataTree_p tree = GetInputDataTree();

    std::vector<int> doms;
    std::vector<int> zones;
    int nleaves = 0;
    vtkDataSet **leaves = tree->GetAllLeaves(nleaves);
    stringVector curAxisVarNames = parCoordsAtts.GetScalarAxisNames();
    for (i = 0 ; i < nleaves ; i++)
    {
        int axisNum;
        int tupleCount, componentCount;
        int cellVertexCount, vertexNum, valueNum;
        bool arrayIsCellData, dataBadOrMissing;
        std::string arrayName;
        vtkDataArray *dataArray;
        vtkIdList *pointIdList;
        float *arrayValues;
        float valueSum;
        
        std::vector<float *> varArrayValues;
        boolVector           varIsCellData;
        intVector            varComponentCounts;
        int cellCount       = leaves[i]->GetNumberOfCells();
        int pointCount      = leaves[i]->GetNumberOfPoints();
    
        for (axisNum = 0; axisNum < axisCount; axisNum++)
        {
            if (isArrayVar)
                arrayName = pipelineVariable;
            else
                arrayName = curAxisVarNames[axisNum];
    
            dataArray = leaves[i]->GetCellData()->GetArray(arrayName.c_str());
            arrayIsCellData = true;
            tupleCount = cellCount;
    
            if (dataArray == NULL)
            {
                dataArray = leaves[i]->GetPointData()->GetArray(arrayName.c_str());
                arrayIsCellData = false;
                tupleCount = pointCount;
            }
    
            if (dataArray == NULL)
                dataBadOrMissing = true;
            else if (dataArray->GetDataType() != VTK_FLOAT)
                dataBadOrMissing = true;
            else if (dataArray->GetNumberOfTuples() != tupleCount)
                dataBadOrMissing = true;
            else
                dataBadOrMissing = false;
            
            if (dataBadOrMissing)
            {
                debug3 << "PCP/aPAF/EDT/5: ParallelCoordinates plot input data array "
                       << arrayName << " is bad or missing." << endl;
                return NULL;
            }
        
            if (isArrayVar)
                varArrayValues.push_back(((float *)dataArray->GetVoidPointer(0))
                                         + axisNum);
            else
                varArrayValues.push_back((float *)dataArray->GetVoidPointer(0));
            varIsCellData.push_back(arrayIsCellData);
            varComponentCounts.push_back(dataArray->GetNumberOfComponents());
        }
    

        floatVector inputTuple = floatVector(axisCount);
    
        vtkDataArray *ocn = leaves[i]->GetCellData()->
                                            GetArray("avtOriginalCellNumbers");
        if (ocn == NULL)
        {
            EXCEPTION0(ImproperUseException);
        }
        unsigned int *ptr = (unsigned int *) ocn->GetVoidPointer(0);
        if (ptr == NULL)
        {
            EXCEPTION0(ImproperUseException);
        }

        int ncells = leaves[i]->GetNumberOfCells();
        int curSize = doms.size();
        int numMatching = 0;
        for (int j = 0 ; j < ncells ; j++)
        {
            int axisNum;
            for (axisNum = 0; axisNum < axisCount; axisNum++)
            {
                arrayValues     = varArrayValues[axisNum];
                componentCount  = varComponentCounts[axisNum];

                if (varIsCellData[axisNum])
                {
                    inputTuple[axisNum] =
                    arrayValues[j*componentCount];
                }
                else
                {
                    leaves[i]->GetCellPoints(j, pointIdList);
                    cellVertexCount = pointIdList->GetNumberOfIds();

                    valueSum = 0.0;

                    for (vertexNum = 0; vertexNum < cellVertexCount; vertexNum++)
                    {
                        valueNum = pointIdList->GetId(vertexNum)*componentCount;
                        valueSum += arrayValues[valueNum];
                    }

                    if (cellVertexCount == 0) cellVertexCount = 1;

                    inputTuple[axisNum] = valueSum / (float)cellVertexCount;
                }
                if (applySubranges[axisNum])
                {
                    if (inputTuple[axisNum] < parCoordsAtts.GetExtentMinima()[axisNum])
                        break;
                    if (inputTuple[axisNum] > parCoordsAtts.GetExtentMaxima()[axisNum])
                        break;
                }
            }
            if (axisNum >= axisCount) // then all axes "passed test"
            {
                doms.push_back(ptr[2*j]);
                zones.push_back(ptr[2*j+1]);
            }
        }
    }

    // Note the poor use of MPI below, coded for expediency, as I believe all
    // of the named selections will be small.
    int *numPerProcIn = new int[PAR_Size()];
    int *numPerProc   = new int[PAR_Size()];
    for (i = 0 ; i < PAR_Size() ; i++)
        numPerProcIn[i] = 0;
    numPerProcIn[PAR_Rank()] = doms.size();
    SumIntArrayAcrossAllProcessors(numPerProcIn, numPerProc, PAR_Size());
    int numTotal = 0;
    for (i = 0 ; i < PAR_Size() ; i++)
        numTotal += numPerProc[i];
    if (numTotal > 1000000)
    {
        EXCEPTION1(VisItException, "You have selected too many zones in your "
                   "named selection.  Disallowing ... no selection created");
    }
    int myStart = 0;
    for (i = 0 ; i < PAR_Rank()-1 ; i++)
        myStart += numPerProc[i];

    int *selForDomsIn = new int[numTotal];
    int *selForDoms   = new int[numTotal];
    for (i = 0 ; i < doms.size() ; i++)
        selForDomsIn[myStart+i] = doms[i];
    SumIntArrayAcrossAllProcessors(selForDomsIn, selForDoms, numTotal);

    int *selForZonesIn = new int[numTotal];
    int *selForZones   = new int[numTotal];
    for (i = 0 ; i < zones.size() ; i++)
        selForZonesIn[myStart+i] = zones[i];
    SumIntArrayAcrossAllProcessors(selForZonesIn, selForZones, numTotal);

    //
    // Now construct the actual named selection and add it to our internal
    // data structure for tracking named selections.
    //
    avtNamedSelection *ns = 
             new avtZoneIdNamedSelection(selName,numTotal,selForDoms,selForZones);

    delete [] numPerProcIn;
    delete [] numPerProc;
    delete [] selForDomsIn;
    delete [] selForDoms;
    delete [] selForZonesIn;
    delete [] selForZones;

    return ns;
}


// ****************************************************************************
//  Function:  UpdateLimitsWithAllHSTimeSteps
//
//  Purpose:
//    Extend the given min/max values with the bounds specified by the given
//    histogram array for the given axis.
//
//  Arguments:
//    axis            the axis to use for updating bounds
//    h               the set of histograms to use
//    varmin,varmax   this min/max to update
//
//  Note: you must initialize varmin/varmax (e.g. to DBL_MAX/-DBL_MAX)
//        before calling this function.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 10, 2008
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 25 16:47:04 EST 2009
//    Port to trunk.
//
// ****************************************************************************
void
UpdateLimitsWithAllHSTimeSteps(int axis,
                              const std::vector<avtHistogramSpecification*> &h,
                              double &varmin, double &varmax)
{
    for (int ts = 0; ts < h.size(); ts++)
    {
        if (h[ts] == NULL)
            continue;

        const vector<double> *bounds;
        if (axis==0)
            bounds = &(h[ts][axis].GetBounds()[0]);
        else
            bounds = &(h[ts][axis-1].GetBounds()[1]);

        if ((*bounds)[0] < varmin)
            varmin = (*bounds)[0];

        int nparts = bounds->size() - 1;
        if ((*bounds)[nparts] > varmax)
            varmax = (*bounds)[nparts];
    }
}


// ****************************************************************************
//  Function:  CheckHistograms
//
//  Purpose:
//    Verify the bounds and bins arrays are appropriately sized.
//
//  Arguments:
//    axisCount       the number of axes to use for updating bounds
//    h               the set of histograms to use
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 26, 2008
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 25 16:47:14 EST 2009
//    Port to trunk.
//
// ****************************************************************************
void
CheckHistograms(int axisCount,
                const std::vector<avtHistogramSpecification*> &h)
{
    for (int ts = 0; ts < h.size(); ts++)
    {
        if (!h[ts])
            continue;

        for (int axis = 0; axis < axisCount-1; axis++)
        {
            if (h[ts][axis].GetNumberOfBins()[0]+1 !=
                h[ts][axis].GetBounds()[0].size())
            {
                cerr << "ERROR: histograms[time="<<ts<<"][axis="<<axis<<"] "
                     << "for the first variable "
                     << "has bounds size of "
                     << h[ts][axis].GetBounds()[0].size()
                     << " and number of bins of "
                     << h[ts][axis].GetNumberOfBins()[0]<<endl;
            }
            if (h[ts][axis].GetNumberOfBins()[1]+1 !=
                h[ts][axis].GetBounds()[1].size())
            {
                cerr << "ERROR: histograms[time="<<ts<<"][axis="<<axis<<"]  "
                     << "for the second variable "
                     << "has bounds size of "
                     << h[ts][axis].GetBounds()[1].size()
                     << " and number of bins of "
                     << h[ts][axis].GetNumberOfBins()[1]<<endl;
            }
        }
    }
}




// ****************************************************************************
//  Method:  avtParallelCoordinatesFilter::InitializeFocusPolyData
//
//  Purpose:
//    Initialize polydata for the current domain's polylines.
//    (Used as an alternative to histograms for the focus when
//    requested by the user.)
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 27, 2008
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 25 16:47:14 EST 2009
//    Port to trunk.
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::InitializeFocusPolyData()
{
    //
    //  Initialize polyline dataset for the output data curves.
    //
    dataCurvePolyData = vtkPolyData::New();

    dataCurvePoints = vtkPoints::New();
    dataCurvePolyData->SetPoints(dataCurvePoints);
    dataCurvePoints->Delete();

    dataCurveLines = vtkCellArray::New();
    dataCurvePolyData->SetLines(dataCurveLines);
    dataCurveLines->Delete();

    dataCurveVerts = vtkCellArray::New();
    dataCurvePolyData->SetVerts(dataCurveVerts);
    dataCurveVerts->Delete();

    outputCurveCount = 0;
}

// ****************************************************************************
//  Method:  avtParallelCoordinatesFilter::AppendDataTupleFocus
//
//  Purpose:
//    Add a tuple to the polydata for the focus lines.
//    (Used as an alternative to histograms for the focus when
//    requested by the user.)
//
//  Arguments:
//    tuple      the tuple to append
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 27, 2008
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 25 16:47:14 EST 2009
//    Port to trunk.
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::AppendDataTupleFocus(const floatVector &tuple)
{
    int axisID;
    double axisMin, axisMax, inputCoord;

    float outputCoords[3];
    outputCoords[2] = 0.0;

    for (axisID = 0; axisID < axisCount; axisID++)
    {
        if (applySubranges[axisID])
        {
            if (tuple[axisID] < parCoordsAtts.GetExtentMinima()[axisID])
                break;
            if (tuple[axisID] > parCoordsAtts.GetExtentMaxima()[axisID])
                break;
        }
    }
    
    if (axisID < axisCount) return;

    for (axisID = 0; axisID < axisCount; axisID++)
    {
        axisMin = axisMinima[axisID];
        axisMax = axisMaxima[axisID];

        if ((inputCoord = (double)tuple[axisID]) < axisMin)
            inputCoord = axisMin;
        else if (inputCoord > axisMax)
            inputCoord = axisMax;

        outputCoords[0] = axisXPositions[axisID];
        outputCoords[1] = (inputCoord-axisMin)/(axisMax-axisMin);
        
        dataCurvePoints->InsertNextPoint(outputCoords);
    }
    
    outputCurveCount++;
}

// ****************************************************************************
//  Method:  avtParallelCoordinatesFilter::DrawFocusPolyLines
//
//  Purpose:
//    Connect the points in the polydata with line segments.
//    (Used as an alternative to histograms for the focus when
//    requested by the user.)
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 27, 2008
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 25 16:47:14 EST 2009
//    Port to trunk.
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::DrawFocusPolyLines()
{
    int timer1 = visitTimer->StartTimer();
    vtkIdType vtkPointIDs[2];

    vtkIdType segmentCount = axisCount-1;
    vtkIdType firstVTKPointID, segmentNum;

    for (int curveNum = 0; curveNum < outputCurveCount; curveNum++)
    {
        firstVTKPointID = (vtkIdType)(curveNum * axisCount);

        for (segmentNum = 0; segmentNum < segmentCount; segmentNum++)
        {
            vtkPointIDs[0] = firstVTKPointID + segmentNum;
            vtkPointIDs[1] = vtkPointIDs[0] + 1;

            dataCurveLines->InsertNextCell(2, vtkPointIDs);
        }
    }
    visitTimer->StopTimer(timer1, "avtParallelCoordinatesFilter::DrawFocusPloyLines()");    
}
