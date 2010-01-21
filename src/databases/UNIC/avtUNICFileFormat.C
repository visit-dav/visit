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
//                             avtUNICFileFormat.C                           //
// ************************************************************************* //

#include <avtUNICFileFormat.h>

#include <string>

#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>

#include <snprintf.h>

// Define this symbol BEFORE including hdf5.h to indicate the HDF5 code
// in this file uses version 1.6 of the HDF5 API. This is harmless for
// versions of HDF5 before 1.8 and ensures correct compilation with
// version 1.8 and thereafter. When, and if, the HDF5 code in this file
// is explicitly upgraded to the 1.8 API, this symbol should be removed.
#define H5_USE_16_API
#include <hdf5.h>
#include <visit-hdf5.h>

using     std::string;


static int vtkCellType210ElementCount[20]={
    0,1,8,25,56,105,176,273,400,561,
    760,1001,1288,1625,2016,2465,2976,3553,4200,4921};
static int vtkCellType210NodeListLength[20]={
    0,5,40,127,288,545,920,1435,2112,2973,
    4040,5335,6880,8697,10808,13235,16000,19125,22632,26543};
#if 0
    //
    // This is the code fragment used to generate the values in
    // the above arrays.
    //
    int vtkCellType210ElementCount[20];
    int vtkCellType210NodeListLength[20];
    vtkCellType210ElementCount[0] = 0;
    vtkCellType210NodeListLength[0] = 0;
    for (int nx = 1; nx < 20; nx++)
    {
        int numElements2 = 0;
        int nlistLen = 0;
        for (j = 0; j < nx - 1; j++)
        {
            for (l = 0; l < nx - 1 - j; l++)
            {
                numElements2 += 3;
                nlistLen += 15;
            }
            numElements2 += 1;
            nlistLen += 5;
            for (k = 1; k < nx - 1 - j; k++)
            {
                for (l = 0; l < k; l++)
                {
                    numElements2 += 4;
                    nlistLen += 22;
                }
                numElements2 += 3;
                nlistLen += 15;
            }
            numElements2 += 3;
            nlistLen += 15;
        }
        numElements2 += 1;
        nlistLen += 5;
        vtkCellType210ElementCount[nx] = numElements2;
        vtkCellType210NodeListLength[nx] = nlistLen;
    }
    cerr << "int vtkCellType210ElementCount[20]={" << endl;
    for (i = 0; i < 20; i++)
        if (i == 9)
            cerr << vtkCellType210ElementCount[i] << "," << endl;
        else if (i == 19)
            cerr << vtkCellType210ElementCount[i] << "};" << endl;
        else
            cerr << vtkCellType210ElementCount[i] << ",";
    cerr << "int vtkCellType210NodeListLength[20]={" << endl;
    for (i = 0; i < 20; i++)
        if (i == 9)
            cerr << vtkCellType210NodeListLength[i] << "," << endl;
        else if (i == 19)
            cerr << vtkCellType210NodeListLength[i] << "};" << endl;
        else
            cerr << vtkCellType210NodeListLength[i] << ",";
#endif


// ****************************************************************************
//  Method: avtUNICFileFormat constructor
//
//  Programmer: childs -- generated by xml2avt
//  Creation:   Wed Jul 9 07:28:15 PDT 2008
//
//  Modifications:
//    Jeremy Meredith, Thu Jan  7 15:36:19 EST 2010
//    Close all open ids when returning an exception.
//
// ****************************************************************************

avtUNICFileFormat::avtUNICFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    file_handle = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file_handle < 0)
    {
        EXCEPTION1(InvalidDBTypeException, "Cannot be a UNIC data file, since "
                                           "it is not even an HDF5 file.");
    }
    int control = H5Dopen(file_handle, "CONTROL");
    if (control < 0)
    {
        H5Fclose(file_handle);
        EXCEPTION1(InvalidDBTypeException, "Cannot be a UNIC data file, since "
                                  "it is not contain the dataset \"control\".");    
    }

    H5Fclose(file_handle);
    file_handle = -1;
}


// ****************************************************************************
//  Method: avtUNICFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: childs -- generated by xml2avt
//  Creation:   Wed Jul 9 07:28:15 PDT 2008
//
// ****************************************************************************

void
avtUNICFileFormat::FreeUpResources(void)
{
    if (file_handle >= 0)
    {
        H5Fclose(file_handle);
        file_handle = -1;
    }
}


// ****************************************************************************
//  Method: avtUNICFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: childs -- generated by xml2avt
//  Creation:   Wed Jul 9 07:28:15 PDT 2008
//
// ****************************************************************************

