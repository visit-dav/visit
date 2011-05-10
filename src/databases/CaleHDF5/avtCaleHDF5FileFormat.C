/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                            avtCaleHDF5FileFormat.C                        //
// ************************************************************************* //

#include <avtCaleHDF5FileFormat.h>

#include <string>
#include <vector>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

#include <avtGhostData.h>
#include <avtDatabaseMetaData.h>
#include <avtMixedVariable.h>
#include <avtVariableCache.h>

#include <Expression.h>
#include <DebugStream.h>

#include <InvalidVariableException.h>
#include <InvalidDBTypeException.h>

#define TRY_MIXED_SCALARS

using     std::string;

typedef enum {NO_TYPE, CHAR_TYPE, INTEGER_TYPE, FLOAT_TYPE, DOUBLE_TYPE,
              LONG_TYPE, CHARARRAY_TYPE, INTEGERARRAY_TYPE, FLOATARRAY_TYPE,
              DOUBLEARRAY_TYPE, LONGARRAY_TYPE, OBJECT_TYPE} TypeEnum;

// ****************************************************************************
// Method: avtCaleHDF5FileFormat::ReadHDF_Entry
//
// Purpose:
//   Read an entry from the HDF5 file.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Rob Managan
// Creation:   Fri Apr 29 11:29:35 PDT 2011
//
// Modifications:
//
// ****************************************************************************

hid_t
avtCaleHDF5FileFormat::ReadHDF_Entry(hid_t group, const char* name, void* ptr)
{
    char msg[128] ;
    hid_t hdf_dataset = H5Dopen(group, name, H5P_DEFAULT) ;
    if (hdf_dataset < 0)
    {
        sprintf(msg,"Error %d reading dataset for %s from file\n",hdf_dataset,name) ;
        EXCEPTION1(InvalidVariableException, msg) ;
    }
    hid_t hdf_type = H5Dget_type(hdf_dataset) ;
    if (hdf_type < 0.0)
    {
        sprintf(msg,"Error %d reading datatype for %s from file\n",hdf_type,name) ;
        EXCEPTION1(InvalidVariableException, msg) ;
    }
    herr_t hdf_err = H5Dread(hdf_dataset, hdf_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, ptr);
    if (hdf_err < 0.0)
    {
        sprintf(msg,"Error %d reading %s from file\n",hdf_err,name) ;
        EXCEPTION1(InvalidVariableException, msg) ;
    }
    hdf_err |= H5Dclose(hdf_dataset) ;
    if (hdf_err < 0.0)
    {
        sprintf(msg,"Error %d closing dataset, %s\n",hdf_err,name) ;
        EXCEPTION1(InvalidVariableException, msg) ;
    }

    return hdf_err ;
}

// ****************************************************************************
// Method: SymbolInformation
//
// Purpose:
//   Looks up information about a symbol.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Rob Managan
// Creation:   Fri Apr 29 11:29:35 PDT 2011
//
// Modifications:
//
// ****************************************************************************

bool
SymbolInformation(hid_t hdf_fid, const char *name, TypeEnum *t,
    std::string &typeString, int *nTotalElements, int **dimensions,
    int *nDims)
{
    bool retval = false;
    char msg[128] ;

    // Indicate that there is no type initially.
    if (t)
        *t = NO_TYPE;
    if (nTotalElements)
        *nTotalElements = 0;
    if (dimensions)
        *dimensions = 0;
    if (nDims)
        *nDims = 0;

    hid_t hdf_dataset = H5Dopen(hdf_fid, name, H5P_DEFAULT) ;
    debug4 << "HDF5File::SymbolExists: name=" << name << " dataset " << hdf_dataset << endl;
    if (hdf_dataset < 0.0)
    {
        sprintf(msg,"Error %d reading dataset for %s from file\n",hdf_dataset,name) ;
        EXCEPTION1(InvalidVariableException, msg) ;
    }
    hid_t hdf_type = H5Dget_type(hdf_dataset) ;
    debug4 << " hdf_type " << hdf_type << endl ;
    if (hdf_type < 0.0)
    {
        sprintf(msg,"Error %d reading datatype for %s from file\n",hdf_type,name) ;
        EXCEPTION1(InvalidVariableException, msg) ;
    }

    if( hdf_type >= 0)
    {
        int i = 0, nd = 0, length = 1;
        int *dims = 0;
        int *maxdims = 0;
        hid_t   hdf_dspace ;

        // Figure out the number of dimensions and the number of elements
        // that are in the entire array.
        // Print the dimensions to the debug log.

        hdf_dspace = H5Dget_space( hdf_dataset ) ;
        nd = H5Sget_simple_extent_ndims( hdf_dspace ) ;
        if (nd > 0)
        {
            // Store the dimensions of the array.
            std::vector<hsize_t> h_maxdims(nd), h_dims(nd);
            dims = new int[nd];
            nd = H5Sget_simple_extent_dims(hdf_dspace, &h_dims[0], &h_maxdims[0] ) ;
            for ( i = 0 ; i < nd ; i++ )
            {
                dims[i] = h_dims[i] ;
                length *= dims[i] ;
            }
        }
        else
        {
            dims = new int[1];
            dims[0] = 1;
        }

        // Print the dimensions to the debug log.
        debug4 << "dimensions={";
        for(i = 0; i < nd; ++i)
            debug4 << dims[i] << ", ";
        debug4 << "}" << endl;

        // Set some of the return values.
        if (dimensions)
            *dimensions = dims;
        if (nDims)
            *nDims = nd;
        if (nTotalElements)
            *nTotalElements = length;

        //
        // Take the storage type along with the length to determine the real
        // type that we want to report. Also allocate memory for the
        // variable.
        //
        if (t)
        {
            hsize_t size ;
            switch (H5Tget_class(hdf_type)) {
                case H5T_STRING:
                    *t = (length > 1) ? CHARARRAY_TYPE : CHAR_TYPE;
                    break;
                case H5T_INTEGER:
                    size = H5Tget_size(hdf_type);
                    if (size == sizeof(int))
                        *t = (length > 1) ? INTEGERARRAY_TYPE : INTEGER_TYPE;
                    else if (size == sizeof(long))
                        *t = (length > 1) ? LONGARRAY_TYPE : LONG_TYPE;
                    break;
                case H5T_FLOAT:
                    size = H5Tget_size(hdf_type);
                    if (size == sizeof(float))
                        *t = (length > 1) ? FLOATARRAY_TYPE : FLOAT_TYPE;
                    else if (size == sizeof(double))
                        *t = (length > 1) ? DOUBLEARRAY_TYPE : DOUBLE_TYPE;
                    break;
                default:
                    *t = OBJECT_TYPE;
             }
        }


        retval = true;
    }

    return retval;
}


// ****************************************************************************
// Method: avtCaleHDF5FileFormat::Identify
//
// Purpose:
//   Detects whether a file is a CaleHDF5 file. The routine throws exceptions if
//   the file is not a CaleHDF5 file. This routine is used in
//   cale_visitCommonPluginInfo::SetupDatabase. Doing it this way allows us to
//   check 1 file in a series for CALE conformanace without having to check
//   each and every file, making file loading *much* faster.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Rob Managan
// Creation:   Fri Apr 29 11:29:35 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void
avtCaleHDF5FileFormat::Identify(const char *filename)
{
    const char *mName = "avtCaleHDF5FileFormat::Identify: ";

    /* Save old error handler */
    /*herr_t (*old_func)(void*);*/
    H5E_auto_t old_func;
    void *old_client_data;

    /* Save old error handler */
    H5Eget_auto( H5E_DEFAULT, &old_func, &old_client_data);

    /* Turn off error handling to avoid library error messages if the file is not HDF5 */
    H5Eset_auto( H5E_DEFAULT, NULL, NULL);

    hid_t hdf_fid = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT) ;

    /* Restore previous error handler */
    H5Eset_auto( H5E_DEFAULT, old_func, old_client_data);

    debug5 << mName << "hdf_fid = " <<hdf_fid << endl ;

    if (hdf_fid < 0)
    {
       EXCEPTION1(InvalidDBTypeException, "The hdf5 file could not be opened, Identify") ;
    }
    else
    {
        int nnalls = 0, kmax = 0, lmax = 0;
        herr_t  hdf_err ;
        hdf_err  = ReadHDF_Entry(hdf_fid,"/parameters/nnalls",&nnalls) ;
        hdf_err |= ReadHDF_Entry(hdf_fid,"/parameters/kmax",&kmax) ;
        hdf_err |= ReadHDF_Entry(hdf_fid,"/parameters/lmax",&lmax) ;

        debug5 << mName << "err = " << hdf_err << endl ;
        debug5 << mName << "nnalls = " << nnalls << endl;
        debug5 << mName << "kmax = " << kmax << endl;
        debug5 << mName << "lmax = " << lmax << endl;

        if ((hdf_err < 0) || (nnalls != (kmax + 2)*(lmax + 2)))
        {
            H5Fclose(hdf_fid);
            EXCEPTION1(InvalidDBTypeException, "The file is not a CaleHDF5 dump") ;
        }
    }
    H5Fclose(hdf_fid);
}

