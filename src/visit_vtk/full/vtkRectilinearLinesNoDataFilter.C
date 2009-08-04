/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <vtkRectilinearLinesNoDataFilter.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>

#include <ImproperUseException.h>


using  std::vector;


//------------------------------------------------------------------------------
vtkRectilinearLinesNoDataFilter* vtkRectilinearLinesNoDataFilter::New()
{
    // First try to create the object from the vtkObjectFactory
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkRectilinearLinesNoDataFilter");
    if(ret)
    {
        return (vtkRectilinearLinesNoDataFilter*)ret;
    }
    // If the factory was unable to create the object, then create it here.
    return new vtkRectilinearLinesNoDataFilter;
}

vtkRectilinearLinesNoDataFilter::vtkRectilinearLinesNoDataFilter()
{
}



#define AddLineToPolyData(ai,aj,ak, bi,bj,bk)                                 \
{                                                                             \
    p[0]=x[ai]; p[1]=y[aj]; p[2]=z[ak];                                       \
    p+=3;                                                                     \
    outPointData->CopyData(inPointData, ((ak*nY) + aj)*nX + ai, pointId++);   \
    p[0]=x[bi]; p[1]=y[bj]; p[2]=z[bk];                                       \
    p+=3;                                                                     \
    outPointData->CopyData(inPointData, ((bk*nY) + bj)*nX + bi, pointId++);   \
    *nl++ = 2;                                                                \
    *nl++ = pointId-2;                                                        \
    *nl++ = pointId-1;                                                        \
    cellId++;                                                                 \
}

    

// ****************************************************************************
//
//
// ****************************************************************************

