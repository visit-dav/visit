/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                         avtQueryOverTimeFilter.C                          //
// ************************************************************************* //

#include <float.h>
#include <snprintf.h>

#include <avtDataObjectQuery.h>
#include <avtQueryFactory.h>
#include <avtQueryOverTimeFilter.h>
#include <avtLocateAndPickNodeQuery.h>
#include <avtLocateAndPickZoneQuery.h>
#include <avtVariableByZoneQuery.h>
#include <avtVariableByNodeQuery.h>

#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItUtility.h>

#include <avtCallback.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtParallel.h>

#include <VisItException.h>
#include <DebugStream.h>
#include <MapNode.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Method: avtQueryOverTimeFilter constructor
//
//  Arguments:
//    atts      The attributes the filter should use.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 15, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan  6 11:16:40 PST 2005
//    Add call to SetTimeLoop.  Initialize finalOutputCreated.
//
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005 
//    Initialize useTimeforXAxis and nResultsToStore. 
//
//    Hank Childs, Thu Feb  8 09:54:01 PST 2007
//    Initialize numAdditionalFilters.
//
//    Kathleen Bonnell, Tue Nov 27 15:35:55 PST 2007
//    Fix memory leak, delete 'query' used to initialize numAdditionalFilters. 
//    Set label from query's ShortDescription if available, otherwise use
//    query name.
//
//    Kathleen Bonnell, Tue Jul  8 15:48:38 PDT 2008
//    Added 'useVarForYAxis'.  Retrieve time curve specs here rather than
//    in Execute method at every timestep.
//
//    Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL.
//
// ****************************************************************************

avtQueryOverTimeFilter::avtQueryOverTimeFilter(const AttributeGroup *a)
{
    atts = *(QueryOverTimeAttributes*)a;
    SetTimeLoop(atts.GetStartTime(), atts.GetEndTime(), atts.GetStride());
    finalOutputCreated = false;
    useTimeForXAxis = true;
    useVarForYAxis = false;
    nResultsToStore = 1;

    TRY
    {
        QueryAttributes qatts = atts.GetQueryAtts();
        qatts.SetTimeStep(currentTime);
        avtDataObjectQuery *query = avtQueryFactory::Instance()->
            CreateQuery(&qatts);
        numAdditionalFilters = query->GetNFilters()+1; // 1 for query itself
        if (query->GetShortDescription() != NULL)
            label = query->GetShortDescription();
        else
            label = qatts.GetName();

        const MapNode &tqs = query->GetTimeCurveSpecs();
        useTimeForXAxis = tqs.GetEntry("useTimeForXAxis")->AsBool(); 
        useVarForYAxis  = tqs.GetEntry("useVarForYAxis")->AsBool(); 
        nResultsToStore = tqs.GetEntry("nResultsToStore")->AsInt(); 
        delete query;
    }
    CATCHALL
    {
        numAdditionalFilters = 2; // it's a guess
        debug1 << "There was a problem trying to instantiate a query for "
               << "a query over time.  Stifling the error handling, because "
               << "this problem will be caught later when we are better "
               << "prepared to do error handling." << endl;
    }         
    ENDTRY
}


// ****************************************************************************
//  Method: avtQueryOverTimeFilter destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 15, 2004 
//
// ****************************************************************************

avtQueryOverTimeFilter::~avtQueryOverTimeFilter()
{
}


// ****************************************************************************
//  Method:  avtQueryOverTimeFilter::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    March 15, 2004
//
// ****************************************************************************

avtFilter *
avtQueryOverTimeFilter::Create(const AttributeGroup *atts)
{
    return new avtQueryOverTimeFilter(atts);
}


