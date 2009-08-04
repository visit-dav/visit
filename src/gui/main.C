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

#include <visitstream.h>
#include <VisItException.h>
#include <QvisGUIApplication.h>
#include <VisItInit.h>

// ****************************************************************************
//  Function: main
//
//  Purpose:
//    This is the main function for the VisIt gui.
//
//  Notes:      
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 26 09:48:46 PDT 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Nov 17 16:15:04 PST 2000
//    Removed initialization of exceptions and added general initialization.
//
//    Brad Whitlock, Mon Nov 27 16:10:21 PST 2000
//    I added a flag preventing the argument list from being messed with during
//    general initialization.
//
//    Jeremy Meredith, Fri May 11 13:52:51 PDT 2001
//    Added initialization of PlotPluginManager.
//
//    Jeremy Meredith, Thu Jul 26 03:34:42 PDT 2001
//    Added initialization of OperatorPluginManager.
//
//    Brad Whitlock, Wed Sep 26 09:46:36 PDT 2001
//    Modified code so it can handle when plugins fail to load.
//
//    Brad Whitlock, Mon Oct 22 18:25:42 PST 2001
//    Changed the exception handling keywords to macros.
//
//    Jeremy Meredith, Tue Jan 14 17:12:34 PST 2003
//    Added setColorSpec(ManyColor) so that it will try to get a 24-bit
//    visual for the GUI windows, even if 8-bit is the default.
//
//    Hank Childs, Tue Jun  1 14:07:05 PDT 2004
//    Added call to finalize.
//
//    Mark C. Miller, Thu Apr  3 14:36:48 PDT 2008
//    Moved setting of component name to before Initialize
//
//    Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL.
// ****************************************************************************

int
main(int argc, char **argv)
{
    int retval = 0;

    TRY
    {
        // Initialize error logging.
        VisItInit::SetComponentName("gui");
        VisItInit::Initialize(argc, argv, 0, 1, false);

        TRY
        {
            // Create the application instance.
            QvisGUIApplication VisitGUI(argc, argv);

            // Execute the GUI and return its return code.
            retval = VisitGUI.Exec();
        }
        CATCHALL
        {
            retval = -1;
        }
        ENDTRY
    }
    CATCH2(VisItException, e)
    {
        cerr << "VisIt encountered the following fatal error during "
                "initialization: " << endl << e.Message().c_str() << endl;
        retval = -1;
    }
    ENDTRY

    VisItInit::Finalize();
    return retval;
}