void
avtUNICFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int  i, j;

    // Clean up from the last invocation, in case this method is called multiple times.
    cellvarnames.clear();
    ptvarnames.clear();

    string meshname = "mesh";
    avtMeshType mt = AVT_UNSTRUCTURED_MESH;

    //
    // We know the file is valid and it has "control" from the constructor.
    //
    if (file_handle < 0)
        file_handle = H5Fopen(filenames[0], H5F_ACC_RDONLY, H5P_DEFAULT);
    int control = H5Dopen(file_handle, "CONTROL");
    int info[5];
    int space_id = H5Dget_space(control);
    H5Dread(control, H5T_NATIVE_INT, H5S_ALL, space_id, H5P_DEFAULT, info);
    H5Dclose(control);

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = meshname;
    mesh->meshType = AVT_UNSTRUCTURED_MESH;
    mesh->numBlocks = info[0];
    mesh->blockOrigin = 1;
    spatialDim = info[1];
    mesh->spatialDimension = spatialDim;
    mesh->topologicalDimension = spatialDim;
    mesh->blockTitle = "Blocks";
    mesh->blockPieceName = "Block%012d";
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    int nPtVars   = info[2];
    int nCellVars = info[3];
    int slen      = info[4];
    char *pt_name_buff   = new char[nPtVars*slen+1];
    char *cell_name_buff = new char[nCellVars*slen+1];

    int pt_names = H5Dopen(file_handle, "VERTEX_VECTOR_NAMES");
    if (pt_names < 0)
    {
        EXCEPTION1(InvalidDBTypeException, "Cannot be a UNIC data file, since "
                     "it is not contain the dataset \"VERTEX_VECTOR_NAMES\".");    
    }
    hid_t type_id = H5Dget_type(pt_names);
    H5Dread(pt_names, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, pt_name_buff);
    //H5Dclose(pt_names);

    int cell_names = H5Dopen(file_handle, "ELEMENT_VECTOR_NAMES");
    if (cell_names < 0)
    {
        EXCEPTION1(InvalidDBTypeException, "Cannot be a UNIC data file, since "
                     "it is not contain the dataset \"ELEMENT_VECTOR_NAMES\".");
    }
    type_id = H5Dget_type(pt_names);
    H5Dread(cell_names, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, cell_name_buff);
    //H5Dclose(cell_names);

    char *fixed_name = new char[slen+1];
    fixed_name[slen] = '\0';
    for (i = 0 ; i < nCellVars ; i++)
    {
        char *name = cell_name_buff + slen*i;
        bool allSpaceSoFar = true;
        for (j = slen-1 ; j >= 0 ; j--)
            if (name[j] == ' ' && allSpaceSoFar)
                fixed_name[j] = '\0';
            else
            {
                fixed_name[j] = name[j];
                allSpaceSoFar = false;
            }

        cellvarnames.push_back(fixed_name);
        AddScalarVarToMetaData(md, fixed_name, meshname, AVT_ZONECENT);
    }

    for (i = 0 ; i < nPtVars ; i++)
    {
        char *name = pt_name_buff + slen*i;
        bool allSpaceSoFar = true;
        for (j = slen-1 ; j >= 0 ; j--)
            if (name[j] == ' ' && allSpaceSoFar)
                fixed_name[j] = '\0';
            else
            {
                fixed_name[j] = name[j];
                allSpaceSoFar = false;
            }

        ptvarnames.push_back(fixed_name);
        AddScalarVarToMetaData(md, fixed_name, meshname, AVT_NODECENT);
    }

    delete [] fixed_name;
    delete [] cell_name_buff;
    delete [] pt_name_buff;
}


// ****************************************************************************
//  Method: avtUNICFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: childs -- generated by xml2avt
//  Creation:   Wed Jul 9 07:28:15 PDT 2008
//
//  Modifications:
//
//    Hank Childs, Thu Jul 24 17:12:34 PDT 2008
//    Add support for 2D.
//
//    Eric Brugger, Thu Jan 21 10:09:12 PST 2010
//    I added support for a bunch of new element types.  The new element types
//    consist of tetrahedral, pyramid, wedge and hexahedral elements that go
//    from linear up to 20th order.  The non-linear elements are broken up
//    into linear elements.
//
// ****************************************************************************

