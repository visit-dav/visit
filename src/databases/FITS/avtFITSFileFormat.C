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
//                            avtFITSFileFormat.C                            //
// ************************************************************************* //

#include <avtFITSFileFormat.h>

#include <string>

#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItUtility.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>

#include <snprintf.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <DebugStream.h>

static void
PrintError(int status)
{
    fits_report_error(stderr, status);
}

// ****************************************************************************
//  Method: avtFITS constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 19 09:54:58 PDT 2006
//
// ****************************************************************************

avtFITSFileFormat::avtFITSFileFormat(const char *filename)
    : avtSTSDFileFormat(filename), meshDimensions(), varToHDU(), varToMesh()
{
    fits = 0;
}

avtFITSFileFormat::~avtFITSFileFormat()
{
    if(fits != 0)
    {
        int status = 0;
        fits_close_file(fits, &status);
    }
}

// ****************************************************************************
//  Method: avtFITSFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 19 09:54:58 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtFITSFileFormat::FreeUpResources(void)
{
    if(fits != 0)
    {
        int status = 0;
        fits_close_file(fits, &status);
        fits = 0;

        meshDimensions.clear();
        varToHDU.clear();
        varToMesh.clear();
    }
}

// ****************************************************************************
// Method: avtFITSFileFormat::GetKeywordValue
//
// Purpose: 
//   Reads a string keyword value.
//
// Arguments:
//   keyword : The keyword we want to get.
//   value   : The return array for the value.
//
// Returns:    True if successful, false otherwise.
//
// Note:       We use this method instead of just calling fits_read_key
//             directly because some keys seem to contain trailing spaces
//             that must be part of the key for the fits_read_key
//             function to succeed. This method tries adding spaces until
//             the value is read.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 20 11:08:13 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
avtFITSFileFormat::GetKeywordValue(const char *keyword, char *value)
{
    std::string key(keyword);
    int start = key.size();
    char comment[FLEN_COMMENT];
    int status = 0;
    for(int i = start; i < FLEN_KEYWORD; ++i)
    {
        // Try and get the key value
        if(fits_read_key(fits, TSTRING, (char *)key.c_str(), value, 
           comment, &status) == 0)
        {
            return true;
        }
        else
            break;

        key += " ";
    }

    return false;
}