// ****************************************************************************
//  Method: avtQueryOverTimeFilter::Execute
//
//  Purpose:
//    Peforms a query over time, and collects the results into a 
//    vtkDataSet consisting of 2d points (time, results).
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 15, 2004
//
//  Modifications:
//    Kathleen Bonnell, Fri Apr  2 13:18:08 PST 2004
//    Pass along to the query: startTime,  endTimes and stride instead of an 
//    intVector representing timesteps.  Send a doubleVector for the query
//    to fill with values requested for x-axis (cycle, time, or timestep).
//
//    Kathleen Bonnell, Tue May  4 14:21:37 PDT 2004 
//    Replaced query->SetSILUseSet with query->SetSILRestriction. 
//
//    Kathleen Bonnell, Thu Jun 24 07:54:44 PDT 2004 
//    Pass storage for skippedTimesteps and error message to the query.
//    Check for skippedTimeSteps and issue a Warning message.
//    Changed CATCH to CATCHALL.  Reset avtDataValidity before processing.
//
//    Kathleen Bonnell, Mon Jan  3 15:21:44 PST 2005
//    Reworked, since time-looping is now handled by parent class 
//    'avtTimeLoopFilter' instead of avtDatasetQuery::PerformQueryInTime.
//    Test for error condition upstream, capture and store query results
//    and times.   Moved IssueWarning callbacks to CreateFinalOutput.
//
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005 
//    Added call to query.GetTimeCurveSpecs.  Only retrive time value
//    if it will be used for the X-axis.  Store the correct number of results.
//
//    Kathleen Bonnell, Tue Oct 24 18:59:27 PDT 2006 
//    Added call to query->SetPickAttsForTimeQuery for VariableByNode/Zone.
//
//    Kathleen Bonnell, Tue Nov 20 10:33:49 PST 2007 
//    Added call to query->SetPickAttsForTimeQuery for LocateAndPickZone.
//
//    Kathleen Bonnell, Tue Jul  8 18:10:34 PDT 2008
//    Removed call to query.GetTimeCurveSpecs.  Now handled in constructor.
//
//    Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL.
//
//    Hank Childs, Sun Dec 26 12:13:19 PST 2010
//    Add support for parallelizing over time.
//
// ****************************************************************************

void
avtQueryOverTimeFilter::Execute(void)
{
    //
    // The real output will be created after all time steps have completed, 
    // so create a dummy output to pass along for now.
    //
    avtDataTree_p dummy = new avtDataTree();

    //
    // Set up error conditions and return early if any processor had an 
    // error upstream.
    //

    int hadError = 0;
    if (GetInput()->GetInfo().GetValidity().HasErrorOccurred())
    {
        errorMessage = GetInput()->GetInfo().GetValidity().GetErrorMessage();
        hadError = 1;
    }
    if (! ParallelizingOverTime())
        hadError = UnifyMaximumValue(hadError);
    if (hadError)
    {
        SetOutputDataTree(dummy);
        success = false;    
        return;
    }

    //
    // Set up the query. 
    //
    QueryAttributes qatts = atts.GetQueryAtts();
    qatts.SetTimeStep(currentTime);
    avtDataObjectQuery *query = avtQueryFactory::Instance()->
        CreateQuery(&qatts);
    query->SetInput(GetInput());
    if (ParallelizingOverTime())
    {
        query->SetParallelizingOverTime(true);
    }

    if (strncmp(query->GetType(), "avtVariableByNodeQuery",22) == 0)
    {
        PickAttributes patts = atts.GetPickAtts();
        ((avtVariableByNodeQuery*)query)->SetPickAttsForTimeQuery(&patts);
    }
    else if (strncmp(query->GetType(), "avtVariableByZoneQuery",22) == 0)
    {
        PickAttributes patts = atts.GetPickAtts();
        ((avtVariableByZoneQuery*)query)->SetPickAttsForTimeQuery(&patts);
    }
    else if (strncmp(query->GetType(), "avtLocateAndPickZoneQuery",25) == 0)
    {
        PickAttributes patts = atts.GetPickAtts();
        ((avtLocateAndPickZoneQuery*)query)->SetPickAttsForTimeQuery(&patts);
    }
    else if (strncmp(query->GetType(), "avtLocateAndPickNodeQuery",25) == 0)
    {
        PickAttributes patts = atts.GetPickAtts();
        ((avtLocateAndPickNodeQuery*)query)->SetPickAttsForTimeQuery(&patts);
    }
    query->SetTimeVarying(true);
    query->SetSILRestriction(currentSILR);

    //
    // HokeyHack ... we want only 1 curve, so limit the
    // query to 1 variable to avoid unnecessary processing.
    //
    if (nResultsToStore==1)
    {
        stringVector useThisVar;
        useThisVar.push_back(qatts.GetVariables()[0]);
        qatts.SetVariables(useThisVar);
    }
    //
    // End HokeyHack. 
    //

    TRY
    {
        query->PerformQuery(&qatts);
    }
    CATCHALL
    {
        SetOutputDataTree(dummy);
        success = false;
        delete query;
        RETHROW;
    }
    ENDTRY

    SetOutputDataTree(dummy);
    delete query;

    doubleVector results = qatts.GetResultsValue();
    if (results.size() == 0)
    {
        success = false;
        return;
    }
    else
    {
        success = true;
    }

    //
    // Store the necessary time value 
    //
    if (useTimeForXAxis)
    {
        double tval;  
        switch(atts.GetTimeType()) 
        {
        case QueryOverTimeAttributes::Cycle:
            tval = (double) GetInput()->GetInfo().GetAttributes().GetCycle();
            break;
        case QueryOverTimeAttributes::DTime: 
            tval = GetInput()->GetInfo().GetAttributes().GetTime();
            break;
        case QueryOverTimeAttributes::Timestep: 
        default: // timestep
            tval = (double)currentTime;
            break;
        }
        times.push_back(tval);
    }
    for (int i = 0; i < nResultsToStore; i++)
        qRes.push_back(results[i]);
}