// ****************************************************************************
//  Method: avtCaleHDF5FileFormat constructor
//
//  Programmer: Rob Managan
//  Creation:   Fri Apr 29 11:29:35 PDT 2011
//
// ****************************************************************************

avtCaleHDF5FileFormat::avtCaleHDF5FileFormat(const char *filename)
    : avtSTSDFileFormat(filename)
{
    kminmesh = kmaxmesh = lminmesh = lmaxmesh = -1 ;

    hdffile = 0;

}


// ****************************************************************************
//  Method: avtCaleHDF5FileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Rob Managan
//  Creation:   Thu Apr 21 15:29:31 PST 2011
//
// ****************************************************************************

void
avtCaleHDF5FileFormat::FreeUpResources(void)
{
    if(hdffile != 0)
        H5Fclose(hdffile);
    hdffile = 0;
}

// ****************************************************************************
// Method: avtCaleHDF5FileFormat::GetHDFFile
//
// Purpose:
//   Opens the HDF file if it's not already open.
//
// Returns:    The HDF file descriptor.
//
// Programmer: Rob Managan
// Creation:   Fri Apr 29 11:29:35 PDT 2011
//
//
// ****************************************************************************

hid_t
avtCaleHDF5FileFormat::GetHDF5File()
{
    if(hdffile == 0)
    {
        /* Save old error handler */
        /*herr_t (*old_func)(void*);*/
        H5E_auto_t old_func;
        void *old_client_data;

        /* Save old error handler */
        H5Eget_auto( H5E_DEFAULT, &old_func, &old_client_data);

        /* Turn off error handling to avoid library error messages if the file is not HDF5 */
        H5Eset_auto( H5E_DEFAULT, NULL, NULL);

        hdffile = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);

        /* Restore previous error handler */
        H5Eset_auto( H5E_DEFAULT, old_func, old_client_data);

        if (hdffile < 0)
        {
            EXCEPTION1(InvalidDBTypeException, "The hdf5 file could not be opened GetHDF5File") ;
        }
    }

    return hdffile;
}


// ****************************************************************************
//  Method: avtCaleHDF5FileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Rob Managan
//  Creation:   Thu Apr 21 15:29:31 PST 2011
//
// ****************************************************************************

