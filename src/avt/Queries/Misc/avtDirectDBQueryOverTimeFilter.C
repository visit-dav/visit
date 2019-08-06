/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//                         avtDirectDBQueryOverTimeFilter.C                          //
// ************************************************************************* //

#include <float.h>
#include <snprintf.h>
#include <sstream>
#include <string>
#include <vector>

#include <avtDataObjectQuery.h>
#include <avtQueryFactory.h>
#include <avtDirectDBQueryOverTimeFilter.h>
#include <avtLocateAndPickNodeQuery.h>
#include <avtLocateAndPickZoneQuery.h>
#include <avtVariableByZoneQuery.h>
#include <avtVariableByNodeQuery.h>

#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItUtility.h>
#include <vtkFloatArray.h>

#include <avtCallback.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtParallel.h>
#include <avtOriginatingSource.h>

#include <VisItException.h>
#include <DebugStream.h>
#include <MapNode.h>

// ****************************************************************************
//  Method: 
//
//  Arguments:
//    atts      The attributes the filter should use.
//
//  Programmer: 
//  Creation:  
//
//  Modifications:
//
// ****************************************************************************

avtDirectDBQueryOverTimeFilter::avtDirectDBQueryOverTimeFilter(const AttributeGroup *a)
{
    cerr << "INITIALIZING DB QOT" << endl;//FIXME
    atts = *(QueryOverTimeAttributes*)a;
    finalOutputCreated = false;
    useTimeForXAxis = true;
    useVarForYAxis = false;
    nResultsToStore = 1;

    TRY
    {
        QueryAttributes qatts = atts.GetQueryAtts();
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
        cerr << "INIT PROB" << endl;//FIXME
        numAdditionalFilters = 2; // it's a guess
        debug1 << "There was a problem trying to instantiate a query for "
               << "a query over time.  Stifling the error handling, because "
               << "this problem will be caught later when we are better "
               << "prepared to do error handling." << endl;
    }
    ENDTRY
}


// ****************************************************************************
//  Method: avtDirectDBQueryOverTimeFilter destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 15, 2004
//
// ****************************************************************************

avtDirectDBQueryOverTimeFilter::~avtDirectDBQueryOverTimeFilter()
{
}


// ****************************************************************************
//  Method:  avtDirectDBQueryOverTimeFilter::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    March 15, 2004
//
// ****************************************************************************

avtFilter *
avtDirectDBQueryOverTimeFilter::Create(const AttributeGroup *atts)
{
    return new avtDirectDBQueryOverTimeFilter(atts);
}