// ****************************************************************************
// Method: avtFITSFileFormat::Initialize
//
// Purpose: 
//   Read over the FITS file and populate the maps that the plugin uses
//   internally.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 19 10:49:46 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtFITSFileFormat::Initialize(avtDatabaseMetaData *md)
{
    const char *mName = "avtFITSFileFormat::Initialize: ";

    if(fits == 0)
    {
        debug4 << mName << "Opening " << filename << endl;
        int status = 0;
        if(fits_open_file(&fits, filename, 0, &status))
        {
            PrintError(status);
            EXCEPTION1(InvalidFilesException, filename);
        }

        char card[FLEN_CARD], tmp[100];
        std::string fileComment;
        int hdutype = 0;

        // Iterate over the HDU's
        for(int hdu = 1; !(fits_movabs_hdu(fits, hdu, &hdutype, &status)); hdu++) 
        {
            debug4 << mName << "Looking at HDU " << hdu << endl;

            // Get no. of keywords
            int nkeys = 0, keypos = 0;
            if(fits_get_hdrpos(fits, &nkeys, &keypos, &status))
                PrintError(status);

            if(hdutype == IMAGE_HDU)
            {
                debug4 << mName << "HDU " << hdu << " contains an image" << endl;
                int status2 = 0;
                char value[FLEN_VALUE];
                std::string objname, bunit, xlabel, ylabel, zlabel;

                // Try and get the key value for BUNIT
                if(GetKeywordValue("BUNIT", value))
                {
                    bunit = std::string(value);
                    debug4 << "\tBUNIT=" << value << endl;
                }

                // Try and get the key value for OBJECT
                if(GetKeywordValue("OBJECT", value))
                {
                    objname = std::string(value);
                    debug4 << "\tOBJECT=" << value << endl;
                }
#if 0
//
// Re-enable these someday when we read the mesh coordinates from the file
// and use them to construct a sensible mesh.
//
                // Try and get the key value for CTYPE1
                if(GetKeywordValue("CTYPE1", value))
                {
                    xlabel = std::string(value);
                    debug4 << "\tCTYPE1=" << value << endl;
                }

                // Try and get the key value for CTYPE2
                if(GetKeywordValue("CTYPE2", value))
                {
                    ylabel = std::string(value);
                    debug4 << "\tCTYPE2=" << value << endl;
                }

                // Try and get the key value for CTYPE3
                if(GetKeywordValue("CTYPE3", value))
                {
                    zlabel = std::string(value);
                    debug4 << "\tCTYPE3=" << value << endl;
                }
#endif
                // Use the keywords to create a comment.
                SNPRINTF(tmp, 100, "Header listing for HDU #%d:\n", hdu);
                fileComment += tmp;
                for(int jj = 1; jj <= nkeys; jj++)
                {
                    if(fits_read_record(fits, jj, card, &status))
                        PrintError(status);
                    std::string cs(card);
                    fileComment += cs;
                    if(cs[cs.size()-1] != '\n')
                        fileComment += "\n";
                }
                fileComment += "\n\n";

                //
                // Get the image's dimensions.
                //
                int ndims = 0;
                if(fits_get_img_dim(fits, &ndims, &status))
                    PrintError(status);
                debug4 << mName << "Num dimensions: " << ndims << ", status=" << status << endl;
                long *dims = new long[ndims];
                if(fits_get_img_size(fits, ndims, dims, &status))
                    PrintError(status);
                if(ndims == 0)
                {
                    debug4 << mName << "The image has no dimensions. Skip it." << endl;
                    continue;
                }

                //
                // Create a mesh name for the image.
                //
                intVector mdims;
                std::string meshName("image");
                debug4 << mName << "Image dimensions: ";
                bool dimensionsNonZero = false;
                for(int i = 0; i < ndims; ++i)
                {
                    char num[20];
                    if(i > 0)
                        SNPRINTF(num, 20, "x%d", (int)dims[i]);
                    else
                        SNPRINTF(num, 20, "%d", (int)dims[i]);
                    meshName += num;
                    mdims.push_back((int)dims[i]);
                    dimensionsNonZero |= (dims[i] != 0);
                    debug4 << dims[i] << ", ";
                }
                debug4 << endl;
                if(!dimensionsNonZero)
                {
                    debug4 << mName << "All dimensions were zero. skip." << endl;
                    continue;
                }
                if(ndims == 1 && objname != "")
                    meshName = objname;
                if(meshDimensions.find(meshName) == meshDimensions.end())
                {
                    meshDimensions[meshName] = mdims;

                    // Create metadata if necessary.
                    if(md != 0)
                    {
                        if(ndims == 1)
                        {
                            debug4 << mName << "Adding a curve called "
                                   << meshName.c_str() << " for HDU "
                                   << hdu << endl;

                            avtCurveMetaData *cmd = new avtCurveMetaData;
                            cmd->name = meshName;
                            if(xlabel != "")
                                cmd->xLabel = xlabel;
                            if(ylabel != "")
                                cmd->yLabel = ylabel;
                            md->Add(cmd);

                            // Do this because we use GetVar to read the data
                            // that we use to create the curve.
                            varToHDU[meshName] = hdu;
                            varToMesh[meshName] = meshName;
                        }
                        else
                        {
                            int sdims, tdims;
                            int nrealdims = (ndims <= 3) ? ndims : 3;
#define VECTOR_SUPPORT
#ifdef VECTOR_SUPPORT
                            if(nrealdims == 3 && dims[2] == 3)
                                nrealdims = 2;
#endif
                            sdims = tdims = nrealdims;

                            debug4 << mName << "Adding a " << sdims
                                   << " dimensional mesh called "
                                   << meshName.c_str() << " for HDU "
                                   << hdu << endl;

                            avtMeshMetaData *mmd = new avtMeshMetaData(
                                meshName, 1, 1, 1, 0, sdims, tdims,
                                AVT_RECTILINEAR_MESH);
                            if(xlabel != "")
                                mmd->xLabel = xlabel;
                            if(ylabel != "")
                                mmd->yLabel = ylabel;
                            if(zlabel != "")
                                mmd->zLabel = zlabel;

                            md->Add(mmd);
                        }
                    }
                }

                //
                // Create a name for the variable at this HDU
                //
                if(ndims > 1)
                {
                    char varname[100];
                    SNPRINTF(varname, 100, "hdu%d", hdu);
                    std::string vName(varname);
                    if(objname != "")
                        vName = objname;
                    varToHDU[vName] = hdu;
                    varToMesh[vName] = meshName;

                    // Create metadata if necessary
                    if(md != 0)
                    {
#ifdef VECTOR_SUPPORT
                        // Limit the dimensions to 3.
                        int ncomps = 1;
                        int nrealdims = (ndims <= 3) ? ndims : 3;
                        if(nrealdims == 3 && dims[2] == 3)
                            ncomps = 3;

                        if(ncomps == 1)
                        {
#endif
                            debug4 << mName << "Adding a scalar called "
                                   << vName.c_str() << " for HDU "
                                   << hdu << endl;
                            // Add the scalar metadata
                            avtScalarMetaData *smd = new avtScalarMetaData(
                                vName, meshName, AVT_ZONECENT);
                            smd->hasUnits = bunit != "";
                            smd->units = bunit;
                            md->Add(smd);
#ifdef VECTOR_SUPPORT
                        }
                        else
                        {
                            debug4 << mName << "Adding a color vector called "
                                   << vName.c_str() << " for HDU "
                                   << hdu << endl;

                            // Add the vector metadata
                            avtVectorMetaData *vmd = new avtVectorMetaData(
                                vName, meshName, AVT_ZONECENT, 4);
//                          vmd->hasUnits = true;
//                          vmd->units = 
                            md->Add(vmd);
                        }
#endif
                    }
                }

                delete [] dims;
            }
            else if(hdutype == ASCII_TBL)
            {
                debug4 << mName << "HDU " << hdu
                       << " contains an ascii table" << endl;
            }
            else if(hdutype == BINARY_TBL)
            {
                debug4 << mName << "HDU " << hdu
                       << " contains a binary table" << endl;

                // Use the keywords to create a comment.
                SNPRINTF(tmp, 100, "Header listing for HDU #%d:\n", hdu);
                debug4 << tmp;
                for(int jj = 1; jj <= nkeys; jj++)
                {
                    if(fits_read_record(fits, jj, card, &status))
                        PrintError(status);
                    std::string cs(card);
                    if(cs[cs.size()-1] != '\n')
                        cs += "\n";
                    debug4 << "\t" << cs.c_str();
                }
            }
        }

        if(md != 0)
            md->SetDatabaseComment(fileComment);
    }
}

