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

#include <float.h>
#include <snprintf.h>
#include <sstream>
#include <string>
#include <vector>

#include <avtDataObjectQuery.h>
#include <avtQueryFactory.h>
#include <avtDirectDatabaseQOTFilter.h>
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
#include <vtkPolyData.h>

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

avtDirectDatabaseQOTFilter::avtDirectDatabaseQOTFilter(const AttributeGroup *a)
 : avtQueryOverTimeFilter(a)
{
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
//  Method: avtDirectDatabaseQOTFilter destructor
//
//  Programmer: 
//  Creation:   
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
//  Programmer:  
//  Creation:    
//
// ****************************************************************************

avtFilter *
avtDirectDatabaseQOTFilter::Create(const AttributeGroup *atts)
{
    return new avtDirectDatabaseQOTFilter(atts);
}



void
avtDirectDatabaseQOTFilter::Execute(void)
{
    avtDataTree_p dataTree = GetInputDataTree();

    if (PAR_Rank() == 0)
    {
        stringVector vars = atts.GetQueryAtts().GetVariables();
        bool multiCurve = false;
        if (atts.GetQueryAtts().GetQueryInputParams().HasNumericEntry("curve_plot_type"))
        {
            multiCurve = (atts.GetQueryAtts().GetQueryInputParams().
                GetEntry("curve_plot_type")->ToInt() == 1);
        }

        int numLeaves = 0;
        vtkDataSet **leaves  = dataTree->GetAllLeaves(numLeaves);
        vtkPolyData *QOTData = (vtkPolyData *) leaves[0];

        avtDataTree_p tree   = CreateTreeFromCurves(QOTData, vars, multiCurve);
        SetOutputDataTree(tree);
    }
    else
    {
        SetOutputDataTree(new avtDataTree());
    }
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
avtDirectDatabaseQOTFilter::UpdateDataObjectInfo(void)
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
//  Method: 
//
//  Purpose:
//
//  Arguments:
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
avtDirectDatabaseQOTFilter::CreateTreeFromCurves(vtkPolyData *polyData,
                                                 stringVector &vars,
                                                 const bool doMultiCurvePlot)
{

    vtkPointData *inputPD = polyData->GetPointData();
    vtkPoints *inputPts   = polyData->GetPoints();
    numCurves             = inputPD->GetNumberOfArrays();
    int numPts            = inputPD->GetScalars()->GetNumberOfTuples();

    if (numPts == 0)
    {
        avtDataTree_p tree = new avtDataTree(NULL, 0);
        return tree;
    }
    else if (vars.size() != (size_t)numCurves)
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

    if (numCurves == 1)
    {
        vtkRectilinearGrid *rgrid = vtkVisItUtility::Create1DRGrid(numPts);
        rgrid->SetDimensions(numPts, 1 , 1);

        vtkDataArray *xCoords   = rgrid->GetXCoordinates();
        vtkDoubleArray *scalars = vtkDoubleArray::New();

        scalars->SetNumberOfComponents(1);
        scalars->SetNumberOfTuples(numPts);

        vtkFloatArray *curve = 
            (vtkFloatArray *) inputPD->GetScalars();

        if (curve == NULL)
        {
            cerr << "CURVE IS NULL" << endl;//FIXME
        }

        double coord[] = {0.0, 0.0, 0.0};

        for (int i = 0; i < numPts; ++i)
        {
            inputPts->GetPoint(i, coord);        
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
        //TODO: test this. 
        cerr << "MULTI CURVE" << endl;//FIXME

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
            inputPts->GetPoint(i, coord);        
            xCoords->SetTuple1(i, coord[0]);
        }

        for (int i = 0; i < numCurves; i++)
        {
            vtkDoubleArray *curve = 
                (vtkDoubleArray *) inputPD->GetArray(i);

            yCoords->SetTuple1(i, i);

            int baseIdx = i*numPts;

            for (int j = 0; j < numPts; j++)
            {
                scalars->SetTuple1(baseIdx + j, curve->GetTuple1(j));
            }
        }
        avtDataTree_p tree = new avtDataTree(rgrid, 0);
        rgrid->Delete();
        scalars->Delete();
        return tree;
    }
    else
    {
        vtkDataSet **grids = new vtkDataSet *[numCurves];

        for (int i = 0; i< numCurves; ++i)
        {
            grids[i] = vtkVisItUtility::Create1DRGrid(numPts, VTK_FLOAT);

            vtkDataArray *xCoords   = ((vtkRectilinearGrid*)grids[i])->GetXCoordinates();
            vtkDoubleArray *scalars = vtkDoubleArray::New();

            scalars->SetNumberOfComponents(1);
            scalars->SetNumberOfTuples(numPts);
            scalars->SetName(vars[i].c_str());

            vtkDoubleArray *curve = 
                (vtkDoubleArray *) inputPD->GetArray(i);
          
            scalars->ShallowCopy(curve);
            grids[i]->GetPointData()->SetScalars(scalars);

            double coord[] = {0.0, 0.0, 0.0};

            for (int i = 0; i < numPts; ++i)
            {
                inputPts->GetPoint(i, coord);        
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

