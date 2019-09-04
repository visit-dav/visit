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
    atts               = *(QueryOverTimeAttributes*)a;
    finalOutputCreated = false;
    useTimeForXAxis    = true;
    useVarForYAxis     = false;
    numCurves          = 0;

    TRY
    {
        QueryAttributes qatts = atts.GetQueryAtts();
        avtDataObjectQuery *query = avtQueryFactory::Instance()->
            CreateQuery(&qatts);
        if (query->GetShortDescription() != NULL)
            label = query->GetShortDescription();
        else
            label = qatts.GetName();

        const MapNode &tqs = query->GetTimeCurveSpecs();
        useTimeForXAxis    = tqs.GetEntry("useTimeForXAxis")->AsBool();
        useVarForYAxis     = tqs.GetEntry("useVarForYAxis")->AsBool();
        numCurves          = tqs.GetEntry("nResultsToStore")->AsInt();
        delete query;
    }
    CATCHALL
    {
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
//  Programmer: 
//  Creation:   
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
//  Programmer:  
//  Creation:    
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
    if (hadError)
    {
        SetOutputDataTree(dummy);
        success = false;
        return;
    }
    
    PickAttributes pAtts = atts.GetPickAtts();
    MapNode timeOpts     = pAtts.GetTimeOptions();
    int domain           = pAtts.GetDomain();
  
    //
    // The domain appears to default to -1 when not using a
    // multi-domain dataset. Change it to 0.  
    //
    if (domain < 0)
    {
        domain = 0;
    }

    intVector elements;
    elements.push_back(pAtts.GetElementNumber()); 

    stringVector vars = atts.GetQueryAtts().GetVariables(); 

    int startT    = atts.GetStartTime();
    int stopT     = atts.GetEndTime();
    int stride    = atts.GetStride();
    int tsRange[] = {startT, stopT};

    cerr << "\nSTARTING TIME SPAN FILTER TIMING: " << endl;
    auto start = std::chrono::high_resolution_clock::now();

    avtOriginatingSource *origSource = GetInput()->GetSource()->GetOriginatingSource();
    vtkFloatArray **spanArray = (vtkFloatArray **) (origSource->
        FetchTimeSpanCurves(0, vars, elements, tsRange, stride));

    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = stop - start;
    cerr << "TIME SPAN FILTER TIMING: " << elapsed.count() << endl;

    if (spanArray != NULL)
    {
        numCurves     = elements.size() * vars.size();
        int numTuples = spanArray[0]->GetNumberOfTuples();
        curves.resize(numCurves);

        for (int cIdx = 0; cIdx < numCurves; ++cIdx)
        {
            if (spanArray[cIdx]->GetNumberOfTuples() != numTuples)
            {
                char msg[128];
                snprintf(msg, 128, "The retreived curves are of different sizes! "
                   "Not good...");
                EXCEPTION1(VisItException, msg);
            }

            for (int i = 0; i < numTuples; ++i)
            {
                curves[cIdx].push_back(spanArray[cIdx]->GetTuple1(i));
            }
        }

        switch (atts.GetTimeType())
        {
            case QueryOverTimeAttributes::Cycle: 
            {
                intVector cycles;
                origSource->FetchCycles(domain, cycles);
                int curStep = startT;

                for (int i = 0; i < numTuples; i++)
                {
                    times.push_back(cycles[curStep]);
                    curStep += stride;
                }
                break;
            } 
            case QueryOverTimeAttributes::DTime: 
            {
                doubleVector simTimes;
                origSource->FetchTimes(domain, simTimes);
                int curStep = startT;

                for (int i = 0; i < numTuples; i++)
                {
                    times.push_back(simTimes[curStep]);
                    curStep += stride;
                }
                break;
            } 
            case QueryOverTimeAttributes::Timestep: 
            {
                int curStep = startT;

                for (int i = 0; i < numTuples; i++)
                {
                    times.push_back(curStep);
                    curStep += stride;
                }
                break;
            } 
        }

        //
        // Clean up or span array memory. 
        //
        for (int i = 0; i < numCurves; ++i)
        {
            if (spanArray[i] != NULL)
            {
                spanArray[i]->Delete();
            }
        }
    
        delete [] spanArray;
    }
    else
    {
        debug1 << "DirectDBQueryOverTime was unable to retrieve " <<
            "any curves..." << endl;
    }

    CreateFinalOutput();
}


bool
avtDirectDBQueryOverTimeFilter::FilterSupportsTimeParallelization(void)
{
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
    {
        outAtts.SetConstructMultipleCurves(true);
    }
    else
    {
        outAtts.SetConstructMultipleCurves(false);
    }
    double bounds[6];
    avtDataset_p ds = GetTypedOutput();
    avtDatasetExaminer::GetSpatialExtents(ds, bounds);
    outAtts.GetThisProcsOriginalSpatialExtents()->Set(bounds);
    
}