void vtkRectilinearLinesNoDataFilter::Execute()
{
    //
    // Set up some objects that we will be using throughout the process.
    //
    vtkRectilinearGrid *input        = GetInput();
    vtkPolyData        *output       = vtkPolyData::New();
    vtkCellData        *inCellData   = input->GetCellData();
    vtkPointData       *inPointData  = input->GetPointData();
    vtkCellData        *outCellData  = output->GetCellData();
    vtkPointData       *outPointData = output->GetPointData();

    //
    // Get the information about X, Y, and Z from the rectilinear grid.
    //
    vtkDataArray *xc = input->GetXCoordinates();
    int nX = xc->GetNumberOfTuples();
    float *x = new float[nX];
    for (int i = 0 ; i < nX ; i++)
    {
        x[i] = xc->GetTuple1(i);
    }
    vtkDataArray *yc = input->GetYCoordinates();
    int nY = yc->GetNumberOfTuples();
    float *y = new float[nY];
    for (int i = 0 ; i < nY ; i++)
    {
        y[i] = yc->GetTuple1(i);
    }
    vtkDataArray *zc = input->GetZCoordinates();
    int nZ = zc->GetNumberOfTuples();
    float *z = new float[nZ];
    for (int i = 0 ; i < nZ ; i++)
    {
        z[i] = zc->GetTuple1(i);
    }

    //
    // Count points.  Be wary of 1D and 2D dimensions.
    //
    vtkPoints *pts = vtkPoints::New();
    int npts = 0;
    if ((nX==1 && nY==1) || (nX==1 && nZ==1) || (nY==1 && nZ==1))
    {
        npts = 2;
    }
    else if (nX == 1)
    {
        npts = nY*2 + nZ*2;
    }
    else if (nY == 1)
    {
        npts = nX*2 + nZ*2;
    }
    else if (nZ == 1)
    {
        npts = nX*2 + nY*2;
    }
    else
    {
        npts = nX*4 + nY*4 + nZ*4;
    }
    pts->SetNumberOfPoints(npts);
    float *p = (float *) pts->GetVoidPointer(0);

    //
    // Count the cells
    //
    int   numOutCells = 0;
    if ((nX==1 && nY==1) || (nX==1 && nZ==1) || (nY==1 && nZ==1))
    {
        numOutCells = 1;
    }
    else if (nX == 1)
    {
        numOutCells = nY + nZ;
    }
    else if (nY == 1)
    {
        numOutCells = nX + nZ;
    }
    else if (nZ == 1)
    {
        numOutCells = nX + nY;
    }
    else
    {
        numOutCells = nX*2 + nY*2 + nZ*2;
    }


    //
    // We will be copying the point data as we go so we need to set this up.
    //
    outPointData->CopyAllocate(input->GetPointData());

    //
    // And set up the cell arrays for creation (but not copying data)
    vtkCellArray *polys = vtkCellArray::New();
    vtkIdTypeArray *list = vtkIdTypeArray::New();
    list->SetNumberOfValues(numOutCells*(2+1));
    vtkIdType *nl = list->GetPointer(0);

    //
    // And now actually create the points/lines
    //
    int pointId = 0;
    int cellId = 0;

    // This case is mutually exclusive with the other ones below....
    if ((nX==1 && nY==1) || (nX==1 && nZ==1) || (nY==1 && nZ==1))
    {
        AddLineToPolyData(0,0,0, nX-1,nY-1,nZ-1);
    }

    if (nX>1 && nY>1)
    {
        // even if nz==1
        {
            // Front, Top to Bottom
            for (int i = 0 ; i < nX ; i++)
                AddLineToPolyData(i   ,0   ,0   ,       i   ,nY-1,0   );

            // Front, Left to Right
            for (int j = 0 ; j < nY ; j++)
                AddLineToPolyData(0   ,j   ,0   ,       nX-1,j   ,0   );
        }

        if (nZ>1)
        {
            // Back, Top to Bottom
            for (int i = 0 ; i < nX ; i++)
                AddLineToPolyData(i   ,0   ,nZ-1,       i   ,nY-1,nZ-1);

            // Back, Left to Right
            for (int j = 0 ; j < nY ; j++)
                AddLineToPolyData(0   ,j   ,nZ-1,       nX-1,j   ,nZ-1);
        }
    }

    if (nX>1 && nZ>1)
    {
        // even if ny==1
        {
            // Top, Front to Back
            for (int i = 0 ; i < nX ; i++)
                AddLineToPolyData(i   ,0   ,0   ,       i   ,0   ,nZ-1);

            // Top, Left to Right
            for (int k = 0 ; k < nZ ; k++)
                AddLineToPolyData(0   ,0   ,k   ,       nX-1,0   ,k   );
        }

        if (nY>1)
        {
            // Bottom, Front to Back
            for (int i = 0 ; i < nX ; i++)
                AddLineToPolyData(i   ,nY-1,0   ,       i   ,nY-1,nZ-1);

            // Bottom, Left to Right
            for (int k = 0 ; k < nZ ; k++)
                AddLineToPolyData(0   ,nY-1,k   ,       nX-1,nY-1,k   );
        }
    }

    if (nY>1 && nZ>1)
    {
        // even if nx==1
        {
            // Left, Front to Back
            for (int j = 0 ; j < nY ; j++)
                AddLineToPolyData(0   ,j   ,0   ,       0   ,j   ,nZ-1);

            // Left, Top to Bottom
            for (int k = 0 ; k < nZ ; k++)
                AddLineToPolyData(0   ,0   ,k   ,       0   ,nY-1,k   );
        }

        if (nX>1)
        {
            // Right, Front to Back
            for (int j = 0 ; j < nY ; j++)
                AddLineToPolyData(nX-1,j   ,0   ,       nX-1,j   ,nZ-1);

            // Right, Top to Bottom
            for (int k = 0 ; k < nZ ; k++)
                AddLineToPolyData(nX-1,0   ,k   ,       nX-1,nY-1,k   );
        }
    }

    //
    // Clean up.....
    //
    delete [] x;
    delete [] y;
    delete [] z;
    output->SetPoints(pts);
    pts->Delete();

  
    polys->SetCells(numOutCells, list);
    list->Delete();
    outCellData->Squeeze();
    output->SetLines(polys);
    polys->Delete();

    GetOutput()->ShallowCopy(output);
    GetOutput()->GetFieldData()->ShallowCopy(GetInput()->GetFieldData());

    output->Delete();
}

#undef AddLineToPolyData



void vtkRectilinearLinesNoDataFilter::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);
}
