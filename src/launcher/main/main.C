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

#include <LauncherApplication.h>
#include <VisItInit.h>
#include <VisItException.h>
#include <DebugStream.h>

// ****************************************************************************
// Function: main
//
// Purpose:
//   This is the main function for the VisIt component launcher.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:02:19 PST 2003
//
// Modifications:
//
//    Mark C. Miller, Thu Apr  3 14:36:48 PDT 2008
//    Moved setting of component name to before Initialize
// ****************************************************************************

int
main(int argc, char *argv[])
{
    int retval = 0;

    // Initialize error logging
    VisItInit::SetComponentName("launcher");
    VisItInit::Initialize(argc, argv);

    debug1 << "VisIt component launcher started." << endl;

    TRY
    {
        LauncherApplication::Instance()->Execute(&argc, &argv);
        delete LauncherApplication::Instance();
    }
    CATCH(VisItException)
    {
        delete LauncherApplication::Instance();
        // Catch exceptions but don't bother doing anything.
        retval = -1;
    }
    ENDTRY

    debug1 << "VisIt component launcher exited." << endl;

    return retval;
}

