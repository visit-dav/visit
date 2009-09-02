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

// ************************************************************************* //
//                              avtTecplotWriter.C                           //
// ************************************************************************* //

#include <avtTecplotWriter.h>

#include <visit-config.h>

#include <vector>

#include <avtDatabaseMetaData.h>

#include <DebugStream.h>
#include <ImproperUseException.h>

#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDataSetWriter.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItCellDataToPointData.h>

#ifndef DBIO_ONLY
#include <Tetrahedralizer.h>
#endif

using     std::string;
using     std::vector;

#define FLOAT_COLUMN_WIDTH 14
#define INT_COLUMN_WIDTH   11

avtTecplotWriter::avtTecplotWriter()
{
    variablesWritten = false;
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
#if 0
    if (mmd->topologicalDimension != 3)
    {
        EXCEPTION1(ImproperUseException, "The Tecplot writer only supports "
                   "meshes with a topological dimension of 3.");
    }
#endif

    variableList = scalars;
    materialList = materials;

    // Check that we really have materials before we try writing the var list.
    if(!ReallyHasMaterials())
        materialList.clear();

    file << "TITLE = \"" << md->GetDatabaseName().c_str() << ": "
         << md->GetDatabaseComment().c_str() <<"\"" << endl;

    variablesWritten = false;

    // Write all of the floats from this point in scientific notation.
    file << std::scientific;
}

// ****************************************************************************
// Method: ReallyHasMaterialsEx
//
// Purpose: 
//   Traverses the data tree and determines if all of the nodes have materials.
//
// Arguments:
//   node  : The root node of the data tree.
//
// Returns:    true if all nodes have avtSubsets; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  2 12:00:03 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

static bool
ReallyHasMaterialsEx(avtDataTree_p node)
{
    if(node->GetNChildren() > 0)
    {
        for(int i = 0; i < node->GetNChildren(); ++i)
        {
            if(!ReallyHasMaterialsEx(node->GetChild(i)))
                return false;
        }
    }
    else
    {
        vtkDataSet *ds = node->GetDataRepresentation().GetDataVTK();
        if(ds->GetCellData()->GetArray("avtSubsets") == 0)
            return false;
    }

    return true;
}

bool
avtTecplotWriter::ReallyHasMaterials()
{
    return ReallyHasMaterialsEx(GetInputDataTree());
}

