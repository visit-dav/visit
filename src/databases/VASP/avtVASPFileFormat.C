// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtFileFormatInterface.h>
#include <DebugStream.h>
#include <VisItException.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>
#include <string>

#include <avtOUTCARFileFormat.h>
#include <avtCHGCARFileFormat.h>
#include <avtPOSCARFileFormat.h>
#include <avtXDATFileFormat.h>

// ****************************************************************************
// Method: VASP_CreateFileFormatInterface
//
// Purpose:
//   Opens the first VASP file in the list and attempts to use the various
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
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Modifications:
//    Jeremy Meredith, Tue Jan  8 10:35:45 EST 2008
//    Added POSCAR support.
//
// ****************************************************************************

avtFileFormatInterface *
VASP_CreateFileFormatInterface(const char * const *list, int nList, int nBlock)
{
    avtFileFormatInterface *ffi = 0;

    if (list != 0 && nList > 0)
    {
        // Determine the type of reader that we want to use.
        int flavor = -1;
        TRY
        {
            std::string fn(list[0]);
            if(flavor == -1 && avtOUTCARFileFormat::Identify(fn))
            {
                flavor = 0;
                debug4 << "Database is avtOUTCARFileFormat" << endl;
            }

            if(flavor == -1 && avtCHGCARFileFormat::Identify(fn))
            {
                flavor = 1;
                debug4 << "Database is avtCHGCARFileFormat" << endl;
            }

            if(flavor == -1 && avtPOSCARFileFormat::Identify(fn))
            {
                flavor = 2;
                debug4 << "Database is avtPOSCARFileFormat" << endl;
            }

            if(flavor == -1 && avtXDATFileFormat::Identify(fn))
            {
                flavor = 3;
                debug4 << "Database is avtXDATFileFormat" << endl;
            }

            if(flavor == -1)
            {
                EXCEPTION1(InvalidFilesException, list[0]);
            }
        }
        CATCH(VisItException)
        {
            RETHROW;
        }
        ENDTRY

        switch(flavor)
        {
          case 0:
            ffi = avtOUTCARFileFormat::CreateInterface(list, nList, nBlock);
            break;
          case 1:
            ffi = avtCHGCARFileFormat::CreateInterface(list, nList, nBlock);
            break;
          case 2:
            ffi = avtPOSCARFileFormat::CreateInterface(list, nList, nBlock);
            break;
          case 3:
            ffi = avtXDATFileFormat::CreateInterface(list, nList, nBlock);
            break;
        }
    }

    return ffi;
}