// ****************************************************************************
//  Method: avtFITSFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 19 09:54:58 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtFITSFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    Initialize(md);

    if(meshDimensions.size() == 0)
    {
         const char *msg = "The file was opened but did not contain "
             "any image HDU's that VisIt can plot.";
         avtCallback::IssueWarning(msg);
         md->SetDatabaseComment(msg);
    }
}


// ****************************************************************************
//  Method: avtFITSFileFormat::GetMesh
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
//  Creation:   Wed Jul 19 09:54:58 PDT 2006
//
//  Modifications:
//    Kathleen Bonnell, Mon Jul 14 13:33:34 PDT 2008
//    Specify curves as 1D rectilinear grids with yvalues stored as point data.
//
// ****************************************************************************

vtkDataSet *
avtFITSFileFormat::GetMesh(const char *meshname)
{
    Initialize(0);

    vtkDataSet *retval = 0;
    MeshNameMap::const_iterator pos = meshDimensions.find(meshname);
    if(pos != meshDimensions.end())
    {
        if(pos->second.size() == 1)
        {
            // Read data for the curve.
            vtkFloatArray *cdata = (vtkFloatArray *)GetVar(meshname);

            // Try and read the variable making up the curve.
            int nPts = pos->second[0];
            vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(nPts, 
                                                                   VTK_FLOAT);
            vtkFloatArray *xc = 
                 vtkFloatArray::SafeDownCast(rg->GetXCoordinates()); 
            for (int j = 0 ; j < nPts ; j++)
            {
                xc->SetValue(j, (float)j);
            }
            rg->GetPointData()->SetScalars(cdata);
 
            cdata->Delete();

            retval = rg;
        }
        else
        {
            // Create a rectilinear mesh.
            int   i, j;
            vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();

            // Determine the mesh dimensions, taking into account that the mesh
            // may turn from 3D to 2D in the event that we look at colors.
            int mdims[3];
            int ndims = (pos->second.size() <= 3) ? pos->second.size() : 3;
#ifdef VECTOR_SUPPORT
            if(ndims == 3 && pos->second[2] == 3)
            {
                mdims[0] = pos->second[0];
                mdims[1] = pos->second[1];
                mdims[2] = 0;
            }
            else
#endif
            {
                for(int i = 0; i < 3; ++i)
                {
                    if(i < pos->second.size())
                        mdims[i] = pos->second[i];
                    else
                        mdims[i] = 1;
                }
            }

            //
            // Populate the coordinates.  Put in 3D points with z=0 if
            // the mesh is 2D.
            //
            int dims[3];
            vtkFloatArray *coords[3];
            for (i = 0 ; i < 3 ; i++)
            {
                // Default number of components for an array is 1.
                coords[i] = vtkFloatArray::New();

                if (i < ndims)
                {
                    dims[i] = mdims[i] + 1;
                    coords[i]->SetNumberOfTuples(dims[i]);
                    for (j = 0 ; j < dims[i] ; j++)
                    {
                        coords[i]->SetComponent(j, 0, j);
                    }
                }
                else
                {
                    dims[i] = 1;
                    coords[i]->SetNumberOfTuples(1);
                    coords[i]->SetComponent(0, 0, 0.);
                }
            }
            rgrid->SetDimensions(dims);
            rgrid->SetXCoordinates(coords[0]);
            coords[0]->Delete();
            rgrid->SetYCoordinates(coords[1]);
            coords[1]->Delete();
            rgrid->SetZCoordinates(coords[2]);
            coords[2]->Delete();

            retval = rgrid;
        }
    }
    else
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    return retval;
}


