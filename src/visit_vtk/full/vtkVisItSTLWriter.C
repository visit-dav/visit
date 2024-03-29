// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// TAKEN FROM THE 7/25 VTK SOURCE
//
// Modifications:
//   Jeremy Meredith, Wed Jul 31 17:40:32 PDT 2002
//   Renamed to vtkVisItSTLWriter.
//   Made the number of cells in the binary writer
//   be an int instead of a long.  It was trying to write
//   4 bytes of a long, which failed on 64-bit platforms
//   (e.g. wrote all zeros).

#include "vtkVisItSTLWriter.h"

#include <visit-config.h> // for LIB_VERSION_LE/LIB_VERSION_GE

#include <vtkByteSwap.h>
#include <vtkCellArray.h>
#if LIB_VERSION_GE(VTK, 9,1,0)
#include <vtkCellArrayIterator.h>
#endif
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkTriangle.h>

vtkStandardNewMacro(vtkVisItSTLWriter);

vtkVisItSTLWriter::vtkVisItSTLWriter()
{
  this->FileType = VTK_ASCII;
}

void vtkVisItSTLWriter::WriteData()
{
  vtkPoints *pts;
  vtkCellArray *polys;
  vtkPolyData *input = this->GetInput();

  polys = input->GetPolys();
  pts = input->GetPoints();
  if (pts == NULL || polys == NULL )
    {
    vtkErrorMacro(<<"No data to write!");
    return;
    }

  if ( this->FileName == NULL)
    {
    vtkErrorMacro(<< "Please specify FileName to write");
    return;
    }

  if ( this->FileType == VTK_BINARY )
    {
    this->WriteBinarySTL(pts,polys);
    }
  else
    {
    this->WriteAsciiSTL(pts,polys);
    }
}

static char header[]="Visualization Toolkit generated SLA File                                        ";

//
// Modifications:
//   Kathleen Biagas, Thu Aug 11, 2022
//   Support VTK9, use vtkCellArrayIterator and const for indx.
//

void vtkVisItSTLWriter::WriteAsciiSTL(vtkPoints *pts, vtkCellArray *polys)
{
  FILE *fp;
  double n[3], v1[3], v2[3], v3[3];
  vtkIdType npts = 0;
#if LIB_VERSION_LE(VTK, 8,1,0)
  vtkIdType *indx = 0;
#else
  const vtkIdType *indx = 0;
#endif

  if ((fp = fopen(this->FileName, "w")) == NULL)
    {
    vtkErrorMacro(<< "Couldn't open file: " << this->FileName);
    return;
    }
//
//  Write header
//
  vtkDebugMacro("Writing ASCII sla file");
  fprintf (fp, "solid ascii\n");
//
//  Write out triangle polygons.  In not a triangle polygon, only first
//  three vertices are written.
//
#if LIB_VERSION_LE(VTK, 8,1,0)
  for (polys->InitTraversal(); polys->GetNextCell(npts,indx); )
    {
#else
  auto iter = vtk::TakeSmartPointer(polys->NewIterator());
  for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal(); iter->GoToNextCell())
    {
    iter->GetCurrentCell(npts,indx);
#endif
    pts->GetPoint(indx[0], v1);
    pts->GetPoint(indx[1], v2);
    pts->GetPoint(indx[2], v3);

    vtkTriangle::ComputeNormal(pts, npts, indx, n);

    fprintf (fp, " facet normal %.6g %.6g %.6g\n  outer loop\n",
            n[0], n[1], n[2]);

    fprintf (fp, "   vertex %.6g %.6g %.6g\n", v1[0], v1[1], v1[2]);
    fprintf (fp, "   vertex %.6g %.6g %.6g\n", v2[0], v2[1], v2[2]);
    fprintf (fp, "   vertex %.6g %.6g %.6g\n", v3[0], v3[1], v3[2]);

    fprintf (fp, "  endloop\n endfacet\n");
    }
  fprintf (fp, "endsolid\n");
  fclose (fp);
}

//
// Modifications:
//   Kathleen Biagas, Thu Aug 11, 2022
//   Support VTK9, use vtkCellArrayIterator and const for indx.
//

void vtkVisItSTLWriter::WriteBinarySTL(vtkPoints *pts, vtkCellArray *polys)
{
  FILE *fp;
  double dn[3], v1[3], v2[3], v3[3];
  vtkIdType npts = 0;
#if LIB_VERSION_LE(VTK, 8,1,0)
  vtkIdType *indx = 0;
#else
  const vtkIdType *indx = 0;
#endif
  int ncells;
  unsigned short ibuff2=0;

  if ((fp = fopen(this->FileName, "wb")) == NULL)
    {
    vtkErrorMacro(<< "Couldn't open file: " << this->FileName);
    return;
    }

  //  Write header
  //
  vtkDebugMacro("Writing Binary STL file");
  fwrite (header, 1, 80, fp);

  ncells = (int) polys->GetNumberOfCells();
  vtkByteSwap::Swap4LE(&ncells);
  fwrite (&ncells, 1, 4, fp);

  //  Write out triangle polygons.  In not a triangle polygon, only first
  //  three vertices are written.
  //
#if LIB_VERSION_LE(VTK, 8,1,0)
  for (polys->InitTraversal(); polys->GetNextCell(npts,indx); )
    {
#else
  auto iter = vtk::TakeSmartPointer(polys->NewIterator());
  for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal(); iter->GoToNextCell())
    {
    iter->GetCurrentCell(npts,indx);
#endif
    pts->GetPoint(indx[0], v1);
    pts->GetPoint(indx[1], v2);
    pts->GetPoint(indx[2], v3);

    vtkTriangle::ComputeNormal(pts, npts, indx, dn);
    float n[3] = {(float)dn[0], (float)dn[1], (float)dn[2]};
    vtkByteSwap::Swap4LE(n);
    vtkByteSwap::Swap4LE(n+1);
    vtkByteSwap::Swap4LE(n+2);
    fwrite (n, 4, 3, fp);

    n[0] = (float)v1[0];  n[1] = (float)v1[1];  n[2] = (float)v1[2];
    vtkByteSwap::Swap4LE(n);
    vtkByteSwap::Swap4LE(n+1);
    vtkByteSwap::Swap4LE(n+2);
    fwrite (n, 4, 3, fp);

    n[0] = (float)v2[0];  n[1] = (float)v2[1];  n[2] = (float)v2[2];
    vtkByteSwap::Swap4LE(n);
    vtkByteSwap::Swap4LE(n+1);
    vtkByteSwap::Swap4LE(n+2);
    fwrite (n, 4, 3, fp);

    n[0] = (float)v3[0];  n[1] = (float)v3[1];  n[2] = (float)v3[2];
    vtkByteSwap::Swap4LE(n);
    vtkByteSwap::Swap4LE(n+1);
    vtkByteSwap::Swap4LE(n+2);
    fwrite (n, 4, 3, fp);

    fwrite (&ibuff2, 2, 1, fp);
    }
  fclose (fp);
}