void
avtCaleHDF5FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    //
    // CODE TO ADD A MESH
    //
    std::string meshname = "hydro";
    //

    // Add the mesh to the metadata. Note that this example will 
    // always expose a mesh called "hydro" to VisIt. A real 
    // plug-in may want to read a list of meshes from the data 
    // file. 
    avtMeshMetaData *mmd = new avtMeshMetaData;
    mmd->name = meshname;
    mmd->spatialDimension = 2;
    mmd->topologicalDimension = 2;
    mmd->meshType = AVT_CURVILINEAR_MESH;
    mmd->numBlocks = 1;
    mmd->xLabel = "z-axis";
    mmd->yLabel = "r-axis";
    // mmd->cellOrigin = 1;
    // mmd->nodeOrigin = 1;
    // mmd->blockOrigin = 1;
    md->Add(mmd);

    //
    // find portion of mesh used in this dump
    if (kminmesh == -1)
        GetUsedMeshLimits();

    //
    // CODE TO ADD A SCALAR VARIABLE
    // Now walk the fpa arrays and put them all out like wsilo.
    // access this list in the dump through the fpalist structure
    //
    int i, nfpa, nnalls, namix ;
    int npbin, ngrps, rdifmix;
    int iftmode ;
    hid_t hdf_err;
    typedef struct
    {
        char name[8];
        int len;
    }  parec;

    hdf_err =           ReadHDF_Entry(GetHDF5File(),"/parameters/nfpa",&nfpa);
    parec *palist = new parec[nfpa];
    hdf_err  = ReadHDF_Entry(GetHDF5File(),"/parameters/nnalls",&nnalls);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/namix",&namix);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/npbin",&npbin);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/rdifmix",&rdifmix);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/ngrps",&ngrps);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"fpalist",palist);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/iftmode",&iftmode);

    if (hdf_err < 0.0)
    {
        EXCEPTION1(InvalidDBTypeException,
            "Corrupt dump; error reading array related variables.");
    }

    std::string mesh_for_this_var = meshname; // ??? -- could be multiple meshes
    avtCentering cent = AVT_ZONECENT;

    for ( i = 0 ; i < nfpa ; i++ )
    {
        char  varname[10];
        char *name = palist[i].name;

        // debug4 << " i " << i << " name " << palist[i].name << " len "
        //        << palist[i].len << endl;

        // z & r go into the mesh data structure
        if (!strcmp(name, "z") || !strcmp(name, "r"))
            continue;

        if ((palist[i].len == nnalls) | (palist[i].len == namix))
        {
            debug4 << " adding scalar variable " << palist[i].name << " len "
                   << palist[i].len << endl;
            strncpy(varname,palist[i].name,8);
            varname[8] = '\0' ;
            if (!strcmp(varname, "rvel")   ||
                !strcmp(varname, "zvel")   ||
                !strcmp(varname, "vr")     ||
                !strcmp(varname, "vt")     ||
                !strcmp(varname, "rad")    ||
                !strcmp(varname, "theta")  ||
                !strcmp(varname, "vtheta") ||
                !strcmp(varname, "vmag")   ||
                !strcmp(varname, "bmhdr")  ||
                !strcmp(varname, "bmhdz")  ||
                !strcmp(varname, "jmhdr")  ||
                !strcmp(varname, "jmhdz")  ||
                !strcmp(varname, "omega")  ||
                !strcmp(varname, "r2w")    ||
                !strcmp(varname, "lt")      )
            {
                cent = AVT_NODECENT;
            }
            else
            {
                cent = AVT_ZONECENT;
            }
            // Add a scalar to the metadata.
            avtScalarMetaData *smd = new avtScalarMetaData;
            smd->name = varname;
            smd->meshName = mesh_for_this_var;
            smd->centering = cent;
            if (!strcmp(name, "rvel")   ||
                !strcmp(name, "zvel")   ||
                !strcmp(name, "vr")     ||
                !strcmp(name, "vt")     ||
                !strcmp(name, "vmag")   ||
                !strcmp(name, "speed"))
            {
                smd->hasUnits = true;
                smd->units = "cm/microsecond";
            }
            else if (!strcmp(name, "cs2"))
            {
                smd->hasUnits = true;
                smd->units = "cm^2/microsecond^2";
            }
            else if (!strcmp(name, "vort"))
            {
                smd->hasUnits = true;
                smd->units = "1/microsecond";
            }
            else if (!strcmp(name, "rad"))
            {
                smd->hasUnits = true;
                smd->units = "cm";
            }
            else if (!strcmp(name, "theta"))
            {
                smd->hasUnits = true;
                smd->units = "degrees";
            }
            else if (!strcmp(name, "zmass"))
            {
                smd->hasUnits = true;
                smd->units = "g";
            }
            else if (!strcmp(name, "den"))
            {
                smd->hasUnits = true;
                smd->units = "g/cm^3";
            }
            else if (!strcmp(name, "neden"))
            {
                smd->hasUnits = true;
                smd->units = "#/cm^3";
            }
            else if (!strcmp(name, "emat") ||
                     !strcmp(name, "eion") ||
                     !strcmp(name, "esum") ||
                     !strcmp(name, "enthal") ||
                     !strcmp(name, "laserdep"))
            {
                smd->hasUnits = true;
                smd->units = "Terg/g";
            }
            else if (!strcmp(name, "tkel"))
            {
                smd->hasUnits = true;
                smd->units = "Kelvin";
            }
            else if (!strcmp(name, "tmat") && iftmode == 0)
            {
                smd->hasUnits = true;
                smd->units = "Terg/ref cm^3";
            }
            else if (!strcmp(name, "tion") ||
                     (!strcmp(name, "tmat") && iftmode == 1))
            {
                smd->hasUnits = true;
                smd->units = "keV";
            }
            else if (!strcmp(name, "p")    ||
                     !strcmp(name, "pion") ||
                     !strcmp(name, "psum") ||
                     !strcmp(name, "pmag") ||
                     !strcmp(name, "pstag") ||
                     !strcmp(name, "qave") ||
                     !strcmp(name, "qb")   ||
                     !strcmp(name, "qt")   ||
                     !strcmp(name, "qr")   ||
                     !strcmp(name, "ql")   ||
                     !strcmp(name, "szz")  ||
                     !strcmp(name, "srr")  ||
                     !strcmp(name, "srz")  ||
                     !strcmp(name, "smu")  ||
                     !strcmp(name, "yld")  ||
                     !strcmp(name, "yrd")  ||
                     !strcmp(name, "s1")   ||
                     !strcmp(name, "s2")   ||
                     !strcmp(name, "s3")   ||
                     !strcmp(name, "tszz") ||
                     !strcmp(name, "tsrr") ||
                     !strcmp(name, "tsrz") ||
                     !strcmp(name, "ts1")  ||
                     !strcmp(name, "ts2")  ||
                     !strcmp(name, "ts3"))
            {
                smd->hasUnits = true;
                smd->units = "Mbar";
            }
            else if (!strcmp(name, "zmom")  ||
                     !strcmp(name, "rmom")  ||
                     !strcmp(name, "zmomp") ||
                     !strcmp(name, "rmomp") ||
                     !strcmp(name, "zmomn") ||
                !strcmp(name, "rmomn"))
            {
                smd->hasUnits = true;
                smd->units = "Terg/g";
            }
            else if (!strcmp(name, "lt"))
            {
                smd->hasUnits = true;
                smd->units = "microseconds";
            }
            else if (!strcmp(name, "bmhdt") ||
                     !strcmp(name, "bmhdr") ||
                     !strcmp(name, "bmhdz"))
            {
                smd->hasUnits = true;
                smd->units = "megagauss";
            }
            else if (!strcmp(name, "sigmhd"))
            {
                smd->hasUnits = true;
                smd->units = "1/(milliohm-cm)";
            }
            else if (!strcmp(name, "amhdt"))
            {
                smd->hasUnits = true;
                smd->units = "megagauss-cm^2";
            }
            else if (!strcmp(name, "jmhdt") ||
                     !strcmp(name, "jmhdr") ||
                     !strcmp(name, "jmhdz"))
            {
                smd->hasUnits = true;
                smd->units = "10^7 amps/cm^2";
            }
            else
            {
                smd->hasUnits = false;
            }
            md->Add(smd);
        }
        else if (palist[i].len == npbin*namix)
        {
            cent = AVT_ZONECENT;
            for (int g = 0 ; g < npbin ; g++ )
            {
                sprintf(varname,"%s_%02d",palist[i].name,g);
                debug4 << " adding scalar variable " << varname << " len "
                       << palist[i].len << endl;
                // Add a scalar to the metadata.
                avtScalarMetaData *smd = new avtScalarMetaData;
                smd->name = varname;
                smd->meshName = mesh_for_this_var;
                smd->centering = cent;
                md->Add(smd);
            }
        }
        else if (palist[i].len == ngrps*nnalls)
        {
            cent = AVT_ZONECENT;
            for (int g = 0 ; g < ngrps ; g++ )
            {
                sprintf(varname,"%s_%02d",palist[i].name,g);
                debug4 << " adding scalar variable " << varname << " len "
                       << palist[i].len << endl;
                // Add a scalar to the metadata.
                avtScalarMetaData *smd = new avtScalarMetaData;
                smd->name = varname;
                smd->meshName = mesh_for_this_var;
                smd->centering = cent;
                md->Add(smd);
            }
        }
        else
        {
            debug4 << " ignoring scalar variable " << palist[i].name << " len "
                   << palist[i].len << endl;
        }
    }
    delete [] palist;


    //
    // CODE TO ADD A MATERIAL
    //
    int nreg;
    typedef struct
    {
        char name[33];
    }  trcname_str;

    hdf_err = ReadHDF_Entry(GetHDF5File(),"/parameters/nreg",&nreg);
    if (hdf_err < 0.0)
    {
        EXCEPTION1(InvalidDBTypeException,
            "Corrupt dump; error reading # of materials.");
    }

    if (nreg >= 1)
    {
        trcname_str *rname = new trcname_str[nreg+1];

        hdf_err = ReadHDF_Entry(GetHDF5File(),"/ppa/rname",rname);
        if (hdf_err < 0.0)
        {
            EXCEPTION1(InvalidDBTypeException,
                "Corrupt dump; error reading material names.");
        }
        strcpy(rname[0].name,"Phony");

        int nmats = nreg + 1;
        avtMaterialMetaData *matmd = new avtMaterialMetaData;
        matmd->name = "Materials";
        matmd->meshName = meshname;
        matmd->numMaterials = nmats;

        for ( i = 0 ; i <= nreg ; i++ )
        {
            matmd->materialNames.push_back(rname[i].name);
            debug4 << " region " << i << " is named " << rname[i].name << endl;
        }
        md->Add(matmd);
    }

    // Here's the way to add expressions:
    Expression velocity_expr;
    velocity_expr.SetName("vel");
    velocity_expr.SetDefinition("{zvel, rvel}");
    velocity_expr.SetType(Expression::VectorMeshVar);
    md->AddExpression(&velocity_expr);

    int ifstr, ifstrain, iftpstr, ifmhda, ifmhdb;
    hdf_err =                      ReadHDF_Entry(GetHDF5File(), "/parameters/ifstr",&ifstr);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/ifstrain",&ifstrain);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/iftpstr",&iftpstr);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/ifmhda",&ifmhda);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/ifmhdb",&ifmhdb);
    if (hdf_err < 0.0)
    {
        EXCEPTION1(InvalidDBTypeException,
            "Corrupt dump; error reading memory layout parameters.");
    }

    if (ifstr == 1)
    {
        Expression stress_expr;
        stress_expr.SetName("stress");
        stress_expr.SetDefinition("{ { szz, srz }, { srz, srr } }");
        stress_expr.SetType(Expression::TensorMeshVar);
        md->AddExpression(&stress_expr);
    }

    if (ifstrain == 1)
    {
        Expression strain_dev_expr;
        strain_dev_expr.SetName("strain_dev");
        strain_dev_expr.SetDefinition("{ { strndzz, strndrz }, "
                                      "{ strndrz, strndrr } }");
        strain_dev_expr.SetType(Expression::TensorMeshVar);
        md->AddExpression(&strain_dev_expr);

        Expression strain_tot_expr;
        strain_tot_expr.SetName("strain_tot");
        strain_tot_expr.SetDefinition("{ { strntzz, strntrz }, "
                                      "{ strntrz, strntrr } }");
        strain_tot_expr.SetType(Expression::TensorMeshVar);
        md->AddExpression(&strain_tot_expr);
    }

    if (iftpstr == 1)
    {
        Expression stress_tot_expr;
        stress_tot_expr.SetName("stress_tot");
        stress_tot_expr.SetDefinition("{ { tszz, tsrz }, { tsrz, tsrr } }");
        stress_tot_expr.SetType(Expression::TensorMeshVar);
        md->AddExpression(&stress_tot_expr);
    }

    if (ifmhda == 1)
    {
        Expression B_field_expr;
        B_field_expr.SetName("B_field");
        B_field_expr.SetDefinition("{ bmhdz, bmhdr }");
        B_field_expr.SetType(Expression::VectorMeshVar);
        md->AddExpression(&B_field_expr);
    }

    if (ifmhdb == 1)
    {
        Expression current_expr;
        current_expr.SetName("current");
        current_expr.SetDefinition("{ jmhdz, jmhdr }");
        current_expr.SetType(Expression::VectorMeshVar);
        md->AddExpression(&current_expr);
    }
    // Use array_compose to group the different energy groups together
    if (npbin > 1)
    {
        Expression pbin_array_expr;
        pbin_array_expr.SetName("pbin_array");
        // std::string expr = "array_compose(expr1, expr2, ..., exprN)"
        std::string expr = "array_compose(";
        char sub_expr[33];
        for ( int g = 0 ; g < npbin ; g++ )
        {
            sprintf(sub_expr,"pbin_%02d",g);
            if (g < npbin-1)
                strcat(sub_expr,", ");
            expr += sub_expr;
        }
        strcpy(sub_expr,")");
        expr += sub_expr;
        pbin_array_expr.SetDefinition(expr);
        pbin_array_expr.SetType(Expression::ArrayMeshVar);
        md->AddExpression(&pbin_array_expr);

        Expression kbin_array_expr;
        kbin_array_expr.SetName("kbin_array");
        // std::string expr = "array_compose(expr1, expr2, ..., exprN)"
        expr = "array_compose(";
        for ( int g = 0 ; g < npbin ; g++ )
        {
            sprintf(sub_expr,"kbin_%02d",g);
            if (g < npbin-1)
                strcat(sub_expr,", ");
            expr += sub_expr;
        }
        strcpy(sub_expr,")");
        expr += sub_expr;
        kbin_array_expr.SetDefinition(expr);
        kbin_array_expr.SetType(Expression::ArrayMeshVar);
        md->AddExpression(&kbin_array_expr);
    }
    if (ngrps > 1)
    {
        Expression flux_array_expr;
        flux_array_expr.SetName("nflux_array");
        // std::string expr = "array_compose(expr1, expr2, ..., exprN)"
        std::string expr = "array_compose(";
        char sub_expr[33] ;
        for ( int g = 0 ; g < ngrps ; g++ )
        {
            sprintf(sub_expr,"nflux_%02d",g);
            if (g < ngrps-1)
                strcat(sub_expr,", ");
            expr += sub_expr;
        }
        strcpy(sub_expr,")");
        expr += sub_expr;
        flux_array_expr.SetDefinition(expr);
        flux_array_expr.SetType(Expression::ArrayMeshVar);
        md->AddExpression(&flux_array_expr);
    }
    //
    // Add time history curve data
    //

    int ntp, ncp, ncurves;
    hdf_err =                      ReadHDF_Entry(GetHDF5File(),"/parameters/ntp",&ntp);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/ncp",&ncp);
    if (hdf_err < 0.0)
    {
        ntp = ncp = 0;
    }

    debug4 << ntp << " time plot sections" << endl;
    typedef struct {
       char tplab[30] ; /*!< curve label */
       int type ;       /*!< the data type */
       int tplen ;      /*!< # points stored so far */
       hvl_t h5_tpdat_arr ; /*!< the curve data */
       int tpx ;       /*!< unused */
       }   h5_tpcur ;
    typedef struct {
       int ntimes ;    /*!< max number of times allowed */
       int ncurs ;     /*!< number of curves */
       }   h5_tpdat ;

    h5_tpcur *h5_tpcurve ;
    h5_tpdat *h5_tpdata = new h5_tpdat[ntp] ;
    h5_tpdat *h5_cpdata = new h5_tpdat[ncp] ;

    if (ntp > 0)
        ReadHDF_Entry(GetHDF5File(),"/ppa/tpdata",h5_tpdata) ;

    for ( int i = 1 ; i <= ntp ; i++ )
    {
        char dataname[128];
        ncurves    = h5_tpdata[i-1].ncurs ;
        h5_tpcurve = new h5_tpcur[ncurves] ;

        sprintf(dataname,"/ppa/tpcurs_%d",i) ;

        hdf_err =  ReadHDF_Entry(GetHDF5File(), dataname, h5_tpcurve) ;

        debug4 << "section " << i << " has " << ncurves << " curves " << endl;

        for ( int icur = 1 ; icur < ncurves ; icur++ )
        {
            char *label = h5_tpcurve[icur].tplab ;
            debug4 << "label '" << label << "'" << endl;
            //if (h5_tpcurve[icur].tplen == 0)
            //    break;
            avtCurveMetaData *cmd = new avtCurveMetaData;
            cmd->name = label;
            // Labels and units are strings so use whatever you want.
            cmd->yLabel = "y-axis";
            // cmd->yUnits = "cm"
            cmd->xLabel = "time";
            cmd->xUnits = "microseconds";
            md->Add(cmd);
        }
        delete [] h5_tpcurve ;
    }

    debug4 << ncp << " cycle plot sections" << endl;

    if (ncp > 0)
        ReadHDF_Entry(GetHDF5File(),"/ppa/cpdata",h5_cpdata) ;

    for ( int i = 1 ; i <= ncp ; i++ )
    {
        char dataname[128];
        ncurves    = h5_cpdata[i-1].ncurs ;
        h5_tpcurve = new h5_tpcur[ncurves] ;

        sprintf(dataname,"/ppa/cpcurs_%d",i) ;

        hdf_err =  ReadHDF_Entry(GetHDF5File(), dataname, h5_tpcurve) ;

        debug4 << "section " << i << " has " << ncurves << " curves " << endl;

        for ( int icur = 1 ; icur < ncurves ; icur++ )
        {
            char *label = h5_tpcurve[icur].tplab ;
            debug4 << "label '" << label << "'" << endl;
            //if (h5_tpcurve[icur].tplen == 0)
            //    break;
            avtCurveMetaData *cmd = new avtCurveMetaData;
            cmd->name = label;
            // Labels and units are strings so use whatever you want.
            cmd->yLabel = "y-axis";
            // cmd->yUnits = "cm"
            cmd->xLabel = "time";
            cmd->xUnits = "microseconds";
            md->Add(cmd);
        }
        delete [] h5_tpcurve ;
    }
    delete [] h5_tpdata;
    delete [] h5_cpdata;

}