// ****************************************************************************
//  Method: avtFITSFileFormat::GetVar
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
//  Creation:   Wed Jul 19 09:54:58 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtFITSFileFormat::GetVar(const char *varname)
{
    vtkFloatArray *arr =0;

    Initialize(0);

    StringIntMap::const_iterator pos = varToHDU.find(varname);
    if(pos != varToHDU.end())
    {
        // Move to the right HDU.
        int hdutype = 0, status = 0;
        if(fits_movabs_hdu(fits, pos->second, &hdutype, &status))
        {
            PrintError(status);
            EXCEPTION1(InvalidVariableException, varname);
        }

        //
        // Determine the size of the image.
        //
        StringStringMap::const_iterator vit = varToMesh.find(varname);
        if(vit == varToMesh.end())
        {
            EXCEPTION1(InvalidVariableException, varname);
        }
        // Now that we have the mesh name, get the dimensions.
        MeshNameMap::const_iterator mit = meshDimensions.find(vit->second);
        if(mit == meshDimensions.end())
        {
            EXCEPTION1(InvalidVariableException, varname);
        }
        long nelements = 1;
        for(int i = 0; i < mit->second.size(); ++i)
            nelements *= mit->second[i];

        // Allocate the VTK return array.
        arr = vtkFloatArray::New();
        arr->SetNumberOfTuples(nelements);
        float *fptr = (float *)arr->GetVoidPointer(0); 

        // Now that we're at the right HDU, read the whole image array.
        float nulval = -1.; // Value to use for no data
        int anynul = 0;
        long fpixel = 1;
        if(fits_read_img(fits, TFLOAT, fpixel, nelements,
           &nulval, fptr, &anynul, &status))
        {
            PrintError(status);
            arr->Delete();
            EXCEPTION1(InvalidVariableException, varname);
        }
    }
    else
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    return arr;
}


