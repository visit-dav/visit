/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
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
#include <NETCDFFileObject.h>

#include <avtADAPTFileFormat.h>
#include <avtBasicNETCDFFileFormat.h>
#include <avtLODIFileFormat.h>
#include <avtLODIParticleFileFormat.h>
#include <avtFVCOM_STSDFileFormat.h>
#include <avtFVCOM_MTSDFileFormat.h>
#include <avtFVCOMParticleFileFormat.h>
#include <avtFVCOM_MTMDFileFormat.h>
#include <avtCCSMFileFormat.h>

// ****************************************************************************
// Method: NETCDF_CreateFileFormatInterface
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
//   Brad Whitlock, Fri Dec 9 17:39:52 PST 2005
//   I renamed the method to avoid namespace conflicts on Tru64.
//
//   Brad Whitlock, Fri Oct 5 11:41:05 PDT 2007
//   Added CCSM file format.
//
// ****************************************************************************

avtFileFormatInterface *
NETCDF_CreateFileFormatInterface(const char * const *list, int nList, int nBlock)
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

            if(flavor == -1 && avtFVCOM_STSDFileFormat::Identify(f))
            {
                flavor = 3;
                debug4 << "Database is avtFVCOM_STSDFileFormat" << endl;
            }
            if(flavor == -1 && avtFVCOM_MTMDFileFormat::Identify(f))
            {
                flavor = 4;
                debug4 << "Database is avtFVCOM_MTMDFileFormat" << endl;
            }

            if(flavor == -1 && avtFVCOM_MTSDFileFormat::Identify(f))
            {
                flavor = 5;
                debug4 << "Database is avtFVCOM_MTSDFileFormat" << endl;
            }

            if(flavor == -1 && avtFVCOMParticleFileFormat::Identify(f))
            {
                flavor = 6;
                debug4 << "Database is avtFVCOMParticleFileFormat" << endl;
            }

            if(flavor == -1 && avtCCSMFileFormat::Identify(f))
            {
                flavor = 7;
                debug4 << "Database is avtFVCOMCCSMFileFormat" << endl;
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
        case 3:
            ffi = avtFVCOM_STSDFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        case 4:
            ffi = avtFVCOM_MTMDFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        case 5:
            ffi = avtFVCOM_MTSDFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        case 6:
            ffi = avtFVCOMParticleFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        case 7:
            ffi = avtCCSMFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        default:
            ffi = avtBasicNETCDFFileFormat::CreateInterface(f, list, nList, nBlock);
            break;
        }
    }

    return ffi;
}
