/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

// ************************************************************************* //
//                            avtANALYZEFileFormat.C                         //
// ************************************************************************* //

#include <avtANALYZEFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>

#include <avtDatabaseMetaData.h>
#include <InvalidVariableException.h>
#include <DebugStream.h>

using     std::string;

// ****************************************************************************
// Function: reverse_endian
//
// Purpose:
//   Reverses the endian of the item that was passed in and returns the
//   reversed data.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 24 17:01:30 PST 2003
//
// Modifications:
//   
// ****************************************************************************

template <class T>
T
reverse_endian(const T &data)
{
    T retval;

    // Rearrange the bytes.
    unsigned char *dest = (unsigned char *)&retval;
    unsigned char *src = (unsigned char *)&data + sizeof(T) - 1;
    for(int i = 0; i < sizeof(T); ++i)
        *dest++ = *src--;

    return retval;
}

// ****************************************************************************
//  Method: avtANALYZE constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 24 16:51:26 PST 2003
//
// ****************************************************************************

avtANALYZEFileFormat::avtANALYZEFileFormat(const char *filename)
    : avtSTSDFileFormat(filename), fileInformation(), dataFilename(filename)
{
    initialized = false;
}

// ****************************************************************************
// Method: avtANALYZEFileFormat::Initialize
//
// Purpose:
//   Reads the header file and gets the information from it.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 24 17:29:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtANALYZEFileFormat::Initialize()
{
    if(!initialized)
    {
        //
        // The filename pointer could point to the header file name or the 
        // image filename. Make sure that we use the right one to open the
        // header and make sure that the dataFilename member contains the
        // name of the image file.
        //
        string headerFilename(filename);
        string extension(headerFilename.substr(headerFilename.size()-4, 4));
        if(extension == ".img")
        {
            headerFilename = headerFilename.substr(0, headerFilename.size()-4)
                           + ".hdr";
            // The extension is .img so make sure that the data filename
            // uses the .img extension.
            dataFilename = filename;
        }
        else
        {
            // The extension is .hdr, make sure that the data filename uses
            // the .img extension.
            dataFilename = headerFilename.substr(0, headerFilename.size()-4)
                           + ".img";
        }

        //
        // Try opening the header file.
        //
        if(!fileInformation.PopulateFromFile(headerFilename.c_str()))
        {
            string msg("Could not open "); msg += filename;
            EXCEPTION1(VisItException, msg);
        }
        initialized = true;
    }
}

// ****************************************************************************
// Method: avtANALYZEFileFormat::GetMeshExtents
//
// Purpose: 
//   Calculates the mesh's extents.
//
// Arguments:
//   extents : The return array for the extents.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 25 10:32:26 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtANALYZEFileFormat::GetMeshExtents(double *extents) const
{
    int dims[3];
    dims[0] = int(fileInformation.data.dime.dim[1]);
    dims[1] = int(fileInformation.data.dime.dim[2]);
    dims[2] = int(fileInformation.data.dime.dim[3]);
    debug1 << "avtANALYZEFileFormat::GetMeshExtents: mesh dims={" << dims[0]
           << ", " << dims[1] << ", " << dims[2] << "}" << endl;

    double halfx = double(dims[0]) * fileInformation.data.dime.pixdim[1];
    double halfy = double(dims[1]) * fileInformation.data.dime.pixdim[2];
    double halfz = double(dims[2]) * fileInformation.data.dime.pixdim[3];
    extents[0] = -halfx;
    extents[1] =  halfx;
    extents[2] = -halfy;
    extents[3] =  halfy;
    extents[4] = -halfz;
    extents[5] =  halfz;
}

// ****************************************************************************
//  Method: avtANALYZEFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 24 16:51:26 PST 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 25 11:35:32 PDT 2005
//    Added group origin to mesh metadata constructor.
//
// ****************************************************************************

void
avtANALYZEFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    Initialize();

    //
    // CODE TO ADD A MESH
    //
    string meshname = "Mesh";
    avtMeshType mt = AVT_RECTILINEAR_MESH;
    int nblocks = 1;
    int block_origin = 0;
    int spatial_dimension = 3;
    int topological_dimension = 3;
    double extents[6];
    GetMeshExtents(extents);
    avtMeshMetaData *mmd = new avtMeshMetaData(
        meshname, nblocks, block_origin, 0, 0, spatial_dimension,
        topological_dimension, mt);
    mmd->SetExtents(extents);
    string unitString(fileInformation.data.dime.vox_units);
    mmd->xUnits = unitString;
    mmd->yUnits = unitString;
    mmd->zUnits = unitString;
    md->Add(mmd);

    //
    // CODE TO ADD A SCALAR VARIABLE
    //
    string mesh_for_this_var = meshname;
    string varname = "Variable";
    avtCentering cent = AVT_NODECENT;
    AddScalarVarToMetaData(md, varname, mesh_for_this_var, cent);
}


// ****************************************************************************
//  Method: avtANALYZEFileFormat::GetMesh
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
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 24 16:51:26 PST 2003
//
// ****************************************************************************