// ****************************************************************************
//  Method: avtCaleHDF5FileFormat::GetMesh
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
//  Programmer: Rob Managan
//  Creation:   Thu Apr 21 15:29:31 PST 2011
//
// ****************************************************************************

vtkDataSet *
avtCaleHDF5FileFormat::GetMesh(const char *meshname)
{
    const char *mName = "avtCaleHDF5FileFormat::GetMesh: ";
    // Determine which mesh to return.
    if (strcmp(meshname, "hydro") == 0)
    {
        // Create a VTK object for "hydro" mesh
        int ndims = 2;
        int dims[3] = {1,1,1};
        int kmax, lmax, lp, nnalls = 0, namix;
        int nk, nl, hdf_err;

        if (kminmesh == -1)
            GetUsedMeshLimits();
        // Read the ndims and number of X,Y,Z nodes from file.
        hdf_err =                     ReadHDF_Entry(GetHDF5File(),"/parameters/kmax",&kmax);
        if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/lmax",&lmax);
        if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/lp",&lp);
        if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/nnalls",&nnalls);
        if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/namix",&namix);
        double *z = new double[nnalls];
        double *r = new double[nnalls];
        if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/arrays/z",z);
        if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/arrays/r",r);
        if (hdf_err < 0.0)
        {
            EXCEPTION1(InvalidDBTypeException,
                       "Corrupt dump; error reading mesh related parameters.");
        }

        nk = kmaxmesh - kminmesh + 1 ;
        nl = lmaxmesh - lminmesh + 1 ;
        dims[0] = nk ;
        dims[1] = nl ;
        dims[2] = 1 ;
        int nnodes = dims[0]*dims[1]*dims[2];
        // Read the X coordinates from the file.
        double *xarray = new double[nnodes];

        // Read the Y coordinates from the file.
        double *yarray = new double[nnodes];

        // debug4 << "nnalls " << nnalls << " nnodes " << nnodes
        //        << " kmax,lmax " << kmax <<","<< lmax << " lp "<< lp << endl;
        // debug4 << "kminmesh " << kminmesh << " kmaxmesh " << kmaxmesh
        //        << " lminmesh " << lminmesh <<" lmaxmesh "<< lmaxmesh << endl;

        for ( int l = 0 ; l < nl ; l++ )
        {
            for ( int k = 0 ; k < nk ; k++ )
            {
                xarray[l*nk + k] = z[(l+lminmesh)*lp + k+kminmesh];
                yarray[l*nk + k] = r[(l+lminmesh)*lp + k+kminmesh];
            }
        }
        //
        // Create the vtkStructuredGrid and vtkPoints objects.
        //
        vtkStructuredGrid *sgrid  = vtkStructuredGrid::New();
        vtkPoints         *points = vtkPoints::New();
        sgrid->SetPoints(points);
        sgrid->SetDimensions(dims);
        points->Delete();
        points->SetNumberOfPoints(nnodes);
        //
        // Copy the coordinate values into the vtkPoints object.
        //
        float *pts = (float *) points->GetVoidPointer(0);
        double *xc = xarray;
        double *yc = yarray;
        for(int j = 0; j < dims[1]; ++j)
            for(int i = 0; i < dims[0]; ++i)
            {
                *pts++ = *xc++;
                *pts++ = *yc++;
                *pts++ = 0.;
            }

        // Delete temporary arrays
        delete [] xarray;
        delete [] yarray;
        delete [] z;
        delete [] r;

        // Now that you have your mesh, figure out which cells need
        // to be removed.

        int nCells = sgrid->GetNumberOfCells();
        int *blanks = new int[namix];
        hdf_err = ReadHDF_Entry(GetHDF5File(),"/arrays/ireg",blanks);

        debug4 << mName <<"nCells " << nCells << " nnalls " << nnalls
               << " k " << kminmesh << ":" << kmaxmesh << " l " << lminmesh
               << ":" << lmaxmesh << endl;
        // Now that we have the blanks array, create avtGhostZones.
        unsigned char realVal = 0, ghost = 0;
        avtGhostData::AddGhostZoneType(ghost, ZONE_NOT_APPLICABLE_TO_PROBLEM);
        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName("avtGhostZones");
        ghostCells->Allocate(nCells);
        for(int i = 0; i < nCells; ++i)
        {
            int k, l;
            l = i/(nk-1);
            k = i - l*(nk-1);
            debug4 << mName << " i " << i << " nk " << nk << " k,l "
                   << k+kminmesh << "," << l+lminmesh << endl;
            if(blanks[(l+lminmesh)*lp + k+kminmesh])
            {
                ghostCells->InsertNextValue(realVal);
                debug4 << mName << "real zone at i " << i << endl;
            }
            else
            {
                ghostCells->InsertNextValue(ghost);
                debug4 << mName << "Ghost zone at i " << i << endl;
            }
        }
        sgrid->GetCellData()->AddArray(ghostCells);
        sgrid->SetUpdateGhostLevel(0);
        ghostCells->Delete();
        // Clean up
        delete [] blanks;
        return sgrid;
    }
    else if (strcmp(meshname, "mesh2") == 0)
    {
        // Create a VTK object for "mesh2"
        return 0;
    }
    else // check for time or cycle plot
    {
        int ntp, ncp, ncurves, ntimes, tplen, foundit=0;
        herr_t hdf_err;
        hdf_err =                      ReadHDF_Entry(GetHDF5File(),"/parameters/ntp",&ntp);
        if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/ncp",&ncp);
        double *ttime, *data;
        typedef struct {
           char tplab[30] ; /*!< curve label */
           int type ;       /*!< the data type */
           int tplen ;      /*!< # points stored so far */
           hvl_t h5_tpdat_arr ; /*!< the curve data */
           int tpx ;       /*!< unused */
           }   h5_tpcur ;
        typedef struct {
           int ntimes ;    /*!< max number of times allowed */
           int ncurs ;     /*!< number of curves */
           }   h5_tpdat ;

        h5_tpcur *h5_tpcurve ;
        h5_tpdat *h5_tpdata = new h5_tpdat[ntp] ;
        h5_tpdat *h5_cpdata = new h5_tpdat[ncp] ;

        if (ntp > 0)
            ReadHDF_Entry(GetHDF5File(),"/ppa/tpdata",h5_tpdata) ;

        for ( int i = 1 ; i <= ntp ; i++ )
        {
            char dataname[128];
            ncurves    = h5_tpdata[i-1].ncurs ;
            ntimes     = h5_tpdata[i-1].ntimes ;
            h5_tpcurve = new h5_tpcur[ncurves] ;

            sprintf(dataname,"/ppa/tpcurs_%d",i) ;

            hdf_err =  ReadHDF_Entry(GetHDF5File(), dataname, h5_tpcurve) ;

            debug4 << "section " << i << " has " << ncurves
                   << " curves " << endl;

            for ( int icur = 1 ; icur < ncurves ; icur++ )
            {
                char *tplab = h5_tpcurve[icur].tplab ;
                tplen = h5_tpcurve[icur].tplen ;
                if (strcmp(meshname,tplab) == 0)
                {
                    debug4 << "matched label '" << tplab << "'" << endl;
                    if (tplen == 0) 
                    {
                        tplen = 1 ;
                        ttime = new double[tplen];
                        data  = new double[tplen];
                        ttime[0] = 0.0 ;
                        data[0]  = 1.0 ;
                    }
                    else
                    {
                        ttime = new double[tplen];
                        data  = new double[tplen];
                        for ( int j = 0 ; j < tplen ; j++ )
                        {
                            ttime[j] = ((double*)(h5_tpcurve[0].h5_tpdat_arr.p))[j] ;
                            data[j]  = ((double*)(h5_tpcurve[icur].h5_tpdat_arr.p))[j] ;
                        }
                    }
                    foundit = 1;
                    break;
                }
            }
            delete [] h5_tpcurve ;
            if (foundit == 1)
                break;
        }

        if (foundit == 0)
        {
            if (ncp > 0)
                ReadHDF_Entry(GetHDF5File(),"/ppa/cpdata",h5_cpdata) ;

            for ( int i = 1 ; i <= ncp ; i++ )
            {
                char dataname[128];
                ncurves    = h5_cpdata[i-1].ncurs ;
                ntimes     = h5_cpdata[i-1].ntimes ;
                h5_tpcurve = new h5_tpcur[ncurves] ;

                sprintf(dataname,"/ppa/cpcurs_%d",i) ;

                hdf_err =  ReadHDF_Entry(GetHDF5File(), dataname, h5_tpcurve) ;

                debug4 << "section " << i << " has " << ncurves
                       << " curves " << endl;

                for ( int icur = 1 ; icur < ncurves ; icur++ )
                {
                    char *tplab = h5_tpcurve[icur].tplab ;
                    tplen = h5_tpcurve[icur].tplen ;
                    if (strcmp(meshname,tplab) == 0)
                    {
                        debug4 << "matched label '" << tplab << "'" << endl;
                        if (tplen == 0) 
                        {
                            tplen = 1 ;
                            ttime = new double[tplen];
                            data  = new double[tplen];
                            ttime[0] = 0.0 ;
                            data[0]  = 1.0 ;
                        }
                        else
                        {
                            ttime = new double[tplen];
                            data  = new double[tplen];
                            for ( int j = 0 ; j < tplen ; j++ )
                            {
                                ttime[j] = ((double*)(h5_tpcurve[0].h5_tpdat_arr.p))[j] ;
                                data[j]  = ((double*)(h5_tpcurve[icur].h5_tpdat_arr.p))[j] ;
                            }
                        }
                        foundit = 1;
                        break;
                    }
                }
                delete [] h5_tpcurve ;
                if (foundit == 1)
                    break ;
            }
        }
        delete [] h5_tpdata;
        delete [] h5_cpdata;
        if (foundit == 1)
        {
            vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(tplen,
                                                                 VTK_DOUBLE);
            vtkDoubleArray *vals = vtkDoubleArray::New();
            vals->SetNumberOfComponents(1);
            vals->SetNumberOfTuples(tplen);
            vals->SetName(meshname);
            rg->GetPointData()->SetScalars(vals);
            vtkDoubleArray *xc = vtkDoubleArray::SafeDownCast(
                                      rg->GetXCoordinates());

            for (int j = 0 ; j < tplen ; j++)
            {
                xc->SetValue(j, ttime[j]);
                vals->SetValue(j, data[j]);
            }

            delete [] ttime;
            delete [] data;
            vals->Delete();
            return rg;
        }
        else
        {
            // No mesh name that we recognize.
            EXCEPTION1(InvalidVariableException, "no data for this curve");
        }
    }
    return 0;
}


