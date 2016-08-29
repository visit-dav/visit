/*****************************************************************************
 *
 * Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
//                         avtGyRadiusQuery.C                                //
// ************************************************************************* //
#include <avtGyRadiusQuery.h>
#include <avtQueryableSource.h>
#include <avtOriginatingSource.h>
#include <avtSILRestrictionTraverser.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtCentroidQuery.h>
#include <avtSummationQuery.h>

#include <vtkDataSet.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkUnsignedIntArray.h>
#include <vtkVisItUtility.h>

#include <ImproperUseException.h>
#include <QueryArgumentException.h>
#include <BadCellException.h>
#include <DebugStream.h>

using std::string;


// ****************************************************************************
//  Method: avtGyRadiusQuery constructor
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jul 20 16:18:51 PDT 2016
//
//  Modifications:
//
// ****************************************************************************
avtGyRadiusQuery::avtGyRadiusQuery()
{
    centroid.resize(3,0);
    overrideCentroid = false;
    totalSum = 0;
    totalMass = 0;
    isZonal = true;
    
    eef = new avtExpressionEvaluatorFilter;
}

// ****************************************************************************
//  Method: avtGyRadiusQuery destructor
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jul 20 16:18:51 PDT 2016
//
//  Modifications:
//
// ****************************************************************************
avtGyRadiusQuery::~avtGyRadiusQuery()
{
    if(eef != NULL)
    {
        delete eef;
        eef = NULL;
    }
}

// ****************************************************************************
//  Method: avtGyRadiusQuery::SetInputParams
//
//  Purpose:    Allows this query to read input parameters set by user.
//
//  Arguments:
//    params:   MapNode containing input.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jul 20 16:18:51 PDT 2016
//
//  Modifications:
//
// ****************************************************************************
void
avtGyRadiusQuery::SetInputParams(const MapNode &params)
{
    // Mass Variable
    if(params.HasEntry("vars"))
    {
        stringVector v = params.GetEntry("vars")->AsStringVector();
        
        if(v.empty())
        {
            EXCEPTION2(QueryArgumentException, "vars", 1);
        }
        
        queryAtts.SetVariables(v);
    }
    else
    {
        EXCEPTION1(QueryArgumentException, "vars");
    }
    
    // Center of Mass
    if(params.HasNumericVectorEntry("centroid"))
    {
        overrideCentroid = true;
        
        doubleVector cvals;
        params.GetEntry("centroid")->ToDoubleVector(cvals);
        
        centroid[0] = cvals[0];
        centroid[1] = cvals[1];
        centroid[2] = cvals[2];
    }
    else
    {
        overrideCentroid = false;
    }
}

// ****************************************************************************
//  Method: avtGyRadiusQuery::GetDefaultInputParams
//
//  Purpose:    This method gets called directly from the avtQueryFactory to
//              help set up the default arguments for your new query. Fill in
//              any values in the MapNode that you want your query to use
//              when it gets initialized.
//
//  Arguments:
//    params:   MapNode to store the default values in.
//
//  Programmer: Kevin Griffin
//  Creation:   Mon Jul 25 16:28:18 PDT 2016
//
//  Modifications:
//
// ****************************************************************************
void
avtGyRadiusQuery::GetDefaultInputParams(MapNode &params)
{
    stringVector v;
    v.push_back("default");
    params["vars"] = v;
    
    params["centroid"] = "default";
}

// ****************************************************************************
//  Method: avtGyRadiusQuery::PreExecute
//
//  Purpose:    Does some initialization work before the query executes.
//
//  Programmer: Kevin Griffin
//  Creation:   Mon Jul 25 16:28:18 PDT 2016
//
// ****************************************************************************
void
avtGyRadiusQuery::PreExecute(void)
{
    totalSum = 0;
    totalMass = 0;
    varName = queryAtts.GetVariables()[0];
    
    if(!overrideCentroid)
    {
        QueryAttributes qa;
        avtCentroidQuery centroidQuery;
        centroidQuery.SetInput(GetInput());
        centroidQuery.PerformQuery(&qa);
        
        doubleVector results = qa.GetResultsValue();
        centroid[0] = results[0];
        centroid[1] = results[1];
        centroid[2] = results[2];
    }
    
    // Get the denominator value (total mass)
//    avtSummationQuery sumQuery;
//    sumQuery.SetInput(GetInput());
//    sumQuery.SetVariableName(varName);
//    sumQuery.SumGhostValues(false);
//    
//    QueryAttributes qa;
//    sumQuery.PerformQuery(&qa);
//    totalMass = qa.GetResultsValue()[0];
}

// ****************************************************************************
//  Method: avtGyRadiusQuery::PostExecute
//
//  Purpose:    Outputs the radius value.
//
//  Programmer: Kevin Griffin
//  Creation:   Mon Jul 25 16:28:18 PDT 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtGyRadiusQuery::PostExecute(void)
{
    SumDoubleAcrossAllProcessors(totalSum);
    SumDoubleAcrossAllProcessors(totalMass);
    
    if(PAR_Rank() == 0)
    {
        // Calculate the radius
        double R = sqrt(totalSum / totalMass);
        
        // Set the query output
        char charBuf[256];
        if(isZonal)
        {
            SNPRINTF(charBuf, 256, "R = %f\n", R);
        }
        else
        {
            SNPRINTF(charBuf, 256, "**Warning** Results are node-based since %s wasn't defined for cells.\nR = %f\n", varName.c_str(), R);
        }
        
        string msg(charBuf);
        SetResultMessage(msg);
        SetResultValue(R);
    }
}

// ****************************************************************************
//  Method: avtGyRadiusQuery::ApplyFilters
//
//  Purpose:    Retrieves the terminating source to use as input.
//
//  Programmer: Kevin Griffin
//  Creation:   Fri Jul 22 09:54:08 PDT 2016
//
//  Modifications:
//
// ****************************************************************************
avtDataObject_p
avtGyRadiusQuery::ApplyFilters(avtDataObject_p inData)
{
    avtDataObject_p ds;
    CopyTo(ds, inData);
    avtDataRequest_p dataRequest = new avtDataRequest(inData->GetOriginatingSource()->GetGeneralContract()->GetDataRequest(), querySILR);
    
    bool requiresUpdate = false;
    string var = queryAtts.GetVariables()[0];
    
    if(dataRequest->GetVariable() != var)
    {
        if (!dataRequest->HasSecondaryVariable(var.c_str()))
        {
            dataRequest->AddSecondaryVariable(var.c_str());
            requiresUpdate = true;
        }
    }
    
    requiresUpdate = (bool)UnifyMaximumValue((int)requiresUpdate);
    
    if (!requiresUpdate)
    {
        return inData;
    }
    
    avtContract_p contract = new avtContract(dataRequest, queryAtts.GetPipeIndex());
    eef->SetInput(ds);
    avtDataObject_p retObj = eef->GetOutput();
    retObj->Update(contract);
    return retObj;
}

// ****************************************************************************
//  Method: avtGyRadiusQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Arguments:
//      ds  the dataset
//      dom the domain
//
//
//  Programmer: Kevin Griffin
//  Creation:   Fri Jul 22 09:54:08 PDT 2016
//
//  Modifications:
//    Kevin Griffin, Mon Aug 15 18:13:40 PDT 2016
//    Removed the assignment of totalSum to zero since this method can be
//    called multiple times for datasets with multiple domains. Setting
//    totalSum to zero at the beginning of the method essentially wiped out
//    all the results for all domains except the last which is not desired.
//  
// ****************************************************************************

void
avtGyRadiusQuery::Execute(vtkDataSet *ds, const int dom)
{
    double Ri[3] = {0, 0, 0};
    vtkDataArray *ghosts = ds->GetCellData()->GetArray("avtGhostZones");
    vtkDataArray *var = ds->GetCellData()->GetArray(varName.c_str());
    
    if(var != NULL)
    {
        isZonal = true;
        int nCells = ds->GetNumberOfCells();
        
        for(int i=0; i<nCells; i++)
        {
            if(ghosts != NULL && ghosts->GetTuple1(i) != 0)
            {
                continue;
            }
            
            vtkCell *cell = ds->GetCell(i);
            vtkVisItUtility::GetCellCenter(cell, Ri);
            
            double mass = var->GetTuple1(i);
            double radius = sqrt(pow((Ri[0] - centroid[0]), 2.0) + pow((Ri[1] - centroid[1]), 2.0) + pow((Ri[2] - centroid[2]), 2.0));
            totalSum += mass * (radius * radius);
            totalMass += mass;
        }
    }
    else
    {
        isZonal = false;
        var = ds->GetPointData()->GetArray(varName.c_str());
        
        if(var != NULL)
        {
            int nPoints = ds->GetNumberOfPoints();
            ghosts = ds->GetPointData()->GetArray("avtGhostNodes");
            
            for(int i=0; i<nPoints; i++)
            {
                if(ghosts != NULL && ghosts->GetTuple1(i) != 0)
                {
                    continue;
                }
                
                ds->GetPoint(i, Ri);
                double mass = var->GetTuple1(i);
                double dist = sqrt(pow((Ri[0] - centroid[0]), 2.0) + pow((Ri[1] - centroid[1]), 2.0) + pow((Ri[2] - centroid[2]), 2.0));
                totalSum += mass * dist * dist;
                totalMass += mass;
            }
        }
        else
        {
            string errMsg;
            errMsg.append(varName);
            errMsg.append(" is not defined for this dataset");
            EXCEPTION1(ImproperUseException, errMsg);
        }
    }
}
