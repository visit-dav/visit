/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <avtFileFormatInterface.h>
#include <DBOptionsAttributes.h>
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
//   Mark C. Miller, Tue Apr 29 23:33:55 PDT 2008
//   Added read options as formal arg. and passed to LEOS interface creator.
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
                ffi = PF3DFileFormat::CreateInterface(pdb, list, nList, nBlock);

            // Check to see if it is a PPZ STSD file..
            if(ffi == 0)
                ffi = PP_Z_STSD_FileFormat::CreateInterface(pdb, list, nList, nBlock);

            // Check to see if it is a PPZ Collected MTSD file.
            if(ffi == 0)
                ffi = PP_Z_MTSD_FileFormat::CreateInterface(pdb, list, nList);

            // Check to see if it is an LEOS file.
            if(ffi == 0)
                ffi = LEOSFileFormat::CreateInterface(pdb, list[0], rdopts);

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