// ****************************************************************************
//  Method: avtFITSFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 19 09:54:58 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtFITSFileFormat::GetVectorVar(const char *varname)
{
    const char *mName = "avtFITSFileFormat::GetVectorVar: ";
    vtkFloatArray *arr =0;

    Initialize(0);

    StringIntMap::const_iterator pos = varToHDU.find(varname);
    if(pos != varToHDU.end())
    {
        // Move to the right HDU.
        int hdutype = 0, status = 0;
        if(fits_movabs_hdu(fits, pos->second, &hdutype, &status))
        {
            PrintError(status);
            EXCEPTION1(InvalidVariableException, varname);
        }

        //
        // Determine the size of the image.
        //
        StringStringMap::const_iterator vit = varToMesh.find(varname);
        if(vit == varToMesh.end())
        {
            EXCEPTION1(InvalidVariableException, varname);
        }
        // Now that we have the mesh name, get the dimensions.
        MeshNameMap::const_iterator mit = meshDimensions.find(vit->second);
        if(mit == meshDimensions.end())
        {
            EXCEPTION1(InvalidVariableException, varname);
        }
        long nelements = 1, n_xy_elements = 1;
        int i;
        for(i = 0; i < mit->second.size(); ++i)
            nelements *= mit->second[i];
        for(i = 0; i < mit->second.size()-1; ++i)
            n_xy_elements *= mit->second[i];
        debug4 << mName << "Number of elements: " << nelements << endl;
        debug4 << mName << "Number of XY elements: " << n_xy_elements << endl;

        // Allocate the VTK return array. Note that we're making it allocate
        // a 4-component vector since that means color in VisIt.
        arr = vtkFloatArray::New();
        arr->SetNumberOfComponents(4);
        arr->SetNumberOfTuples(n_xy_elements);
        float *fptr = (float *)arr->GetVoidPointer(0); 
        debug4 << mName << "Allocated VTK memory. Reading data." << endl;

        // Now that we're at the right HDU, read the whole image array.
        float nulval = -1.; // Value to use for no data
        int anynul = 0;
        long fpixel = 1;
        float *pixels = new float[nelements];
        if(fits_read_img(fits, TFLOAT, fpixel, nelements,
           &nulval, pixels, &anynul, &status))
        {
            PrintError(status);
            arr->Delete();
            EXCEPTION1(InvalidVariableException, varname);
        }

        debug4 << "Data has been read. Reorder it into 4-components" << endl;

        // We've read the data as RGB into the RGBA array so we need to
        // rearrange a little. We can do this by shifting values towards
        // the end of the array while inserting alpha values.
        float *rgba = fptr;
        float *r = pixels;
        float *g = pixels + n_xy_elements;
        float *b = pixels + n_xy_elements*2;
        for(i = 0; i < n_xy_elements; ++i)
        {
            *rgba++ = *r++;
            *rgba++ = *g++;
            *rgba++ = *b++;
            *rgba++ = 255.;
        }

        delete [] pixels;
    }
    else
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    return arr;
}
