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
#include <LauncherProxy.h>
#include <stdio.h>
#include <VisItInit.h>
#include <ConnectionGroup.h>
#include <LostConnectionException.h>

// ****************************************************************************
// Function: main
//
// Purpose:
//   The main function for the testvcl program.
//
// Notes:      Tests that we get both stdout and stderr from the remote process.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 21 13:46:58 PST 2007
//
// Modifications:
//   
//    Mark C. Miller, Thu Apr  3 14:36:48 PDT 2008
//    Moved setting of component name to before Initialize
// ****************************************************************************

int
main(int argc, char *argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s hostname [args ...]\n", argv[0]);
        return -1;
    }
    std::string launchHost(argv[1]);

    // Initialize error logging.
    VisItInit::SetComponentName("launcher");
    VisItInit::Initialize(argc, argv);

    // Create a launcher proxy object.
    LauncherProxy *launcher = new LauncherProxy;
    if(argc > 2)
    {
        for(int i = 2; i < argc; ++i)
            launcher->AddArgument(argv[i]);
    }
    launcher->AddArgument("-debug");
    launcher->AddArgument("5");

    // Launch VCL and connect to it.
    printf("Creating launcher...\n\n");
    launcher->Create(launchHost, HostProfile::MachineName, "", false, 0, false);

    // Now that we have a launcher, launch a process that writes to stderr
    stringVector args;
    args.push_back("visit");
    args.push_back("-help");
    launcher->LaunchProcess(args);

    // Launch a process that writes to stdout.
    args.pop_back();
    args.push_back("-env");
    launcher->LaunchProcess(args);

    // Open a file to record the output from VCL
    printf("\nOpening testvcl.output file to record the output of the process "
           "launched by vcl. Check the contents of the file to see what "
           "the process output. NOTE that you will need to Ctrl+C this "
           "program to stop it.\n\n");
    FILE *outputFile = fopen("testvcl.output", "wt");

    // Now, do an event loop to watch the proxy's 2nd output socket for 
    // asynchronous output from VCL. Write all output to the output file.
    ConnectionGroup connGroup;
    connGroup.AddConnection(launcher->GetWriteConnection(1));
    bool keepGoing = true;
    while(keepGoing)
    {
        // Check the connections for input that needs to be processed.
        if(connGroup.CheckInput())
        {
            if(connGroup.NeedsRead(0))
            {
                TRY
                {
                    // Try reading input from the launcher process's write socket.
                    printf("Reading from vcl...\n");
                    launcher->GetWriteConnection(1)->Fill();

                    // Print the output that we read to stdout.
                    while(launcher->GetWriteConnection(1)->Size() > 0)
                    {
                        unsigned char c;
                        launcher->GetWriteConnection(1)->Read(&c);
                        fputc((int)c, outputFile);
                    }
                    fflush(outputFile);
                }
                CATCH(LostConnectionException)
                {
                    // Terminate the application.
                    keepGoing = false;
                }
                ENDTRY
            }
        }
    }

    fclose(outputFile);

    launcher->Close();
    delete launcher;

    return 0;
}
