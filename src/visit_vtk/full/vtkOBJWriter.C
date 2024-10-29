// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// Heavily borrowed from vtkOBJExporter.
//
// Written by Hank Childs, May 27, 2002.

// Modifications:
//   Justin Privitera, Fri Nov  3 15:25:32 PDT 2023
//   Added writeMTL, MTLHasTexture, basename, and texFilename.
//   Reformatted the entire file.
//   Added ability to write out mtllib with a linked texture.
// ****************************************************************************

#include "vtkOBJWriter.h"

#include <visit-config.h>

#include <vtkCellArray.h>
#if LIB_VERSION_GE(VTK, 9,1,0)
#include <vtkCellArrayIterator.h>
#endif
#include <vtkFloatArray.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>

vtkStandardNewMacro(vtkOBJWriter);

vtkOBJWriter::vtkOBJWriter()
{
    this->Label = NULL;
    writeMTL = false;
    hasTex = false;
    basename = "";
}

vtkOBJWriter::~vtkOBJWriter()
{
    if (this->Label != NULL)
    {
        delete [] this->Label;
    }
}

void vtkOBJWriter::SetWriteMTL(bool _writeMTL)
{
    writeMTL = _writeMTL;
}

void vtkOBJWriter::SetMTLHasTexture(bool _hasTex)
{
    hasTex = _hasTex;
}

void vtkOBJWriter::SetBasename(std::string _basename)
{
    basename = _basename;
}

void vtkOBJWriter::SetTexFilename(std::string _texFilename)
{
    texFilename = _texFilename;
}

//
//  Modifications:
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK9: use vtkCellArrayIterator.
//
void vtkOBJWriter::WriteData()
{
    double *p;
    vtkIdType npts;
#if LIB_VERSION_LE(VTK, 8,1,0)
    vtkCellArray *cells;
    vtkIdType *indx;
#else
    const vtkIdType *indx;
#endif

    vtkPolyData *polydata = this->GetInput(0);

    if (polydata == NULL)
    {
        vtkErrorMacro(<< "No input to writer");
        return;
    }

    std::string objFilename = basename + ".obj";
    std::string mtlFilename = basename + ".mtl";

    if (basename.empty())
    {
        vtkErrorMacro(<< "Please specify FileName to write");
        return;
    }

    FILE *fpObj;
    fpObj = fopen(objFilename.c_str(), "w");
    if (!fpObj)
    {
        vtkErrorMacro(<< "unable to open output obj file");
        return;
    }

    //
    //  Write obj header
    //
    vtkDebugMacro("Writing wavefront files");
    fprintf(fpObj, "# wavefront obj file written by VisIt\n");
    if (this->Label)
    {
        fprintf(fpObj, "# Description: %s\n", this->Label);
    }
    fprintf(fpObj, "\n");

    vtkIdType idStart = 1;
    
    FILE *fpMtl;
    if (writeMTL)
    {
        // open mtl file
        fpMtl = fopen(mtlFilename.c_str(), "w");
        if (!fpMtl)
        {
            vtkErrorMacro(<< "unable to open output mtl file");
            return;
        }

        // if we are writing mtl, we need to link it in the obj file
        fprintf (fpObj, "\nmtllib %s\n", mtlFilename.c_str());

        // write out a group name and material
        fprintf (fpObj, "\ng grp%lli\n", idStart);
        fprintf (fpObj, "usemtl mtl%lli\n", idStart);

        // now write to mtl file
        //
        //  Write mtl header
        //
        fprintf(fpMtl, "# wavefront mtl file written by VisIt\n");
        if (this->Label)
        {
            fprintf(fpMtl, "# Description: %s\n", this->Label);
        }
        fprintf(fpMtl, "\n");

        fprintf(fpMtl,"newmtl mtl%lli\n",idStart);
        fprintf(fpMtl,"\tKa 0.000 0.000 0.000\n");
        fprintf(fpMtl,"\tKd 0.000 0.000 0.000\n");
        fprintf(fpMtl,"\tKs 0.000 0.000 0.000\n");
        fprintf(fpMtl,"\tNs 100\n");
        fprintf(fpMtl,"\tillum 2\n\n");

        if (hasTex)
        {
            if (texFilename.empty())
            {
                vtkErrorMacro(<< "Please specify texture FileName");
                return;
            }

            fprintf(fpMtl,"\tmap_Ka %s\n", texFilename.c_str());
            fprintf(fpMtl,"\tmap_Kd %s\n", texFilename.c_str());
            fprintf(fpMtl,"\tmap_Ks %s\n", texFilename.c_str());
        }

        fclose(fpMtl);
    }

    // write out the points
    vtkPoints *points = polydata->GetPoints();
    for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
    {
        p = points->GetPoint(i);
        fprintf (fpObj, "v %g %g %g\n", p[0], p[1], p[2]);
    }
    vtkIdType idNext = idStart + (int)(points->GetNumberOfPoints());

    // write out the point data
    vtkPointData *pntData = polydata->GetPointData();
    vtkDataArray *normals = pntData->GetNormals();
    if (normals)
    {
        for (vtkIdType i = 0; i < normals->GetNumberOfTuples(); i++)
        {
            p = normals->GetTuple(i);
            fprintf (fpObj, "vn %g %g %g\n", p[0], p[1], p[2]);
        }
    }

    vtkDataArray *tcoords = pntData->GetTCoords();
    if (tcoords)
    {
        for (vtkIdType i = 0; i < tcoords->GetNumberOfTuples(); i++)
        {
            p = tcoords->GetTuple(i);
            fprintf (fpObj, "vt %g %g\n", p[0], p[1]);
        }
    }

    // write out polys if any
    if (polydata->GetNumberOfPolys() > 0)
    {
#if LIB_VERSION_LE(VTK, 8,1,0)
        cells = polydata->GetPolys();
        for (cells->InitTraversal(); cells->GetNextCell(npts,indx); )
        {
#else
        auto cells = vtk::TakeSmartPointer(polydata->GetPolys()->NewIterator());
        for (cells->GoToFirstCell(); !cells->IsDoneWithTraversal(); cells->GoToNextCell())
        {
            cells->GetCurrentCell(npts,indx);
#endif
            fprintf(fpObj,"f ");
            for (vtkIdType i = 0; i < npts; i++)
            {
                if (normals)
                {
                    if (tcoords)
                    {
                        // treating vtkIdType as int
                        fprintf(fpObj,"%i/%i/%i ", int(indx[i]+idStart),
                                int(indx[i] + idStart), int(indx[i] + idStart));
                    }
                    else
                    {
                        // treating vtkIdType as int
                        fprintf(fpObj,"%i//%i ",int(indx[i]+idStart),
                                int(indx[i] + idStart));
                    }
                }
                else
                {
                    if (tcoords)
                    {
                        // treating vtkIdType as int
                        fprintf(fpObj,"%i/%i ", int(indx[i]+idStart),
                                int(indx[i] + idStart));
                    }
                    else
                    {
                        // treating vtkIdType as int
                        fprintf(fpObj,"%i ", int(indx[i]+idStart));
                    }
                }
            }
            fprintf(fpObj,"\n");
        }
    }

    idStart = idNext;

    fclose(fpObj);
}

