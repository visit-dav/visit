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
//                            avtStreamlineInfoQuery.C                       //
// ************************************************************************* //

#include <avtStreamlineInfoQuery.h>
#include <avtDatasetExaminer.h>
#include <avtParallel.h>
#include <snprintf.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <string>
#include <NonQueryableInputException.h>
#ifdef PARALLEL
#include <mpi.h>
#endif


// ****************************************************************************
//  Method: avtStreamlineInfoQuery constructor
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
//  Modifications:
//
// ****************************************************************************

avtStreamlineInfoQuery::avtStreamlineInfoQuery() : avtDatasetQuery() 
{
    dumpSteps = false;
}

// ****************************************************************************
// Method:  avtStreamlineInfoQuery destructor
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
//  Modifications:
//
// ****************************************************************************

avtStreamlineInfoQuery::~avtStreamlineInfoQuery() 
{
}

// ****************************************************************************
// Method:  avtMomentOfInertiaQuery::VerifyInput
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
// ****************************************************************************

void
avtStreamlineInfoQuery::VerifyInput()
{
    avtDatasetQuery::VerifyInput();
}

// ****************************************************************************
// Method:  avtStreamlineInfoQuery::PreExecute()
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtStreamlineInfoQuery::PreExecute()
{
    avtDatasetQuery::PreExecute();
    slData.resize(0);
}

// ****************************************************************************
// Method:  avtStreamlineInfoQuery::PostExecute()
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtStreamlineInfoQuery::PostExecute()
{
    //Everyone communicate data to proc 0.
#ifdef PARALLEL
    int nProcs = PAR_Size();
    int *counts = new int[nProcs];
    for (int i = 0; i < nProcs; i++)
        counts[i] = 0;
    
    counts[PAR_Rank()] = slData.size();
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
                    slData.push_back(vals[j]);
                delete [] vals;
            }
        }
    }
    else
    {
        if (slData.size() > 0)
        {
            void *ptr = (void *)&slData[0];
            MPI_Send(ptr, slData.size(), MPI_FLOAT, 0, tag, VISIT_MPI_COMM);
        }
    }
    delete [] counts;
#endif
    
    std::string msg;
    char str[128];
    int i = 0, sz = slData.size();
    int endOfLineIdx = 3;
    
    int slIdx = 0;
    while (i < sz)
    {
        sprintf(str, "Streamline %d: Seed %f %f %f Arclength %f\n", slIdx, slData[i], slData[i+1], slData[i+2], slData[i+3]);
        i+=4;
        msg += str;

        if (dumpSteps)
        {
            int numSteps =  (int)slData[i++];
            for (int j = 0; j < numSteps; j++)
            {
                sprintf(str, " %f %f %f \n", slData[i], slData[i+1], slData[i+2]);// slData[i+3], slData[i+4]);
                i+=5;
                msg += str;
            }
        }
        slIdx++;
    }
    
    SetResultMessage(msg.c_str());
}

// ****************************************************************************
// Method:  avtStreamlineInfoQuery::Execute()
//
// Programmer:  Dave Pugmire
// Creation:    November  9, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtStreamlineInfoQuery::Execute(vtkDataSet *data, const int chunk)
{
    if (!data->IsA("vtkPolyData") ||
        data->GetPointData()->GetArray("colorVar") == NULL ||
        data->GetPointData()->GetArray("params") == NULL)
    {
        EXCEPTION1(NonQueryableInputException,"Streamline Info query only valid on streamline plots");
    }
    
    vtkPolyData *ds = (vtkPolyData *)data;
    vtkPoints *points = ds->GetPoints();
    vtkCellArray *lines = ds->GetLines();
    vtkIdType *segments = lines->GetPointer();
    float *scalar = (float *)data->GetPointData()->GetArray("colorVar");
    float *param = (float *)data->GetPointData()->GetArray("params");

    int *segptr = segments;
    double pt[3], p0[3];
    
    for (int i=0; i<ds->GetNumberOfLines(); i++)
    {
        int nPts = *segptr;
        segptr++; //Now segptr points at vtx0.
        
        float arcLen = 0.0;
        vector<float> steps;
        
        //Seed point.
        points->GetPoint(segptr[0], p0);
        slData.push_back(p0[0]);
        slData.push_back(p0[1]);
        slData.push_back(p0[2]);
        
        for (int j = 1; j < nPts; j++)
        {
            points->GetPoint(segptr[j], pt);
            if (dumpSteps)
            {
                steps.push_back(pt[0]);
                steps.push_back(pt[1]);
                steps.push_back(pt[2]);
                
                float p = param[segptr[j]];
                float s = scalar[segptr[j]];
                steps.push_back(p);
                steps.push_back(s);
            }

            double x = p0[0]-pt[0];
            double y = p0[1]-pt[1];
            double z = p0[2]-pt[2];
            arcLen += sqrt(x*x + y*y + z*z);

            p0[0] = pt[0];
            p0[1] = pt[1];
            p0[2] = pt[2];
        }

        slData.push_back(arcLen);
        if (dumpSteps)
        {
            slData.push_back((float)(nPts-1));
            slData.insert(slData.end(), steps.begin(), steps.end());
        }

        segptr += nPts;
    }
}
