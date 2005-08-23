#include <avtFileFormatInterface.h>
#include <DebugStream.h>
#include <VisItException.h>
#include <InvalidDBTypeException.h>
#include <NETCDFFileObject.h>

#include <avtADAPTFileFormat.h>
#include <avtBasicNETCDFFileFormat.h>
#include <avtLODIFileFormat.h>
#include <avtLODIParticleFileFormat.h>

// ****************************************************************************
// Method: CreateFileFormatInterface
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
//
// ****************************************************************************

avtFileFormatInterface *
CreateFileFormatInterface(const char * const *list, int nList, int nBlock)
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

            if(flavor == -1)
                debug4 << "Database is avtBasicNETCDFFileFormat" << endl;
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
        default:
            ffi = avtBasicNETCDFFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        }
    }

    return ffi;
}
