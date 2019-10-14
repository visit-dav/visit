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
//                         avtDirectDatabaseQOTFilter.C                          //
// ************************************************************************* //

#include <string>

#include <avtDirectDatabaseQOTFilter.h>
#include <avtDataObjectQuery.h>
#include <avtQueryFactory.h>

#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItUtility.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>

#include <avtCallback.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtParallel.h>
#include <avtCommonDataFunctions.h>

#include <VisItException.h>
#include <DebugStream.h>
#include <MapNode.h>


// ****************************************************************************
//  Method: avtDirectDatabaseQOTFilter constructor
//
//  Arguments:
//    atts      The attributes the filter should use.
//
//  Programmer: Alister Maguire 
//  Creation:   Tue Sep 24 11:15:10 MST 2019 
//
//  Modifications:
//
// ****************************************************************************

avtDirectDatabaseQOTFilter::avtDirectDatabaseQOTFilter(const AttributeGroup *a)
 : avtQueryOverTimeFilter(a)
{
    success            = true;
    finalOutputCreated = false;
    useTimeForXAxis    = true;
    useVarForYAxis     = false;
    YLabel             = "";

    //
    // Let's first try to retreive some information about the query. 
    //
    TRY
    {
        QueryAttributes qatts     = atts.GetQueryAtts();
        avtDataObjectQuery *query = avtQueryFactory::Instance()->
            CreateQuery(&qatts);

        if (query->GetShortDescription() != NULL)
        {
            YLabel = query->GetShortDescription();
        }
        else
        {
            YLabel = qatts.GetName();
        }

        const MapNode &tqs = query->GetTimeCurveSpecs();
        useTimeForXAxis    = tqs.GetEntry("useTimeForXAxis")->AsBool();
        useVarForYAxis     = tqs.GetEntry("useVarForYAxis")->AsBool();
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
//  Method: avtDirectDatabaseQOTFilter destructor
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 11:15:10 MST 2019 
//
// ****************************************************************************

avtDirectDatabaseQOTFilter::~avtDirectDatabaseQOTFilter()
{
}


// ****************************************************************************
//  Method:  avtDirectDatabaseQOTFilter::Create
//
//  Purpose:
//    Call the constructor.
//
//  Arguments:
//    atts      The attributes the filter should use.
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 11:15:10 MST 2019 
//
// ****************************************************************************

avtFilter *
avtDirectDatabaseQOTFilter::Create(const AttributeGroup *atts)
{
    return new avtDirectDatabaseQOTFilter(atts);
}


// ****************************************************************************
//  Method:  avtDirectDatabaseQOTFilter::Execute
//
//  Purpose:
//      Construct our time query data tree. 
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 11:15:10 MST 2019 
//
//  Modifications:
//
// ****************************************************************************

void
avtDirectDatabaseQOTFilter::Execute(void)
{
    //
    // Assume success until proven otherwise. 
    //
    success = true;

    avtDataTree_p dataTree = GetInputDataTree();

    if (PAR_Rank() == 0)
    {
        bool multiCurve   = false;
        if (atts.GetQueryAtts().GetQueryInputParams().
            HasNumericEntry("curve_plot_type"))
        {
            multiCurve = (atts.GetQueryAtts().GetQueryInputParams().
                GetEntry("curve_plot_type")->ToInt() == 1);
        }

        int numLeaves = 0;
        vtkDataSet **leaves = dataTree->GetAllLeaves(numLeaves);

        if (numLeaves > 0)
        {
            vtkPolyData *QOTData = (vtkPolyData *) leaves[0];

            if (QOTData == NULL)
            {
                debug1 << "Direct Database QOT leaf is NULL!" << endl;

                SetOutputDataTree(new avtDataTree());
            }

            vtkPolyData *refined = VerifyAndRefineTimesteps(QOTData);
            avtDataTree_p tree   = ConstructCurveTree(refined, multiCurve);
            SetOutputDataTree(tree);
            refined->Delete();
        }
        else
        {
            debug1 << "Direct Database QOT recieved no leaves!" << endl;

            SetOutputDataTree(new avtDataTree());
        }
    }
    else
    {
        SetOutputDataTree(new avtDataTree());
    }

    finalOutputCreated = true;
    UpdateDataObjectInfo();
}


// ****************************************************************************
//  Method:  avtDirectDatabaseQOTFilter::VerifyAndRefineTimesteps
//
//  Purpose:
//      Verify that all timesteps have been retrieved. If we are
//      missing any, then let's reduce our dataset to only include
//      the valid data, and let the user know which timesteps
//      were skipped. 
//
//      Note: timesteps that encountered errors will have added
//            NaN values to the associated data positions. 
//
//  Arguments:
//      polyData    The polydata containing the curves. 
//
//  Returns:
//      A vtkPolyData object only containing valid curves. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Sep 30 14:17:20 MST 2019
//
//  Modifications:
//
// ****************************************************************************

vtkPolyData *
avtDirectDatabaseQOTFilter::VerifyAndRefineTimesteps(vtkPolyData *inPolyData)
{
    vtkPolyData *outPolyData = vtkPolyData::New();
    outPolyData->ShallowCopy(inPolyData);

    vtkPointData *inPtData = inPolyData->GetPointData();
    vtkPoints *inPts       = inPolyData->GetPoints();

    if (inPtData ==  NULL || inPts == NULL ||
        inPtData->GetScalars() == NULL)
    {
        return outPolyData;
    }

    int numCurves = inPtData->GetNumberOfArrays();
    int numPts    = inPtData->GetScalars()->GetNumberOfTuples();

    if (numCurves == 0)
    { 
        return outPolyData;
    }

    vtkPoints *outPts = vtkPoints::New();

    //
    // If we're missing data, this will be an over-estimate, but
    // it should never be an under-estimate. 
    //
    outPts->Resize(numPts);

    int stride = atts.GetStride();
    int startT = atts.GetStartTime();
    int stopT  = atts.GetEndTime();

    //
    // First pass: look for invalid data and mark their locations
    // and record their time states.
    //
    bool missingData = false;
    intVector invalidStateList;
    invalidStateList.reserve(numPts);

    boolVector isValid;
    isValid.resize(numPts, true);

    double coord[] = {0.0, 0.0, 0.0};

    //
    // In cases with multiple variables, only time states that 
    // are valid across ALL variables will be kept.
    //
    for (int c = 0; c < numCurves; ++c)
    {
        vtkFloatArray *inCurve = 
            (vtkFloatArray *) inPtData->GetArray(c);

        int ts = startT;
        for (int i = 0; i < numPts; ++i, ts += stride)
        {
            //
            // Invalid states will contain NaN values. 
            //
            if (visitIsNan(inCurve->GetTuple1(i)))
            {
                missingData = true;
 
                if (isValid[i])
                {
                    isValid[i]  = false;
                    invalidStateList.push_back(ts);
                }
            }
        }
    }

    //
    // If we found missing data, we have more work to do. Otherwise, 
    // we're done. 
    //
    if (missingData)
    {
        int numInvalid = invalidStateList.size();
        int numValid   = numPts - numInvalid;

        //
        // Report the missing timesteps. 
        //
        std::ostringstream osm;
        osm << "\nQueryOverTime (" << atts.GetQueryAtts().GetName().c_str()
            << ") experienced\n"
            << "problems with the following timesteps and \n"
            << "skipped them while generating the curve:\n   ";
        
        for (int j = 0; j < numInvalid; j++)
        {
            osm << invalidStateList[j] << " ";
        }
        debug4 << osm.str() << endl;
        avtCallback::IssueWarning(osm.str().c_str());

        if (numValid == 0)
        {
            //
            // We have no valid output. Let's not do any work here. 
            //
            outPolyData->Delete();
            vtkPolyData *empty = vtkPolyData::New();
            return empty;
        }

        //
        // Second pass: re-write the arrays so that they only contain
        // valid time states. 
        //
        vtkPointData *outPtData = outPolyData->GetPointData();

        for (int c = 0; c < numCurves; ++c) 
        {
            vtkFloatArray *inCurve = 
                (vtkFloatArray *) inPtData->GetArray(c);

            const char *name = inCurve->GetName();
        
            vtkFloatArray *outCurve = vtkFloatArray::New();
            outCurve->SetNumberOfTuples(numValid);
            outCurve->SetNumberOfComponents(1);
            outCurve->SetName(name);
        
            int vIdx = 0;
            for (int i = 0; i < numPts; ++i)
            {
                if (isValid[i])
                {
                    outCurve->SetTuple1(vIdx++, inCurve->GetTuple1(i));

                    if (c == 0) 
                    {
                        inPts->GetPoint(i, coord);        
                        outPts->InsertNextPoint(coord[0], coord[1], coord[2]);
                    }
                }
            }

            outPtData->RemoveArray(name);
            if (c == 0)
            {
                outPtData->SetScalars(outCurve);
            }
            else
            {
                outPtData->AddArray(outCurve);
            }

            outCurve->Delete();
        }

        outPolyData->SetPoints(outPts);
        outPts->Delete();
    }

    return outPolyData;
}


// ****************************************************************************
//  Method:  avtDirectDatabaseQOTFilter::ConstructCurveTree
//
//  Purpose:
//      Construct a tree from the time query curves. 
//
//  Arguments:
//      polyData             The polydata containing the curves. 
//      doMultiCurvePlot     Whether or not to do a multi curve plot. 
//
//  Returns:
//      A data tree containing the curves. 
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 11:15:10 MST 2019 
//
//  Modifications:
//
// ****************************************************************************

avtDataTree_p
avtDirectDatabaseQOTFilter::ConstructCurveTree(vtkPolyData *polyData,
                                               const bool doMultiCurvePlot)
{
    vtkPointData *inPtData = polyData->GetPointData();
    vtkPoints *inPts       = polyData->GetPoints();

    if (inPtData ==  NULL || inPts == NULL ||
        inPtData->GetScalars() == NULL)
    {
        debug1 << "avtDirectDatabaseQOTFilter: missing curves and/or points."
            << endl;
        avtDataTree_p tree = new avtDataTree(NULL, 0);
        return tree;
    }

    int numCurves = inPtData->GetNumberOfArrays();
    int numPts    = inPtData->GetScalars()->GetNumberOfTuples();

    if (numPts == 0 || numCurves == 0)
    {
        success = false;
        debug2 << "avtDirectDatabaseQOTFilter: missing curves and/or points" 
            << endl;
        avtDataTree_p tree = new avtDataTree(NULL, 0);
        return tree;
    }

    if (numCurves == 1)
    {
        vtkFloatArray *curve = 
            (vtkFloatArray *) inPtData->GetScalars();

        if (curve == NULL)
        {
            success = false;
            char msg[512];
            snprintf(msg, 512, "VisIt was unable to retreive data for the "
                "following variable: %s\n", curve->GetName());
            avtCallback::IssueWarning(msg);
            avtDataTree_p tree = new avtDataTree(NULL, 0);
            return tree;
        }

        vtkRectilinearGrid *rgrid = vtkVisItUtility::Create1DRGrid(numPts);
        rgrid->SetDimensions(numPts, 1 , 1);

        vtkDataArray *xCoords   = rgrid->GetXCoordinates();
        vtkDoubleArray *scalars = vtkDoubleArray::New();

        scalars->SetNumberOfComponents(1);
        scalars->SetNumberOfTuples(numPts);
        scalars->SetName(curve->GetName());

        double coord[] = {0.0, 0.0, 0.0};

        for (int i = 0; i < numPts; ++i)
        {
            inPts->GetPoint(i, coord);        
            xCoords->SetTuple1(i, coord[0]);
        }

        scalars->ShallowCopy(curve);
        rgrid->GetPointData()->SetScalars(scalars);

        avtDataTree_p tree = new avtDataTree(rgrid, 0);
        rgrid->Delete();
        scalars->Delete(); 
        return tree;
    }
    else if (doMultiCurvePlot)
    {
        vtkRectilinearGrid *rgrid =
            vtkVisItUtility::CreateEmptyRGrid(numPts, numCurves, 1, VTK_FLOAT);

        vtkDataArray *xCoords   = rgrid->GetXCoordinates();
        vtkDataArray *yCoords   = rgrid->GetYCoordinates();
        vtkDoubleArray *scalars = vtkDoubleArray::New();

        scalars->SetNumberOfComponents(1);
        scalars->SetNumberOfTuples(numPts * numCurves);

        rgrid->GetPointData()->SetScalars(scalars);

        double coord[] = {0.0, 0.0, 0.0};

        for (int i = 0; i < numPts; ++i)
        {
            inPts->GetPoint(i, coord);        
            xCoords->SetTuple1(i, coord[0]);
        }

        for (int i = 0; i < numCurves; i++)
        {
            vtkDoubleArray *curve = 
                (vtkDoubleArray *) inPtData->GetArray(i);

            if (curve == NULL)
            {
                char msg[512];
                snprintf(msg, 512, "VisIt was unable to retreive data for the "
                    "following variable: %s\n", curve->GetName()); 
                continue;
            }

            yCoords->SetTuple1(i, i);

            int baseIdx = i*numPts;

            for (int j = 0; j < numPts; j++)
            {
                scalars->SetTuple1(baseIdx + j, curve->GetTuple1(j));
            }
        }

        avtDataTree_p tree = new avtDataTree(rgrid, 0);
        scalars->Delete();
        rgrid->Delete();
        return tree;
    }
    else
    {
        stringVector vars;
        vars.reserve(numCurves);

        vtkDataSet **grids = new vtkDataSet *[numCurves];

        for (int i = 0; i< numCurves; ++i)
        {
            vtkDoubleArray *curve = 
                (vtkDoubleArray *) inPtData->GetArray(i);

            if (curve == NULL)
            {
                char msg[512];
                snprintf(msg, 512, "VisIt was unable to retreive data for the "
                    "following variable: %s\n", curve->GetName());
                continue;
            }

            vars.push_back(curve->GetName());

            grids[i] = vtkVisItUtility::Create1DRGrid(numPts, VTK_FLOAT);

            vtkDataArray *xCoords   = ((vtkRectilinearGrid*)grids[i])->GetXCoordinates();
            vtkDoubleArray *scalars = vtkDoubleArray::New();

            scalars->SetNumberOfComponents(1);
            scalars->SetNumberOfTuples(numPts);
            scalars->SetName(vars[i].c_str());
          
            scalars->ShallowCopy(curve);
            grids[i]->GetPointData()->SetScalars(scalars);

            double coord[] = {0.0, 0.0, 0.0};

            for (int i = 0; i < numPts; ++i)
            {
                inPts->GetPoint(i, coord);        
                xCoords->SetTuple1(i, coord[0]);
            }
        }

        avtDataTree_p tree = new avtDataTree(numCurves, grids, -1, vars);

        for (int i = 0; i< numCurves; ++i)
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


// ****************************************************************************
//  Method:  avtDirectDatabaseQOTFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Update the attributes and validity. 
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 11:15:10 MST 2019 
//
//  Modifications:
//
// ****************************************************************************

void
avtDirectDatabaseQOTFilter::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();

    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetTopologicalDimension(1);
    outAtts.GetOriginalSpatialExtents()->Clear();
    outAtts.GetDesiredSpatialExtents()->Clear();

    if (finalOutputCreated)
    {
        stringVector qotVars = atts.GetQueryAtts().GetVariables();

        if (qotVars.size() > 0)
        {
            //
            // The first requested variable becomes the active one. 
            //
            outAtts.SetActiveVariable(qotVars[0].c_str());
        }

        if (useTimeForXAxis)
        {
            outAtts.SetXLabel("Time");
            outAtts.SetYLabel(YLabel);

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
        else if (qotVars.size() >= 2)
        {
            std::string xl = qotVars[0] + "(t)";
            std::string yl = qotVars[1] + "(t)";
            outAtts.SetXLabel(xl);
            outAtts.SetYLabel(yl);
            outAtts.SetXUnits(atts.GetQueryAtts().GetXUnits());
            outAtts.SetYUnits(atts.GetQueryAtts().GetYUnits());
        }

        outAtts.SetLabels(qotVars);

        if (qotVars.size() > 1)
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
}
