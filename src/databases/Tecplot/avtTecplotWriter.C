/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                              avtTecplotWriter.C                             //
// ************************************************************************* //

#include <avtTecplotWriter.h>

#include <vector>

#include <vtkDataSetWriter.h>

#include <avtDatabaseMetaData.h>

#include <ImproperUseException.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>
#include <vtkCellType.h>
#include <vtkCell.h>
#include <vtkVisItCellDataToPointData.h>
#include <Tetrahedralizer.h>

using     std::string;
using     std::vector;

avtTecplotWriter::avtTecplotWriter()
{
}

avtTecplotWriter::~avtTecplotWriter()
{
}

// ****************************************************************************
//  Method: avtTecplotWriter::OpenFile
//
//  Purpose:
//      Does no actual work.  Just records the stem name for the files.
//
//  Programmer: Jeremy Meredith
//  Creation:   Wed Feb 9 13:44:32 PST 2005
//
//  Modifications:
//    Jeremy Meredith, Tue Mar 27 17:03:47 EDT 2007
//    Added numblocks (currently ignored) to the OpenFile interface.
//
// ****************************************************************************

void
avtTecplotWriter::OpenFile(const string &stemname, int)
{
    stem = stemname;
    string filename = stemname+".tec";
    file.open(filename.c_str());
    if (!file)
    {
        EXCEPTION1(ImproperUseException, "Could not open file for writing.");
    }
}


// ****************************************************************************
//  Method: avtTecplotWriter::WriteHeaders
//
//  Purpose:
//      Writes out a VisIt file to tie the Tecplot files together.
//
//  Programmer: Jeremy Meredith
//  Creation:   Wed Feb 9 13:44:32 PST 2005
//
// ****************************************************************************

void
avtTecplotWriter::WriteHeaders(const avtDatabaseMetaData *md,
                               vector<string> &scalars,
                               vector<string> &vectors,
                               vector<string> &materials)
{
     const avtMeshMetaData *mmd = md->GetMesh(0);
    if (mmd->topologicalDimension != 3)
    {
        EXCEPTION1(ImproperUseException, "The Tecplot writer only supports "
                   "meshes with a topological dimension of 3.");
    }
 
    hadMaterial = materials.size() > 0;

    // BAD -- ASSUMES 3D
    variableList.clear();
    file << "TITLE = \"" << md->GetDatabaseName().c_str() << ": "
         << md->GetDatabaseComment().c_str() <<"\"" << endl;
    file << "VARIABLES = \"X\", \"Y\", \"Z\", ";
    for (int i=0; i < scalars.size(); i++)
    {
        variableList.push_back(scalars[i]);
        file << "\"" << scalars[i].c_str() << "\"";
        if (i < scalars.size()-1 || hadMaterial)
            file << ", ";
    }

    if (hadMaterial)
        file << "\"" << materials[0].c_str() << "\"";

    file << endl;
}


// ****************************************************************************
//  Method: avtTecplotWriter::WriteChunk
//
//  Purpose:
//      This writes out one chunk of an avtDataset.
//
//  Programmer: Jeremy Meredith
//  Creation:   Wed Feb 9 13:44:32 PST 2005
//
// ****************************************************************************

void
avtTecplotWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    switch (ds->GetDataObjectType())
    {
       case VTK_UNSTRUCTURED_GRID:
         WriteUnstructuredMesh((vtkUnstructuredGrid *) ds, chunk);
         break;

       case VTK_STRUCTURED_GRID:
         WriteCurvilinearMesh((vtkStructuredGrid *) ds, chunk);
         break;

       case VTK_RECTILINEAR_GRID:
       case VTK_POLY_DATA:
       default:
         EXCEPTION1(ImproperUseException, "Unsupported mesh type");
    }
}


// ****************************************************************************
//  Method: avtTecplotWriter::CloseFile
//
//  Purpose:
//      Closes the file.  This does nothing in this case.
//
//  Programmer: Jeremy Meredith
//  Creation:   Wed Feb 9 13:44:32 PST 2005
//
// ****************************************************************************

