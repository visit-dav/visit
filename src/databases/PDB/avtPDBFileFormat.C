// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtFileFormatInterface.h>
#include <DBOptionsAttributes.h>
#include <DebugStream.h>
#include <VisItException.h>
#include <InvalidDBTypeException.h>
#include <PDBFileObject.h>

#include <PF3DFileFormat.h>
#include <PP_Z_MTSD_FileFormat.h>
#include <LEOSFileFormat.h>
#include <JMFileFormat.h>

// ****************************************************************************
// Method: PDB_CreateFileFormatInterface
//
// Purpose: 
//   Opens the first PDB file in the list and attempts to use the various file
//   formats to create a file format interface.
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
//   Brad Whitlock, Fri Dec 9 17:38:23 PST 2005
//   Renamed to avoid namespace conflicts on Tru64.
//
//   Mark C. Miller, Tue Apr 29 23:33:55 PDT 2008
//   Added read options as formal arg. and passed to LEOS interface creator.
//
//   Brad Whitlock, Thu Apr 30 15:48:08 PDT 2009
//   I added support for Jose's file format.
//
//   Mark C. Miller, Fri Sep 25 15:17:54 PDT 2015
//   Removed PP_Z_STSD as it was a special case of more general PP_Z_MTSD
//   especially since the changes Jeremy made to support catenating multile MT
//   databases into a coherent time series using 'timestep groups'. Tested
//   this with a series of 5 single step MTSD files and it works fine.
// ****************************************************************************

avtFileFormatInterface *
PDB_CreateFileFormatInterface(const char * const *list, int nList, int nBlock,
    const DBOptionsAttributes *rdopts)
{
    avtFileFormatInterface *ffi = 0;

    if(list != 0 || nList > 0)
    {
        // Try and open the PDB file.
        PDBFileObject *pdb = new PDBFileObject(list[0]);

        //
        // Check to see if it is a Silo file. If it is a Silo file then throw
        // an exception so we don't try to read the file.
        //
        if(pdb->SymbolExists("_meshtv_defvars") ||
           pdb->SymbolExists("_meshtv_info") ||
           pdb->SymbolExists("_fileinfo"))
        {
            delete pdb;
            EXCEPTION1(InvalidDBTypeException,
                       "The PDB reader does not read Silo files.");
        }

        TRY
        {
            // Check to see if it is a PF3D file.
            if(ffi == 0)
            {
                debug4 << "Testing if file contains PF3D data" << endl;
                ffi = PF3DFileFormat::CreateInterface(pdb, list, nList, nBlock);
            }

            // Check to see if it is a PPZ Collected MTSD file.
            if(ffi == 0)
            {
                debug4 << "Testing if file contains Flash MT data" << endl;
                ffi = PP_Z_MTSD_FileFormat::CreateInterface(pdb, list, nList);
            }

            // Check to see if it is an LEOS file.
            if(ffi == 0)
            {
                debug4 << "Testing if file contains LEOS data" << endl;
                ffi = LEOSFileFormat::CreateInterface(pdb, list[0], rdopts);
            }

            // Check to see if it is one of Jose's files.
            if(ffi == 0)
            {
                debug4 << "Testing if file contains JM data" << endl;
                ffi = JMFileFormat::CreateInterface(pdb, list, nList);
            }

            // Add more file formats here.

        }
        CATCH(VisItException)
        {
            delete pdb;
            RETHROW;
        }
        ENDTRY

        if(ffi == 0)
            delete pdb;
    }

    return ffi;
}
