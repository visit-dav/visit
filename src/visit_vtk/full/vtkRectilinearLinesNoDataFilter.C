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

#include <vtkRectilinearLinesNoDataFilter.h>

#include <vtkAccessors.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>

#include <ImproperUseException.h>

using  std::vector;

#define AddLineToPolyData(ai,aj,ak, bi,bj,bk)                                 \
{                                                                             \
    p.SetComponent(0, x.GetComponent(ai));                                    \
    p.SetComponent(1, y.GetComponent(aj));                                    \
    p.SetComponent(2, z.GetComponent(ak));                                    \
    ++p;                                                                      \
    outPointData->CopyData(inPointData, ((ak*nY) + aj)*nX + ai, pointId++);   \
    p.SetComponent(0, x.GetComponent(bi));                                    \
    p.SetComponent(1, y.GetComponent(bj));                                    \
    p.SetComponent(2, z.GetComponent(bk));                                    \
    ++p;                                                                      \
    outPointData->CopyData(inPointData, ((bk*nY) + bj)*nX + bi, pointId++);   \
    *nl++ = 2;                                                                \
    *nl++ = pointId-2;                                                        \
    *nl++ = pointId-1;                                                        \
    cellId++;                                                                 \
}


template <class Accessor> inline void
vtkRectilinearLinesNoDataFilter_AddLines(int nX, int nY, int nZ,
    vtkIdType *nl, vtkPointData *outPointData, vtkPointData *inPointData,
    Accessor x, Accessor y, Accessor z, Accessor p)
{
    //
    // And now actually create the points/lines
    //
    int pointId = 0;
    int cellId = 0;

    p.InitTraversal();

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
}

#undef AddLineToPolyData

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


// ****************************************************************************
//  Method: vtkRectilinearLinesNoDataFilter::RequestData
//
// ****************************************************************************

int
vtkRectilinearLinesNoDataFilter::RequestData(
    vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    //
    // Initialize some frequently used values.
    //
    vtkRectilinearGrid *input = vtkRectilinearGrid::SafeDownCast(
        inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkPolyData *output = vtkPolyData::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));

    //
    // Set up some objects that we will be using throughout the process.
    //
    vtkPolyData        *outPD        = vtkPolyData::New();
    vtkPointData       *inPointData  = input->GetPointData();
    vtkCellData        *outCellData  = outPD->GetCellData();
    vtkPointData       *outPointData = outPD->GetPointData();

    //
    // Get the information about X, Y, and Z from the rectilinear grid.
    //
    vtkDataArray *xc = input->GetXCoordinates();
    int nX = xc->GetNumberOfTuples();
    int tX = xc->GetDataType();
    vtkDataArray *yc = input->GetYCoordinates();
    int nY = yc->GetNumberOfTuples();
    int tY = yc->GetDataType();
    vtkDataArray *zc = input->GetZCoordinates();
    int nZ = zc->GetNumberOfTuples();
    int tZ = zc->GetDataType();

    bool same = (tX == tY && tY == tZ); 
    int type = (same ? tX : (tX == VTK_DOUBLE ? tX : (tY == VTK_DOUBLE ? tY : 
               (tZ == VTK_DOUBLE ? tZ : VTK_FLOAT))));

    //
    // Count points.  Be wary of 1D and 2D dimensions.
    //
    vtkPoints *pts = vtkPoints::New(type);
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
    if (same && type == VTK_FLOAT)
    {
        vtkRectilinearLinesNoDataFilter_AddLines(nX, nY, nZ,
            nl, outPointData, inPointData,
            vtkDirectAccessor<float>(xc), 
            vtkDirectAccessor<float>(yc), 
            vtkDirectAccessor<float>(zc), 
            vtkDirectAccessor<float>(pts->GetData())); 
    }
    else if (same && type == VTK_DOUBLE)
    {
        vtkRectilinearLinesNoDataFilter_AddLines(nX, nY, nZ,
            nl, outPointData, inPointData,
            vtkDirectAccessor<double>(xc), 
            vtkDirectAccessor<double>(yc), 
            vtkDirectAccessor<double>(zc), 
            vtkDirectAccessor<double>(pts->GetData())); 
    }
    else
    {
        vtkRectilinearLinesNoDataFilter_AddLines(nX, nY, nZ,
            nl, outPointData, inPointData,
            vtkGeneralAccessor(xc), 
            vtkGeneralAccessor(yc), 
            vtkGeneralAccessor(zc), 
            vtkGeneralAccessor(pts->GetData())); 
    }

    //
    // Clean up.....
    //
    outPD->SetPoints(pts);
    pts->Delete();
  
    polys->SetCells(numOutCells, list);
    list->Delete();
    outCellData->Squeeze();
    outPD->SetLines(polys);
    polys->Delete();

    output->ShallowCopy(outPD);
    output->GetFieldData()->ShallowCopy(input->GetFieldData());

    outPD->Delete();
    return 1;
}


// ****************************************************************************
//  Method: vtkRectilinearLinesNoDataFilter::FillInputPortInformation
//
// ****************************************************************************

int
vtkRectilinearLinesNoDataFilter::FillInputPortInformation(int,
    vtkInformation *info)
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkRectilinearGrid");
    return 1;
}


// ****************************************************************************
//  Method: vtkRectilinearLinesNoDataFilter::PrintSelf
//
// ****************************************************************************

void
vtkRectilinearLinesNoDataFilter::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);
}