// ****************************************************************************
//  Method: avtCaleHDF5FileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Rob Managan
//  Creation:   Thu Apr 21 15:29:31 PST 2011
//
// ****************************************************************************

vtkDataArray *
avtCaleHDF5FileFormat::GetVar(const char *varname)
{
    const char *mName = "avtCaleHDF5FileFormat::GetVar: ";

    debug4 << mName << varname << endl;

    int kmax, lmax, lp, nnalls, namix, nvals, hdf_err, nk, nl;
    int npbin, ngrps, rdifmix;
    int length, group, grplen;
    hdf_err =                      ReadHDF_Entry(GetHDF5File(),"/parameters/kmax",&kmax);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/lmax",&lmax);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/lp",&lp);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/nnalls",&nnalls);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/namix",&namix);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/npbin",&npbin);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/rdifmix",&rdifmix);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/ngrps",&ngrps);
    length = namix;
    char    vstring[33];

    if (strncmp(varname,"pbin",4) == 0)
    {
        strcpy(vstring,"/arrays/pbin");
        grplen = namix;
        length = npbin*grplen;
        sscanf(varname,"pbin_%d",&group);
    }
    else if (strncmp(varname,"kbin",4) == 0)
    {
        strcpy(vstring,"/arrays/kbin");
        grplen = namix;
        length = npbin*grplen;
        sscanf(varname,"kbin_%d",&group);
    }
    else if (strncmp(varname,"nflux",5) == 0)
    {
        strcpy(vstring,"/arrays/nflux");
        grplen = nnalls;
        length = ngrps*grplen;
        sscanf(varname,"nflux_%d",&group);
    }
    else
    {
        sprintf(vstring,"/arrays/%s",varname);
        grplen = 1;
        length = namix;
        group = 0;
    }

    double *vararray = new double[length];
    hdf_err = ReadHDF_Entry(GetHDF5File(),vstring,vararray);
    double *varray = vararray + group*grplen;

    if (kminmesh == -1)
        GetUsedMeshLimits();
    nk = kmaxmesh - kminmesh + 1;
    nl = lmaxmesh - lminmesh + 1;

    if (!strcmp(varname, "r")      ||
        !strcmp(varname, "z")      ||
        !strcmp(varname, "rvel")   ||
        !strcmp(varname, "zvel")   ||
        !strcmp(varname, "vr")     ||
        !strcmp(varname, "vz")     ||
        !strcmp(varname, "rad")    ||
        !strcmp(varname, "theta")  ||
        !strcmp(varname, "vtheta") ||
        !strcmp(varname, "vmag")   ||
        !strcmp(varname, "bmhdr")  ||
        !strcmp(varname, "bmhdz")  ||
        !strcmp(varname, "jmhdr")  ||
        !strcmp(varname, "jmhdz")  ||
        !strcmp(varname, "omega")  ||
        !strcmp(varname, "r2w")    ||
        !strcmp(varname, "lt")      )
    {
        int i = 0;
        // node centered
        nvals = nk*nl;
        // debug4 << "getvar zc: nk " << nk << " nl " << nl << " nvals "
        //        << nvals << endl;
        vtkFloatArray *rv = vtkFloatArray::New();
        rv->SetNumberOfTuples(nvals);
        for ( int l = 0 ; l < nl ; l++ )
        {
            for (int k = 0 ; k < nk ; k++ )
            {
                rv->SetTuple1(i++, varray[(l+lminmesh)*lp + k+kminmesh]);
            }
        }
        return rv;
    }
    else
    {
        int i = 0;
        // zone centered
        nvals = (nk-1)*(nl-1);
        // debug4 << "getvar zc: nk " << nk << " nl " << nl << " nvals "
        //        << nvals << endl;
        vtkFloatArray *rv = vtkFloatArray::New();
        rv->SetNumberOfTuples(nvals);
        for ( int l = 0 ; l < nl-1 ; l++ )
        {
            for ( int k = 0 ; k < nk-1 ; k++ )
            {
                rv->SetTuple1(i++, varray[(l+lminmesh)*lp + k+kminmesh]);
            }
        }

#ifdef TRY_MIXED_SCALARS
        TypeEnum t = NO_TYPE;
        std::string typeString;
        int nTotalElements, *dims = 0, ndims = 0;
        if(SymbolInformation(GetHDF5File(), vstring, &t,
                             typeString, &nTotalElements, &dims, &ndims))
        {
            debug4 << mName << "nTotalElements = " << nTotalElements << endl;
            debug4 << mName << "nnalls = " << nnalls << endl;
            // truncate zmass to nnalls since the mixslots hold volume fractions
            if (strcmp(varname,"zmass") == 0)
            {
                nTotalElements = nnalls;
            }
            // for spectral variable set nTotalElements for the group.
            else if ((strncmp(varname,"kbin",4) == 0) |
                     (strncmp(varname,"pbin",4) == 0))
            {
                if (rdifmix == 0)
                {
                    nTotalElements = nnalls;
                }
                else
                {
                    nTotalElements = namix;
                }
            }
            else if (strncmp(varname,"nflux",5) == 0)
            {
                nTotalElements = nnalls;
            }

            if (nTotalElements > nnalls)
            {
                debug4 << mName << "Variable is mixed. Creating mixvar" << endl;

                // Convert the doubles to floats.
                int mixlen = nTotalElements - nnalls;
                debug4 << mName << "mixlen = " << mixlen << endl;
                float *mixvar = new float[mixlen];
                for(int i = 0; i < mixlen; ++i)
                    mixvar[i] = (float)varray[nnalls + i];

                avtMixedVariable *mv = new avtMixedVariable(mixvar,
                                                            mixlen, varname);
                void_ref_ptr vr = void_ref_ptr(mv, avtMixedVariable::Destruct);
                cache->CacheVoidRef(varname, AUXILIARY_DATA_MIXED_VARIABLE,
                                    0, 0, vr);
                delete [] mixvar;
            }
            else
                debug4 << mName << "Variable is not mixed." << endl;

            delete [] dims;
        }
        else
        {
            debug4 << mName << "Could not get information for "
                   << varname << endl;
        }
#endif

        return rv;
    }
}


