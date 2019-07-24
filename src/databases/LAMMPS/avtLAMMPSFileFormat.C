// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtFileFormatInterface.h>
#include <DebugStream.h>
#include <VisItException.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>
#include <string>

#include <avtLAMMPSStructureFileFormat.h>
#include <avtLAMMPSDumpFileFormat.h>

// ****************************************************************************
// Method: LAMMPS_CreateFileFormatInterface
//
// Purpose:
//   Opens the first LAMMPS file in the list and attempts to use the various
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
// Creation:   February  9, 2009
//
// Modifications:
//
// ****************************************************************************

avtFileFormatInterface *
LAMMPS_CreateFileFormatInterface(const char * const *list, int nList, int nBlock)
{
    avtFileFormatInterface *ffi = 0;
    enum Flavor { FLAVOR_FAIL, FLAVOR_STRUCTURE, FLAVOR_DUMP };

    if (list != 0 && nList > 0)
    {
        // Determine the type of reader that we want to use.
        Flavor flavor = FLAVOR_FAIL;
        TRY
        {
            std::string fn(list[0]);

            if (flavor == FLAVOR_FAIL && avtLAMMPSDumpFileFormat::FileExtensionIdentify(fn))
            {
                flavor = FLAVOR_DUMP;
                debug4 << "Based on filename, database is LAMMPS Dump File" << endl;
            }

            if (flavor == FLAVOR_FAIL && avtLAMMPSStructureFileFormat::FileExtensionIdentify(fn))
            {
                flavor = FLAVOR_STRUCTURE;
                debug4 << "Based on filename, database is LAMMPS Structure Input File" << endl;
            }

            if (flavor == FLAVOR_FAIL && avtLAMMPSDumpFileFormat::FileContentsIdentify(fn))
            {
                flavor = FLAVOR_DUMP;
                debug4 << "Based on contents, database is LAMMPS Dump File" << endl;
            }

            if (flavor == FLAVOR_FAIL && avtLAMMPSStructureFileFormat::FileContentsIdentify(fn))
            {
                flavor = FLAVOR_STRUCTURE;
                debug4 << "Based on contents, database is LAMMPS Structure Input File" << endl;
            }

            if (flavor == FLAVOR_FAIL)
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
          case FLAVOR_STRUCTURE:
            ffi = avtLAMMPSStructureFileFormat::CreateInterface(list, nList, nBlock);
            break;
          case FLAVOR_DUMP:
            ffi = avtLAMMPSDumpFileFormat::CreateInterface(list, nList, nBlock);
            break;
          default: // case FLAVOR_FAIL
            break;
        }
    }

    return ffi;
}