void
avtDirectDBQueryOverTimeFilter::Execute(void)
{
    cerr << "EXECUTING DB QOT" << endl;//FIXME
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
        //errorMessage = GetInput()->GetInfo().GetValidity().GetErrorMessage();
        hadError = 1;
    }
    //if (! ParallelizingOverTime())
    //    hadError = UnifyMaximumValue(hadError);
    if (hadError)
    {
        SetOutputDataTree(dummy);
        success = false;
        return;
    }


    //
    // Set up the query.
    //
    //QueryAttributes qatts = atts.GetQueryAtts();
    //avtDataObjectQuery *query = avtQueryFactory::Instance()->
    //    CreateQuery(&qatts);
    //query->SetInput(GetInput());
    //if (ParallelizingOverTime())
    //{
    //    query->SetParallelizingOverTime(true);
    //}

    //if (strncmp(query->GetType(), "avtVariableByNodeQuery",22) == 0)
    //{
    //    PickAttributes patts = atts.GetPickAtts();
    //    ((avtVariableByNodeQuery*)query)->SetPickAttsForTimeQuery(&patts);
    //}
    //else if (strncmp(query->GetType(), "avtVariableByZoneQuery",22) == 0)
    //{
    //    PickAttributes patts = atts.GetPickAtts();
    //    ((avtVariableByZoneQuery*)query)->SetPickAttsForTimeQuery(&patts);
    //}
    //else if (strncmp(query->GetType(), "avtLocateAndPickZoneQuery",25) == 0)
    //{
    //    PickAttributes patts = atts.GetPickAtts();
    //    ((avtLocateAndPickZoneQuery*)query)->SetPickAttsForTimeQuery(&patts);
    //}
    //else if (strncmp(query->GetType(), "avtLocateAndPickNodeQuery",25) == 0)
    //{
    //    PickAttributes patts = atts.GetPickAtts();
    //    ((avtLocateAndPickNodeQuery*)query)->SetPickAttsForTimeQuery(&patts);
    //}
    //query->SetTimeVarying(true);
    //query->SetSILRestriction(currentSILR);

    ////  
    //// HokeyHack ... we want only 1 curve, so limit the
    //// query to 1 variable to avoid unnecessary processing.
    //// 
    //if (nResultsToStore==1)
    //{
    //    stringVector useThisVar;
    //    useThisVar.push_back(qatts.GetVariables()[0]);
    //    qatts.SetVariables(useThisVar);
    //}
    //// 
    //// End HokeyHack.
    ////

    //TRY
    //{
    //    query->PerformQuery(&qatts);
    //}
    //CATCHALL
    //{
    //    SetOutputDataTree(dummy);
    //    success = false;
    //    delete query;
    //    RETHROW;
    //}
    //ENDTRY

    //SetOutputDataTree(dummy);
    //delete query;

    //doubleVector results = qatts.GetResultsValue();
    //if (results.size() == 0)
    //{
    //    success = false;
    //    return;
    //}
    //else
    //{
    //    success = true;
    //}

    
    PickAttributes pAtts = atts.GetPickAtts();
    MapNode timeOpts     = pAtts.GetTimeOptions();

    intVector elements;
    elements.push_back(pAtts.GetElementNumber()); 

    stringVector vars = atts.GetQueryAtts().GetVariables(); 

    int stride = atts.GetStride();//TODO: integrate stride into query.
    int startT = atts.GetStartTime();
    int stopT = atts.GetEndTime();
    int tsRange[] = {startT, stopT};


    cerr << "\nSTARTING TIME SPAN FILTER TIMING: " << endl;
    auto start = std::chrono::high_resolution_clock::now();

    vtkFloatArray **spanArray = (vtkFloatArray **) (GetInput()->GetSource()->
        GetOriginatingSource()->FetchTimeAndElementSpanVars(0, elements, vars, tsRange));

    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = stop - start;
    cerr << "TIME SPAN FILTER TIMING: " << elapsed.count() << endl;

    if (spanArray != NULL)
    {
        int numSpans  = elements.size() * vars.size();
        int numTuples = tsRange[1] - tsRange[0];
        
        int curT = startT;
        for (int i = 0; i < numTuples; i++)
        {
            qRes.push_back(spanArray[0]->GetTuple1(i));
            times.push_back(curT);
            curT += stride;
        }
    }
    else
    {
        cerr << "NULL!!" << endl;
    }

    CreateFinalOutput();
}



bool
avtDirectDBQueryOverTimeFilter::FilterSupportsTimeParallelization(void)
{
    //QueryAttributes qatts = atts.GetQueryAtts();
    //qatts.SetTimeStep(currentTime);
    //avtDataObjectQuery *query = avtQueryFactory::Instance()->
    //    CreateQuery(&qatts);
    //bool result = query->QuerySupportsTimeParallelization();
    //delete query;
    //return result;
    return false;
}

// ****************************************************************************
//  Method: 
//
//  Purpose:
//    
//
//  Programmer: 
//  Creation:   
//
//  Modifications:
//
// ****************************************************************************

void
avtDirectDBQueryOverTimeFilter::UpdateDataObjectInfo(void)
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
//  Method: avtDirectDBQueryOverTimeFilter::SetSILAtts
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
avtDirectDBQueryOverTimeFilter::SetSILAtts(const SILRestrictionAttributes *silAtts)
{
    querySILAtts = *silAtts;
}


// ****************************************************************************
//  Method: 
//
//  Purpose:
//    
//
//  Programmer: 
//  Creation:   
//
//  Modifications:
//
// ****************************************************************************