// ****************************************************************************
// Method: avtTecplotWriter::WriteVariables
//
// Purpose: 
//   Write the names of the variables.
//
// Arguments:
//   coordvars : The names of the coordinate variables.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  2 11:14:40 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtTecplotWriter::WriteVariables(const vector<string> &coordvars)
{
    if(!variablesWritten)
    {
        file << "VARIABLES = ";
        for(int i = 0; i < coordvars.size(); ++i)
        {
            file << "\"" << coordvars[i] << "\"";
            if(i < coordvars.size()-1 || (variableList.size()+materialList.size()) > 0)
                file << ", ";
        }

        for(int i = 0; i < variableList.size(); ++i)
        {
            file << "\"" << variableList[i] << "\"";
            if (i < variableList.size()-1 || (materialList.size() > 0))
                file << ", ";
        }

        if(materialList.size() > 0)
            file << "\"" << materialList[0] << "\"";

        file << endl;

        variablesWritten = true;
    }
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
//  Modifications:
//    Brad Whitlock, Tue Sep  1 13:58:43 PDT 2009
//    I added rectilinear grid support and polydata support.
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
         WriteRectilinearMesh((vtkRectilinearGrid *) ds, chunk);
         break;

       case VTK_POLY_DATA:
         WritePolyData((vtkPolyData *) ds, chunk);
         break;

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
//  Modifications:
//    Mark C. Miller, Tue Mar 10 11:15:29 PDT 2009
//    Added conditional compilation for dbio-only build. 
// ****************************************************************************

void
avtTecplotWriter::CloseFile(void)
{
    file.close();
}

// ****************************************************************************
// Method: avtTecplotWriter::WriteCurvilinearMesh
//
// Purpose: 
//   Writes one curvilinear mesh's data as a tecplot zone.
//
// Arguments:
//   sg : The curvilinear data to save.
//   chunk : The domain id of the dataset.
//
// Programmer: Jeremy Meredith
// Creation:   Wed Feb 9 13:44:32 PST 2005
//
// Modifications:
//   Brad Whitlock, Wed Sep  2 09:59:41 PDT 2009
//   I made the variables be written here instead of in WriteHeaders.
//
// ****************************************************************************

void
avtTecplotWriter::WriteCurvilinearMesh(vtkStructuredGrid *sg, int chunk)
{
    int dims[3];
    sg->GetDimensions(dims);

    // Write the variables line
    vector<string> coordVars;
    coordVars.push_back("I");
    coordVars.push_back("J");
    if(dims[2] > 1)
        coordVars.push_back("K");
    WriteVariables(coordVars);

    // Write the zone line
    file << "ZONE "
         << "T=\"DOMAIN "<<chunk<<"\", "
         << "I="<<dims[0]<<", "
         << "J="<<dims[1]<<", ";
    if(dims[2] > 1)
        file << "K="<<dims[2]<<", ";
    file << "F=BLOCK" <<endl;
    file << endl;

    int npts = sg->GetNumberOfPoints();
    vtkPoints *vtk_pts = sg->GetPoints();
    float *vtk_ptr = (float *) vtk_pts->GetVoidPointer(0);
    for (int d = 0; d < ((dims[2]>1) ? 3 : 2); d++)
    {
        for (int i=0; i<npts; i++)
        {
            file.width(FLOAT_COLUMN_WIDTH);
            file << vtk_ptr[3*i + d];
            if ((i+1)%10==0 || i==npts-1)
                file <<"\n";
        }
        file << endl;
    }

    WriteDataArrays(sg);
}

// ****************************************************************************
// Method: avtTecplotWriter::WriteRectilinearMesh
//
// Purpose: 
//   Writes one rectilinear mesh's data as a tecplot zone.
//
// Arguments:
//   rgrid : The rectilinear data to save.
//   chunk : The domain id of the dataset.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  2 10:02:07 PDT 2009
//
// Modifications:
//
// ****************************************************************************

void
avtTecplotWriter::WriteRectilinearMesh(vtkRectilinearGrid *rgrid, int chunk)
{
    int dims[3];
    rgrid->GetDimensions(dims);

    // Write the variables line
    vector<string> coordVars;
    coordVars.push_back("I");
    coordVars.push_back("J");
    if(dims[2] > 1)
        coordVars.push_back("K");
    WriteVariables(coordVars);

    // Write the zone line.
    file << "ZONE "
         << "T=\"DOMAIN "<<chunk<<"\", "
         << "I="<<dims[0]<<", "
         << "J="<<dims[1]<<", ";
    if(dims[2] > 1)
         file << "K="<<dims[2]<<", ";
    file << "F=BLOCK" <<endl;
    file << endl;

    double pt[3];
    for(int d = 0; d < ((dims[2] > 1) ? 3 : 2); ++d)
    {
        vtkIdType id = 0;
        vtkIdType npts_1 = dims[0] * dims[1] * dims[2] - 1;
        for(vtkIdType k = 0; k < rgrid->GetZCoordinates()->GetNumberOfTuples(); ++k)
        {
            pt[2] = rgrid->GetZCoordinates()->GetTuple1(k);
            for(vtkIdType j = 0; j < rgrid->GetYCoordinates()->GetNumberOfTuples(); ++j)
            {
                pt[1] = rgrid->GetYCoordinates()->GetTuple1(j);
                for(vtkIdType i = 0; i < rgrid->GetXCoordinates()->GetNumberOfTuples(); ++i,++id)
                {
                    pt[0] = rgrid->GetXCoordinates()->GetTuple1(i);
                    file.width(FLOAT_COLUMN_WIDTH);
                    file << pt[d];
                    if ((id+1)%10==0 || id==npts_1)
                        file <<"\n";
                }
            }
        }
    }

    WriteDataArrays(rgrid);
}

// ****************************************************************************
// Method: avtTecplotWriter::WriteUnstructuredMesh
//
// Purpose: 
//   Writes one unstructured mesh's data as a tecplot zone.
//
// Arguments:
//   sg : The unstructured data to save.
//   chunk : The domain id of the dataset.
//
// Programmer: Jeremy Meredith
// Creation:   Wed Feb 9 13:44:32 PST 2005
//
// Modifications:
//    Brad Whitlock, Wed Sep  2 10:04:15 PDT 2009
//    I made the variables get written here instead of in WriteHeaders. I also
//    added support for 2D.
//
// ****************************************************************************

void
avtTecplotWriter::WriteUnstructuredMesh(vtkUnstructuredGrid *ug, int chunk)
{
    int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    int npts = ug->GetNumberOfPoints();
    int nzones = ug->GetNumberOfCells();

    // Write the variables line
    vector<string> coordVars;
    coordVars.push_back("X");
    coordVars.push_back("Y");
    if(dim > 2)
        coordVars.push_back("Z");
    WriteVariables(coordVars);

    //
    // Count the various cell types.
    //
    int nhex = 0;
    int ntet = 0;
    int npyr = 0;
    int nwdg = 0;
    int ntri = 0;
    int nquad = 0;
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
          case VTK_TRIANGLE:
            ntri++;
            break;
          case VTK_QUAD:
            nquad++;
            break;
          default:
            // ignore the rest
            break;
        }
    }

    //
    // Look at the cell types and determine the dimensionality and whether
    // we need to subdivide the cells (such as when they're not the same type)
    //
    int nCellTypes3D = 0;
    nCellTypes3D += (nhex > 0) ? 1 : 0;
    nCellTypes3D += (ntet > 0) ? 1 : 0;
    nCellTypes3D += (nwdg > 0) ? 1 : 0;
    nCellTypes3D += (npyr > 0) ? 1 : 0;

    int nCellTypes2D = 0;
    nCellTypes2D += (ntri > 0) ? 1 : 0;
    nCellTypes2D += (nquad > 0) ? 1 : 0;

    int tdims = 3;
    if(nCellTypes3D == 0)
        tdims = 2;
    else if(nCellTypes2D > 0)
    {
        debug5 << "The Tecplot writer found an unstructured mesh containing "
                  "2D and 3D cell types. For simplicity, and for now, it will "
                  "only save the 3D cells." << endl;
    }

    bool subdivide = false;
    string elemType;
    if(tdims == 3)
    {
        subdivide = nCellTypes3D > 1;
        if(subdivide || ntet > 0)
            elemType = "TETRAHEDRON";
        else
            elemType = "BRICK";
    }
    else
    {
        subdivide = nCellTypes2D > 1;
        if(subdivide || ntri > 0)
            elemType = "TRIANGLE";
        else
            elemType = "QUADRILATERAL";
    }

    //
    // Determine the number of elements in the final output mesh.
    //
    int nelements = 0;
    if(subdivide)
    {
        for (int c = 0 ; c < nzones ; c++)
        {
            vtkCell *cell = ug->GetCell(c);
            int n = cell->GetNumberOfPoints();
            int ids[8];
            for (int i=0; i<n; i++)
                ids[i] = cell->GetPointId(i);

            int tetids[1000];

            // Do the connectivity
#ifndef DBIO_ONLY
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
              case VTK_TRIANGLE:
                if(tdims == 2)
                    nelements += 1;
                break;
              case VTK_QUAD:
                if(tdims == 2)
                    nelements += 2;
                break;
            }
#endif
        }
    }
    else if(tdims == 3)
        nelements =   ntet  +   nhex  +  npyr  +   nwdg;
    else if(tdims == 2)
        nelements = ntri + nquad;


    file << "ZONE "
         << "T=\"DOMAIN "<<chunk<<"\", "
         << "N="<<npts<<", "
         << "E="<<nelements<<", "
         << "F=FEBLOCK, "
         << "ET=" << elemType << endl;
    file << endl;

    vtkPoints *vtk_pts = ug->GetPoints();
    WritePoints(vtk_pts, dim);

    WriteDataArrays(ug);

    if (subdivide)
    {
        for (int c = 0 ; c < nzones ; c++)
        {
            vtkCell *cell = ug->GetCell(c);
            int n = cell->GetNumberOfPoints();
            int ids[8];
            for (int i=0; i<n; i++)
                ids[i] = cell->GetPointId(i);

            int subids[1000];
            int ntets = 0, ntris = 0;

            // Do the connectivity
#ifndef DBIO_ONLY
            switch (cell->GetCellType())
            {
              case VTK_HEXAHEDRON:
                ntets = Tetrahedralizer::GetLowTetNodesForHex(n,ids,subids);
                break;

              case VTK_WEDGE:
                ntets = Tetrahedralizer::GetLowTetNodesForWdg(n,ids,subids);
                break;

              case VTK_PYRAMID:
                ntets = Tetrahedralizer::GetLowTetNodesForPyr(n,ids,subids);
                break;

              case VTK_TETRA:
                ntets = Tetrahedralizer::GetLowTetNodesForTet(n,ids,subids);
                break;

              case VTK_TRIANGLE:
                subids[0] = 0;
                subids[1] = 1;
                subids[2] = 2;
                ntris = (tdims == 2) ? 1 : 0;
                break;

              case VTK_QUAD:
                subids[0] = 0;
                subids[1] = 1;
                subids[2] = 2;

                subids[3] = 0;
                subids[4] = 2;
                subids[5] = 3;
                ntris = (tdims == 2) ? 2 : 0;
                break;
            }
#endif

            for (int t = 0 ; t<ntets; t++)
            {
                for (int i = 0 ; i < 4 ; i++)
                {
                    file.width(INT_COLUMN_WIDTH);
                    file << ids[subids[t*4 + i]]+1;
                }
                file << endl;
            }

            for (int t = 0 ; t<ntris; t++)
            {
                for (int i = 0 ; i < 3 ; i++)
                {
                    file.width(INT_COLUMN_WIDTH);
                    file << ids[subids[t*3 + i]]+1;
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
            // hexes or tets for 3D and triangles or quads for 2D.
            // in either case, if we're here then the cell types
            // for all cells were the same.
            if (cell->GetCellType() == VTK_HEXAHEDRON ||
                cell->GetCellType() == VTK_TETRA ||
                cell->GetCellType() == VTK_TRIANGLE ||
                cell->GetCellType() == VTK_QUAD)
            {
                for (int j = 0 ; j < cell->GetNumberOfPoints() ; j++)
                {
                    file.width(INT_COLUMN_WIDTH);
                    file << cell->GetPointId(j)+1;
                }
                file << endl;
            }
        }
    }
}

// ****************************************************************************
// Method: avtTecplotWriter::WritePolyData
//
// Purpose: 
//   Saves a polydata dataset to Tecplot format.
//
// Arguments:
//   pd    : The polydata to save.
//   chunk : The polydata to save.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  2 14:22:26 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtTecplotWriter::WritePolyData(vtkPolyData *pd, int chunk)
{
    if(pd->GetLines()->GetNumberOfCells() > 0)
    {
        EXCEPTION1(ImproperUseException, "The polydata dataset contains lines "
            "and that is not currently supported in the Tecplot writer.");
    }

    // Search through the poly cells and see what we have.
    int ntri = 0, nquad = 0;
    pd->GetPolys()->InitTraversal();
    vtkIdType npts, *pts = 0;
    while(pd->GetPolys()->GetNextCell(npts, pts))
    {
        if(npts == 3)
            ntri++;
        else if(npts == 4)
            nquad++;
    }

    std::string elemType("TRIANGLE");
    int nelements = 0;
    bool subdivide = (ntri > 0 && nquad > 0);
    if(subdivide)
        nelements = ntri + 2 * nquad;
    else if(ntri > 0)
        nelements = ntri;
    else if(nquad > 0)
    {
        nelements = nquad;
        elemType="QUADRILATERAL";
    }

    // Write the variables line
    int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    vector<string> coordVars;
    coordVars.push_back("X");
    coordVars.push_back("Y");
    if(dim > 2)
        coordVars.push_back("Z");
    WriteVariables(coordVars);

    file << "ZONE "
         << "T=\"DOMAIN "<<chunk<<"\", "
         << "N="<<pd->GetPoints()->GetNumberOfPoints()<<", "
         << "E="<<nelements<<", "
         << "F=FEBLOCK, "
         << "ET=" << elemType << endl;
    file << endl;

    // Save the points
    WritePoints(pd->GetPoints(), dim);

    // Save the data.
    WriteDataArrays(pd);

    // Save the polygons.
    pd->GetPolys()->InitTraversal();
    while(pd->GetPolys()->GetNextCell(npts, pts))
    {
        if(npts == 3 || npts == 4)
        {
            if(subdivide && npts == 4)
            {
                static const int q2t[2][3] = {{0,1,2}, {0,2,3}};
                for(int i = 0; i < 3; ++i)
                {
                    file.width(INT_COLUMN_WIDTH);
                    file << pts[q2t[0][i]]+1;
                }
                file << endl;
                for(int i = 0; i < 3; ++i)
                {
                    file.width(INT_COLUMN_WIDTH);
                    file << pts[q2t[1][i]]+1;
                }
                file << endl;
            }
            else
            {
                for(int i = 0; i < npts; ++i)
                {
                    file.width(INT_COLUMN_WIDTH);
                    file << pts[i]+1;
                }
                file << endl;
            }            
        }        
    }
}

// ****************************************************************************
// Method: avtTecplotWriter::WriteDataArrays
//
// Purpose: 
//   Writes the data arrays for the dataset.
//
// Arguments:
//   ds1 : The dataset
//
// Programmer: Jeremy Meredith
// Creation:   Wed Feb 9 13:44:32 PST 2005
//
// Modifications:
//    Brad Whitlock, Wed Sep  2 10:04:15 PDT 2009
//    I changed how the numbers get formatted.
//
// ****************************************************************************

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
                file.width(FLOAT_COLUMN_WIDTH);
                file << ptr[i];
                if ((i+1)%10==0 || i==npts-1)
                    file <<"\n";
            }
            file << endl;
        }
    }

    if (materialList.size() > 0)
    {
        vtkDataArray *arr = pd2->GetArray("avtSubsets");
        if (!arr)
        {
            EXCEPTION1(ImproperUseException,"Couldn't find avtSubsets array.");
        }
        int *ptr = (int*)arr->GetVoidPointer(0);
        for (int i=0; i<npts; i++)
        {
            file.width(INT_COLUMN_WIDTH);
            file << ptr[i];
            if ((i+1)%10==0 || i==npts-1)
                file <<"\n";
        }
        file << endl;
    }

    c2p->Delete();
}

// ****************************************************************************
// Method: avtTecplotWriter::WritePoints
//
// Purpose: 
//   Write the VTK points to the file.
//
// Arguments:
//   pts : The points to write.
//   dim : How many of the coordinate dimensions should be written (2 or 3).
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  2 14:25:38 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtTecplotWriter::WritePoints(vtkPoints *pts, int dim)
{ 
    int npts = pts->GetNumberOfPoints();

    for (int d = 0; d < dim; d++)
    {
        float *vtk_ptr = (float *) pts->GetVoidPointer(0);
        vtk_ptr += d;

        for (int i=0; i<npts; i++, vtk_ptr += 3)
        {
            file.width(FLOAT_COLUMN_WIDTH);
            file << *vtk_ptr;
            if ((i+1)%10==0 || i==npts-1)
                file <<"\n";
        }
        file << endl;
    }
}
