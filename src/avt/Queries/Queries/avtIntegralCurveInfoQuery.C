/*****************************************************************************
*
* Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
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
//                            avtIntegralCurveInfoQuery.C                    //
// ************************************************************************* //

#include <avtIntegralCurveInfoQuery.h>
#include <avtDatasetExaminer.h>
#include <avtParallel.h>
#include <snprintf.h>
#include <vtkDoubleArray.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <NonQueryableInputException.h>
#ifdef PARALLEL
#include <mpi.h>
#endif

#include <string>
#include <vector>


// ****************************************************************************
//  Method: avtIntegralCurveInfoQuery constructor
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
//  Modifications:
//
// ****************************************************************************
avtIntegralCurveInfoQuery::avtIntegralCurveInfoQuery() : avtDatasetQuery() 
{
    dumpIndex = false;
    dumpCoordinates = false;
    dumpArcLength = false;
    dumpValues = false;
}

// ****************************************************************************
// Method:  avtIntegralCurveInfoQuery destructor
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
//  Modifications:
//
// ****************************************************************************
avtIntegralCurveInfoQuery::~avtIntegralCurveInfoQuery() 
{
}

// ****************************************************************************
// Method:  avtIntegralCurveInfoQuery::SetInputParams
//
// Programmer:  Kathleen Biagas 
// Creation:    June 17, 2011
//
//  Modifications:
//    Kathleen Biagas, Thu Jan 10 08:12:47 PST 2013
//    Use newer MapNode methods that check for numeric entries and retrieves 
//    to specific type.
//
// ****************************************************************************
void
avtIntegralCurveInfoQuery::SetInputParams(const MapNode &params)
{
    if (params.HasNumericEntry("dump_index"))
        SetDumpIndex(params.GetEntry("dump_index")->ToBool());

    if (params.HasNumericEntry("dump_coordinates"))
        SetDumpCoordinates(params.GetEntry("dump_coordinates")->ToBool());

    if (params.HasNumericEntry("dump_arclength"))
        SetDumpArcLength(params.GetEntry("dump_arclength")->ToBool());

    if (params.HasNumericEntry("dump_values"))
        SetDumpValues(params.GetEntry("dump_values")->ToBool());
}

// ****************************************************************************
// Method:  avtIntegralCurveInfoQuery::GetDefaultInputParams
//
// Programmer:  Kathleen Biagas 
// Creation:    July 15, 2011
//
// ****************************************************************************
void
avtIntegralCurveInfoQuery::GetDefaultInputParams(MapNode &params)
{
    params["dump_opts"] = 0;
    params["dump_values"] = 0;
}

// ****************************************************************************
// Method:  avtIntegralCurveInfoQuery::VerifyInput
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
// ****************************************************************************
void
avtIntegralCurveInfoQuery::VerifyInput()
{
    avtDatasetQuery::VerifyInput();
}

// ****************************************************************************
// Method:  avtIntegralCurveInfoQuery::PreExecute()
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
//  Modifications:
//
// ****************************************************************************
void
avtIntegralCurveInfoQuery::PreExecute()
{
    avtDatasetQuery::PreExecute();
    slData.resize(0);
}

// ****************************************************************************
// Method:  avtIntegralCurveInfoQuery::PostExecute()
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
//  Modifications:
//    Kathleen Biagas, Wed Feb 26 10:22:56 PST 2014
//    Add XML results.
//
// ****************************************************************************
void
avtIntegralCurveInfoQuery::PostExecute()
{
    // Collect all data to proc 0.
#ifdef PARALLEL
    int nProcs = PAR_Size();
    int *counts = new int[nProcs];
    
    for (unsigned int i = 0; i < nProcs; i++)
        counts[i] = 0;
    
    counts[PAR_Rank()] = slData.size();
    Collect(counts, nProcs);
    
    int tag = GetUniqueMessageTag();
    MPI_Status stat;

    if (PAR_Rank() == 0)
    {
        for (int i=1; i<nProcs; ++i)
        {
            if (counts[i] > 0)
            {
                double *vals = new double[counts[i]];
                void *ptr = (void *) &vals[0];

                MPI_Recv(ptr, counts[i], MPI_DOUBLE, i, tag, VISIT_MPI_COMM, &stat);

                for (int j=0; j<counts[i]; ++j)
                    slData.push_back(vals[j]);
                
                delete [] vals;
            }
        }
    }
    else
    {
        if (slData.size() > 0)
        {
            void *ptr = (void *) &slData[0];
            MPI_Send(ptr, slData.size(), MPI_DOUBLE, 0, tag, VISIT_MPI_COMM);
        }
    }

    delete [] counts;
#endif
    
    std::string msg;
    char str[128];
    unsigned int i = 0, sz = slData.size();

    int slIdx = 0;
    MapNode result_node;
    while (i < sz)
    {
        MapNode sl_res_node;
        doubleVector sl_res_seed;

        double x = slData[i++], y = slData[i++], z = slData[i++];
        double value, arcLength = slData[i++];

        sl_res_seed.push_back(x);
        sl_res_seed.push_back(y);
        sl_res_seed.push_back(z);
        sl_res_node["seed"] = sl_res_seed;
        sl_res_node["arcLength"] = arcLength;
        sprintf(str, "IntegralCurve %d: Seed %lf %lf %lf, Arclength %lf",
                slIdx, x, y, z, arcLength );
        msg += str;

        if (dumpIndex || dumpCoordinates || dumpArcLength || dumpValues)
        {
            unsigned int numSteps = (unsigned int) slData[i++];
            sl_res_node["numSteps"] = (int) numSteps;

            sprintf(str, ", Steps %d\n", numSteps);
            msg += str;
            
            doubleVector sl_steps;

            for (unsigned int j = 0; j < numSteps; j++)
            {
              str[0] = '\0';
              
              if (dumpIndex) // Index
              {
                sl_steps.push_back(j);
                sprintf(str, "%s  %i", str, j);
              }

              if (dumpCoordinates) // Coordinates
              {
                x = slData[i++]; y = slData[i++]; z = slData[i++];

                sl_steps.push_back(x);
                sl_steps.push_back(y);
                sl_steps.push_back(z);
                sprintf(str, "%s  %lf %lf %lf", str, x, y, z);
              }

              if (dumpArcLength) // Arc Length
              {
                arcLength = slData[i++];

                sl_steps.push_back(arcLength);
                sprintf(str, "%s  %lf", str, arcLength);
              }
                
              if (dumpValues) // Value
              {
                value = slData[i++];

                sl_steps.push_back(value);
                sprintf(str, "%s  %lf", str, value);
              }
              
              sprintf(str, "%s\n", str);
              msg += str;
            }

            sl_res_node["steps"] = sl_steps;

            sprintf(str, "\n");
            msg += str;
        }
        else
        {
          sprintf(str, "\n");
          msg += str;
        }
        
        sprintf(str, "IntegralCurve %d", slIdx);
        result_node[str] = sl_res_node;
        ++slIdx;
    }

    SetResultMessage(msg.c_str());
    SetXmlResult(result_node.ToXML());
}

// ****************************************************************************
// Method:  avtIntegralCurveInfoQuery::Execute()
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
//  Modifications:
//
// ****************************************************************************
void
avtIntegralCurveInfoQuery::Execute(vtkDataSet *data, const int chunk)
{
    if (!data->IsA("vtkPolyData") ||
        data->GetPointData()->GetArray("colorVar") == NULL)
    {
        EXCEPTION1(NonQueryableInputException,
                   "Integral Curve Info query only valid on integral curve operator data");
    }

    vtkPolyData *ds = (vtkPolyData *) data;
    vtkPoints *points   = ds->GetPoints();
    vtkCellArray *lines = ds->GetLines();
    vtkIdType *segptr = lines->GetPointer();
    vtkDoubleArray *scalar =
      (vtkDoubleArray *) data->GetPointData()->GetArray("colorVar");

    unsigned int nLines = ds->GetNumberOfLines();
    double pt0[3], pt1[3];
    
    for (unsigned int i=0; i<nLines; ++i)
    {
        unsigned int nPts = *segptr;
        ++segptr;  // Segptr now points to the first vertex index.

        double arcLength = 0.0;
        std::vector<double> steps;
        
        // Push the seed point on to the stack.
        points->GetPoint(segptr[0], pt0);
        slData.push_back(pt0[0]);
        slData.push_back(pt0[1]);
        slData.push_back(pt0[2]);

        for (unsigned int j=0; j<nPts; ++j)
        {
            points->GetPoint(segptr[j], pt1);
            arcLength += (avtVector( pt1 ) - avtVector( pt0 )).length();

            if (dumpCoordinates)
            {
                steps.push_back(pt1[0]);
                steps.push_back(pt1[1]);
                steps.push_back(pt1[2]);
            }

            if (dumpArcLength)
            {
                steps.push_back(arcLength);
            }

            if (dumpValues)
            {
                double val = scalar->GetTuple1(segptr[j]);
                steps.push_back(val);
            }

            pt0[0] = pt1[0];
            pt0[1] = pt1[1];
            pt0[2] = pt1[2];
        }

        // Push the arc length on to the stack.
        slData.push_back(arcLength);

        // Now push the optional values on to the stack.
        if (dumpIndex || dumpCoordinates || dumpArcLength || dumpValues)
        {
            slData.push_back((double)(nPts));
            slData.insert(slData.end(), steps.begin(), steps.end());
        }

        segptr += nPts;
    }
}
