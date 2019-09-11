// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkVisItStructuredGrid.h"
#include <vtkObjectFactory.h>
#include "vtkCellData.h"
#include "vtkEmptyCell.h"
#include "vtkGenericCell.h"
#include "vtkHexahedron.h"
#include "vtkLine.h"
#include "vtkMath.h"
#include "vtkQuad.h"
#include "vtkVertex.h"


vtkStandardNewMacro(vtkVisItStructuredGrid);


// ****************************************************************************
//  Method: vtkVisItStructuredGrid::vtkVisItStructuredGrid
//
//  Purpose:
//      Constructor for vtkVisItStructuredGrid.  Deletes most of what the base 
//      class allocates by default, so it can be created on demand instead.
//
//  Programmer: Dave Bremer
//  Creation:   May 25, 2007
// ****************************************************************************

vtkVisItStructuredGrid::vtkVisItStructuredGrid()
{
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
    if (this->Quad)
    {
        this->Quad->Delete();
        this->Quad = NULL;
    }
    if (this->Hexahedron)
    {
        this->Hexahedron->Delete();
        this->Hexahedron = NULL;
    }
    if (this->EmptyCell)
    {
        this->EmptyCell->Delete();
        this->EmptyCell = NULL;
    }
}


// ****************************************************************************
//  Method: vtkVisItStructuredGrid::~vtkVisItStructuredGrid
//
//  Purpose:
//      Destructor for vtkVisItStructuredGrid.  Unfortunately, I need to 
//      allocate some data members because ~vtkStructuredGrid will segv
//      otherwise, when it deletes them.
//
//  Programmer: Dave Bremer
//  Creation:   May 25, 2007
// ****************************************************************************

vtkVisItStructuredGrid::~vtkVisItStructuredGrid()
{
    if (!this->Vertex)
    {
        this->Vertex = vtkVertex::New();
    }
    if (!this->Line)
    {
        this->Line = vtkLine::New();
    }
    if (!this->Quad)
    {
        this->Quad = vtkQuad::New();
    }
    if (!this->Hexahedron)
    {
        this->Hexahedron = vtkHexahedron::New();
    }
    if (!this->EmptyCell)
    {
        this->EmptyCell = vtkEmptyCell::New();
    }
}


// ****************************************************************************
//  Method: vtkVisItStructuredGrid::GetCell
//
//  Purpose:
//      Reimplementation of vtkStructuredGrid::GetCell, which allocates the
//      cells on demand.  This is exactly the same as the implementation of 
//      vtkStructuredGrid::GetCell in VTK 5.0.0, except for formatting 
//      changes and a few lines for on-demand allocation.
//
//  Programmer: Dave Bremer
//  Creation:   May 25, 2007
//
//  Modifications:
//
//    Hank Childs, Fri Feb 15 11:56:28 PST 2008
//    Add a check in case a new cell type is added later.
//
//    Hank Childs, Tue Feb 19 08:47:50 PST 2008
//    Fix stupid problem with last change.
//
// ****************************************************************************