void
avtTecplotWriter::CloseFile(void)
{
    file.close();
}

void
avtTecplotWriter::WriteCurvilinearMesh(vtkStructuredGrid *sg, int chunk)
{
    // BAD -- ASSUMES 3D
    int dims[3];
    sg->GetDimensions(dims);
    file << "ZONE "
         << "T=\"DOMAIN "<<chunk<<"\", "
         << "I="<<dims[0]<<", "
         << "J="<<dims[1]<<", "
         << "K="<<dims[2]<<", "
         << "F=BLOCK" <<endl;
    file << endl;

    int npts = sg->GetNumberOfPoints();
    vtkPoints *vtk_pts = sg->GetPoints();
    float *vtk_ptr = (float *) vtk_pts->GetVoidPointer(0);
    for (int d = 0; d<3; d++)
    {
        for (int i=0; i<npts; i++)
        {
            file << vtk_ptr[3*i + d] << " ";
            if ((i+1)%10==0 || i==npts-1)
                file <<"\n";
        }
        file << endl;
    }

    WriteDataArrays(sg);
}

void
avtTecplotWriter::WriteUnstructuredMesh(vtkUnstructuredGrid *ug, int chunk)
{
    // BAD -- ASSUMES 3D HEXAHEDRAL MESH
    int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    int npts = ug->GetNumberOfPoints();
    int nzones = ug->GetNumberOfCells();

    int nhex = 0;
    int ntet = 0;
    int npyr = 0;
    int nwdg = 0;
    for (int c = 0 ; c < nzones ; c++)
    {
        vtkCell *cell = ug->GetCell(c);
        switch (cell->GetCellType())
        {
          case VTK_HEXAHEDRON:
            nhex++;
            break;
          case VTK_TETRA:
            ntet++;
            break;
          case VTK_WEDGE:
            nwdg++;
            break;
          case VTK_PYRAMID:
            npyr++;
            break;
          default:
            // ignore the rest
            break;
        }
    }

    bool subdivide = false;
    if (nwdg>0 || npyr>0 || (nhex>0 && ntet>0))
        subdivide = true;

    string elemType = "TETRAHEDRON";
    if (nhex>0  &&  !subdivide)
        elemType = "BRICK";

    int nelements;
    if (subdivide)
    {
        nelements = 0;
        for (int c = 0 ; c < nzones ; c++)
        {
            vtkCell *cell = ug->GetCell(c);
            int n = cell->GetNumberOfPoints();
            int ids[8];
            for (int i=0; i<n; i++)
                ids[i] = cell->GetPointId(i);

            int tetids[1000];
            int ntets = 0;

            // Do the connectivity
            switch (cell->GetCellType())
            {
              case VTK_HEXAHEDRON:
                nelements+=Tetrahedralizer::GetLowTetNodesForHex(n,ids,tetids);
                break;
              case VTK_WEDGE:
                nelements+=Tetrahedralizer::GetLowTetNodesForWdg(n,ids,tetids);
                break;
              case VTK_PYRAMID:
                nelements+=Tetrahedralizer::GetLowTetNodesForPyr(n,ids,tetids);
                break;
              case VTK_TETRA:
                nelements+=Tetrahedralizer::GetLowTetNodesForTet(n,ids,tetids);
                break;
            }
        }
    }
    else
    {
        nelements =   ntet  +   nhex  +  npyr  +   nwdg;
    }

    file << "ZONE "
         << "T=\"DOMAIN "<<chunk<<"\", "
         << "N="<<npts<<", "
         << "E="<<nelements<<", "
         << "F=FEBLOCK, "
         << "ET=" << elemType.c_str() << endl;
    file << endl;

    vtkPoints *vtk_pts = ug->GetPoints();
    float *vtk_ptr = (float *) vtk_pts->GetVoidPointer(0);

    for (int d = 0; d<3; d++)
    {
        for (int i=0; i<npts; i++)
        {
            file << vtk_ptr[3*i + d] << " ";
            if ((i+1)%10==0 || i==npts-1)
                file <<"\n";
        }
        file << endl;
    }

    WriteDataArrays(ug);

    file << endl;

    if (subdivide)
    {
        for (int c = 0 ; c < nzones ; c++)
        {
            vtkCell *cell = ug->GetCell(c);
            int n = cell->GetNumberOfPoints();
            int ids[8];
            for (int i=0; i<n; i++)
                ids[i] = cell->GetPointId(i);

            int tetids[1000];
            int ntets = 0;

            // Do the connectivity
            switch (cell->GetCellType())
            {
              case VTK_HEXAHEDRON:
                ntets = Tetrahedralizer::GetLowTetNodesForHex(n,ids,tetids);
                break;

              case VTK_WEDGE:
                ntets = Tetrahedralizer::GetLowTetNodesForWdg(n,ids,tetids);
                break;

              case VTK_PYRAMID:
                ntets = Tetrahedralizer::GetLowTetNodesForPyr(n,ids,tetids);
                break;

              case VTK_TETRA:
                ntets = Tetrahedralizer::GetLowTetNodesForTet(n,ids,tetids);
                break;
            }

            for (int t = 0 ; t<ntets; t++)
            {
                for (int i = 0 ; i < 4 ; i++)
                {
                    file << ids[tetids[t*4 + i]]+1 << " ";
                }
                file << endl;
            }
        }
    }
    else
    {
        for (int c = 0 ; c < nzones ; c++)
        {
            vtkCell *cell = ug->GetCell(c);
            // if we're not subdividing, we better only be getting
            // hexes or tets
            if (cell->GetCellType() == VTK_HEXAHEDRON ||
                cell->GetCellType() == VTK_TETRA)
            {
                for (int j = 0 ; j < cell->GetNumberOfPoints() ; j++)
                {
                    file << cell->GetPointId(j)+1 << " ";
                }
                file << endl;
            }
        }
    }
}