// ****************************************************************************
//  Method: avtQueryOverTimeFilter::FilterSupportsTimeParallelization
//
//  Purpose:
//      Declares whether or not this filter supports time parallelization.
//      This filter does support time parallelization, provided the underlying
//      query supports time parallelization.
//
//  Programmer: Hank Childs
//  Creation:   December 24, 2010
//
// ****************************************************************************

bool
avtQueryOverTimeFilter::FilterSupportsTimeParallelization(void)
{
    QueryAttributes qatts = atts.GetQueryAtts();
    qatts.SetTimeStep(currentTime);
    avtDataObjectQuery *query = avtQueryFactory::Instance()->
        CreateQuery(&qatts);
    return query->QuerySupportsTimeParallelization();
}


// ****************************************************************************
//  Method: avtQueryOverTimeFilter::UpdateDataObjectInfo
//
//  Purpose:
//    Indicates the zones no longer correspond to the original problem.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 15, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan  6 11:21:44 PST 2005
//    Moved setting of  dataAttributes from PostExecute to here.
//
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005 
//    Use different labels/units if Time not used for X-Axis. 
//
//    Kathleen Bonnell, Wed Nov 28 16:33:22 PST 2007 
//    Use new 'label' member for Y axis label. 
//
//    Kathleen Bonnell, Tue Jul  8 15:48:38 PDT 2008
//    Set y-axis labels and units from query var if useVarForYAxis is true.
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Change extents names.
//
//    Kathleen Bonnell, Thu Mar  3 12:40:41 PST 2011
//    Set output Atts labels to be the variable names used for the query.
//
//    Kathleen Biagas, Thu Sep 29 06:13:54 PDT 2011
//    Set ConstructMultipleCurves in the output's DataAttributes.
//
// ****************************************************************************

void
avtQueryOverTimeFilter::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetTopologicalDimension(1);

    if (finalOutputCreated)
    {
        outAtts.GetOriginalSpatialExtents()->Clear();
        outAtts.GetDesiredSpatialExtents()->Clear();
        if (useTimeForXAxis)
        {
            outAtts.SetXLabel("Time");
            outAtts.SetYLabel(label);
            if (atts.GetTimeType() == QueryOverTimeAttributes::Cycle)
            {
                outAtts.SetXUnits("cycle");
            }
            else if (atts.GetTimeType() == QueryOverTimeAttributes::DTime)
            {
                outAtts.SetXUnits("time");
            }
            else if (atts.GetTimeType() == QueryOverTimeAttributes::Timestep)
            {
                outAtts.SetXUnits("timestep");
            }
            else 
            {
                outAtts.SetXUnits("");
            }
            if (useVarForYAxis)
            {
                std::string yl = outAtts.GetVariableName();
                outAtts.SetYLabel(yl);
                outAtts.SetYUnits(outAtts.GetVariableUnits(yl.c_str()));
            }
        }
        else 
        {
            std::string xl = atts.GetQueryAtts().GetVariables()[0] + "(t)";
            std::string yl = atts.GetQueryAtts().GetVariables()[1] + "(t)";
            outAtts.SetXLabel(xl);
            outAtts.SetYLabel(yl);
            outAtts.SetXUnits(atts.GetQueryAtts().GetXUnits());
            outAtts.SetYUnits(atts.GetQueryAtts().GetYUnits());
        }
        outAtts.SetLabels(atts.GetQueryAtts().GetVariables());
        if (atts.GetQueryAtts().GetVariables().size() > 1)
            outAtts.SetConstructMultipleCurves(true);
        else 
            outAtts.SetConstructMultipleCurves(false);
        double bounds[6];
        avtDataset_p ds = GetTypedOutput();
        avtDatasetExaminer::GetSpatialExtents(ds, bounds);
        outAtts.GetThisProcsOriginalSpatialExtents()->Set(bounds);
    }
}


