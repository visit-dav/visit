// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtLineSamplerInfoQuery.C                       //
// ************************************************************************* //

#include <avtLineSamplerInfoQuery.h>

#include <visit-config.h> // For LIB_VERSION_GE
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#if LIB_VERSION_GE(VTK,9,1,0)
#include <vtkCellArrayIterator.h>
#endif
#include <avtDatasetExaminer.h>
#include <avtParallel.h>

#include <NonQueryableInputException.h>
#ifdef PARALLEL
#include <mpi.h>
#endif

#include <string>
#include <vector>


// ****************************************************************************
//  Method: avtLineSamplerInfoQuery constructor
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
//  Modifications:
//
// ****************************************************************************

avtLineSamplerInfoQuery::avtLineSamplerInfoQuery() : avtDatasetQuery()
{
    dumpCoordinates = false;
    dumpValues = false;
}

// ****************************************************************************
// Method:  avtLineSamplerInfoQuery destructor
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
//  Modifications:
//
// ****************************************************************************

avtLineSamplerInfoQuery::~avtLineSamplerInfoQuery()
{
}


// ****************************************************************************
// Method:  avtLineSamplerInfoQuery::SetInputParams
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
avtLineSamplerInfoQuery::SetInputParams(const MapNode &params)
{
    if (params.HasNumericEntry("dump_coordinates"))
        SetDumpCoordinates(params.GetEntry("dump_coordinates")->ToBool());
    if (params.HasNumericEntry("dump_values"))
        SetDumpValues(params.GetEntry("dump_values")->ToBool());
}

// ****************************************************************************
// Method:  avtLineSamplerInfoQuery::GetDefaultInputParams
//
// Programmer:  Kathleen Biagas
// Creation:    July 15, 2011
//
// ****************************************************************************

void
avtLineSamplerInfoQuery::GetDefaultInputParams(MapNode &params)
{
    params["dump_coordinates"] = 0;
    params["dump_values"] = 0;
}

// ****************************************************************************
// Method:  avtLineSamplerInfoQuery::VerifyInput
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
// ****************************************************************************

void
avtLineSamplerInfoQuery::VerifyInput()
{
    avtDatasetQuery::VerifyInput();
}

// ****************************************************************************
// Method:  avtLineSamplerInfoQuery::PreExecute()
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtLineSamplerInfoQuery::PreExecute()
{
    avtDatasetQuery::PreExecute();
    lsData.resize(0);
}

// ****************************************************************************
// Method:  avtLineSamplerInfoQuery::PostExecute()
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtLineSamplerInfoQuery::PostExecute()
{
    //Everyone communicate data to proc 0.
#ifdef PARALLEL
    int nProcs = PAR_Size();
    int *counts = new int[nProcs];
    for (int i = 0; i < nProcs; i++)
        counts[i] = 0;

    counts[PAR_Rank()] = lsData.size();
    Collect(counts, nProcs);

    int tag = GetUniqueMessageTag();
    MPI_Status stat;
    if (PAR_Rank() == 0)
    {
        for (int i = 1; i < nProcs; i++)
        {
            if (counts[i] > 0)
            {
                float *vals = new float[counts[i]];
                void *ptr = (void *)&vals[0];
                MPI_Recv(ptr, counts[i], MPI_FLOAT, i, tag, VISIT_MPI_COMM, &stat);

                for (int j = 0; j < counts[i]; j++)
                    lsData.push_back(vals[j]);
                delete [] vals;
            }
        }
    }
    else
    {
        if (lsData.size() > 0)
        {
            void *ptr = (void *)&lsData[0];
            MPI_Send(ptr, lsData.size(), MPI_FLOAT, 0, tag, VISIT_MPI_COMM);
        }
    }
    delete [] counts;
#endif

    std::string msg;
    char str[128];
    size_t i = 0, sz = lsData.size();

    int lsIdx = 0;
    while (i < sz)
    {
        float cordLength = lsData[i++];
        unsigned int nSamples = (int) lsData[i++];

        if( cordLength > 0 )
        {
          sprintf( str, "LineSample %d:  Number of samples %d  ",
                   lsIdx, nSamples );
          msg += str;

          if( dumpCoordinates )
            sprintf(str, "Cord length %f  \n", cordLength );
          else
            sprintf(str, "\n" );
        }
        else
          sprintf(str, "LineSample vertices:  Number of vertices %d \n",
                  nSamples );

        msg += str;

        if (dumpCoordinates || dumpValues)
        {
            for (unsigned int j=0; j<nSamples; j++)
            {
                if (dumpCoordinates)
                {
                  sprintf(str, "%f %f %f",
                          lsData[i], lsData[i+1], lsData[i+2]);
                  i += 3;
                  msg += str;
                }

                if (dumpValues)
                {
                  sprintf(str, "    %f", lsData[i] );
                  i += 1;
                  msg += str;
                }

                sprintf(str, "\n" );
                msg += str;
            }
        }

        lsIdx++;
    }

    SetResultMessage(msg.c_str());
}