void
avtDirectDBQueryOverTimeFilter::CreateFinalOutput()
{
    if (PAR_Rank() == 0)
    {
        //if (qRes.size() == 0)
        //{
        //    debug4 << "Query failed at all timesteps" << endl;
        //    avtCallback::IssueWarning("Query failed at all timesteps");
        //    avtDataTree_p dummy = new avtDataTree();
        //    SetOutputDataTree(dummy);
        //    return;
        //}
        //if (useTimeForXAxis && qRes.size()/nResultsToStore != times.size())
        //{
        //    debug4 << "QueryOverTime ERROR, number of results ("
        //           << qRes.size() << ") does not equal number "
        //           << "of timesteps (" << times.size() << ")." << endl;
        //    avtCallback::IssueWarning(
        //        "\nQueryOverTime error, number of results does not equal "
        //        "number of timestates.  Curve being created may be missing "
        //        "some values.  Please contact a VisIt developer.");
        //}
        //else if (nResultsToStore > 1 && qRes.size() % nResultsToStore != 0)
        //{
        //    debug4 << "QueryOverTime ERROR, number of results ("
        //           << qRes.size() << ") is not a multiple of " << nResultsToStore
        //           << "and therefore cannot generate x,y pairs." << endl;
        //    avtCallback::IssueWarning(
        //        "\nQueryOverTime error, number of results is incorrect.  "
        //        "Curve being created may be missing some values.  "
        //        "Please contact a VisIt developer.");
        //}
        //if (skippedTimes.size() != 0)
        //{
        //    std::ostringstream osm;
        //    osm << "\nQueryOverTime (" << atts.GetQueryAtts().GetName().c_str()
        //        << ") experienced\n"
        //        << "problems with the following timesteps and \n"
        //        << "skipped them while generating the curve:\n   ";
    
        //    for (size_t j = 0; j < skippedTimes.size(); j++)
        //        osm << skippedTimes[j] << " ";
        //    osm << "\nLast message received: " << errorMessage.c_str() << ends;
        //    debug4 << osm.str() << endl;
        //    avtCallback::IssueWarning(osm.str().c_str());
        //}

        stringVector vars = atts.GetQueryAtts().GetVariables();
        bool multiCurve = false;
        if (atts.GetQueryAtts().GetQueryInputParams().HasNumericEntry("curve_plot_type"))
        {
            multiCurve = (atts.GetQueryAtts().GetQueryInputParams().GetEntry("curve_plot_type")->ToInt() == 1);
        }
        avtDataTree_p tree = CreateTree(times, qRes, vars, multiCurve);
        SetOutputDataTree(tree);
        finalOutputCreated = true;
    }
    else
    {
        SetOutputDataTree(new avtDataTree());
        finalOutputCreated = true;
    }
}


// ****************************************************************************
//  Method: 
//
//  Purpose:
//    Creates a 1D Rectilinear datasets with point data scalars.
//
//  Arguments:
//    times       
//    res         
//    vars       
//    doMultiCurvePlot   
//
//  Programmer:   
//  Creation:    
//
//  Modifications:
//
// ****************************************************************************

avtDataTree_p
avtDirectDBQueryOverTimeFilter::CreateTree(const doubleVector &times,
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
       nPts = (int)(times.size() <= res.size() ? times.size() : res.size());
    }
    else if (!useTimeForXAxis && nResultsToStore == 2)
    {
       // Single curve, res[odd] = x, res[even] = y.
       nPts = (int)res.size() / 2;
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
        cerr << "SINGLE CURVE" << endl;//FIXME
        vtkRectilinearGrid *rgrid = vtkVisItUtility::Create1DRGrid(nPts);

        if (nPts == 0)
        {
            cerr << "NO POINTS?!" << endl;//FIXME
            avtDataTree_p tree = new avtDataTree(rgrid, 0);
            rgrid->Delete();
            return tree;
        }

        vtkDataArray *xc = rgrid->GetXCoordinates();
        vtkDoubleArray *sc = vtkDoubleArray::New();

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
        cerr << "MULTI CURVE" << endl;//FIXME
        // Setup for a MultiCurve plot
        nPts = (int)times.size();

        vtkRectilinearGrid *rgrid =
            vtkVisItUtility::CreateEmptyRGrid(nPts, nResultsToStore, 1, VTK_FLOAT);

        if (nPts == 0)
        {
            avtDataTree_p tree = new avtDataTree(rgrid, 0);
            rgrid->Delete();
            return tree;
        }

        if (res.size() != (size_t)nPts*nResultsToStore)
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
        vtkDoubleArray *sc = vtkDoubleArray::New();

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
        cerr << "MULTIPLE CURVES" << endl;//FIXME
        // Setup for a Curve plot with multiple curves.
        nPts = (int)times.size();
        if (nPts == 0)
        {
            vtkRectilinearGrid *rgrid =
                vtkVisItUtility::Create1DRGrid(nPts, VTK_FLOAT);
            avtDataTree_p tree = new avtDataTree(rgrid, 0);
            rgrid->Delete();
            return tree;
        }
        if (res.size() != (size_t)nPts*nResultsToStore)
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
        if (vars.size() != (size_t)nResultsToStore)
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
            vtkDoubleArray *sc = vtkDoubleArray::New();
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

