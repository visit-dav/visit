// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtFileFormatInterface.h>
#include <DebugStream.h>
#include <VisItException.h>
#include <InvalidDBTypeException.h>
#include <NETCDFFileObject.h>

#include <avtADAPTFileFormat.h>
#include <avtBOUTFileFormat.h>
#include <avtBasicNETCDFFileFormat.h>
#include <avtLODIFileFormat.h>
#include <avtLODIParticleFileFormat.h>
#include <avtFVCOM_STSDFileFormat.h>
#include <avtFVCOM_MTSDFileFormat.h>
#include <avtFVCOMParticleFileFormat.h>
#include <avtFVCOM_MTMDFileFormat.h>
#include <avtCCSMFileFormat.h>

// ****************************************************************************
// Method: DetectAWENETCDF
//
// Purpose: 
//   Determine whether the file is AWE NETCDF.
//
// Returns:    True if the file looks like AWE NETCDF; false otherwise.
//
// Programmer: Satheesh Maheswaran
// Creation:   Thu Oct 25 10:31:04 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

static bool DetectAWENETCDF(NETCDFFileObject *f)
{
    bool retval = false;

    // If format has not been recognised yet, see if it looks like an AWEnetCDF file
    // Look for a variable called "codevers" (but not concerned what's in it)
    // (InqVariable() would also work if we really don't want to see codevers contents)
    TypeEnum vartyp = NO_TYPE;
    int ndims = 0, *dims = 0;
    void *values = 0;

    if(f->ReadVariable("codevers", &vartyp, &ndims, &dims, &values))
    {
        debug4 << "Database seems to be AWEnetCDF format - found codevers";
        if(vartyp == CHARARRAY_TYPE && ndims == 1 && *dims > 0)
        {
          char *codevers_val = (char *)values;
          debug4 << " = \"" << codevers_val  << "\"" ;
        }
        debug4 << "; Abandoning NETCDF reader." << endl;
        delete [] dims;
        free_void_mem(values, vartyp);
        retval = true;
    }

    return retval;
}

// ****************************************************************************
// Method: NETCDF_CreateFileFormatInterface
//
// Purpose:
//   Opens the first NETCDF file in the list and attempts to use the various
//   file formats to create a file format interface.
//
// Arguments:
//   list   : The list of filenames.
//   nList  : The number of filenames in the list.
//   nBlock : The number of files in a timestep.
//
// Returns:    A file format interface or 0 if no file format interface
//             was created.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 12:06:19 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Dec 9 17:39:52 PST 2005
//   I renamed the method to avoid namespace conflicts on Tru64.
//
//   Brad Whitlock, Fri Oct 5 11:41:05 PDT 2007
//   Added CCSM file format.
//
//   Brad Whitlock, Tue Oct 27 14:37:05 PDT 2009
//   I separated CCSM into MT and ST flavors.
//
//   Brad Whitlock, Thu Oct 29 16:23:33 PDT 2009
//   I separated the Basic reader into MT and ST flavors.
//
//   Satheesh Maheswaran, Thu Oct 25 10:31:04 PDT 2012
//   Add DetectAWENETCDF function call.
//
//   Eric Brugger, Thu Aug  1 13:15:07 PDT 2013
//   Add BOUT file format.
//
// ****************************************************************************

avtFileFormatInterface *
NETCDF_CreateFileFormatInterface(const char * const *list, int nList, int nBlock)
{
    avtFileFormatInterface *ffi = 0;

    if(list != 0 || nList > 0)
    {
        // Determine the type of reader that we want to use.
        NETCDFFileObject *f = 0;
        int flavor = -1;
        TRY
        {
            f = new NETCDFFileObject(list[0]);
            if(flavor == -1 && avtADAPTFileFormat::Identify(f))
            {
                flavor = 0;
                debug4 << "Database is avtADAPTFileFormat" << endl;
            }

            if(flavor == -1 && avtLODIParticleFileFormat::Identify(f))
            {
                flavor = 2;
                debug4 << "Database is avtLODIParticleFileFormat" << endl;
            }

            if(flavor == -1 && avtLODIFileFormat::Identify(f))
            {
                flavor = 1;
                debug4 << "Database is avtLODIFileFormat" << endl;
            }

            if(flavor == -1 && avtFVCOM_STSDFileFormat::Identify(f))
            {
                flavor = 3;
                debug4 << "Database is avtFVCOM_STSDFileFormat" << endl;
            }
            if(flavor == -1 && avtFVCOM_MTMDFileFormat::Identify(f))
            {
                flavor = 4;
                debug4 << "Database is avtFVCOM_MTMDFileFormat" << endl;
            }

            if(flavor == -1 && avtFVCOM_MTSDFileFormat::Identify(f))
            {
                flavor = 5;
                debug4 << "Database is avtFVCOM_MTSDFileFormat" << endl;
            }

            if(flavor == -1 && avtFVCOMParticleFileFormat::Identify(f))
            {
                flavor = 6;
                debug4 << "Database is avtFVCOMParticleFileFormat" << endl;
            }

            if(flavor == -1 && avtCCSM_MTSD_FileFormat::Identify(f))
            {
                flavor = 7;
                debug4 << "Database is avtCCSM_MTSD_FileFormat" << endl;
            }

            if(flavor == -1 && avtCCSM_STSD_FileFormat::Identify(f))
            {
                flavor = 8;
                debug4 << "Database is avtCCSM_STSD_FileFormat" << endl;
            }

            if(flavor == -1 && avtBOUTFileFormat::Identify(f))
            {
                flavor = 9;
                debug4 << "Database is avtBOUTFileFormat" << endl;
            }

            if(flavor == -1 && avtBasic_MTSD_NETCDFFileFormat::Identify(f))
            {
                flavor = 10;
                debug4 << "Database is avtBasic_MTSD_NETCDFFileFormat" << endl;
            }

            if(flavor == -1)
            {
                if(DetectAWENETCDF(f))
                {
                    delete f;
                    return NULL;
                }

                debug4 << "Database is avtBasic_STSD_NETCDFFileFormat" << endl;
            }
        }
        CATCH(VisItException)
        {
            delete f;
            RETHROW;
        }
        ENDTRY

        switch(flavor)
        {
        case 0:
            ffi = avtADAPTFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        case 1:
            ffi = avtLODIFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        case 2:
            ffi = avtLODIParticleFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        case 3:
            ffi = avtFVCOM_STSDFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        case 4:
            ffi = avtFVCOM_MTMDFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        case 5:
            ffi = avtFVCOM_MTSDFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        case 6:
            ffi = avtFVCOMParticleFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        case 7:
            ffi = avtCCSM_MTSD_FileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        case 8:
            ffi = avtCCSM_STSD_FileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        case 9:
            ffi = avtBOUTFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        case 10:
            ffi = avtBasic_MTSD_NETCDFFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        default:
            ffi = avtBasic_STSD_NETCDFFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        }
    }

    return ffi;
}
