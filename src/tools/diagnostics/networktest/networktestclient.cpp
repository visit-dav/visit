#include <DebugStream.h>
#include <ExistingRemoteProcess.h>
#include <Connection.h>
#include <LaunchProfile.h>
#include <MachineProfile.h>
#include <VisItInit.h>

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <algorithm>

#ifdef _WIN32
#include <process.h>
#include <winsock2.h>
#else
#include <sys/select.h>
#include <unistd.h>
#endif

using std::cout;
using std::endl;

// ****************************************************************************
// Class: Client
//
// Purpose:
//   A client class that helps us test networking.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 14:56:08 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class Client
{
public:
    Client() : remote(NULL), programArgs()
    {
    }

    ~Client()
    {
    }

    void PrintUsage(const char *argv0)
    {
        cout << "Usage: " << argv0 << " serverprogram [args]" << endl;
        cout << endl;
        cout << "Examples:" << endl;
        cout << "    " << argv0 << " visittestserver" << endl;
        cout << "    " << argv0 << " srun -N1 -n16 visittestserver_par" << endl;
    }

    void CaptureArgs(int argc, char *argv[])
    {
        for(int i = 1; i < argc; ++i)
            programArgs.push_back(argv[i]);
    }

    bool Initialize(int argc, char *argv[])
    {
        if(argc < 2)
        {
            PrintUsage(argv[0]);
            return false;
        }

        std::string program(argv[1]);
        MachineProfile profile;
        LaunchProfile launch;
        launch.SetProfileName("serial");
        profile.AddLaunchProfiles(launch);
        profile.SetActiveProfile(0);

        remote = new ExistingRemoteProcess(program);
        remote->SetConnectCallback(ConnectCallback);
        remote->SetConnectCallbackData((void *)this);
        for(int i = 2; i < argc; ++i)
            remote->AddArgument(argv[i]);

        return remote->Open(profile, 1, 1, true);
    }

    void EventLoop()
    {
        bool keepGoing = true;
        Connection *fromServer = remote->GetWriteConnection();
        Connection *toServer = remote->GetReadConnection();
#ifdef _WIN32
        int consoleFileDescriptor = -1;
#else
        int consoleFileDescriptor = fileno(stdin);
#endif

        cout << "Waiting for input....\n> " << endl;

        while(keepGoing)
        {
            int maxDescriptor = std::max(consoleFileDescriptor,
                                         fromServer->GetDescriptor());

            fd_set readSet;
            FD_ZERO(&readSet);
            if(consoleFileDescriptor > -1)
                FD_SET(consoleFileDescriptor, &readSet);
            FD_SET(fromServer->GetDescriptor(), &readSet);

            int status = select(maxDescriptor+1,
                                &readSet, (fd_set*)NULL, (fd_set*)NULL, NULL);
            if(status > 0)
            {
                if(FD_ISSET(consoleFileDescriptor, &readSet))
                {
                    char buffer[1000];
                    if(fgets(buffer, 1000, stdin) != NULL)
                    {
                        int len = (int)strlen(buffer);
                        if(len > 0 && buffer[len-1] == '\n')
                        {
                            buffer[len-1] = '\0';
                            --len;
                        }

                        if(len > 0)
                        {
                            cout << "Client: sending \"" << buffer << "\"" << endl;
                            toServer->Append((const unsigned char *)buffer, len+1);
                            toServer->Flush();

                            if(strcmp(buffer, "quit") == 0)
                                keepGoing = false;
                        }
                    }
                }
                else if(FD_ISSET(fromServer->GetDescriptor(), &readSet))
                {
                    fromServer->Fill();
                    PrintConnection(fromServer);

                    cout << "> ";
                }
            }
        }
    }

    void PrintConnection(Connection *c)
    {
        int size = c->Size();
        char *s = new char[size + 1];
        for(int i = 0; i < size; ++i)
            c->Read((unsigned char *)(s + i));
        s[size] = '\0';

        cout << "Client: received: \"" << s << "\"" << endl;

        delete [] s;
    }

    static void ConnectCallback(const std::string &remoteHost,
                                const stringVector &args, void *cbdata)
    {
        Client *This = (Client *)cbdata;
        This->Connect(remoteHost, args);
    }

    bool Find(const stringVector &haystack, const std::string &needle) const
    {
        for(size_t i = 0; i < haystack.size(); ++i)
    {
        if(haystack[i] == needle)
            return true;
        }
        return false;
    }

    void Connect(const std::string &remoteHost, const stringVector &args)
    {
        // We want to launch the program ourselves. We can probably ignore most of
        // what was constructed for us.
        cout << "remoteHost=" << remoteHost << endl;
        std::string host, port, key, debug, clobber_vlogs;
        for(size_t i = 0; i < args.size(); ++i)
        {
            if(args[i] == "-host" && (i+1) < args.size())
            {
                host = args[i+1]; ++i;
            }
            else if(args[i] == "-port" && (i+1) < args.size())
            {
                port = args[i+1]; ++i;
            }
            else if(args[i] == "-key" && (i+1) < args.size())
            {
                key = args[i+1]; ++i;
            }
            else if(args[i] == "-debug" && (i+1) < args.size())
            {
                debug = args[i+1]; ++i;
            }
            else if(args[i] == "-clobber_vlogs")
            {
                clobber_vlogs = args[i];
            }
       }

        std::vector<std::string> newArgs(programArgs);
        if(!host.empty() && !Find(newArgs, host))
        {
            newArgs.push_back("-host");
            newArgs.push_back(host);
        }
        if(!port.empty() && !Find(newArgs, port))
        {
            newArgs.push_back("-port");
            newArgs.push_back(port);
        }
        if(!key.empty() && !Find(newArgs, key))
        {
            newArgs.push_back("-key");
            newArgs.push_back(key);
        }
        if(!debug.empty() && !Find(newArgs, debug))
        {
            newArgs.push_back("-debug");
            newArgs.push_back(debug);
        }
        if(!clobber_vlogs.empty() && !Find(newArgs, clobber_vlogs))
        {
            newArgs.push_back(clobber_vlogs);
        }
        for(size_t i = 0; i < newArgs.size(); ++i)
            cout << "newArgs[" << i << "] = " << newArgs[i] << endl;

        // Run the program.
        Launch(newArgs);
    }

    void Launch(const stringVector &args)
    {
        const char *mName = "Client::Launch ";

        // 
        // Create the parameters for the exec
        //
        int  argc = 0;
        char **argv = CreateSplitCommandLine(args, argc);

        //
        // Start the program on localhost.
        //
#if defined(_WIN32)
        // Do it the WIN32 way where we use the _spawnvp system call.
        debug5 << mName << "Starting child process using _spawnvp" << endl;
        _spawnvp(_P_NOWAIT, argv[0], argv);
#else
        debug5 << mName << "Starting child process using fork" << endl;
        int rv, i;
        switch (fork())
        {
        case -1:
            // Could not fork.
            exit(-1); // HOOKS_IGNORE
            break;
        case 0:
            // Close stdin and any other file descriptors.
            fclose(stdin);
            for (int k = 3 ; k < 32 ; ++k)
            {
                close(k);
            }
            // Execute the process on the local machine.
            rv = execvp(argv[0], argv);
            if (rv < 0) // should only return from execvp if rv < 0
            {
                cerr << "Unable to launch program " << argv[0] << endl;
                cerr << "Command was: ";
                for (i = 0 ; i < argc ; i++)
                    cerr << argv[i] << " ";
                cerr << endl;
            }
            exit(-1); // HOOKS_IGNORE
            break;   // OCD
        default:
            break;
        }
#endif

        // Clean up memory
        DestroySplitCommandLine(argv, argc);
    }

    char **
    CreateSplitCommandLine(const stringVector &args, int &argc) const
    {
        argc = (int)args.size();
        char **retval = new char*[argc + 1];
        for(int i = 0; i < argc; ++i)
            retval[i] = strdup(args[i].c_str());
        retval[argc] = NULL;

        return retval;
    }

    void DestroySplitCommandLine(char **args, int argc) const
    {
        for (int i = 0 ; i < argc ; ++i)
            free(args[i]);
        delete [] args;
    }

private:
    ExistingRemoteProcess *remote;
    stringVector           programArgs;
};

int
main(int argc, char *argv[])
{
    Client client;
    client.CaptureArgs(argc, argv);

    // Initialize
    VisItInit::SetComponentName(argv[0]);
    VisItInit::Initialize(argc, argv, 0, 1, true, true);

    if(client.Initialize(argc, argv))
    {
        client.EventLoop();
    }

    VisItInit::Finalize();
    cout << argv[0] << " exiting." << endl;

    return 0;
}