// ****************************************************************************
//  Method: avtQueryOverTimeFilter::SetSILAtts
//
//  Purpose:
//    Sets the SILRestriction atts necessary to create a SILRestriction. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 4, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtQueryOverTimeFilter::SetSILAtts(const SILRestrictionAttributes *silAtts)
{
    querySILAtts = *silAtts;
}


// ****************************************************************************
//  Method: avtQueryOverTimeFilter::CreateFinalOutput
//
//  Purpose:
//    Combines the results of all timesteps into one vtkDataSet output.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 3, 2005 
//
//  Modifications:
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005 
//    Time not always used for X-Axis. 
//
//    Kathleen Bonnell, Mon Dec 19 08:01:21 PST 2005
//    Don't issue warning about 'multiples of 2' unless nResultsToStore > 1. 
//
//    Kathleen Bonnell, Thu Jul 27 17:43:38 PDT 2006 
//    Curves now represented as 1D RectilinearGrid.
//
//    Hank Childs, Fri Dec 24 21:01:29 PST 2010
//    Add support for parallelization over time.
//
//    Kathleen Bonnell, Thu Mar  3 12:42:35 PST 2011
//    In support of multiple-variable time picks, create an output Tree instead
//    of a single grid.
//
//    Kathleen Biagas, Tue Jun 21 09:52:26 PDT 2011
//    Change setting of 'multiCurve' to reflect use of MapNode for
//    query Input parameters.
//
//    Kathleen Biagas, Thu Jul  7 11:26:31 PDT 2011
//    Fixed incorrect generation of warning when nResults > 2.
//
// ****************************************************************************

void
avtQueryOverTimeFilter::CreateFinalOutput()
{
    if (ParallelizingOverTime())
    {
        double *totalQRes;
        int    *qResMsgs;
        CollectDoubleArraysOnRootProc(totalQRes, qResMsgs, 
                                      &(qRes[0]), qRes.size());
        double *totalTimes;
        int    *timesMsgs;
        CollectDoubleArraysOnRootProc(totalTimes, timesMsgs, 
                                      &(times[0]), times.size());
        if (PAR_Rank() == 0)
        {
            int i;
            int nResults = 0;
            int maxIterations = 0;
            for (i = 0 ; i < PAR_Size() ; i++)
            {
                nResults += timesMsgs[i];
                maxIterations = (timesMsgs[i] > maxIterations ? timesMsgs[i] 
                                                              : maxIterations);
            }

            std::vector<double> finalQRes(nResults, 0.);
            std::vector<double> finalTimes(nResults, 0.);
            int index = 0;
            for (int j = 0 ; j < maxIterations ; j++)
            {
                int loc = 0;
                for (i = 0 ; i < PAR_Size() ; i++)
                {
                    if (timesMsgs[i] > j)
                    {
                        finalQRes[index]  = totalQRes[loc+j];
                        finalTimes[index] = totalTimes[loc+j];
                        index++;
                    }
                    loc += timesMsgs[i];
                }
            }
            qRes = finalQRes;
            times = finalTimes;
            delete [] totalQRes;
            delete [] qResMsgs;
            delete [] totalTimes;
            delete [] timesMsgs;
        }
        else
        {
            SetOutputDataTree(new avtDataTree());
            finalOutputCreated = true;
            return;
        }
    }

    if (qRes.size() == 0)
    {
        debug4 << "Query failed at all timesteps" << endl;
        avtCallback::IssueWarning("Query failed at all timesteps");
        avtDataTree_p dummy = new avtDataTree();
        SetOutputDataTree(dummy);
        return;
    }
    if (useTimeForXAxis && qRes.size()/nResultsToStore != times.size())
    {
        debug4 << "QueryOverTime ERROR, number of results (" 
               << qRes.size() << ") does not equal number "
               << "of timesteps (" << times.size() << ")." << endl;
        avtCallback::IssueWarning(
            "\nQueryOverTime error, number of results does not equal "
            "number of timestates.  Curve being created may be missing "
            "some values.  Please contact a VisIt developer."); 
    }
    else if (nResultsToStore > 1 && qRes.size() % nResultsToStore != 0)
    {
        debug4 << "QueryOverTime ERROR, number of results (" 
               << qRes.size() << ") is not a multiple of " << nResultsToStore
               << "and therefore cannot generate x,y pairs." << endl;
        avtCallback::IssueWarning(
            "\nQueryOverTime error, number of results is incorrect.  "
            "Curve being created may be missing some values.  "
            "Please contact a VisIt developer."); 
    }
    if (skippedTimes.size() != 0)
    {
        ostrstream osm;
        osm << "\nQueryOverTime (" << atts.GetQueryAtts().GetName().c_str()
            << ") experienced\n"
            << "problems with the following timesteps and \n"
            << "skipped them while generating the curve:\n   ";

        for (int j = 0; j < skippedTimes.size(); j++)
            osm << skippedTimes[j] << " ";
        osm << "\nLast message received: " << errorMessage.c_str() << ends;
        debug4 << osm.str() << endl;
        avtCallback::IssueWarning(osm.str());
    }

    stringVector vars = atts.GetQueryAtts().GetVariables();
    bool multiCurve = false;
    if (atts.GetQueryAtts().GetQueryInputParams().HasEntry("curve_plot_type"))
    {
        multiCurve = (atts.GetQueryAtts().GetQueryInputParams().GetEntry("curve_plot_type")->AsInt() == 1);
    }
    avtDataTree_p tree = CreateTree(times, qRes, vars, multiCurve);
    SetOutputDataTree(tree);
    finalOutputCreated = true;
}


