/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "vtkVisItRectilinearGrid.h"
#include <vtkObjectFactory.h>
#include "vtkCellData.h"
#include "vtkEmptyCell.h"
#include "vtkGenericCell.h"
#include "vtkLine.h"
#include "vtkMath.h"
#include "vtkPoints.h"
#include "vtkPixel.h"
#include "vtkVertex.h"
#include "vtkVoxel.h"

vtkStandardNewMacro(vtkVisItRectilinearGrid);


// ****************************************************************************
//  Method: vtkVisItRectilinearGrid::vtkVisItRectilinearGrid
//
//  Purpose:
//      Constructor for vtkVisItRectilinearGrid.  Deletes most of what the base 
//      class allocates by default, so it can be created on demand instead.
//
//  Programmer: Dave Bremer
//  Creation:   May 25, 2007
// ****************************************************************************

vtkVisItRectilinearGrid::vtkVisItRectilinearGrid()
{
    // Delete most of what the base class makes by default, and create it on demand.
    if (this->Vertex)
    {
        this->Vertex->Delete();
        this->Vertex = NULL;
    }
    if (this->Line)
    {
        this->Line->Delete();
        this->Line = NULL;
    }
    if (this->Pixel)
    {
        this->Pixel->Delete();
        this->Pixel = NULL;
    }
    if (this->Voxel)
    {
        this->Voxel->Delete();
        this->Voxel = NULL;
    }
}


// ****************************************************************************
//  Method: vtkVisItRectilinearGrid::~vtkVisItRectilinearGrid
//
//  Purpose:
//      Destructor for vtkVisItRectilinearGrid.  Unfortunately, I need to 
//      allocate some data members because ~vtkRectilinearGrid will segv
//      otherwise, when it deletes them.
//
//  Programmer: Dave Bremer
//  Creation:   May 25, 2007
// ****************************************************************************

vtkVisItRectilinearGrid::~vtkVisItRectilinearGrid()
{
    // The base class destructor expects all these classes to exist.  So I have
    // to instantiate them right before destruction.
    if (!this->Vertex)
    {
        this->Vertex = vtkVertex::New();
    }
    if (!this->Line)
    {
        this->Line = vtkLine::New();
    }
    if (!this->Pixel)
    {
        this->Pixel = vtkPixel::New();
    }
    if (!this->Voxel)
    {
        this->Voxel = vtkVoxel::New();
    }
}


// ****************************************************************************
//  Method: vtkVisItRectilinearGrid::GetCell
//
//  Purpose:
//      Reimplementation of vtkVisItRectilinearGrid::GetCell, which allocates the
//      cells on demand.  This is exactly the same as the implementation of 
//      vtkVisItRectilinearGrid::GetCell in VTK 5.0.0, except for formatting 
//      changes and a few lines for on-demand allocation.
//
//  Programmer: Dave Bremer
//  Creation:   May 25, 2007
// ****************************************************************************

vtkCell *
vtkVisItRectilinearGrid::GetCell(vtkIdType cellId)
{
    vtkCell *cell = NULL;
    vtkIdType idx, npts;
    int loc[3];
    int iMin, iMax, jMin, jMax, kMin, kMax;
    int d01 = this->Dimensions[0]*this->Dimensions[1];
    double x[3];
    
    iMin = iMax = jMin = jMax = kMin = kMax = 0;
    
    switch (this->DataDescription)
    {
    case VTK_EMPTY:
        return NULL;
    
    case VTK_SINGLE_POINT: // cellId can only be = 0
        if (!this->Vertex)
        {
            this->Vertex = vtkVertex::New();
        }
    
        cell = this->Vertex;
        break;
    
    case VTK_X_LINE:
        if (!this->Line)
        {
            this->Line = vtkLine::New();
        }
        iMin = cellId;
        iMax = cellId + 1;
        cell = this->Line;
        break;
    
    case VTK_Y_LINE:
        if (!this->Line)
        {
            this->Line = vtkLine::New();
        }
        jMin = cellId;
        jMax = cellId + 1;
        cell = this->Line;
        break;
    
    case VTK_Z_LINE:
        if (!this->Line)
        {
            this->Line = vtkLine::New();
        }
        kMin = cellId;
        kMax = cellId + 1;
        cell = this->Line;
        break;
    
    case VTK_XY_PLANE:
        if (!this->Pixel)
        {
            this->Pixel = vtkPixel::New();
        }
        iMin = cellId % (this->Dimensions[0]-1);
        iMax = iMin + 1;
        jMin = cellId / (this->Dimensions[0]-1);
        jMax = jMin + 1;
        cell = this->Pixel;
        break;
    
    case VTK_YZ_PLANE:
        if (!this->Pixel)
        {
            this->Pixel = vtkPixel::New();
        }
        jMin = cellId % (this->Dimensions[1]-1);
        jMax = jMin + 1;
        kMin = cellId / (this->Dimensions[1]-1);
        kMax = kMin + 1;
        cell = this->Pixel;
        break;
    
    case VTK_XZ_PLANE:
        if (!this->Pixel)
        {
            this->Pixel = vtkPixel::New();
        }
        iMin = cellId % (this->Dimensions[0]-1);
        iMax = iMin + 1;
        kMin = cellId / (this->Dimensions[0]-1);
        kMax = kMin + 1;
        cell = this->Pixel;
        break;
    
    case VTK_XYZ_GRID:
        if (!this->Voxel)
        {
            this->Voxel = vtkVoxel::New();
        }
        iMin = cellId % (this->Dimensions[0] - 1);
        iMax = iMin + 1;
        jMin = (cellId / (this->Dimensions[0] - 1)) % (this->Dimensions[1] - 1);
        jMax = jMin + 1;
        kMin = cellId / ((this->Dimensions[0] - 1) * (this->Dimensions[1] - 1));
        kMax = kMin + 1;
        cell = this->Voxel;
        break;
    }
    // Extract point coordinates and point ids
    for (npts=0,loc[2]=kMin; loc[2]<=kMax; loc[2]++)
    {
        x[2] = this->ZCoordinates->GetComponent(loc[2], 0);
        for (loc[1]=jMin; loc[1]<=jMax; loc[1]++)
        {
            x[1] = this->YCoordinates->GetComponent(loc[1], 0);
            for (loc[0]=iMin; loc[0]<=iMax; loc[0]++)
            {
                x[0] = this->XCoordinates->GetComponent(loc[0], 0);
        
                idx = loc[0] + loc[1]*this->Dimensions[0] + loc[2]*d01;
                cell->PointIds->SetId(npts,idx);
                cell->Points->SetPoint(npts++,x);
            }
        }
    }
    return cell;
}




