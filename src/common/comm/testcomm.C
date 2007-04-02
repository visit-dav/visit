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

#include <visitstream.h>
#include <string>
#include <string.h>

#include <RemoteProcess.h>
#include <ParentProcess.h>
#include <Connection.h>

#include <BadHostException.h>
#include <CancelledConnectException.h>
#include <CouldNotConnectException.h>
#include <IncompatibleVersionException.h>
#include <IncompatibleSecurityTokenException.h>

#include <DebugStream.h>

using std::string;

#define MSG_LENGTH 100

// ****************************************************************************
// Function: main
//
// Purpose:
//   This is the main function for testcomm, a program that tests RemoteProcess
//   and ParentProcess classes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 2 11:01:42 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

int
main(int argc, char *argv[])
{
    int    retval = 0;
    bool   isLauncher = true;
    string host("localhost");
    char buf[MSG_LENGTH];
    buf[0] = '\0';

    // Look through the command line args.
    for(int i = 1; i < argc; ++i)
    {
        string arg(argv[i]);
        if(arg == "-child")
        {
            isLauncher = false;
        }
        else if(arg == "-host")
        {
            if(i+1 < argc)
            {
                host = argv[i + 1];
                ++i;
            }
        }
    }

    if(isLauncher)
    {
        DebugStream::Initialize("testcomm_parent", 2);

        // Create the parent process object.
        RemoteProcess parent("testcomm");
        parent.AddArgument("-child");

        TRY
        {
            cout << "Opening a remote process on " << host.c_str() << endl;
            // Try and create a remote process.
            parent.Open(host, 1, 1);

            // Wait for the child to send some data on its write connection.
            parent.GetWriteConnection(0)->DirectRead((unsigned char *)buf,
                                                     MSG_LENGTH);
            // Print out the message.
            cout << "MESSAGE=" << buf << endl;

            parent.WaitForTermination();
        }
        CATCH(BadHostException)
        {
            cerr << "PARENT: Caught BadHostException. " << host.c_str()
                 << " is not a valid host name." << endl;
            retval = -1;
        }
        CATCH(CouldNotConnectException)
        {
            cerr << "PARENT: Caught CouldNotConnectException" << endl;
            retval = -1;
        }
        CATCH(IncompatibleVersionException)
        {
            cerr << "PARENT: Caught IncompatibleVersionException" << endl;
            retval = -1;
        }
        CATCH(IncompatibleSecurityTokenException)
        {
            cerr << "PARENT: Caught IncompatibleSecurityTokenException" << endl;
            retval = -1;
        }
        CATCH(CancelledConnectException)
        {
            cerr << "PARENT: Caught CancelledConnectException" << endl;
            retval = -1;
        }
        ENDTRY        
    }
    else
    {
        DebugStream::Initialize("testcomm_child", 2);

        // Create the child process (the remote one) and connect back to
        // the parent.
        ParentProcess child;

        TRY
        {
            // connect to the parent.
            child.Connect(&argc, &argv, true);

            // Create a message to send back to the parent.
            strcpy(buf, "This is a message from the remote process!");

            // Send a message to the parent.
            child.GetReadConnection(0)->DirectWrite((unsigned char *)buf,
                                                    MSG_LENGTH);
        }
        CATCH(CouldNotConnectException)
        {
            cerr << "CHILD: Caught CouldNotConnectException" << endl;
            retval = -1;
        }
        CATCH(IncompatibleVersionException)
        {
            cerr << "CHILD: Caught IncompatibleVersionException" << endl;
            retval = -1;
        }
        CATCH(IncompatibleSecurityTokenException)
        {
            cerr << "CHILD: Caught IncompatibleSecurityTokenException" << endl;
            retval = -1;
        }
        CATCH(CancelledConnectException)
        {
            cerr << "CHILD: Caught CancelledConnectException" << endl;
            retval = -1;
        }
        ENDTRY
    }

    return retval;
}
