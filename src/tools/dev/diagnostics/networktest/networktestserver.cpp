#include <stdio.h>
#include <string.h>

#include <DebugStream.h>
#include <ParentProcess.h>
#include <Connection.h>
#include <VisItInit.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

// ****************************************************************************
// Class: Server
//
// Purpose:
//   This Server class connects to the client that launched it via sockets and
//   reads messages from the client (broadcasting to other ranks if needed) and
//   sends the message back to the client.
//
// Notes:    This program is meant to aid in debugging network connections when
//           setting up new machines.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 14:06:07 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class Server
{
public:
    Server() : par_rank(0), par_size(1), parent(), messageNumber(1)
    {
#ifdef PARALLEL
        MPI_Comm_rank(MPI_COMM_WORLD, &par_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &par_size);
#endif
    }

    ~Server()
    {
    }

    bool Initialize(int *argc, char **argv[])
    {
        int status = 0;
        if(par_rank == 0)
            status = parent.Connect(1, 1, argc, argv, true) ? 1 : 0;
#ifdef PARALLEL
        MPI_Bcast((void*)&status, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
        return status > 0;
    }

    void EventLoop()
    {
        Connection *fromClient = parent.GetWriteConnection();
        Connection *toClient = parent.GetReadConnection();
        bool keepGoing = true;

        while(keepGoing)
        {
            bool haveMessage = true;
            if(par_rank == 0)
                haveMessage = fromClient->NeedsRead(true);

            if(haveMessage)
            {
                // Read the message and send it to the other ranks.
                int amountRead = 0;
                if(par_rank == 0)
                    amountRead = fromClient->Fill();
#ifdef PARALLEL
                MPI_Bcast((void*)&amountRead, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
                if(amountRead > 0)
                {
                    char *str = new char[amountRead + 1];
                    if(par_rank == 0)
                    {
                        for(int i = 0; i < amountRead; ++i)
                            fromClient->Read((unsigned char *)(str + i));
                        str[amountRead] = '\0';
                    }
                    
#ifdef PARALLEL
                    MPI_Bcast((void*)str, amountRead+1, MPI_CHAR, 0, MPI_COMM_WORLD);
#endif

                    if(par_rank == 0)
                        cout << "Server: received: \"" << str << "\"" << endl;
                    debug1 << "MESSAGE FROM CLIENT: " << messageNumber << ": " << str << endl;
                    messageNumber++;

                    // Send the message back to the client.
                    if(par_rank == -1)
                    {
                        debug1 << "SENDING MESSAGE TO CLIENT: " << str << endl;
                        toClient->Append((const unsigned char *)str, amountRead+1);
                        toClient->Flush();
                    }

                    if(strcmp(str, "quit") == 0)
                        keepGoing = false;

                    delete [] str;
                }
            }
        }
    }

    int PARRank() const { return par_rank; }
    int PARSize() const { return par_size; }

private:
    int           par_rank;
    int           par_size; 
    ParentProcess parent;
    int           messageNumber;
};


int
main(int argc, char *argv[])
{
#ifdef PARALLEL
    /* Init MPI */
    MPI_Init(&argc, &argv);
#endif
    Server server;

    // Initialize
    VisItInit::SetComponentName(argv[0]);
    VisItInit::Initialize(argc, argv, server.PARRank(), server.PARSize(), true, true);

    if(server.Initialize(&argc, &argv))
    {
        server.EventLoop();
    }

    VisItInit::Finalize();
    cout << argv[0] << " exiting." << endl;
#ifdef PARALLEL
    /* Finalize MPI */
    MPI_Finalize();
#endif

    return 0;
}