// ****************************************************************************
//  Method: avtDirectDBQueryOverTimeFilter::SetSILAtts
//
//  Purpose:
//    Sets the SILRestriction atts necessary to create a SILRestriction.
//
//  Programmer: 
//  Creation:   
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
        //if (curves.size() == 0)
        //{
        //    debug4 << "Query failed at all timesteps" << endl;
        //    avtCallback::IssueWarning("Query failed at all timesteps");
        //    avtDataTree_p dummy = new avtDataTree();
        //    SetOutputDataTree(dummy);
        //    return;
        //}

        stringVector vars = atts.GetQueryAtts().GetVariables();
        bool multiCurve = false;
        if (atts.GetQueryAtts().GetQueryInputParams().HasNumericEntry("curve_plot_type"))
        {
            multiCurve = (atts.GetQueryAtts().GetQueryInputParams().
                GetEntry("curve_plot_type")->ToInt() == 1);
        }
        avtDataTree_p tree = CreateTreeFromCurves(times, vars, multiCurve);
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
avtDirectDBQueryOverTimeFilter::CreateTreeFromCurves(const doubleVector &times,
                                                     stringVector &vars,
                                                     const bool doMultiCurvePlot)
{
    int numPts = (int) times.size();

    if (numCurves == 1)
    {
        vtkRectilinearGrid *rgrid = vtkVisItUtility::Create1DRGrid(numPts);

        if (numPts == 0)
        {
            avtDataTree_p tree = new avtDataTree(rgrid, 0);
            rgrid->Delete();
            return tree;
        }

        vtkDataArray *xCoords   = rgrid->GetXCoordinates();
        vtkDoubleArray *scalars = vtkDoubleArray::New();

        scalars->SetNumberOfComponents(1);
        scalars->SetNumberOfTuples(numPts);

        rgrid->GetPointData()->SetScalars(scalars);
        rgrid->SetDimensions(numPts, 1 , 1);

        scalars->Delete();

        for (int i = 0; i < numPts; i++)
        {
            if (useTimeForXAxis)
            {
                xCoords->SetTuple1(i, times[i]);
                scalars->SetTuple1(i, curves[0][i]);
            }
            else
            {
                xCoords->SetTuple1(i, curves[0][i*2]);
                scalars->SetTuple1(i, curves[0][i*2+1]);
            }
        }
        avtDataTree_p tree = new avtDataTree(rgrid, 0);
        rgrid->Delete();
        return tree;
    }
    else if(doMultiCurvePlot)
    {
        cerr << "MULTI CURVE" << endl;//FIXME

        vtkRectilinearGrid *rgrid =
            vtkVisItUtility::CreateEmptyRGrid(numPts, numCurves, 1, VTK_FLOAT);

        if (numPts == 0)
        {
            avtDataTree_p tree = new avtDataTree(rgrid, 0);
            rgrid->Delete();
            return tree;
        }

        vtkDataArray *xCoords   = rgrid->GetXCoordinates();
        vtkDataArray *yCoords   = rgrid->GetYCoordinates();
        vtkDoubleArray *scalars = vtkDoubleArray::New();

        scalars->SetNumberOfComponents(1);
        scalars->SetNumberOfTuples(numPts*numCurves);

        rgrid->GetPointData()->SetScalars(scalars);

        for (int i = 0; i < numPts; i++)
        {
            xCoords->SetTuple1(i, times[i]);
        }

        for (int i = 0; i < numCurves; i++)
        {
            yCoords->SetTuple1(i, i);
            for (int j = 0; j < numPts; j++)
            {
                scalars->SetTuple1(i*numPts+j, curves[i][j]);
            }
        }
        avtDataTree_p tree = new avtDataTree(rgrid, 0);
        rgrid->Delete();
        scalars->Delete();
        return tree;
    }
    else
    {
        cerr << "MULTIPLE CURVES" << endl;//FIXME
        // Setup for a Curve plot with multiple curves.

        if (numPts == 0)
        {
            vtkRectilinearGrid *rgrid =
                vtkVisItUtility::Create1DRGrid(numPts, VTK_FLOAT);
            avtDataTree_p tree = new avtDataTree(rgrid, 0);
            rgrid->Delete();
            return tree;
        }

        if (vars.size() != (size_t)numCurves)
        {
            debug1 << "Mismatch in Direct DB QOT vars/curves sizes: " << endl;
            debug1 << "    vars size: " << vars.size() << endl;
            debug1 << "    numCurves: " << numCurves << endl;

            vtkRectilinearGrid *rgrid =
                vtkVisItUtility::Create1DRGrid(numPts, VTK_FLOAT);
            avtDataTree_p tree = new avtDataTree(rgrid, 0);
            rgrid->Delete();
            return tree;
        }

        int numVars = vars.size();
        vtkDataSet **grids = new vtkDataSet *[numVars];

        for (int i = 0; i< numVars; ++i)
        {
            grids[i] = vtkVisItUtility::Create1DRGrid(numPts, VTK_FLOAT);

            vtkDataArray *xCoords   = ((vtkRectilinearGrid*)grids[i])->GetXCoordinates();
            vtkDoubleArray *scalars = vtkDoubleArray::New();

            scalars->SetNumberOfComponents(1);
            scalars->SetNumberOfTuples(numPts);
            scalars->SetName(vars[i].c_str());

            grids[i]->GetPointData()->SetScalars(scalars);

            for (int j = 0; j < numPts; ++j)
            {
                xCoords->SetTuple1(j, times[j]);
                scalars->SetTuple1(j, curves[i][j]);
            }

            scalars->Delete();
        }
        avtDataTree_p tree = new avtDataTree(numVars, grids, -1, vars);

        for (int i = 0; i< numVars; ++i)
        {
            if (grids[i] != NULL)
            {
                grids[i]->Delete();
            }
        }

        delete [] grids;

        return tree;
    }
}