vtkCell *
vtkVisItStructuredGrid::GetCell(vtkIdType cellId)
{
    vtkCell *cell = NULL;
    vtkIdType idx;
    int i, j, k;
    int d01, offset1, offset2;
    
    // Make sure data is defined
    if (!this->Points)
    {
        vtkErrorMacro (<<"No data");
        return NULL;
    }
    
    // see whether the cell is blanked
    if(!this->IsCellVisible(cellId))
      {
      if (!this->EmptyCell)
        this->EmptyCell = vtkEmptyCell::New();
      return this->EmptyCell;
      }
    
    // Update dimensions
    this->GetDimensions();
    
    switch (this->DataDescription)
    {
    case VTK_EMPTY:
        if (!this->EmptyCell)
            this->EmptyCell = vtkEmptyCell::New();
        return this->EmptyCell;
    
    case VTK_SINGLE_POINT: // cellId can only be = 0
        if (!this->Vertex)
            this->Vertex = vtkVertex::New();
        cell = this->Vertex;
        cell->PointIds->SetId(0,0);
        break;
    
    case VTK_X_LINE:
        if (!this->Line)
            this->Line = vtkLine::New();
        cell = this->Line;
        cell->PointIds->SetId(0,cellId);
        cell->PointIds->SetId(1,cellId+1);
        break;
    
    case VTK_Y_LINE:
        if (!this->Line)
            this->Line = vtkLine::New();
        cell = this->Line;
        cell->PointIds->SetId(0,cellId);
        cell->PointIds->SetId(1,cellId+1);
        break;
    
    case VTK_Z_LINE:
        if (!this->Line)
            this->Line = vtkLine::New();
        cell = this->Line;
        cell->PointIds->SetId(0,cellId);
        cell->PointIds->SetId(1,cellId+1);
        break;
    
    case VTK_XY_PLANE:
        if (!this->Quad)
            this->Quad = vtkQuad::New();
        cell = this->Quad;
        i = cellId % (this->Dimensions[0]-1);
        j = cellId / (this->Dimensions[0]-1);
        idx = i + j*this->Dimensions[0];
        offset1 = 1;
        offset2 = this->Dimensions[0];
    
        cell->PointIds->SetId(0,idx);
        cell->PointIds->SetId(1,idx+offset1);
        cell->PointIds->SetId(2,idx+offset1+offset2);
        cell->PointIds->SetId(3,idx+offset2);
        break;
    
    case VTK_YZ_PLANE:
        if (!this->Quad)
            this->Quad = vtkQuad::New();
        cell = this->Quad;
        j = cellId % (this->Dimensions[1]-1);
        k = cellId / (this->Dimensions[1]-1);
        idx = j + k*this->Dimensions[1];
        offset1 = 1;
        offset2 = this->Dimensions[1];
    
        cell->PointIds->SetId(0,idx);
        cell->PointIds->SetId(1,idx+offset1);
        cell->PointIds->SetId(2,idx+offset1+offset2);
        cell->PointIds->SetId(3,idx+offset2);
        break;
    
    case VTK_XZ_PLANE:
        if (!this->Quad)
            this->Quad = vtkQuad::New();
        cell = this->Quad;
        i = cellId % (this->Dimensions[0]-1);
        k = cellId / (this->Dimensions[0]-1);
        idx = i + k*this->Dimensions[0];
        offset1 = 1;
        offset2 = this->Dimensions[0];
    
        cell->PointIds->SetId(0,idx);
        cell->PointIds->SetId(1,idx+offset1);
        cell->PointIds->SetId(2,idx+offset1+offset2);
        cell->PointIds->SetId(3,idx+offset2);
        break;
    
    case VTK_XYZ_GRID:
        if (!this->Hexahedron)
            this->Hexahedron = vtkHexahedron::New();
        cell = this->Hexahedron;
        d01 = this->Dimensions[0]*this->Dimensions[1];
        i = cellId % (this->Dimensions[0] - 1);
        j = (cellId / (this->Dimensions[0] - 1)) % (this->Dimensions[1] - 1);
        k = cellId / ((this->Dimensions[0] - 1) * (this->Dimensions[1] - 1));
        idx = i+ j*this->Dimensions[0] + k*d01;
        offset1 = 1;
        offset2 = this->Dimensions[0];
    
        cell->PointIds->SetId(0,idx);
        cell->PointIds->SetId(1,idx+offset1);
        cell->PointIds->SetId(2,idx+offset1+offset2);
        cell->PointIds->SetId(3,idx+offset2);
        idx += d01;
        cell->PointIds->SetId(4,idx);
        cell->PointIds->SetId(5,idx+offset1);
        cell->PointIds->SetId(6,idx+offset1+offset2);
        cell->PointIds->SetId(7,idx+offset2);
        break;
    }
    
    // Extract point coordinates and point ids. NOTE: the ordering of the vtkQuad
    // and vtkHexahedron cells are tricky.
    int NumberOfIds = 0;
    if (cell != NULL)
        NumberOfIds = cell->PointIds->GetNumberOfIds();
    for (i=0; i<NumberOfIds; i++)
    {
        idx = cell->PointIds->GetId(i);
        cell->Points->SetPoint(i,this->Points->GetPoint(idx));
    }
    return cell;
}