vtkDataSet *
avtUNICFileFormat::GetMesh(int domain, const char *meshname)
{
    int  i, j, k, l;

    char blockname[1024];
    if (file_handle < 0)
        file_handle = H5Fopen(filenames[0], H5F_ACC_RDONLY, H5P_DEFAULT);
    SNPRINTF(blockname, 1024, "BLOCK%012d", domain+1);
    hid_t block = H5Gopen(file_handle, blockname);
    if (block < 0)
    {
        EXCEPTION1(InvalidDBTypeException, "Cannot locate BLOCK data");
    }

    int control = H5Dopen(block, "INFO");
    int info[3];
    H5Dread(control, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, info);
    H5Dclose(control);

    int numElements = info[0];
    int verticesPerElement = info[1];
    int numVertices = numElements*verticesPerElement;
    int theVtkCellType = info[2];

    float *xyz = new float[3*numVertices];
    int xyz_id = H5Dopen(block, "XYZ");
    H5Dread(xyz_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, xyz);
    H5Dclose(xyz_id);
    
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(numVertices);
    for (i = 0 ; i < numElements ; i++)
    {
        int base = spatialDim*verticesPerElement*i;
        for (j = 0 ; j < verticesPerElement ; j++)
        {
            int idx = verticesPerElement*i + j;
            pts->SetPoint(idx, xyz[base+j], xyz[base+verticesPerElement+j], 
                          (spatialDim == 2 ? 0. : xyz[base+2*verticesPerElement+j]));
        }
    }

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(pts);
    pts->Delete();

    vtkIdTypeArray *nlist = vtkIdTypeArray::New();
    vtkUnsignedCharArray *cellTypes = vtkUnsignedCharArray::New();
    vtkIdTypeArray *cellLocations = vtkIdTypeArray::New();
    if (theVtkCellType < 100)
    {
        nlist->SetNumberOfValues(numElements*(verticesPerElement+1));
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements);
        int *cl = cellLocations->GetPointer(0);

        for (i = 0 ; i < numElements ; i++)
        {
            *nl++ = verticesPerElement;
            for (j = 0 ; j < verticesPerElement ; j++)
                *nl++ = i*(verticesPerElement) + j;
            *ct++ = theVtkCellType;
            *cl++ = i*(verticesPerElement+1);
        }
    }
    else if (theVtkCellType == 100)
    {
        nlist->SetNumberOfValues(numElements*(verticesPerElement+1));
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements);
        int *cl = cellLocations->GetPointer(0);

        for (i = 0 ; i < numElements ; i++)
        {
            *nl++ = verticesPerElement;
            *nl++ = i*(verticesPerElement);
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 2;
            *ct++ = VTK_TRIANGLE;
            *cl++ = i*(verticesPerElement+1);
        }
    }
    else if (theVtkCellType == 101)
    {
        int verticesPerElement2 = 3;
        int numElements2 = numElements * 4;

        nlist->SetNumberOfValues(numElements2*(verticesPerElement2+1));
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements2);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements2);
        int *cl = cellLocations->GetPointer(0);

        int iCell = 0;
        for (i = 0 ; i < numElements ; i++)
        {
            *nl++ = verticesPerElement2;
            *nl++ = i*(verticesPerElement);
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 1;
            *ct++ = VTK_TRIANGLE;
            *cl++ = iCell*(verticesPerElement2+1);
            iCell++;

            *nl++ = verticesPerElement2;
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 3;
            *ct++ = VTK_TRIANGLE;
            *cl++ = iCell*(verticesPerElement2+1);
            iCell++;

            *nl++ = verticesPerElement2;
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 3;
            *nl++ = i*(verticesPerElement) + 2;
            *ct++ = VTK_TRIANGLE;
            *cl++ = iCell*(verticesPerElement2+1);
            iCell++;

            *nl++ = verticesPerElement2;
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 4;
            *nl++ = i*(verticesPerElement) + 3;
            *ct++ = VTK_TRIANGLE;
            *cl++ = iCell*(verticesPerElement2+1);
            iCell++;
        }
        numElements = numElements2;
    }
    else if (theVtkCellType >= 110 && theVtkCellType < 130)
    {
        int verticesPerElement2 = 3;
        int nx = theVtkCellType - 110 + 1;
        int numElements2 = numElements * nx * nx;

        nlist->SetNumberOfValues(numElements2*(verticesPerElement2+1));
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements2);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements2);
        int *cl = cellLocations->GetPointer(0);

        int iCell = 0;
        for (i = 0 ; i < numElements ; i++)
        {
            int voffset = 0;
            for (j = 0; j < nx; j++)
            {
                int width = nx + 1 - j;
                for (k = 0; k < nx - 1 - j; k++)
                {
                    *nl++ = verticesPerElement2;
                    *nl++ = i*(verticesPerElement) + voffset + k;
                    *nl++ = i*(verticesPerElement) + voffset + width + k;
                    *nl++ = i*(verticesPerElement) + voffset + k + 1;
                    *ct++ = VTK_TRIANGLE;
                    *cl++ = iCell*(verticesPerElement2+1);
                    iCell++;

                    *nl++ = verticesPerElement2;
                    *nl++ = i*(verticesPerElement) + voffset + width + k;
                    *nl++ = i*(verticesPerElement) + voffset + k + 1;
                    *nl++ = i*(verticesPerElement) + voffset + width + k + 1;
                    *ct++ = VTK_TRIANGLE;
                    *cl++ = iCell*(verticesPerElement2+1);
                    iCell++;
                }
                *nl++ = verticesPerElement2;
                *nl++ = i*(verticesPerElement) + voffset + nx - 1 - j;
                *nl++ = i*(verticesPerElement) + voffset + width + nx - 1 - j;
                *nl++ = i*(verticesPerElement) + voffset + nx - 1 - j + 1;
                *ct++ = VTK_TRIANGLE;
                *cl++ = iCell*(verticesPerElement2+1);
                iCell++;

                voffset += width;
            }
        }
        numElements = numElements2;
    }
    else if (theVtkCellType == 150)
    {
        nlist->SetNumberOfValues(numElements*(verticesPerElement+1));
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements);
        int *cl = cellLocations->GetPointer(0);

        for (i = 0 ; i < numElements ; i++)
        {
            *nl++ = verticesPerElement;
            *nl++ = i*(verticesPerElement);
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 2;
            *nl++ = i*(verticesPerElement) + 3;
            *ct++ = VTK_QUAD;
            *cl++ = i*(verticesPerElement+1);
        }
    }
    else if (theVtkCellType == 151)
    {
        int verticesPerElement2 = 3;
        int numElements2 = numElements * 6;

        nlist->SetNumberOfValues(numElements2*(verticesPerElement2+1));
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements2);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements2);
        int *cl = cellLocations->GetPointer(0);

        int iCell = 0;
        for (i = 0 ; i < numElements ; i++)
        {
            *nl++ = 3;
            *nl++ = i*(verticesPerElement);
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 7;
            *ct++ = VTK_TRIANGLE;
            *cl++ = iCell;
            iCell += 4;

            *nl++ = 3;
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 2;
            *nl++ = i*(verticesPerElement) + 3;
            *ct++ = VTK_TRIANGLE;
            *cl++ = iCell;
            iCell += 4;

            *nl++ = 3;
            *nl++ = i*(verticesPerElement) + 3;
            *nl++ = i*(verticesPerElement) + 4;
            *nl++ = i*(verticesPerElement) + 5;
            *ct++ = VTK_TRIANGLE;
            *cl++ = iCell;
            iCell += 4;

            *nl++ = 3;
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 6;
            *nl++ = i*(verticesPerElement) + 7;
            *ct++ = VTK_TRIANGLE;
            *cl++ = iCell;
            iCell += 4;

            *nl++ = 3;
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 3;
            *nl++ = i*(verticesPerElement) + 5;
            *ct++ = VTK_TRIANGLE;
            *cl++ = iCell;
            iCell += 4;

            *nl++ = 3;
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 7;
            *ct++ = VTK_TRIANGLE;
            *cl++ = iCell;
            iCell += 4;
        }
        numElements = numElements2;
    }
    else if (theVtkCellType >= 160 && theVtkCellType < 180)
    {
        int verticesPerElement2 = 4;
        int nx = theVtkCellType - 160 + 1;
        int numElements2 = numElements * nx * nx;

        nlist->SetNumberOfValues(numElements2*(verticesPerElement2+1));
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements2);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements2);
        int *cl = cellLocations->GetPointer(0);

        int iCell = 0;
        for (i = 0 ; i < numElements ; i++)
        {
            for (j = 0; j < nx; j++)
            {
                for (k = 0; k < nx; k++)
                {
                    *nl++ = verticesPerElement2;
                    *nl++ = i*(verticesPerElement) + j * (nx + 1) + k;
                    *nl++ = i*(verticesPerElement) + j * (nx + 1) + k + 1;
                    *nl++ = i*(verticesPerElement) + (j + 1) * (nx + 1) + k + 1;
                    *nl++ = i*(verticesPerElement) + (j + 1) * (nx + 1) + k;
                    *ct++ = VTK_QUAD;
                    *cl++ = iCell*(verticesPerElement2+1);
                    iCell++;
                }
            }
        }
        numElements = numElements2;
    }
    else if (theVtkCellType == 200)
    {
        nlist->SetNumberOfValues(numElements*(verticesPerElement+1));
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements);
        int *cl = cellLocations->GetPointer(0);

        for (i = 0 ; i < numElements ; i++)
        {
            *nl++ = verticesPerElement;
            *nl++ = i*(verticesPerElement);
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 2;
            *nl++ = i*(verticesPerElement) + 3;
            *ct++ = VTK_TETRA;
            *cl++ = i*(verticesPerElement+1);
        }
    }
    else if (theVtkCellType == 201)
    {
        int numElements2 = numElements * 6;

        nlist->SetNumberOfValues(numElements*32);
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements2);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements2);
        int *cl = cellLocations->GetPointer(0);

        int iCell = 0;
        for (i = 0 ; i < numElements ; i++)
        {
            *nl++ = 4;
            *nl++ = i*(verticesPerElement);
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 6;
            *ct++ = VTK_TETRA;
            *cl++ = iCell;
            iCell += 5;

            *nl++ = 4;
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 2;
            *nl++ = i*(verticesPerElement) + 3;
            *nl++ = i*(verticesPerElement) + 7;
            *ct++ = VTK_TETRA;
            *cl++ = iCell;
            iCell += 5;

            *nl++ = 4;
            *nl++ = i*(verticesPerElement) + 3;
            *nl++ = i*(verticesPerElement) + 4;
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 8;
            *ct++ = VTK_TETRA;
            *cl++ = iCell;
            iCell += 5;

            *nl++ = 4;
            *nl++ = i*(verticesPerElement) + 6;
            *nl++ = i*(verticesPerElement) + 7;
            *nl++ = i*(verticesPerElement) + 8;
            *nl++ = i*(verticesPerElement) + 9;
            *ct++ = VTK_TETRA;
            *cl++ = iCell;
            iCell += 5;

            *nl++ = 5;
            *nl++ = i*(verticesPerElement) + 3;
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 6;
            *nl++ = i*(verticesPerElement) + 7;
            *nl++ = i*(verticesPerElement) + 8;
            *ct++ = VTK_PYRAMID;
            *cl++ = iCell;
            iCell += 6;

            *nl++ = 5;
            *nl++ = i*(verticesPerElement) + 3;
            *nl++ = i*(verticesPerElement) + 7;
            *nl++ = i*(verticesPerElement) + 6;
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 1;
            *ct++ = VTK_PYRAMID;
            *cl++ = iCell;
            iCell += 6;
        }
        numElements = numElements2;
    }
    else if (theVtkCellType >= 210 && theVtkCellType < 230)
    {
        int nx = theVtkCellType - 210 + 1;
        int numElements2 = numElements * vtkCellType210ElementCount[nx];
        int nlistLen = numElements * vtkCellType210NodeListLength[nx];

        nlist->SetNumberOfValues(nlistLen);
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements2);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements2);
        int *cl = cellLocations->GetPointer(0);

        int iCell = 0;
        for (i = 0 ; i < numElements ; i++)
        {
            int hoffset = 0;
            for (j = 0; j < nx - 1; j++)
            {
                int dhoffset = (((nx-j) + 1) * ((nx-j) + 2)) / 2;
                int voffset = 0;
                int voffset2 = 0;
                int width = nx + 1 - j;

                for (l = 0; l < nx - 1 - j; l++)
                {
                    *nl++ = 4;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + l;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + l + 1;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + width + l;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + dhoffset + l;
                    *ct++ = VTK_TETRA;
                    *cl++ = iCell;
                    iCell += 5;

                    *nl++ = 4;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + width + l;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + width + l + 1;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + dhoffset + l + 1;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + l + 1;
                    *ct++ = VTK_TETRA;
                    *cl++ = iCell;
                    iCell += 5;

                    *nl++ = 4;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + width + l;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + dhoffset + l + 1;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + dhoffset + l;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + l + 1;
                    *ct++ = VTK_TETRA;
                    *cl++ = iCell;
                    iCell += 5;
                }
                l = nx - 1 - j;
                *nl++ = 4;
                *nl++ = i*(verticesPerElement) + hoffset +
                     voffset + l;
                *nl++ = i*(verticesPerElement) + hoffset +
                     voffset + l + 1;
                *nl++ = i*(verticesPerElement) + hoffset +
                     voffset + width + l;
                *nl++ = i*(verticesPerElement) + hoffset +
                     voffset + dhoffset + l;
                *ct++ = VTK_TETRA;
                *cl++ = iCell;
                iCell += 5;
                voffset += width;

                for (k = 1; k < nx - 1 - j; k++)
                {
                    int width = nx + 1 - k;

                    for (l = 0; l < k; l++)
                    {
                        *nl++ = 5;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset + l;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset2 + dhoffset + width + l;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset2 + dhoffset + width + l + 1;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset + l + 1;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset + width + l;
                        *ct++ = VTK_PYRAMID;
                        *cl++ = iCell;
                        iCell += 6;

                        *nl++ = 4;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset + l + 1;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset2 + dhoffset + width + l + 1;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset + width + l + 1;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset + width + l;
                        *ct++ = VTK_TETRA;
                        *cl++ = iCell;
                        iCell += 5;

                        *nl++ = 5;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset + l;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset + l + 1;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset2 + dhoffset + width + l + 1;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset2 + dhoffset + width + l;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset2 + dhoffset + l + 1;
                        *ct++ = VTK_PYRAMID;
                        *cl++ = iCell;
                        iCell += 6;

                        *nl++ = 4;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset + l;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset2 + dhoffset + width + l;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset2 + dhoffset + l;
                        *nl++ = i*(verticesPerElement) + hoffset +
                             voffset2 + dhoffset + l + 1;
                        *ct++ = VTK_TETRA;
                        *cl++ = iCell;
                        iCell += 5;
                    }
                    l = k;
                    *nl++ = 4;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + l;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset2 + dhoffset + l;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset2 + dhoffset + l + 1;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset2 + dhoffset + width + l;
                    *ct++ = VTK_TETRA;
                    *cl++ = iCell;
                    iCell += 5;

                    *nl++ = 4;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + l;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset2 + dhoffset + l + 1;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + l + 1;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset2 + dhoffset + width + l;
                    *ct++ = VTK_TETRA;
                    *cl++ = iCell;
                    iCell += 5;

                    *nl++ = 4;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + l;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + l + 1;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset + width + l;
                    *nl++ = i*(verticesPerElement) + hoffset +
                         voffset2 + dhoffset + width + l;
                    *ct++ = VTK_TETRA;
                    *cl++ = iCell;
                    iCell += 5;

                    voffset += width;
                    voffset2 += width;
                }

                *nl++ = 4;
                *nl++ = i*(verticesPerElement) + hoffset +
                     voffset;
                *nl++ = i*(verticesPerElement) + hoffset +
                     voffset + 1;
                *nl++ = i*(verticesPerElement) + hoffset +
                     voffset + 1 + 1;
                *nl++ = i*(verticesPerElement) + hoffset +
                     dhoffset + voffset2 + 1 + 1;
                *ct++ = VTK_TETRA;
                *cl++ = iCell;
                iCell += 5;

                *nl++ = 4;
                *nl++ = i*(verticesPerElement) + hoffset +
                     voffset;
                *nl++ = i*(verticesPerElement) + hoffset +
                     voffset + 1;
                *nl++ = i*(verticesPerElement) + hoffset +
                     dhoffset + voffset2 + 1;
                *nl++ = i*(verticesPerElement) + hoffset +
                     dhoffset + voffset2 + 1 + 1;
                *ct++ = VTK_TETRA;
                *cl++ = iCell;
                iCell += 5;

                *nl++ = 4;
                *nl++ = i*(verticesPerElement) + hoffset +
                     voffset;
                *nl++ = i*(verticesPerElement) + hoffset +
                     dhoffset + voffset2 + 1;
                *nl++ = i*(verticesPerElement) + hoffset +
                     dhoffset + voffset2;
                *nl++ = i*(verticesPerElement) + hoffset +
                     dhoffset + voffset2 + 1 + 1;
                *ct++ = VTK_TETRA;
                *cl++ = iCell;
                iCell += 5;

                hoffset += dhoffset;
            }

            *nl++ = 4;
            *nl++ = i*(verticesPerElement) + hoffset;
            *nl++ = i*(verticesPerElement) + hoffset + 1;
            *nl++ = i*(verticesPerElement) + hoffset + 2;
            *nl++ = i*(verticesPerElement) + hoffset + 3;
            *ct++ = VTK_TETRA;
            *cl++ = iCell;
            iCell += 5;
        }
        numElements = numElements2;
    }
    else if (theVtkCellType == 250)
    {
        nlist->SetNumberOfValues(numElements*(verticesPerElement+1));
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements);
        int *cl = cellLocations->GetPointer(0);

        for (i = 0 ; i < numElements ; i++)
        {
            *nl++ = verticesPerElement;
            *nl++ = i*(verticesPerElement);
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 2;
            *nl++ = i*(verticesPerElement) + 3;
            *nl++ = i*(verticesPerElement) + 4;
            *nl++ = i*(verticesPerElement) + 5;
            *ct++ = VTK_WEDGE;
            *cl++ = i*(verticesPerElement+1);
        }
    }
    else if (theVtkCellType == 251)
    {
        int numElements2 = numElements * 10;

        nlist->SetNumberOfValues(numElements*55);
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements2);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements2);
        int *cl = cellLocations->GetPointer(0);

        int iCell = 0;
        for (i = 0 ; i < numElements ; i++)
        {
            *nl++ = 4;
            *nl++ = i*(verticesPerElement);
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 6;
            *ct++ = VTK_TETRA;
            *cl++ = iCell;
            iCell += 5;

            *nl++ = 4;
            *nl++ = i*(verticesPerElement) + 9;
            *nl++ = i*(verticesPerElement) + 14;
            *nl++ = i*(verticesPerElement) + 10;
            *nl++ = i*(verticesPerElement) + 6;
            *ct++ = VTK_TETRA;
            *cl++ = iCell;
            iCell += 5;

            *nl++ = 5;
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 10;
            *nl++ = i*(verticesPerElement) + 14;
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 6;
            *ct++ = VTK_PYRAMID;
            *cl++ = iCell;
            iCell += 6;

            *nl++ = 4;
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 2;
            *nl++ = i*(verticesPerElement) + 3;
            *nl++ = i*(verticesPerElement) + 7;
            *ct++ = VTK_TETRA;
            *cl++ = iCell;
            iCell += 5;

            *nl++ = 4;
            *nl++ = i*(verticesPerElement) + 10;
            *nl++ = i*(verticesPerElement) + 12;
            *nl++ = i*(verticesPerElement) + 11;
            *nl++ = i*(verticesPerElement) + 7;
            *ct++ = VTK_TETRA;
            *cl++ = iCell;
            iCell += 5;

            *nl++ = 5;
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 3;
            *nl++ = i*(verticesPerElement) + 12;
            *nl++ = i*(verticesPerElement) + 10;
            *nl++ = i*(verticesPerElement) + 7;
            *ct++ = VTK_PYRAMID;
            *cl++ = iCell;
            iCell += 6;

            *nl++ = 4;
            *nl++ = i*(verticesPerElement) + 3;
            *nl++ = i*(verticesPerElement) + 4;
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 8;
            *ct++ = VTK_TETRA;
            *cl++ = iCell;
            iCell += 5;

            *nl++ = 4;
            *nl++ = i*(verticesPerElement) + 12;
            *nl++ = i*(verticesPerElement) + 14;
            *nl++ = i*(verticesPerElement) + 13;
            *nl++ = i*(verticesPerElement) + 8;
            *ct++ = VTK_TETRA;
            *cl++ = iCell;
            iCell += 5;

            *nl++ = 5;
            *nl++ = i*(verticesPerElement) + 3;
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 14;
            *nl++ = i*(verticesPerElement) + 12;
            *nl++ = i*(verticesPerElement) + 8;
            *ct++ = VTK_PYRAMID;
            *cl++ = iCell;
            iCell += 6;

            *nl++ = 6;
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 3;
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 10;
            *nl++ = i*(verticesPerElement) + 12;
            *nl++ = i*(verticesPerElement) + 14;
            *ct++ = VTK_WEDGE;
            *cl++ = iCell;
            iCell += 7;
        }
        numElements = numElements2;
    }
    else if (theVtkCellType >= 260 && theVtkCellType < 280)
    {
        int verticesPerElement2 = 6;
        int nx = theVtkCellType - 260 + 1;
        int numElements2 = numElements * nx * nx * nx;

        nlist->SetNumberOfValues(numElements2*(verticesPerElement2+1));
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements2);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements2);
        int *cl = cellLocations->GetPointer(0);

        int iCell = 0;
        int hoffset = ((nx + 1) * (nx + 2)) / 2;
        for (i = 0 ; i < numElements ; i++)
        {
            for (j = 0; j < nx; j++)
            {
                int voffset = 0;
                for (k = 0; k < nx; k++)
                {
                    int width = nx + 1 - k;
                    for (l = 0; l < nx - 1 - k; l++)
                    {
                        *nl++ = verticesPerElement2;
                        *nl++ = i*(verticesPerElement) + j*hoffset +
                             voffset + l;
                        *nl++ = i*(verticesPerElement) + j*hoffset +
                             voffset + l + 1;
                        *nl++ = i*(verticesPerElement) + j*hoffset +
                             voffset + width + l;
                        *nl++ = i*(verticesPerElement) + j*hoffset +
                             hoffset + voffset + l;
                        *nl++ = i*(verticesPerElement) + j*hoffset +
                             hoffset + voffset + l + 1;
                        *nl++ = i*(verticesPerElement) + j*hoffset +
                             hoffset + voffset + width + l;
                        *ct++ = VTK_WEDGE;
                        *cl++ = iCell*(verticesPerElement2+1);
                        iCell++;

                        *nl++ = verticesPerElement2;
                        *nl++ = i*(verticesPerElement) + j*hoffset +
                             voffset + width + l;
                        *nl++ = i*(verticesPerElement) + j*hoffset +
                             voffset + l + 1;
                        *nl++ = i*(verticesPerElement) + j*hoffset +
                             voffset + width + l + 1;
                        *nl++ = i*(verticesPerElement) + j*hoffset +
                             hoffset + voffset + width + l;
                        *nl++ = i*(verticesPerElement) + j*hoffset +
                             hoffset + voffset + l + 1;
                        *nl++ = i*(verticesPerElement) + j*hoffset +
                             hoffset + voffset + width + l + 1;
                        *ct++ = VTK_WEDGE;
                        *cl++ = iCell*(verticesPerElement2+1);
                        iCell++;
                    }
                    *nl++ = verticesPerElement2;
                    *nl++ = i*(verticesPerElement) + j*hoffset +
                             voffset + nx - 1 - k;
                    *nl++ = i*(verticesPerElement) + j*hoffset +
                             voffset + nx - 1 - k + 1;
                    *nl++ = i*(verticesPerElement) + j*hoffset +
                             voffset + width + nx - 1 - k;
                    *nl++ = i*(verticesPerElement) + j*hoffset +
                             hoffset + voffset + nx - 1 - k;
                    *nl++ = i*(verticesPerElement) + j*hoffset +
                             hoffset + voffset + nx - 1 - k + 1;
                    *nl++ = i*(verticesPerElement) + j*hoffset +
                             hoffset + voffset + width + nx - 1 - k;
                    *ct++ = VTK_WEDGE;
                    *cl++ = iCell*(verticesPerElement2+1);
                    iCell++;

                    voffset += width;
                }
            }
        }
        numElements = numElements2;
    }
    else if (theVtkCellType == 300)
    {
        nlist->SetNumberOfValues(numElements*(verticesPerElement+1));
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements);
        int *cl = cellLocations->GetPointer(0);

        for (i = 0 ; i < numElements ; i++)
        {
            *nl++ = verticesPerElement;
            *nl++ = i*(verticesPerElement);
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 2;
            *nl++ = i*(verticesPerElement) + 3;
            *nl++ = i*(verticesPerElement) + 4;
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 6;
            *nl++ = i*(verticesPerElement) + 7;
            *ct++ = VTK_HEXAHEDRON;
            *cl++ = i*(verticesPerElement+1);
        }
    }
    else if (theVtkCellType == 301)
    {
        nlist->SetNumberOfValues(numElements*(verticesPerElement+1));
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements);
        int *cl = cellLocations->GetPointer(0);

        for (i = 0 ; i < numElements ; i++)
        {
            *nl++ = verticesPerElement;
            *nl++ = i*(verticesPerElement);
            *nl++ = i*(verticesPerElement) + 2;
            *nl++ = i*(verticesPerElement) + 4;
            *nl++ = i*(verticesPerElement) + 6;
            *nl++ = i*(verticesPerElement) + 12;
            *nl++ = i*(verticesPerElement) + 14;
            *nl++ = i*(verticesPerElement) + 16;
            *nl++ = i*(verticesPerElement) + 18;
            *nl++ = i*(verticesPerElement) + 1;
            *nl++ = i*(verticesPerElement) + 3;
            *nl++ = i*(verticesPerElement) + 5;
            *nl++ = i*(verticesPerElement) + 7;
            *nl++ = i*(verticesPerElement) + 13;
            *nl++ = i*(verticesPerElement) + 15;
            *nl++ = i*(verticesPerElement) + 17;
            *nl++ = i*(verticesPerElement) + 19;
            *nl++ = i*(verticesPerElement) + 8;
            *nl++ = i*(verticesPerElement) + 9;
            *nl++ = i*(verticesPerElement) + 10;
            *nl++ = i*(verticesPerElement) + 11;
            *ct++ = VTK_QUADRATIC_HEXAHEDRON;
            *cl++ = i*(verticesPerElement+1);
        }
    }
    else if (theVtkCellType >= 310 && theVtkCellType < 330)
    {
        int verticesPerElement2 = 8;
        int nx = theVtkCellType - 310 + 1;
        int numElements2 = numElements * nx * nx * nx;

        nlist->SetNumberOfValues(numElements2*(verticesPerElement2+1));
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements2);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements2);
        int *cl = cellLocations->GetPointer(0);

        int iCell = 0;
        for (i = 0 ; i < numElements ; i++)
        {
            for (j = 0; j < nx; j++)
            {
                for (k = 0; k < nx; k++)
                {
                    for (l = 0; l < nx; l++)
                    {
                        *nl++ = verticesPerElement2;
                        *nl++ = i*(verticesPerElement) +
                             j*(nx+1)*(nx+1) + k*(nx+1) + l;
                        *nl++ = i*(verticesPerElement) +
                             j*(nx+1)*(nx+1) + k*(nx+1) + l+1;
                        *nl++ = i*(verticesPerElement) +
                             j*(nx+1)*(nx+1) + (k+1)*(nx+1) + l+1;
                        *nl++ = i*(verticesPerElement) +
                             j*(nx+1)*(nx+1) + (k+1)*(nx+1) + l;
                        *nl++ = i*(verticesPerElement) +
                             (j+1)*(nx+1)*(nx+1) + k*(nx+1) + l;
                        *nl++ = i*(verticesPerElement) +
                             (j+1)*(nx+1)*(nx+1) + k*(nx+1) + l+1;
                        *nl++ = i*(verticesPerElement) +
                             (j+1)*(nx+1)*(nx+1) + (k+1)*(nx+1) + l+1;
                        *nl++ = i*(verticesPerElement) +
                             (j+1)*(nx+1)*(nx+1) + (k+1)*(nx+1) + l;
                        *ct++ = VTK_HEXAHEDRON;
                        *cl++ = iCell*(verticesPerElement2+1);
                        iCell++;
                    }
                }
            }
        }
        numElements = numElements2;
    }
    else if (theVtkCellType >= 500 && theVtkCellType < 520)
    {
        int verticesPerElement2 = 2;
        int numElements2 = numElements * (verticesPerElement-1);

        nlist->SetNumberOfValues(numElements2*(verticesPerElement2+1));
        vtkIdType *nl = nlist->GetPointer(0);

        cellTypes->SetNumberOfValues(numElements2);
        unsigned char *ct = cellTypes->GetPointer(0);

        cellLocations->SetNumberOfValues(numElements2);
        int *cl = cellLocations->GetPointer(0);

        int iCell = 0;
        for (i = 0 ; i < numElements ; i++)
        {
            for (j = 0; j < verticesPerElement - 1; j++)
            {
                *nl++ = verticesPerElement2;
                *nl++ = i*(verticesPerElement) + j;
                *nl++ = i*(verticesPerElement) + j + 1;
                *ct++ = VTK_LINE;
                *cl++ = iCell*(verticesPerElement2+1);
                iCell++;
            }
        }
    }

    vtkCellArray *cells = vtkCellArray::New();
    cells->SetCells(numElements, nlist);
    nlist->Delete();

    ugrid->SetCells(cellTypes, cellLocations, cells);
    cellTypes->Delete();
    cellLocations->Delete();
    cells->Delete();

    return ugrid;
}


