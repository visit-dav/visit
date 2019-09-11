// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtADIOSFileFormat.C                           //
// ************************************************************************* //

#include <avtFileFormatInterface.h>
#include <DebugStream.h>
#include <VisItException.h>
#include <ADIOSFileObject.h>

#include "avtADIOSBasicFileFormat.h"
#include "avtXGCFileFormat.h"
//#include "avtPixieFileFormat.h"
#include "avtSpecFEMFileFormat.h"
#include "avtPIConGPUFileFormat.h"
#include "avtADIOSSchemaFileFormat.h"
//#include "avtLAMMPSFileFormat.h"
//#include "avtStagingFileFormat.h"

// ****************************************************************************
// Method: ADIOS_CreateFileFormatInterface
//
// Purpose:
//   Opens the first ADIOS file in the list and attempts to use the various
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
// Programmer: Dave Pugmire
// Creation:   Wed Feb  3 13:10:35 EST 2010
//
// Modifications:
//
//  Dave Pugmire, Tue Mar  9 12:40:15 EST 2010
//  Added XGC reader.
//
//   Dave Pugmire, Wed Apr  9 13:39:04 EDT 2014
//   Added specFM and schema readers.
//
//   Dave Pugmire, Thu Oct 30 11:59:40 EDT 2014
//   Added a LAMMPS reader. Modified the flavor flag to an enum for clarity.
//
// ****************************************************************************

avtFileFormatInterface *
ADIOS_CreateFileFormatInterface(const char * const *list, int nList, int nBlock)
{
    avtFileFormatInterface *ffi = NULL;
    enum Flavor {XGC, PIXIE, SPECFEM, SCHEMA, BASIC, LAMMPS, STAGING, PICONGPU, FAIL};
    
    Flavor flavor = FAIL;
    if (list != NULL || nList > 0)
    {
        // Determine the type of reader that we want to use.
        TRY
        {
            /*
            if (avtStagingFileFormat::Identify(list[0]))
            {
                debug5<<"Database is avtStagingFileFormat"<<endl;
                cout<<"Database is avtStagingFileFormat"<<endl;
                flavor = STAGING;
            }
            */
            if (avtXGCFileFormat::Identify(list[0]))
            {
                debug5<<"Database is avtXGCFileFormat"<<endl;
                flavor = XGC;
            }
            /*
            else if (avtPixieFileFormat::Identify(list[0]))
            {
                debug5<<"Database is avtPixieFileFormat"<<endl;
                flavor = PIXIE;
            }
            */
            else if (avtSpecFEMFileFormat::Identify(list[0]))
            {
                debug5<<"Database is avtSpecFEMFileFormat"<<endl;
                flavor = SPECFEM;
            }
            else if (avtPIConGPUFileFormat::Identify(list[0]))
            {
                debug5<<"Database is avtPIConGPUFileFormat"<<endl;
                flavor = PICONGPU;
            }
            else if (avtADIOSSchemaFileFormat::Identify(list[0]))
            {
                debug5<<"Database is avtADIOSSchemaFileFormat"<<endl;
                flavor = SCHEMA;
            }
            /*
            else if (avtLAMMPSFileFormat::Identify(list[0]))
            {
                debug5<<"Database is avtLAMMPSFileFormat"<<endl;
                flavor = LAMMPS;
            }
            */
            else if (avtADIOSBasicFileFormat::Identify(list[0]))
            {
                debug5<<"Database is avtADIOSBasicFileFormat"<<endl;
                flavor = BASIC;
            }
        }
        CATCH(VisItException)
        {
            RETHROW;
        }
        ENDTRY
        
        switch(flavor)
        {

          case XGC:
            ffi = avtXGCFileFormat::CreateInterface(list, nList, nBlock);
            break;
            /*
          case PIXIE:
            ffi = avtPixieFileFormat::CreateInterface(list, nList, nBlock);
            break;
            */
          case SPECFEM:
            ffi = avtSpecFEMFileFormat::CreateInterface(list, nList, nBlock);
            break;
          case PICONGPU:
            ffi = avtPIConGPUFileFormat::CreateInterface(list, nList, nBlock);
            break;
          case SCHEMA:
            ffi = avtADIOSSchemaFileFormat::CreateInterface(list, nList, nBlock);
            break;
            /*
          case LAMMPS:
            ffi = avtLAMMPSFileFormat::CreateInterface(list, nList, nBlock);
            break;
            */
            /*
          case STAGING:
            ffi = avtStagingFileFormat::CreateInterface(list, nList, nBlock);
            break;
            */
          case BASIC:
            ffi = avtADIOSBasicFileFormat::CreateInterface(list, nList, nBlock);
            break;
          default:
            return NULL;
        }
            
    }

    return ffi;
}