// ****************************************************************************
//  Method: avtCaleHDF5FileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Rob Managan
//  Creation:   Thu Apr 21 15:29:31 PST 2011
//
// ****************************************************************************

vtkDataArray *
avtCaleHDF5FileFormat::GetVectorVar(const char *varname)
{
    return 0 ;
}
// ***************************************************************************
//  Method: avtCaleHDF5FileFormat::GetCycle
//
//  Purpose: Return the cycle associated with this file
//
//  Programmer: Rob Managan
//  Creation:   Fri Apr 29 11:29:35 PDT 2011
//
// ***************************************************************************
int
avtCaleHDF5FileFormat::GetCycle(void)
{
    int hdf_err ;
    int cycle ;

    hdf_err = ReadHDF_Entry(GetHDF5File(),"/parameters/cycle",&cycle) ;
    debug4 << " cycle " << cycle << endl ;
    return cycle;
}

// ***************************************************************************
//  Method: avtCaleHDF5FileFormat::GetCycleFromFilename
//
//  Purpose: Return the cycle associated with this file
//
// ***************************************************************************

int
avtCaleHDF5FileFormat::GetCycleFromFilename(const char *f) const
{
    int i,j,n,c;
    char cycstr[10];

    n = strlen(f) ;
    if (strcmp(f+(n-3),".h5") == 0)
       n = strlen(f) - 3; // To get here there had to be a ".h5" on the file
    else if (strcmp(f+(n-4),".ch5") == 0)
       n = strlen(f) - 4; // To get here there had to be a ".ch5" on the file
    else if (strcmp(f+(n-5),".cale") == 0)
       n = strlen(f) - 5; // To get here there had to be a ".cale" on the file

    j = 0;

    for ( i = n-1 ; i  >= 0 ; i-- )
    {
        if ((f[i] >= '0') && (f[i] <= '9'))
        {
            j++;
        }
        else
        {
            break;
        }
    }

    if (j > 0)
    {
        strncpy(cycstr,f+i+1,j);
        c = atoi(cycstr);
    }
    else
    {
        c = -1;
    }

    debug4 << " cycle from name '" << f << "' is " << cycstr << endl;
    return(c);
}