vtkDataSet *
avtANALYZEFileFormat::GetMesh(const char *meshname)
{
    Initialize();

    //
    // Populate the coordinates.
    //
    vtkFloatArray *coords[3];
    double extents[6];
    GetMeshExtents(extents);
    int dims[3];
    dims[0] = int(fileInformation.data.dime.dim[1]);
    dims[1] = int(fileInformation.data.dime.dim[2]);
    dims[2] = int(fileInformation.data.dime.dim[3]);
    for (int i = 0 ; i < 3 ; i++)
    {
        // Default number of components for an array is 1.
        coords[i] = vtkFloatArray::New();
        coords[i]->SetNumberOfTuples(dims[i]);
        int i2 = i * 2;
        int i21 = i2 + 1;
        for (int j = 0 ; j < dims[i] ; j++)
        {
            float t = float(j) / float(dims[i] - 1);
            float c = (1.-t)*extents[i2] + t*extents[i21];
            coords[i]->SetComponent(j, 0, c);
        }
    }
    vtkRectilinearGrid *grid = vtkRectilinearGrid::New(); 
    grid->SetDimensions(dims);
    grid->SetXCoordinates(coords[0]);
    coords[0]->Delete();
    grid->SetYCoordinates(coords[1]);
    coords[1]->Delete();
    grid->SetZCoordinates(coords[2]);
    coords[2]->Delete();

    return grid;
}


// ****************************************************************************
//  Method: avtANALYZEFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 24 16:51:26 PST 2003
//
// ****************************************************************************

vtkDataArray *
avtANALYZEFileFormat::GetVar(const char *varname)
{
    Initialize();

    //
    // Try and open the data file.
    //
    vtkFloatArray *rv = 0;
    debug1 << "avtANALYZEFileFormat::GetVar: Opening data file: "
           << dataFilename.c_str() << endl;
    FILE *fp = fopen(dataFilename.c_str(), "rb");

    if(fp != 0)
    {
        int dims[3], nb = 0;
        dims[0] = int(fileInformation.data.dime.dim[1]);
        dims[1] = int(fileInformation.data.dime.dim[2]);
        dims[2] = int(fileInformation.data.dime.dim[3]);
        int ntuples = dims[0] * dims[1] * dims[2];
        rv = vtkFloatArray::New();
        rv->SetNumberOfTuples(ntuples);

        //
        // Read the data from the file, convert it to float, and put it
        // into the data array.
        //
        if(fileInformation.data.dime.datatype == DT_UNSIGNED_CHAR)
        {
            unsigned char *data = new unsigned char[ntuples];
            nb = fread((void *)data, ntuples * sizeof(unsigned char), 1, fp);
            for (int i = 0 ; i < ntuples ; i++)
                rv->SetTuple1(i, float(data[i]));
            delete [] data;
        }
        else if(fileInformation.data.dime.datatype == DT_SIGNED_SHORT)
        {
            short *data = new short[ntuples];
            nb = fread((void *)data, ntuples * sizeof(short), 1, fp);

            if(fileInformation.ReversedEndian())
            {
                for (int i = 0 ; i < ntuples ; i++)
                {
                    short tmp = reverse_endian(data[i]);
                    rv->SetTuple1(i, float(tmp));
                }
            }
            else
            {
                for (int i = 0 ; i < ntuples ; i++)
                    rv->SetTuple1(i, float(data[i]));
            }

            delete [] data;
        }
        else if(fileInformation.data.dime.datatype == DT_SIGNED_INT)
        {
            int *data = new int[ntuples];
            nb = fread((void *)data, ntuples * sizeof(int), 1, fp);

            if(fileInformation.ReversedEndian())
            {
                for (int i = 0 ; i < ntuples ; i++)
                {
                    int tmp = reverse_endian(data[i]);
                    rv->SetTuple1(i, float(tmp));
                }
            }
            else
            {
                for (int i = 0 ; i < ntuples ; i++)
                    rv->SetTuple1(i, float(data[i]));
            }
            delete [] data;
        }
        else if(fileInformation.data.dime.datatype == DT_FLOAT)
        {
            float *data = new float[ntuples];
            nb = fread((void *)data, ntuples * sizeof(float), 1, fp);

            if(fileInformation.ReversedEndian())
            {
                for (int i = 0 ; i < ntuples ; i++)
                {
                    float tmp = reverse_endian(data[i]);
                    rv->SetTuple1(i, tmp);
                }
            }
            else
            {
                for (int i = 0 ; i < ntuples ; i++)
                     rv->SetTuple1(i, data[i]);
            }

            delete [] data;
        }
        else
        {
            fclose(fp);
            debug1 << "Unsupported DSR data type. dime.datatype="
                   << fileInformation.data.dime.datatype << endl;
            EXCEPTION1(InvalidVariableException, varname);
        }
        fclose(fp);

        debug1 << "avtANALYZEFileFormat::GetVar: Read " << nb << " bytes from "
               << dataFilename.c_str() << endl;
    }
   
    return rv;
}

