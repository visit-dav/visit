// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtIntegralCurveInfoQuery.C                    //
// ************************************************************************* //

#include <avtIntegralCurveInfoQuery.h>

#include <avtDatasetExaminer.h>
#include <avtParallel.h>

#include <vtkCellArray.h>
#include <vtkCellArrayIterator.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

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
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK9: use vtkCellArrayIterator.
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
    vtkDoubleArray *scalar =
      (vtkDoubleArray *) data->GetPointData()->GetArray("colorVar");
    double pt0[3], pt1[3];
    if (ds->GetNumberOfLines() > 0)
    {
      vtkIdType nLines = ds->GetNumberOfLines();
      vtkIdType nPts;
      auto lines = vtk::TakeSmartPointer(ds->GetLines()->NewIterator());
      for (lines->GoToFirstCell(); !lines->IsDoneWithTraversal(); lines->GoToNextCell())
      {
        const vtkIdType *segptr;
        lines->GetCurrentCell(nPts, segptr);
        double arcLength = 0.0;
        std::vector<double> steps;

        // Push the seed point on to the stack.
        points->GetPoint(segptr[0], pt0);
        slData.push_back(pt0[0]);
        slData.push_back(pt0[1]);
        slData.push_back(pt0[2]);

        for (vtkIdType j=0; j<nPts; ++j)
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
      }
    }
}