// ****************************************************************************
// Method:  avtLineSamplerInfoQuery::Execute()
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
avtLineSamplerInfoQuery::Execute(vtkDataSet *data, const int chunk)
{
    if (!data->IsA("vtkPolyData") )
    {
        EXCEPTION1(NonQueryableInputException,"LineSampler Info query only valid on line sampler operations");
    }

    vtkPolyData *ds = (vtkPolyData *) data;
    vtkPoints *points = ds->GetPoints();
    float *scalar =
      (float *) data->GetPointData()->GetScalars()->GetVoidPointer(0);

    double pt[3] = {0,0,0};
    double p0[3] = {0,0,0};

    if ( ds->GetNumberOfVerts() > 0 )
    {
      std::vector<float> steps;
      vtkIdType nPts; // = 1 for a vertex
#if LIB_VERSION_LE(VTK,8,1,0)
      vtkCellArray *vertices = ds->GetVerts();
      vtkIdType *verts = vertices->GetPointer();
      vtkIdType *vertPtr = verts;

      for (int i=0; i<ds->GetNumberOfVerts(); i++)
      {
        nPts = *vertPtr;
        vertPtr++; //Now segptr points at vtx0.
#else
      auto verts = vtk::TakeSmartPointer(ds->GetVerts()->NewIterator());
      for (verts->GoToFirstCell(); !verts->IsDoneWithTraversal(); verts->GoToNextCell());
      {
        const vtkIdType *vertPtr;
        verts->GetCurrentCell(nPts, vertPtr);
#endif
        points->GetPoint(vertPtr[0], pt);

        if (dumpCoordinates)
        {
          steps.push_back(pt[0]);
          steps.push_back(pt[1]);
          steps.push_back(pt[2]);
        }

        if (dumpValues)
        {
          float s = scalar[vertPtr[0]];
          steps.push_back(s);
        }
#if LIB_VERSION_LE(VTK,8,1,0)
        vertPtr += nPts;
#endif
      }

      lsData.push_back(0.0); // No cord length as individual points

      if (dumpCoordinates || dumpValues)
      {
        lsData.push_back( (float) (ds->GetNumberOfVerts()) );
        lsData.insert( lsData.end(), steps.begin(), steps.end() );
      }
    }


    if (ds->GetNumberOfLines() > 0)
    {
      vtkIdType nPts;
#if LIB_VERSION_LE(VTK,8,1,0)
      vtkCellArray *lines = ds->GetLines();
      vtkIdType *segments = lines->GetPointer();
      vtkIdType *segPtr = segments;

      for (int i=0; i<ds->GetNumberOfLines(); i++)
      {
        nPts = *segPtr;
        segPtr++; //Now segptr points at vtx0.
#else
      auto lines = vtk::TakeSmartPointer(ds->GetLines()->NewIterator());
      for (lines->GoToFirstCell(); !lines->IsDoneWithTraversal(); lines->GoToNextCell());
      {
        const vtkIdType *segPtr;
        lines->GetCurrentCell(nPts, segPtr);
#endif
        float cordLength = 0.0;
        std::vector<float> steps;

        for (vtkIdType j = 0; j < nPts; j++)
        {
            points->GetPoint(segPtr[j], pt);

            if (dumpCoordinates)
            {
                steps.push_back(pt[0]);
                steps.push_back(pt[1]);
                steps.push_back(pt[2]);
            }

            if (dumpValues)
            {
                float s = scalar[segPtr[j]];
                steps.push_back(s);
            }

            if( j > 0 )
            {
              double x = p0[0]-pt[0];
              double y = p0[1]-pt[1];
              double z = p0[2]-pt[2];

              cordLength += sqrt(x*x + y*y + z*z);
            }

            p0[0] = pt[0];
            p0[1] = pt[1];
            p0[2] = pt[2];
        }

        lsData.push_back(cordLength);

        if (dumpCoordinates || dumpValues)
        {
            lsData.push_back( (float) (nPts) );
            lsData.insert( lsData.end(), steps.begin(), steps.end() );
        }
#if LIB_VERSION_LE(VTK,8,1,0)
        segPtr += nPts;
#endif
      }
    }
}