// ***************************************************************************
//  Method: avtCaleHDF5FileFormat::GetTime
//
//  Purpose: Return the time associated with this file
//
//  Programmer: Rob Managan
//  Creation:   Fri Apr 29 11:29:35 PDT 2011
//
// ***************************************************************************

double
avtCaleHDF5FileFormat::GetTime(void)
{
    int hdf_err ;
    double dtime ;

    hdf_err = ReadHDF_Entry(GetHDF5File(),"/parameters/time",&dtime) ;
    debug4 << " time " << dtime << endl ;
    return dtime;
}

#include <avtMaterial.h>
// ***************************************************************************
//  Method: avtCaleHDF5FileFormat::GetAuxiliaryData
//
//  Purpose: STMD version of GetAuxiliaryData.
//
//  Programmer: Rob Managan
//  Creation:   Fri Apr 29 11:29:35 PDT 2011
//
// ***************************************************************************

void *
avtCaleHDF5FileFormat::GetAuxiliaryData(const char *var,
    const char *type, void *, DestructorFunction &df)
{
    const char *mName = "avtCaleHDF5FileFormat::GetAuxiliaryData: ";
    void *retval = 0;
    debug4 << mName << "type " << type << " var " << var << endl;
    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        int i, kmax, lmax, lp, nnalls, namix, hdf_err;
        int nreg, nregx, nk, nl, mixmax;
        int dims[3] = {1,1,1}, ndims = 2;

        debug4 << mName << "Asked to read material information." << endl;
        hdf_err =                      ReadHDF_Entry(GetHDF5File(),"/parameters/kmax",&kmax);
        if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/lmax",&lmax);
        if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/lp",&lp);
        if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/nnalls",&nnalls);
        if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/namix",&namix);
        if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/mixmax",&mixmax);
        if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/nregx",&nregx);
        if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/nreg",&nreg);

        if (kminmesh == -1)
            GetUsedMeshLimits();
        nk = kmaxmesh - kminmesh + 1;
        nl = lmaxmesh - lminmesh + 1;

        debug4 << mName << "kmax = " << kmax << endl;
        debug4 << mName << "lmax = " << lmax << endl;
        debug4 << mName << "lp = " << lp << endl;
        debug4 << mName << "nnalls = " << nnalls << endl;
        debug4 << mName << "namix = " << namix << endl;
        debug4 << mName << "nreg = " << nreg << endl;

        // Structured mesh case
        ndims = 2;
        dims[0] = nk-1;
        dims[1] = nl-1;
        dims[2] = 1;
        // Read the number of materials from the file. This
        // must have already been read from the file when
        // PopulateDatabaseMetaData was called.
        int nmats = nreg+1;
        // The matnos array contains the list of numbers that
        // are associated with particular materials. For example,
        // matnos[0] is the number that will be associated with
        // the first material and any time it is seen in the
        // matlist array, that number should be taken to mean
        // material 1. The numbers in the matnos array must
        // all be greater than or equal to 1.
        int *matnos = new int[nmats];
        // READ nmats INTEGER VALUES INTO THE matnos ARRAY.
        // Read the material names from your file format or
        // make up names for the materials. Use the same
        // approach as when you created material names in
        // the PopulateDatabaseMetaData method.
        char **names = new char *[nmats];
        //      READ MATERIAL NAMES FROM YOUR FILE FORMAT UNTIL EACH
        //      ELEMENT OF THE names ARRAY POINTS TO ITS OWN STRING.
        typedef struct
        {
            char name[33];
        }  trcname_str;

        if (nreg >= 1)
        {
            debug4 << mName << "nreg>=1, we have a material." << endl;
            trcname_str *rname = new trcname_str[nreg+1];

            hdf_err = ReadHDF_Entry(GetHDF5File(),"/ppa/rname",rname);
            strcpy(rname[0].name,"Phony");

            for ( i = 0 ; i < nmats ; i++ )
            {
                matnos[i] = i;
                names[i] = rname[i].name;
            }
            matnos[0] = nmats; // can't have a zero material number

            // Read the matlist array, which tells what the material
            // is for each zone in the mesh.
            int nzones = dims[0] * dims[1] * dims[2];
            int *matlist = new int[nnalls];
            int *ireg    = new int[namix];
            int *mix_mat  = new int[mixmax];
            int *mix_next = new int[mixmax];
            int *mix_zone = new int[mixmax];
            double *zmass = new double[namix]; // volume fractions are in this

            int maxmixindx, ir, j;
            hdf_err =                     ReadHDF_Entry(GetHDF5File(),"/arrays/ireg",ireg);
            if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/arrays/zmass",zmass);

            for ( i = 0 ; i < nnalls ; i++ )
            {
                matlist[i] = 0;
            }
            for ( i = 0 ; i < mixmax ; i++ )
            {
                mix_mat[i] = 0;
                mix_next[i] = 0;
                mix_zone[i] = 0;
            }
            debug4 << mName << "zeroed out arrays." << endl;

            int *nmatlst = new int[namix];
            int *grdlst  = new int[mixmax];
            hdf_err =                      ReadHDF_Entry(GetHDF5File(),"/arrays/nmatlst",nmatlst);
            if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/arrays/grdlst",grdlst);

            int *rlen  = new int[nregx];
            int *rlencln  = new int[nregx];
            int *rlenmix  = new int[nregx];
            if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/arrays/rlen",rlen);
            if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/arrays/rlencln",rlencln);
            if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/arrays/rlenmix",rlenmix);

            int **rndx = new int*[nreg+1];
            int **rndxmix = new int*[nreg+1];
            int **rgrdmix = new int*[nreg+1];
            //
            // set regional index pointers
            //
            rlen[0] = rlencln[0];
            rlenmix[0] = 0;

            i = 0;
            for ( ir = 0 ; ir <= nreg ; ir++ )
            {
                rndx[ir] = nmatlst + i;
                rndxmix[ir] = rndx[ir] + rlencln[ir];
                i = i + rlen[ir];
                //debug4 << mName << "ir " << ir << " rlen " << rlen[ir]
                //       <<  " rlenmix " << rlenmix[ir] << " rndx "
                //       << rndx[ir]-nmatlst << endl;
            }

            i = 0;
            for ( ir = 0 ; ir <= nreg ; ir++ )
            {
                rgrdmix[ir] = grdlst + i;
                i = i + rlenmix[ir];
                // debug4 << mName << "ir " << ir << " rgrdmix "
                //        << rgrdmix[ir]-grdlst << endl;
            }
            //
            //  By setting up the mixed material arrays correctly, we
            //  don't have to reorder the mixed volume fractions or the
            //  mixed values of the zonal variables.  It's more work now,
            //  but it saves time when writing every zonal variable.
            //
            maxmixindx = 0;
            for ( ir = 0; ir <= nreg; ir++)
            {
                matnos[ir]=ir;
                // debug4 << mName << " start region " << ir << endl;

                // Do clean zones...
                for (i = 0 ; i < rlencln[ir] ; i++)
                {
                    matlist[rndx[ir][i]] = ir;
                }

                // Do mixed zones...
                for (i = 0 ; i < rlenmix[ir] ; i++)
                {
                    int done  = 0;
                    int izone = rgrdmix[ir][i];
                    int indx  = rndxmix[ir][i]-nnalls;
                    // debug4 << mName << " ir " << ir << " i " << i
                    //        << " indx " <<indx << " izone " << izone << endl;

                    // Find the next mixed index for this same zone
                    for (int jr = ir+1; jr <= nreg && !done; jr++)
                    {
                        // debug4 << mName << " check region " << jr << endl;
                        for (j = 0 ; j < rlenmix[jr] && !done ; j++)
                        {
                            int jzone = rgrdmix[jr][j];
                            int jndx  = rndxmix[jr][j]-nnalls;
                            // debug4 << mName << " jzone " << jzone
                            //        << " jndx " << jndx << endl;

                            if (izone == jzone)
                            {
                                // found it -- set it and stop looking
                                mix_next[indx] = jndx + 1;
                                done=1;
                            }
                        }
                    }

                    mix_mat [indx] = ir;
                    // removezoneghost_index converts a CALE zone index into
                    // one without ghost zones
                    mix_zone[indx] =
                            removezoneghost_index(izone,dims[0],dims[1],lp)+1;
                    // debug4 << mName << " izone " << izone << " ->  "
                    //        << mix_zone[indx] << endl;
                    // debug4 << mName << " matlist[izone]  "
                    //        << matlist[izone] << endl;

                    // If we're the first mixed mat in this zone,
                    // set the matlist ptr
                    if (matlist[izone] == 0)
                        matlist[izone] = -indx-1;
                    // debug4 << mName << " matlist[izone]  "
                    //        << matlist[izone] << endl;
                    if (indx>maxmixindx)
                        maxmixindx=indx;
                    // debug4 << mName << " maxmixindx " << maxmixindx << endl;
                }
            }
            // matnos[0] = nmats;

            int *matlistnp=new int[nzones];
            // copy matlist into array matlistnp without ghost zones
            for (int l = 0 ; l < nl-1 ; l++ )
            {
                for (int k = 0 ; k < nk-1 ; k++ )
                {
                    matlistnp[l*(nk-1) + k] =
                             matlist[(l + lminmesh)*lp + (k + kminmesh)];
                }
            }
            //
            // convert volume fractions to float
            //
            float *matvf = new float[maxmixindx+1];
            for ( i = 0 ; i < maxmixindx+1 ; i++ )
            {
                matvf[i] = zmass[nnalls+i];
            }

            // READ nzones INTEGERS INTO THE matlist array.
            // Optionally create mix_mat, mix_next, mix_zone, mix_vf
            // arrays and read their contents from the file format.
            // Use the information to create an avtMaterial object.
            avtMaterial *mat = new avtMaterial(
                                nmats,
                                matnos,
                                names,
                                ndims,
                                dims,
                                0,
                                matlistnp,
                                maxmixindx+1, // length of mix arrays
                                mix_mat, // mix_mat array
                                mix_next, // mix_next array
                                mix_zone, // mix_zone array
                                matvf  // mix_vf array
                                );
            // Clean up.
            delete [] matnos;
            delete [] rname;
            delete [] names; // top
            delete [] matlist;
            delete [] ireg; // bottom
            delete [] mix_mat;
            delete [] mix_next;
            delete [] mix_zone;
            delete [] zmass;
            delete [] nmatlst;
            delete [] grdlst;
            delete [] rlen;
            delete [] rlencln;
            delete [] rlenmix;
            delete [] rndx;
            delete [] rndxmix;
            delete [] rgrdmix;
            delete [] matlistnp;
            delete [] matvf;
            debug4 << "Returning mat: " << ((void*)mat) << endl;
            // Set the return values.
            retval = (void *)mat;
        }
        else
        {
            debug4 << "nreg < 1. No material information" << endl;
            delete [] names; // top
            delete [] matnos;
        }
        df = avtMaterial::Destruct;
    }
    return retval;
}

