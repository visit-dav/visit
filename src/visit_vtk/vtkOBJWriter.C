// Heavily borrowed from vtkOBJExporter.
//
// Written by Hank Childs, May 27, 2002.

#include "vtkOBJWriter.h"
#include "vtkGeometryFilter.h"
#include "vtkAssemblyNode.h"
#include "vtkObjectFactory.h"
#include "vtkFloatArray.h"

vtkCxxRevisionMacro(vtkOBJWriter, "$Revision: 1.42 $");
vtkStandardNewMacro(vtkOBJWriter);

vtkOBJWriter::vtkOBJWriter()
{
  this->Label = NULL;
}

vtkOBJWriter::~vtkOBJWriter()
{
  if (this->Label != NULL)
    {
    delete [] this->Label;
    }
}

void vtkOBJWriter::WriteData()
{
  FILE *fpObj;
  int idStart = 1;
  vtkPolyData *pd = this->GetInput();
  vtkPointData *pntData;
  vtkPoints *points = NULL;
  vtkDataArray *normals = NULL;
  vtkDataArray *tcoords = NULL;
  int i, idNext;
  float *p;
  vtkCellArray *cells;
  vtkIdType npts;
  vtkIdType *indx;
  
  if (pd == NULL)
    {
    vtkErrorMacro(<< "No input to writer");
    return;
    }

  if (this->FileName == NULL)
    {
    vtkErrorMacro(<< "Please specify FileName to write");
    return;
    }

  fpObj = fopen(this->FileName,"w");
  if (!fpObj)
    {
    vtkErrorMacro(<< "unable to open output file");
    return;
    }
  
  //
  //  Write header
  //
  vtkDebugMacro("Writing wavefront files");
  fprintf(fpObj, "# wavefront obj file written by VisIt\n");
  if (this->Label)
    {
    fprintf(fpObj, "# Description: %s\n", this->Label);
    }
  fprintf(fpObj, "\n");
  
  // write out the points
  points = pd->GetPoints();
  for (i = 0; i < points->GetNumberOfPoints(); i++)
    {
    p = points->GetPoint(i);
    fprintf (fpObj, "v %g %g %g\n", p[0], p[1], p[2]);
    }
  idNext = idStart + (int)(points->GetNumberOfPoints());
  
  // write out the point data
  pntData = pd->GetPointData();
  if (pntData->GetNormals())
    {
    normals = pntData->GetNormals();
    for (i = 0; i < normals->GetNumberOfTuples(); i++)
      {
      p = normals->GetTuple(i);
      fprintf (fpObj, "vn %g %g %g\n", p[0], p[1], p[2]);
      }
    }
  
  tcoords = pntData->GetTCoords();
  if (tcoords)
    {
    for (i = 0; i < tcoords->GetNumberOfTuples(); i++)
      {
      p = tcoords->GetTuple(i);
      fprintf (fpObj, "vt %g %g\n", p[0], p[1]);
      }
    }
  
  // write out polys if any
  if (pd->GetNumberOfPolys() > 0)
    {
    cells = pd->GetPolys();
    for (cells->InitTraversal(); cells->GetNextCell(npts,indx); )
      {
      fprintf(fpObj,"f ");
      for (i = 0; i < npts; i++)
        {
        if (normals)
          {
          if (tcoords)
            {
            // treating vtkIdType as int
            fprintf(fpObj,"%i/%i/%i ", ((int)indx[i])+idStart, 
                    ((int)indx[i]) + idStart, ((int)indx[i]) + idStart);
            }
          else
            {
            // treating vtkIdType as int
            fprintf(fpObj,"%i//%i ",((int)indx[i])+idStart,
                    ((int)indx[i]) + idStart);
            }
          }
        else
          {
          if (tcoords)
            {
            // treating vtkIdType as int
            fprintf(fpObj,"%i/%i ", ((int)indx[i])+idStart, 
                    ((int)indx[i]) + idStart);
            }
          else
            {
            // treating vtkIdType as int
            fprintf(fpObj,"%i ", ((int)indx[i])+idStart);
            }
          }
        }
      fprintf(fpObj,"\n");
      }
    }

  idStart = idNext;

  fclose(fpObj);
}