// ****************************************************************************
//  Method: CreateTree
//
//  Purpose:
//    Creates a 1D Rectilinear datasets with point data scalars.
//
//  Arguments:
//    times       The values to use for x-coordinates. 
//    res         The values to use for point data scalars.
//    vars        The variables associated with each output.
//    doMultiCurvePlot   The type of plot to create. 
//
//  Programmer:   Kathleen Bonnell
//  Creation:     March 15, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Made this a member method. Time not always used for x-axis.
//
//    Kathleen Bonnell, Thu Jul 27 17:43:38 PDT 2006 
//    Renamed from CreatePolys to CreateRGrid. 
//
//    Kathleen Bonnell, Thu Feb 17 09:43:16 PST 2011
//    Renamed from CreateRGrid to CreateTree, to reflect the possibility
//    of creating multiple outputs.  Added vars and doMultiCurvePlot args.
//
// ****************************************************************************

avtDataTree_p 
avtQueryOverTimeFilter::CreateTree(const doubleVector &times, 
                                   const doubleVector &res,
                                   stringVector &vars,
                                   const bool doMultiCurvePlot)
{
    int nPts = 0;
    bool singleCurve = true;
    if (useTimeForXAxis && nResultsToStore == 1)
    {
       // Single curve with time for x axis.  NORMAL case.
       // Most queries currently use this option. 
       nPts = (times.size() <= res.size() ? times.size() : res.size());
    }
    else if (!useTimeForXAxis && nResultsToStore == 2)
    {
       // Single curve, res[odd] = x, res[even] = y.
       nPts = res.size() / 2;
    }
    else if (useTimeForXAxis && nResultsToStore > 1)
    {
       singleCurve = false;
    }
    else if (!useTimeForXAxis && nResultsToStore > 2)
    {
       // multiple curves, res[odd] = x, res[even] = y.
    }

    if (singleCurve)
    {
  
        vtkRectilinearGrid *rgrid = vtkVisItUtility::Create1DRGrid(nPts);

        if (nPts == 0)
        {
            avtDataTree_p tree = new avtDataTree(rgrid, 0);
            rgrid->Delete();
            return tree;
        }

        vtkDataArray *xc = rgrid->GetXCoordinates();
        vtkFloatArray *sc = vtkFloatArray::New();

        sc->SetNumberOfComponents(1);
        sc->SetNumberOfTuples(nPts);

        rgrid->GetPointData()->SetScalars(sc);
        rgrid->SetDimensions(nPts, 1 , 1);

        sc->Delete();

        for (int i = 0; i < nPts; i++)
        {
            if (useTimeForXAxis)
            {
                xc->SetTuple1(i, times[i]);
                sc->SetTuple1(i, res[i]);
            }
            else 
            {
                xc->SetTuple1(i, res[i*2]);
                sc->SetTuple1(i, res[i*2+1]);
            }
        }
        avtDataTree_p tree = new avtDataTree(rgrid, 0);
        rgrid->Delete();
        return tree;
    }
    else  if(doMultiCurvePlot)
    {
        // Setup for a MultiCurve plot
        nPts = times.size();
  
        vtkRectilinearGrid *rgrid = 
            vtkVisItUtility::CreateEmptyRGrid(nPts, nResultsToStore, 1, VTK_FLOAT);

        if (nPts == 0)
        {
            avtDataTree_p tree = new avtDataTree(rgrid, 0);
            rgrid->Delete();
            return tree;
        }

        if (res.size() != nPts*nResultsToStore)
        {
            debug1 << "Mismatch in QOT times/results sizes: " << endl;
            debug1 << "    times size:      " << times.size() << endl;
            debug1 << "    nResultsToStore: " << nResultsToStore << endl;
            debug1 << "    results size:    " << res.size() << endl;
            avtDataTree_p tree = new avtDataTree(rgrid, 0);
            rgrid->Delete();
            return tree;
        }

        vtkDataArray *xc = rgrid->GetXCoordinates();
        vtkDataArray *yc = rgrid->GetYCoordinates();
        vtkFloatArray *sc = vtkFloatArray::New();

        sc->SetNumberOfComponents(1);
        sc->SetNumberOfTuples(nPts*nResultsToStore);

        rgrid->GetPointData()->SetScalars(sc);

        sc->Delete();

        for (int i = 0; i < nPts; i++)
        {
            xc->SetTuple1(i, times[i]);
        }
        for (int i = 0; i < nResultsToStore; i++)
        {
            yc->SetTuple1(i, i);
            for (int j = 0; j < nPts; j++)
                sc->SetTuple1(i*nPts+j, res[i + nResultsToStore*j]);
        }
        avtDataTree_p tree = new avtDataTree(rgrid, 0);
        rgrid->Delete();
        return tree;
    }
    else  
    {
        // Setup for a Curve plot with multiple curves.
        nPts = times.size();
        if (nPts == 0)
        {
            vtkRectilinearGrid *rgrid = 
                vtkVisItUtility::Create1DRGrid(nPts, VTK_FLOAT);
            avtDataTree_p tree = new avtDataTree(rgrid, 0);
            rgrid->Delete();
            return tree;
        }
        if (res.size() != nPts*nResultsToStore)
        {
            vtkRectilinearGrid *rgrid = 
                vtkVisItUtility::Create1DRGrid(nPts, VTK_FLOAT);
            debug1 << "Mismatch in QOT times/results sizes: " << endl;
            debug1 << "    times size:      " << times.size() << endl;
            debug1 << "    nResultsToStore: " << nResultsToStore << endl;
            debug1 << "    results size:    " << res.size() << endl;
            avtDataTree_p tree = new avtDataTree(rgrid, 0);
            rgrid->Delete();
            return tree;
        }
        if (vars.size() != nResultsToStore)
        {
            vtkRectilinearGrid *rgrid = 
                vtkVisItUtility::Create1DRGrid(nPts, VTK_FLOAT);
            debug1 << "Mismatch in QOT vars/nresults sizes: " << endl;
            debug1 << "    vars size:       " << times.size() << endl;
            debug1 << "    nResultsToStore: " << nResultsToStore << endl;
            avtDataTree_p tree = new avtDataTree(rgrid, 0);
            rgrid->Delete();
            return tree;
        }
        int nVars = vars.size();
        vtkDataSet **grids = new vtkDataSet *[nVars];
        for (int i = 0; i< nVars; ++i)
        {
            grids[i] = vtkVisItUtility::Create1DRGrid(nPts, VTK_FLOAT);
            vtkDataArray *xc = ((vtkRectilinearGrid*)grids[i])->GetXCoordinates();
            vtkFloatArray *sc = vtkFloatArray::New();
            sc->SetNumberOfComponents(1);
            sc->SetNumberOfTuples(nPts);
            sc->SetName(vars[i].c_str());
            grids[i]->GetPointData()->SetScalars(sc);
            sc->Delete();
            for (int j = 0; j < nPts; ++j)
            {
                xc->SetTuple1(j, times[j]);
                sc->SetTuple1(j, res[i + nResultsToStore*j]);
            }
        }
        avtDataTree_p tree = new avtDataTree(nVars, grids, -1, vars);
        for (int i = 0; i< nVars; ++i)
            grids[i]->Delete();
        delete [] grids;
        return tree;
    }
}

