// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

#include <SocketBridge.h>
#include <StringHelpers.h>


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
//    Mark C. Miller, Fri Jan 12 17:04:46 PST 2024
//    Replace atoi with vstrtonum
// ****************************************************************************

int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Error: Usage " << argv[0] << " <remoteHost> <remotePort>" << std::endl;
        exit(1); // HOOKS_IGNORE
    }

    const char *remoteHost = argv[1];
    int remotePort = StringHelpers::vstrtonum<int>(argv[2]);

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
            exit(-1); // HOOKS_IGNORE
            break;
        case 0:
            {
                // The child process will start the bridge
                // Close stdin and any other file descriptors.
                std::fclose(stdin);
                for (int k = 3 ; k < 32 ; ++k)
                {
                    close(k);
                }
                SocketBridge bridge(newlocalport, remotePort, remoteHost);
                bridge.Bridge();
                exit(0); // HOOKS_IGNORE
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
    return 0;
}