void
avtTecplotWriter::WriteDataArrays(vtkDataSet *ds1)
{
    int npts = ds1->GetNumberOfPoints();

    vtkVisItCellDataToPointData *c2p = vtkVisItCellDataToPointData::New();
    c2p->SetInput(ds1);
    c2p->Update();
    vtkUnstructuredGrid *ds2 = (vtkUnstructuredGrid*)(c2p->GetOutput());

    vtkPointData *pd = ds1->GetPointData();
    vtkPointData *pd2 = ds2->GetPointData();
    for (int v = 0 ; v < variableList.size() ; v++)
    {
        vtkDataArray *arr = pd->GetArray(variableList[v].c_str());
        if (!arr)
        {
            arr = pd2->GetArray(variableList[v].c_str());
            if (!arr)
                EXCEPTION1(ImproperUseException,
                           string("Couldn't find array ")+variableList[v]+".");
        }
        int ncomps = arr->GetNumberOfComponents();
        float *ptr = (float *) arr->GetVoidPointer(0);
        if (ncomps == 1)
        {
            for (int i=0; i<npts; i++)
            {
                file << ptr[i] << " ";
                if ((i+1)%10==0 || i==npts-1)
                    file <<"\n";
            }
            file << endl;
        }
    }

    if (hadMaterial)
    {
        vtkDataArray *arr = pd2->GetArray("avtSubsets");
        if (!arr)
        {
            EXCEPTION1(ImproperUseException,"Couldn't find avtSubsets array.");
        }
        int *ptr = (int*)arr->GetVoidPointer(0);
        for (int i=0; i<npts; i++)
        {
            file << ptr[i] << " ";
            if ((i+1)%10==0 || i==npts-1)
                file <<"\n";
        }
        file << endl;
    }

    c2p->Delete();
}
