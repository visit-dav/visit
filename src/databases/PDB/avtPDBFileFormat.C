#include <avtFileFormatInterface.h>
#include <DebugStream.h>
#include <VisItException.h>
#include <InvalidDBTypeException.h>
#include <PDBFileObject.h>

#include <PF3DFileFormat.h>
#include <PP_Z_STSD_FileFormat.h>
#include <PP_Z_MTSD_FileFormat.h>
#include <LEOSFileFormat.h>

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
// ****************************************************************************

avtFileFormatInterface *
PDB_CreateFileFormatInterface(const char * const *list, int nList, int nBlock)
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
                ffi = PF3DFileFormat::CreateInterface(pdb, list, nList, nBlock);

            // Check to see if it is a PPZ STSD file..
            if(ffi == 0)
                ffi = PP_Z_STSD_FileFormat::CreateInterface(pdb, list, nList, nBlock);

            // Check to see if it is a PPZ Collected MTSD file.
            if(ffi == 0)
                ffi = PP_Z_MTSD_FileFormat::CreateInterface(pdb, list, nList);

            // Check to see if it is an LEOS file.
            if(ffi == 0)
                ffi = LEOSFileFormat::CreateInterface(pdb, list[0]);

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
