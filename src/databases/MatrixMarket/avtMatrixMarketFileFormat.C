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
//                            avtMatrixMarketFileFormat.C                           //
// ************************************************************************* //

#include <avtMatrixMarketFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtDatabase.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

extern "C" {
#include "mmio.h"
}

using     std::string;


// ****************************************************************************
//  Method: avtMatrixMarketFileFormat constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   March 13, 2009
//
// ****************************************************************************

avtMatrixMarketFileFormat::avtMatrixMarketFileFormat(const char *fn)
    : avtSTSDFileFormat(fn)
{
    filename = fn;
    dataread = false;
    width = height = 0;
    matrix = NULL;
}


// ****************************************************************************
//  Method: avtMatrixMarketFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 13, 2009
//
// ****************************************************************************

void
avtMatrixMarketFileFormat::FreeUpResources(void)
{
    if (matrix)
        matrix->Delete();
    width = height = 0;
    dataread = false;
}


// ****************************************************************************
//  Method: avtMatrixMarketFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 13, 2009
//
// ****************************************************************************

void
avtMatrixMarketFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    ReadData();

    AddMeshToMetaData(md, "cellmesh", AVT_RECTILINEAR_MESH, NULL, 1,0, 2,2);
    AddScalarVarToMetaData(md, "cellvals", "cellmesh", AVT_ZONECENT);

    AddMeshToMetaData(md, "nodemesh", AVT_RECTILINEAR_MESH, NULL, 1,0, 2,2);
    AddScalarVarToMetaData(md, "nodevals", "nodemesh", AVT_NODECENT);
}


// ****************************************************************************
//  Method: avtMatrixMarketFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 13, 2009
//
// ****************************************************************************

vtkDataSet *
avtMatrixMarketFileFormat::GetMesh(const char *meshname)
{
    ReadData();

    int w=width;
    int h=height;

    // If we want the mesh as a cell-centered variable, we'll
    // just create the mesh one larger in each dimension
    if (string(meshname)=="cellmesh")
    {
        w++;
        h++;
    }

    // rectilinear grid
    vtkRectilinearGrid   *rgrid   = vtkRectilinearGrid::New(); 

    vtkFloatArray   *coords[3];
    int dims[3] = {w, h, 1};
    for (int i = 0 ; i < 3 ; i++)
    {
        coords[i] = vtkFloatArray::New();
        coords[i]->SetNumberOfTuples(dims[i]);

        for (int j = 0 ; j < dims[i] ; j++)
        {
            coords[i]->SetComponent(j, 0, j);
        }
    }
    rgrid->SetDimensions(dims);
    rgrid->SetXCoordinates(coords[0]);
    rgrid->SetYCoordinates(coords[1]);
    rgrid->SetZCoordinates(coords[2]);
    coords[0]->Delete();
    coords[1]->Delete();
    coords[2]->Delete();

    return rgrid;        

}


// ****************************************************************************
//  Method: avtMatrixMarketFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 13, 2009
//
// ****************************************************************************

vtkDataArray *
avtMatrixMarketFileFormat::GetVar(const char *varname)
{
    ReadData();

    matrix->Register(NULL);
    return matrix;
}


// ****************************************************************************
//  Method: avtMatrixMarketFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 13, 2009
//
//  Modifications:
//
//   Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL
// ****************************************************************************

vtkDataArray *
avtMatrixMarketFileFormat::GetVectorVar(const char *varname)
{
    return NULL;
}

void
avtMatrixMarketFileFormat::ReadData()
{
    if (dataread)
        return;

    FILE *f;
    if ((f=fopen(filename.c_str(), "r")) == NULL)
        EXCEPTION2(InvalidFilesException, filename, "File didn't exist.");

    TRY
    {
        // Get the matrix type
        MM_typecode matcode;
        if (mm_read_banner(f, &matcode) != 0)
            EXCEPTION2(InvalidFilesException, filename,
                       "Header was malformed.");

        // If we're on the mdserver, we're done and now assume a good
        // header is suffient
        if (!avtDatabase::OnlyServeUpMetaData())
        {
            // Read the types we support
            if (mm_is_matrix(matcode) &&
                mm_is_real(matcode) &&
                mm_is_array(matcode) &&
                mm_is_general(matcode))
            {
                // Dense, real-valued, general array
                mm_read_mtx_array_size(f, &height, &width);
                int size = width*height;
                matrix = vtkDoubleArray::New();
                matrix->SetNumberOfComponents(1);
                matrix->SetNumberOfTuples(size);
                double val;
                for (int j=0; j<width; j++)
                {
                    for (int i=0; i<height; i++)
                    {
                        int nread = fscanf(f, "%lg\n", &val);
                        if (nread != 1)
                            EXCEPTION2(InvalidFilesException, filename,
                                       "Premature EOF.");
                        matrix->SetComponent(i*width+j, 0, val);
                    }
                }
            }
            else if (mm_is_matrix(matcode) &&
                     mm_is_real(matcode) &&
                     mm_is_coordinate(matcode) &&
                     mm_is_general(matcode))
            {
                // Sparse, real-valued, general array
                int nvals;
                mm_read_mtx_crd_size(f, &height, &width, &nvals);
                int size = width*height;
                matrix = vtkDoubleArray::New();
                matrix->SetNumberOfComponents(1);
                matrix->SetNumberOfTuples(size);
                for (int e=0; e<size; e++)
                    matrix->SetComponent(e, 0, 0.0);
                for (int v=0; v<nvals; v++)
                {
                    int i, j;
                    double val;
                    int nread = fscanf(f, "%d %d %lg\n", &i, &j, &val);
                    --i; // 1-origin in the file, so
                    --j; // convert it to 0-origin
                    if (nread != 3)
                        EXCEPTION2(InvalidFilesException, filename,
                                   "Premature EOF.");
                    matrix->SetComponent(width*i+j, 0, val);
                }
            }
            else
            {
                // Not supported yet....
                EXCEPTION2(InvalidFilesException, filename,
                           "Unsupported matrix type.");
            }
        }
    }
    CATCHALL
    {
        fclose(f);
        RETHROW;
    }
    ENDTRY;

    fclose(f);
}