// ****************************************************************************
//  Method: avtUNICFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: childs -- generated by xml2avt
//  Creation:   Wed Jul 9 07:28:15 PDT 2008
//
//  Modifications:
//
//    Eric Brugger, Thu Jan 21 10:09:12 PST 2010
//    I added support for a bunch of new element types.  The new element types
//    consist of tetrahedral, pyramid, wedge and hexahedral elements that go
//    from linear up to 20th order.  The non-linear elements are broken up
//    into linear elements.
//
// ****************************************************************************

vtkDataArray *
avtUNICFileFormat::GetVar(int domain, const char *varname)
{
    int i, j;

    bool isPtVar = false;
    int  idx     = -1;

    for (i = 0 ; i < cellvarnames.size() ; i++)
        if (cellvarnames[i] == varname)
            idx = i;
    for (i = 0 ; i < ptvarnames.size() ; i++)
        if (ptvarnames[i] == varname)
        {
            idx = i;
            isPtVar = true;
        }

    if (idx < 0)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    char blockname[1024];
    if (file_handle < 0)
        file_handle = H5Fopen(filenames[0], H5F_ACC_RDONLY, H5P_DEFAULT);
    SNPRINTF(blockname, 1024, "BLOCK%012d", domain+1);
    hid_t block = H5Gopen(file_handle, blockname);
    if (block < 0)
    {
        EXCEPTION1(InvalidDBTypeException, "Cannot locate BLOCK data");
    }

    int control = H5Dopen(block, "INFO");
    int info[3];
    H5Dread(control, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, info);
    H5Dclose(control);

    int numElements = info[0];
    int verticesPerElement = info[1];
    int numVertices = numElements*verticesPerElement;
    int theVtkCellType = info[2];

    vtkDoubleArray *rv = vtkDoubleArray::New();

    int ntups, ntupsRead;
    double *valsRead;
    if (isPtVar)
    {
        ntups = numVertices;
        ntupsRead = ntups;
        rv->SetNumberOfTuples(ntups);
        valsRead = (double *) rv->GetVoidPointer(0);
    }
    else
    {
        if (theVtkCellType < 100 || theVtkCellType == 100 ||
            theVtkCellType == 150 || theVtkCellType == 200 ||
            theVtkCellType == 250 ||
            theVtkCellType == 300 || theVtkCellType == 301)
        {
            ntups = numElements;
            ntupsRead = ntups;
            rv->SetNumberOfTuples(ntups);
            valsRead = (double *) rv->GetVoidPointer(0);
        }
        else if (theVtkCellType == 101)
        {
            ntups = numElements * 4;
            ntupsRead = numElements;
            rv->SetNumberOfTuples(ntups);
            valsRead = new double[ntupsRead];
        }
        else if (theVtkCellType >= 110 && theVtkCellType < 130)
        {
            int nx = theVtkCellType - 110 + 1;
            ntups = numElements * nx * nx;
            ntupsRead = numElements;
            rv->SetNumberOfTuples(ntups);
            valsRead = new double[ntupsRead];
        }
        else if (theVtkCellType == 151)
        {
            ntups = numElements * 6;
            ntupsRead = numElements;
            rv->SetNumberOfTuples(ntups);
            valsRead = new double[ntupsRead];
        }
        else if (theVtkCellType >= 160 && theVtkCellType < 180)
        {
            int nx = theVtkCellType - 160 + 1;
            ntups = numElements * nx * nx;
            ntupsRead = numElements;
            rv->SetNumberOfTuples(ntups);
            valsRead = new double[ntupsRead];
        }
        else if (theVtkCellType == 201)
        {
            ntups = numElements * 6;
            ntupsRead = numElements;
            rv->SetNumberOfTuples(ntups);
            valsRead = new double[ntupsRead];
        }
        else if (theVtkCellType >= 210 && theVtkCellType < 230)
        {
            int nx = theVtkCellType - 210 + 1;
            ntups = numElements * vtkCellType210ElementCount[nx];
            ntupsRead = numElements;
            rv->SetNumberOfTuples(ntups);
            valsRead = new double[ntupsRead];
        }
        else if (theVtkCellType == 251)
        {
            ntups = numElements * 10;
            ntupsRead = numElements;
            rv->SetNumberOfTuples(ntups);
            valsRead = new double[ntupsRead];
        }
        else if (theVtkCellType >= 260 && theVtkCellType < 280)
        {
            int nx = theVtkCellType - 260 + 1;
            ntups = numElements * nx * nx * nx;
            ntupsRead = numElements;
            rv->SetNumberOfTuples(ntups);
            valsRead = new double[ntupsRead];
        }
        else if (theVtkCellType >= 310 && theVtkCellType < 330)
        {
            int nx = theVtkCellType - 310 + 1;
            ntups = numElements * nx * nx * nx;
            ntupsRead = numElements;
            rv->SetNumberOfTuples(ntups);
            valsRead = new double[ntupsRead];
        }
    }

#if HDF5_VERSION_GE(1,6,4)
    hsize_t offsets[2];
    hsize_t counts[2];
#else
    hssize_t offsets[2];
    hssize_t counts[2];
#endif

    int id = -1;
    if (isPtVar)
        id = H5Dopen(block, "VERTEXDATA");
    else
        id = H5Dopen(block, "ELEMENTDATA");

    hid_t dataspace = H5Dget_space(id);
    hid_t rank      = H5Sget_simple_extent_ndims(dataspace);
    hsize_t dims[2];
    int status_n   = H5Sget_simple_extent_dims(dataspace, dims, NULL);
    offsets[0] = idx;
    offsets[1] = 0;
    counts[0]  = 1;
    counts[1]  = ntupsRead;
    H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offsets, NULL, counts, NULL);

    dims[0] = 1;
    dims[1] = ntupsRead;
    hid_t memdataspace = H5Screate_simple(2, dims, NULL);

    H5Dread(id, H5T_NATIVE_DOUBLE, memdataspace, dataspace, H5P_DEFAULT,
            valsRead);

    H5Sclose(memdataspace);
    H5Sclose(dataspace);
    H5Dclose(id);

    if (ntups != ntupsRead)
    {
        int replicateFactor = ntups / ntupsRead;

        double *vals = (double *) rv->GetVoidPointer(0);
        for (i = 0; i < ntupsRead; i++)
        {
            for (j = 0; j < replicateFactor; j++)
            {
                *vals++ = valsRead[i];
            }
        }

        delete [] valsRead;
    }

    return rv;
}


// ****************************************************************************
//  Method: avtUNICFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: childs -- generated by xml2avt
//  Creation:   Wed Jul 9 07:28:15 PDT 2008
//
// ****************************************************************************

vtkDataArray *
avtUNICFileFormat::GetVectorVar(int domain, const char *varname)
{
    EXCEPTION1(InvalidVariableException, varname);
}


