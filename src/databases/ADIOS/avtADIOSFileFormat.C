/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

// ************************************************************************* //
//                            avtADIOSFileFormat.C                           //
// ************************************************************************* //

#include <avtFileFormatInterface.h>
#include <DebugStream.h>
#include <VisItException.h>
#include <ADIOSFileObject.h>

#include "avtADIOSBasicFileFormat.h"
#include "avtXGCFileFormat.h"
#include "avtPixieFileFormat.h"

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
// ****************************************************************************

avtFileFormatInterface *
ADIOS_CreateFileFormatInterface(const char * const *list, int nList, int nBlock)
{
    avtFileFormatInterface *ffi = NULL;

    if(list != NULL || nList > 0)
    {
        // Determine the type of reader that we want to use.
        ADIOSFileObject *f = NULL;
        int flavor = -1;
        TRY
        {
            f = new ADIOSFileObject(list[0]);
            f->Open();
            if (avtXGCFileFormat::Identify(f))
            {
                debug5<<"Database is avtXGCFileFormat"<<endl;
                flavor = 1;
            }
            else if (avtPixieFileFormat::Identify(f))
            {
                debug5<<"Database is avtPixieFileFormat"<<endl;
                flavor = 2;
            }
            else if (avtADIOSBasicFileFormat::Identify(f))
            {
                debug5<<"Database is avtADIOSBasicFileFormat"<<endl;
                flavor = 0;
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
          case 1:
            ffi = avtXGCFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
          case 2:
            ffi = avtPixieFileFormat::CreateInterface(f, list, nList, nBlock);
            break;

          case 0:
            ffi = avtADIOSBasicFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
            
          default:
            delete f;
            return NULL;
        }
    }

    return ffi;
}
