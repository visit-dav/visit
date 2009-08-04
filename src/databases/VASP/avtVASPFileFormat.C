/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <avtOUTCARFileFormat.h>
#include <avtCHGCARFileFormat.h>
#include <avtPOSCARFileFormat.h>

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
        }
    }

    return ffi;
}
