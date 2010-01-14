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

#include <iostream>
#include <cstdlib>
#include <SocketBridge.h>


// ****************************************************************************
// Function: main
//
// Purpose:
//   This is the main function for the VisIt socket relay that forwards the
//   VisIt communication between engine and component launcher.
//
// Notes:      
//
// Programmer: Gunther H. Weber
// Creation:   Thu Jan 14 15:21:38 PST 2010
//
// Modifications:
//
// ****************************************************************************

int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Error: Usage " << argv[0] << " <remoteHost> <remotePort>" << std::endl;
        exit(1);
    }

    const char *remoteHost = argv[1];
    int remotePort = std::atoi(argv[2]);

    // Like in the component launcher we pick a random port and assume that
    // it is available.
    int lowerLocalPort = 10000;
    int upperLocalPort = 40000;
    int localPortRange = 1+upperLocalPort-lowerLocalPort;
    srand48(long(time(0)));
    int newlocalport = lowerLocalPort+(lrand48()%localPortRange);
    std::cout << newlocalport << std::endl;

    switch (fork())
    {
        case -1:
            // Could not fork.
            exit(-1);
            break;
        case 0:
            {
                // The child process will start the bridge
                // Close stdin and any other file descriptors.
                fclose(stdin);
                for (int k = 3 ; k < 32 ; ++k)
                {
                    close(k);
                }
                SocketBridge bridge(newlocalport, remotePort, remoteHost);
                bridge.Bridge();
                exit(0);
                break;
            }
        default:
            // Parent process continues on as normal
            // Caution: there is a slight race condition here, though
            // it would require the engine to launch and try to connect
            // back before the child process got the bridge set up.
            // The odds of this happening are low, but it should be fixed.
            break;
    }
}