// ***************************************************************************
//  Method: avtCaleHDF5FileFormat::GetUsedMeshLimits
//
//  Purpose:    find largest k,l used in mesh
//
//  Programmer: Rob Managan
//  Creation:   Fri Apr 29 11:29:35 PDT 2011
//
// ***************************************************************************

void
avtCaleHDF5FileFormat::GetUsedMeshLimits (void)
{
    int ibc;
    int kmax, lmax, nbc, nbcx;
    hid_t hdf_err ;

    hdf_err =                      ReadHDF_Entry(GetHDF5File(),"/parameters/kmax",&kmax);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/lmax",&lmax);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/nbc",&nbc);
    if (hdf_err >= 0.0) hdf_err |= ReadHDF_Entry(GetHDF5File(),"/parameters/nbcx",&nbcx);

    int *bck1 = new int[nbcx];
    int *bck2 = new int[nbcx];
    int *bcl1 = new int[nbcx];
    int *bcl2 = new int[nbcx];
    if (hdf_err >= 0.0) hdf_err = ReadHDF_Entry(GetHDF5File(),"/arrays/bck1",bck1);
    if (hdf_err >= 0.0) hdf_err = ReadHDF_Entry(GetHDF5File(),"/arrays/bck2",bck2);
    if (hdf_err >= 0.0) hdf_err = ReadHDF_Entry(GetHDF5File(),"/arrays/bcl1",bcl1);
    if (hdf_err >= 0.0) hdf_err = ReadHDF_Entry(GetHDF5File(),"/arrays/bcl2",bcl2);

    kminmesh = kmax;
    lminmesh = lmax;

    kmaxmesh = 1;
    lmaxmesh = 1;
    // debug4 << "GetUsedMeshLimits: nbc "<< nbc << " nbcx " << nbcx << endl;

    for ( ibc = 1 ; ibc <= nbc ; ibc++ )
    {
        //debug4 << " ibc " << ibc << " k1 " << bck1[ibc] << " l1 " << bcl1[ibc]
        //       << " k2 " << bck2[ibc] << " l2 " << bcl2[ibc] << endl;

        kminmesh = (bck1[ibc] < kminmesh) ? bck1[ibc] : kminmesh;
        kminmesh = (bck2[ibc] < kminmesh) ? bck2[ibc] : kminmesh;

        lminmesh = (bcl1[ibc] < lminmesh) ? bcl1[ibc] : lminmesh;
        lminmesh = (bcl2[ibc] < lminmesh) ? bcl2[ibc] : lminmesh;

        kmaxmesh = (bck1[ibc] > kmaxmesh) ? bck1[ibc] : kmaxmesh;
        kmaxmesh = (bck2[ibc] > kmaxmesh) ? bck2[ibc] : kmaxmesh;

        lmaxmesh = (bcl1[ibc] > lmaxmesh) ? bcl1[ibc] : lmaxmesh;
        lmaxmesh = (bcl2[ibc] > lmaxmesh) ? bcl2[ibc] : lmaxmesh;
    }

    nnallsmesh = (kmaxmesh - kminmesh + 1)*(lmaxmesh - lminmesh + 1);

    delete [] bck1;
    delete [] bck2;
    delete [] bcl1;
    delete [] bcl2;
}


// ***************************************************************************
//  Function:  removezoneghost_index()
//
//  Inputs  :  in (int): the index to convert
//              zk, zl (int): the new dimensions of the array
//              mode : 0 one row of phony zones, 1 none
//
//  Returns :  (int): the converted index
//
//  Abstract:  Transform a 2D index into an array with an extra row of ghost
//             zones around the max k & l edges into a 2D index into an array
//             without them.
// ***************************************************************************

int
avtCaleHDF5FileFormat::removezoneghost_index(int in,int zk,int zl, int lp)
{
    int k,l;

    l=in/lp;    /* calculate Cale's (k,l) values */
    k=in-(l*lp);

    if (kminmesh == -1)
        GetUsedMeshLimits() ;
    l -= lminmesh; /* remove unused zones at origin of mesh */
    k -= kminmesh;

    if (k >= zk  ||  l >= zl)
        return (-1);
    else
        return (l*zk + k);
}

