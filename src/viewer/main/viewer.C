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

// ************************************************************************* //
//                                 viewer.C                                  //
// ************************************************************************* //

#include <visit-config.h>

#include <DebugStream.h>
#include <ViewerSubject.h>
#include <VisItException.h>
#include <Init.h>
#include <InitVTK.h>
#include <RemoteProcess.h>
#include <ViewerPasswordWindow.h>
#include <avtCallback.h>

static void ErrorCallback(void *, const char *);
static void ViewerWarningCallback(void *, const char *);

#ifdef AIX
#ifdef HAVE_LIBBZ2
#include <bzlib.h>
void foobz2(void)
{
   BZ2_bzlibVersion();
}
#endif
#endif

// ****************************************************************************
//  Method: main
//
//  Purpose:
//      The viewer main program.
//
//  Arguments:
//      argc    The number of command line arguments.
//      argv    The command line arguments.
//
//  Programmer: Eric Brugger
//  Creation:   August 16, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Oct 27 14:49:04 PST 2000
//    I passed argc, argv to the ViewerSubject constructor and made it
//    return an error code.
//
//    Jeremy Meredith, Fri Nov 17 16:15:04 PST 2000
//    Removed initialization of exceptions and added general initialization.
//
//    Brad Whitlock, Mon Nov 27 17:24:37 PST 2000
//    Changed the call to Init::Initialize.
//
//    Hank Childs, Tue Apr 24 15:23:35 PDT 2001
//    Initialize VTK modules.
//
//    Brad Whitlock, Thu Apr 26 13:34:18 PST 2001
//    Removed top-level catch so uncaught exceptions will cause the code to
//    core. That makes it easier to find the exceptions and more likely that
//    they will be caught.
//
//    Jeremy Meredith, Fri Apr 27 15:46:47 PDT 2001
//    Added initialization of the remote process instantiation
//    authentication callback.
//
//    Jeremy Meredith, Thu May 10 14:56:48 PDT 2001
//    Addded plot plugin manager initialization.
//
//    Jeremy Meredith, Thu Jul 26 03:14:23 PDT 2001
//    Added operator plugin manager initialization.
//
//    Brad Whitlock, Wed Jul 18 09:12:22 PDT 2001
//    Registered a view callback.
//
//    Hank Childs, Mon Aug 20 21:41:59 PDT 2001
//    Changed format for view callbacks.
//
//    Eric Brugger, Tue Aug 21 10:21:18 PDT 2001
//    I removed the registration of the view callback.
//
//    Kathleen Bonnell, Fri Feb  7 09:09:47 PST 2003 
//    I moved the registration of the authentication callback to ViewerSubject.
//
//    Brad Whitlock, Mon Jun 9 11:23:50 PDT 2003
//    I made plugins get loaded later.
//
//    Hank Childs, Fri Aug  8 08:13:21 PDT 2003
//    Register an error function.
//
//    Hank Childs, Tue Jun  1 13:54:48 PDT 2004
//    Call Init::Finalize.
//
//    Hank Childs, Tue Feb 15 12:16:38 PST 2005
//    Register a warning function.
//
//    Brad Whitlock, Mon Feb 12 17:18:15 PST 2007
//    Pass the ViewerSubject address to the error and warning callback 
//    registration functions.
//
// ****************************************************************************

int
main(int argc, char *argv[])
{
    int retval = 0;

    TRY
    {
        //
        // Do basic initialization.
        //
        Init::Initialize(argc, argv, 0, 1, false);

        //
        // Create the viewer subject.
        //
        ViewerSubject viewer;

        //
        // Initialize the error logging.
        //
        Init::SetComponentName("viewer");
        Init::ComponentRegisterErrorFunction(ErrorCallback, (void*)&viewer);
        InitVTK::Initialize();
        avtCallback::RegisterWarningCallback(ViewerWarningCallback, 
            (void*)&viewer);

        //
        // Connect back to the client and perform some initialization.
        //
        viewer.Connect(&argc, &argv);

        //
        // Execute the viewer.
        //
        TRY
        {
            retval = viewer.Execute();
        }
        CATCH2(VisItException, e)
        {
            debug1 << "VisIt's viewer encountered the following uncaught "
                   "exception: " << e.GetExceptionType().c_str()
                   << " from (" << e.GetFilename().c_str()
                   << ":" << e.GetLine() << ")" << endl
                   << e.Message().c_str() << endl;
            retval = -1;
        }
        ENDTRY
    }
    CATCH2(VisItException, e)
    {
        debug1 << "VisIt's viewer encountered the following fatal "
                  "initialization error: " << endl
               << e.Message().c_str() << endl;
        retval = -1;
    }
    ENDTRY

    Init::Finalize();

    return retval;
}


// ****************************************************************************
//  Function: ErrorCallback
//
//  Purpose:
//      A callback routine that can issue error messages.
//
//  Arguments:
//      args    Arguments to the callback.
//      msg     The message to issue.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2003
//
//  Modifications:
//    Brad Whitlock, Mon Feb 12 17:17:49 PST 2007
//    Passed in the ViewerSubject pointer.
//
// ****************************************************************************

static void
ErrorCallback(void *ptr, const char *msg)
{
    ((ViewerSubject *)ptr)->Error(msg);
}


// ****************************************************************************
//  Function: ViewerWarningCallback
//
//  Purpose:
//      A callback routine that can issue warning messages.
//
//  Arguments:
//      args    Arguments to the callback.
//      msg     The message to issue.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2005
//
//  Modifications:
//    Brad Whitlock, Mon Feb 12 17:17:49 PST 2007
//    Passed in the ViewerSubject pointer.
//
// ****************************************************************************

static void
ViewerWarningCallback(void *ptr, const char *msg)
{
    ((ViewerSubject *)ptr)->Warning(msg);
}


