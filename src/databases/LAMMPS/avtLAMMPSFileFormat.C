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
