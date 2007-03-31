#include <stdio.h>   // for sprintf
#include <stdlib.h>
#include <snprintf.h>
#include <ViewerFileServer.h>

#include <avtDatabaseMetaData.h>
#include <avtSIL.h>
#include <BadHostException.h>
#include <GetMetaDataException.h>
#include <HostProfileList.h>
#include <HostProfile.h>
#include <LostConnectionException.h>
#include <IncompatibleVersionException.h>
#include <IncompatibleSecurityTokenException.h>
#include <CouldNotConnectException.h>
#include <CancelledConnectException.h>
#include <MDServerProxy.h>
#include <ParentProcess.h>
#include <ViewerConnectionProgressDialog.h>
#include <ViewerMessaging.h>
#include <ViewerWindowManager.h>
#include <ViewerAnimation.h>

#include <DebugStream.h>

// A static pointer to the one and only instance of ViewerFileServer
ViewerFileServer *ViewerFileServer::instance = NULL;

// ****************************************************************************
// Method: ViewerFileServer::ViewerFileServer
//
// Purpose: 
//   Constructor for the ViewerFileServer class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 21 12:24:52 PDT 2000
//
// Modifications:
//   Jeremy Meredith, Fri Apr 27 15:40:09 PDT 2001
//   Made it catch VisItExceptions from StartServer.
//
//   Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//   Changed the exception keywords to macros.
//
//   Brad Whitlock, Fri May 3 16:18:56 PST 2002
//   Made it inherit from ViewerServerManager.
//
//   Brad Whitlock, Fri Jan 17 12:32:22 PDT 2003
//   I removed the code to start a local mdserver.
//
// ****************************************************************************

ViewerFileServer::ViewerFileServer() : ViewerServerManager(), servers(),
    fileMetaData(), fileSIL()
{
}

// ****************************************************************************
// Method: ViewerFileServer::ViewerFileServer
//
// Purpose: 
//   Copy constructor that does nothing.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 3 16:20:07 PST 2002
//
// Modifications:
//   
// ****************************************************************************

ViewerFileServer::ViewerFileServer(const ViewerFileServer &) : ViewerServerManager()
{
}

// ****************************************************************************
// Method: ViewerFileServer::~ViewerFileServer
//
// Purpose:
//   Destructor for the ViewerFileServer class.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 21 12:25:37 PDT 2000
//
// Modifications:
//
// ****************************************************************************

ViewerFileServer::~ViewerFileServer()
{
    //
    // This should never be called.
    //
}

// ****************************************************************************
// Method: ViewerFileServer::CloseServers
//
// Purpose: 
//   Closes down all of the mdservers and cleans out the cached file metadata.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 21 12:26:20 PDT 2000
//
// Modifications:
//   Hank Childs, Thu Jun 14 08:54:57 PDT 2001
//   Added deletion of SIL attributes.
//
//   Brad Whitlock, Thu Feb 7 15:18:06 PST 2002
//   Changed the SIL attributes to a SIL.
//
//   Brad Whitlock, Fri Dec 27 14:47:54 PST 2002
//   I added debugging information.
//
// ****************************************************************************

void
ViewerFileServer::CloseServers()
{
    // Iterate through the server list, close and destroy each
    // server and remove the server from the map.
    ServerMap::iterator pos;
    for(pos = servers.begin(); pos != servers.end(); ++pos)
    {
        TRY
        {
            debug1 << "Closing metadata server on " << pos->first.c_str()
                   << "." << endl;

            // Tell the server to quit.
            pos->second->proxy->Close();
        }
        CATCHALL(...)
        {
            debug1 << "Caught an exception while closing the server."
                   << endl;
        }
        ENDTRY

        // Delete the server
        delete pos->second;
    }
    servers.clear();

    // Iterate through the file metadata list, detroying each
    // metadata entry.
    FileMetaDataMap::iterator fpos;
    for(fpos = fileMetaData.begin(); fpos != fileMetaData.end(); ++fpos)
    {
        // Delete the metadata
        delete fpos->second;
    }
    fileMetaData.clear();
    FileSILMap::iterator fpos2;
    for (fpos2 = fileSIL.begin() ; fpos2 != fileSIL.end() ; fpos2++)
    {
        // Delete the SIL
        delete fpos2->second;
    }
    fileSIL.clear();      
}

// ****************************************************************************
// Method: ViewerFileServer::Instance
//
// Purpose: 
//   Returns a pointer to the only ViewerFileServer object that can be created.
//
// Returns:    A pointer to the only ViewerFileServer object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 21 12:27:31 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

ViewerFileServer *
ViewerFileServer::Instance()
{
    // If the sole instance hasn't been instantiated, then instantiate it.
    if(instance == NULL)
    {
        instance = new ViewerFileServer;
    }

    return instance;
}

// ****************************************************************************
// Method: ViewerFileServer::MetaDataIsInvariant
//
// Purpose: 
//   We include this method because in some cases all we are interested in
//   learning from a GetMetaData call is whether the MetaData is invariant.
//   We'd like to do it WITHOUT sending RPCs to the MD server. Even in the
//   case that the MetaData is NOT invariant, all we need to avoid an RPC to
//   the MD server is MetaData from any one time-state of the specified
//   database. If no such MetaData exists, this method will then call
//   GetMetaData to obtain it. For this reason, this method cannot be a const
//   method.
//
// Arguments:
//   host     : Identifies the mdserver from which to get the metadata.
//   filename : The path and filename of the file for which we want metadata.
//
// ****************************************************************************
bool
ViewerFileServer::MetaDataIsInvariant(const std::string &host, 
                                      const std::string &filename,
                                      const int timeState)
{
    int workingTimeState = -1;
    if (timeState < 0)
        workingTimeState = ViewerWindowManager::Instance()->
                                          GetActiveAnimation()->GetTimeIndex();
    else
        workingTimeState = timeState;

    // Create a filename of the form host:filename.
    std::string fullname(host);
    fullname += ":";
    fullname += filename;

    // search for any entry in the cache that at least begins host:filename 
    FileMetaDataMap::iterator i;
    for (i = fileMetaData.begin(); i != fileMetaData.end(); i++)
    {
       if (i->first.find(fullname) != std::string::npos)
          return ! i->second->GetMustRepopulateOnStateChange();
    }

    // ok, do it the hard way
    const avtDatabaseMetaData *md = GetMetaData(host, filename, timeState);

    return ! md->GetMustRepopulateOnStateChange();
}

// ****************************************************************************
// Method: ViewerFileServer::GetMetaData
//
// Purpose: 
//   Gets metadata for the specified file.
//
// Arguments:
//   host     : Identifies the mdserver from which to get the metadata.
//   filename : The path and filename of the file for which we want metadata.
//
// Returns:    A pointer to a metadata object for the specified file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 21 12:23:28 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Apr 23 14:32:35 PST 2001
//   Added error message code.
//
//   Brad Whitlock, Tue Apr 24 18:02:20 PST 2001
//   Added code to detect and restart dead mdservers.
//
//   Jeremy Meredith, Fri Apr 27 15:40:09 PDT 2001
//   Made it catch VisItExceptions from StartServer.
//
//   Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//   Changed the exception keywords to macros.
//
//   Brad Whitlock, Fri Dec 14 15:55:03 PST 2001
//   Fixed a crash when the metadata cannot be read.
//
//   Brad Whitlock, Thu Feb 7 15:19:15 PST 2002
//   Modified error messages that are returned. Fixed a metadata caching
//   bug.
//
//   Brad Whitlock, Tue Feb 12 15:09:40 PST 2002
//   Moved some code into the function NoFaultStartServer.
//
//   Hank Childs, Mon May  6 16:08:40 PDT 2002
//   Fixed typo in error statement (mets-data -> metadata).
//
//   Brad Whitlock, Mon Jan 13 10:41:36 PDT 2003
//   I made it restart the mdserver with the old arguments.
//
//   Brad Whitlock, Wed May 14 11:49:45 PDT 2003
//   I added a timeState argument.
//
//   Hank Childs, Mon Sep 15 17:18:36 PDT 2003
//   Account for databases that change over time.
//
// ****************************************************************************

const avtDatabaseMetaData *
ViewerFileServer::GetMetaData(const std::string &host, 
                              const std::string &filename,
                              const int timeState)
{
    int workingTimeState = -1;
    if (timeState < 0)
        workingTimeState = ViewerWindowManager::Instance()->
                                          GetActiveAnimation()->GetTimeIndex();
    else
        workingTimeState = timeState;
    // Create a filename of the form host:filename.
    std::string fullname(host);
    fullname += ":";
    fullname += filename;

    // If the filename is in the cache, return its metadata, otherwise query
    // the appropriate mdserver for it.
    if(fileMetaData.find(fullname) != fileMetaData.end())
    {
        return fileMetaData[fullname];
    }

    // Some databases have meta-data that varies over time.  If this is the
    // case, then we store the timeState with the meta-data.  Give this a try
    // as well.
    std::string fullname_state(host);
    fullname_state += ":";
    fullname_state += filename;
    fullname_state += ":";
    char state_str[64];
    sprintf(state_str, "%d", workingTimeState);
    fullname_state += state_str;
    if(fileMetaData.find(fullname_state) != fileMetaData.end())
    {
        return fileMetaData[fullname_state];
    }

    // Try and start a server if one does not exist.
    NoFaultStartServer(host);

    // Initialize the return value.
    const avtDatabaseMetaData *retval = NULL;

    // If a server exists, get the metadata.
    if(servers.find(host) != servers.end())
    {
        int numAttempts = 0;
        bool tryAgain = false;

        // We have a server from which we can get the metadata for the 
        // specified filename so let's try and get it now and put it in the
        // metadata map.
        do
        {
            tryAgain = false;

            TRY
            {
                const avtDatabaseMetaData *md =
                    servers[host]->proxy->GetMetaData(filename, 
                                                      workingTimeState);

                if(md != NULL)
                {
                    avtDatabaseMetaData *mdCopy = new avtDatabaseMetaData(*md);

                    // If the meta-data changes for each state, then cache
                    // the meta-data on a per state basis.  This is done by
                    // encoding the state into the name.
                    std::string key;
                    if (mdCopy->GetMustRepopulateOnStateChange())
                        key = fullname_state;
                    else
                        key = fullname;
                    fileMetaData[key] = mdCopy;

                    retval = mdCopy;
                }
            }
            CATCH2(GetMetaDataException, gmde)
            {
                char msg[1000];
                SNPRINTF(msg, 1000, "VisIt cannot read the metadata for the file "
                         "\"%s\" on host %s.\n\nThe metadata server returned "
                         "the following message:\n\n%s", filename.c_str(),
                        host.c_str(), gmde.GetMessage().c_str());
                Error(msg);
            }
            CATCH(LostConnectionException)
            {
                // Tell the GUI that the mdserver is dead.
                if(numAttempts == 0)
                {
                    char message[200];
                    SNPRINTF(message, 200, "The metadata server running on host "
                             "%s has exited abnormally. VisIt is trying to "
                             "restart it.", host.c_str());
                    Warning(message);
                }

                ++numAttempts;
                tryAgain = (numAttempts < 2);

                TRY
                {
                    stringVector startArgs(servers[host]->arguments);
                    CloseServer(host, false);
                    StartServer(host, startArgs);
                }
                CATCHALL(...)
                {
                    tryAgain = false;
                }
                ENDTRY
            }
            ENDTRY
        } while(tryAgain);
    }

    return retval;
}

// ****************************************************************************
// Method: ViewerFileServer::GetSIL
//
// Purpose: 
//   Gets the sil for the specified file.
//
// Arguments:
//   host     : Identifies the mdserver from which to get the sil.
//   filename : The path and filename of the file for which we want the sil.
//
// Returns:    A pointer to the SIL attributes.
//
// Programmer: Hank Childs
// Creation:   June 13, 2001
//
// Modifications:
//   Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//   Changed the exception keywords to macros.
//
//   Eric Brugger, Thu Nov 29 13:42:56 PST 2001
//   I corrected a bug that prevented the caching of SILs to work where the
//   hostname was used as the key to store the SIL into the cache instead
//   of the hostname and filename concatenation.
//
//   Brad Whitlock, Fri Dec 14 15:55:35 PST 2001
//   Fixed a crash when the SIL cannot be read.
//
//   Brad Whitlock, Thu Feb 7 15:21:17 PST 2002
//   Made it store an avtSIL instead of SILAttributes.
//
//   Brad Whitlock, Tue Feb 12 15:09:40 PST 2002
//   Moved some code into the function NoFaultStartServer.
//
//   Hank Childs, Mon May  6 16:08:40 PDT 2002
//   Fixed typo in error statement (mets-data -> metadata).
//
//   Brad Whitlock, Wed May 14 11:49:45 PDT 2003
//   I added a timeState argument.
//
//   Hank Childs, Mon Sep 15 17:18:36 PDT 2003
//   Account for SILs that change over time.
//
// ****************************************************************************

const avtSIL *
ViewerFileServer::GetSIL(const std::string &host, const std::string &filename,
    const int timeState)
{
    int workingTimeState = -1;
    if (timeState < 0)
        workingTimeState = ViewerWindowManager::Instance()->
                                          GetActiveAnimation()->GetTimeIndex();
    else
        workingTimeState = timeState;

    // Create a filename of the form host:filename:time.
    // If we request a SIL from time A, and then request it from time B,
    // it will calculate a new SIL, even if the SIL does not change over time.
    // However, this routine will only be called one time if the SIL does
    // not change over time, so this is a non-issue.
    std::string fullName(host);
    fullName += ":";
    fullName += filename;
    fullName += ":";
    char state_str[64];
    sprintf(state_str, "%d", workingTimeState);
    fullName += state_str;

    // If the filename is in the cache, return its SIL attributes,
    // otherwise query the appropriate mdserver for it.
    if(fileSIL.find(fullName) != fileSIL.end())
    {
        return fileSIL[fullName];
    }

    // Try and start a server if one does not exist.
    NoFaultStartServer(host);

    // Initialize the return value.
    const avtSIL *retval = NULL;

    // If a server exists, get the metadata.
    if(servers.find(host) != servers.end())
    {
        int numAttempts = 0;
        bool tryAgain = false;

        // We have a server from which we can get the SIL atts for the 
        // specified filename so let's try and get it now and put it in the
        // map.
        do
        {
            tryAgain = false;

            TRY
            {
                const SILAttributes *atts =
                    servers[host]->proxy->GetSIL(filename, workingTimeState);

                if(atts != NULL)
                {
                    avtSIL *silCopy = new avtSIL(*atts);
                    fileSIL[fullName] = silCopy;
                    retval = silCopy;
                }
            }
            // if we can't get the SIL correctly, it also throws an metadata
            // exception (so this isn't a cut-n-paste error from GetMetaData)
            CATCH2(GetMetaDataException, gmde)
            {
                char msg[1000];
                SNPRINTF(msg, 1000, "VisIt cannot read the SIL for the file "
                         "\"%s\" on host %s.\n\nThe metadata server returned "
                         "the following message:\n\n%s", filename.c_str(),
                         host.c_str(), gmde.GetMessage().c_str());
                Error(msg);
            }
            CATCH(LostConnectionException)
            {
                // Tell the GUI that the mdserver is dead.
                if(numAttempts == 0)
                {
                    char message[200];
                    SNPRINTF(message, 200, "The metadata server running on host "
                             "%s has exited abnormally. VisIt is trying to "
                             "restart it.", host.c_str());
                    Warning(message);
                }

                ++numAttempts;
                tryAgain = (numAttempts < 2);

                TRY
                {
                    stringVector startArgs(servers[host]->arguments);
                    CloseServer(host, false);
                    StartServer(host, startArgs);
                }
                CATCHALL(...)
                {
                    tryAgain = false;
                }
                ENDTRY
            }
            ENDTRY
        } while(tryAgain);
    }

    return retval;
}

// ****************************************************************************
// Method: ViewerFileServer::ExpandedFileName
//
// Purpose: 
//   Returns the filename that gets expanded by the mdserver.
//
// Arguments:
//   host     : The host on which the mdserver is running.
//   filename : The filename to expand.
//
// Returns:   The expanded filename. 
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 15:12:46 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Jan 13 16:36:11 PST 2003
//   I added code to handle a LostConnectionException.
//
// ****************************************************************************

std::string
ViewerFileServer::ExpandedFileName(const std::string &host,
    const std::string &filename)
{
    std::string retval(filename);

    // Try and start a server.
    NoFaultStartServer(host);

    if(servers.find(host) != servers.end())
    {
        int  numAttempts = 0;
        bool tryAgain = false;

        do
        {
            TRY
            {
                retval = servers[host]->proxy->ExpandPath(filename);
                tryAgain = false;
            }
            CATCH(LostConnectionException)
            {
                // Tell the GUI that the mdserver is dead.
                if(numAttempts == 0)
                {
                    char message[200];
                    SNPRINTF(message, 200, "The metadata server running on host "
                             "%s has exited abnormally. VisIt is trying to "
                             "restart it.", host.c_str());
                    Warning(message);
                }

                ++numAttempts;
                tryAgain = (numAttempts < 2);

                TRY
                {
                    stringVector startArgs(servers[host]->arguments);
                    CloseServer(host, false);
                    StartServer(host, startArgs);
                }
                CATCHALL(...)
                {
                    tryAgain = false;
                }
                ENDTRY
            }
            ENDTRY
        } while(tryAgain);
    }

    return retval;
}

// ****************************************************************************
// Method: ViewerFileServer::StartServer
//
// Purpose: 
//   Starts a mdserver on the specified host.
//
// Arguments:
//   host : The host on which to start the server.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 13 10:39:29 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerFileServer::StartServer(const std::string &host)
{
    stringVector noArgs;
    StartServer(host, noArgs);
}

// ****************************************************************************
// Method: ViewerFileServer::StartServer
//
// Purpose: 
//   Starts an mdserver on the specified host.
//
// Arguments:
//   host : The host on which to start the mdserver.
//
// Returns:    
//
// Note:       A server will not be started on the specified host if one
//             already exists.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 21 12:21:58 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Nov 27 17:38:23 PST 2000
//   Added code to pass the -debug flag if the debugLevel has been set.
//
//    Jeremy Meredith, Fri Apr 20 10:35:06 PDT 2001
//    Added code to pass other arguments.
//
//    Brad Whitlock, Mon Apr 23 14:34:31 PST 2001
//    Added some error messages.
//
//    Jeremy Meredith, Fri Apr 27 15:44:56 PDT 2001
//    Added catching of CouldNotConnectException.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Sean Ahern, Thu Feb 21 14:11:59 PST 2002
//    Added timeout support.
//
//    Brad Whitlock, Fri May 3 17:17:15 PST 2002
//    Made it use the host profiles.
//
//    Brad Whitlock, Fri Sep 27 15:58:29 PST 2002
//    I added support for launch progress.
//
//    Brad Whitlock, Thu Dec 26 16:30:39 PST 2002
//    I changed the exception handling code to handle several types of 
//    exceptions more generally.
//
//    Brad Whitlock, Mon Jan 13 10:33:41 PDT 2003
//    I added support for launching the mdserver with more arguments.
//
//    Brad Whitlock, Wed May 7 10:00:56 PDT 2003
//    I moved some code into the base class and made it possible to launch
//    an mdserver through a launcher program.
//
//    Jeremy Meredith, Thu Jun 26 10:49:08 PDT 2003
//    Make the mdserver launch under the vcl if we're sharing a batch
//    job with the engine.
//
//    Jeremy Meredith, Thu Oct  9 13:56:12 PDT 2003
//    Added ability to manually specify a client host name or to have it
//    parsed from the SSH_CLIENT (or related) environment variables.  Added
//    ability to specify an SSH port.
//
//    Eric Brugger, Wed Dec  3 10:49:39 PST 2003
//    I modified the routine to add the profile arguments before the unknown
//    arguments so that they can override the profile arguments.
//
// ****************************************************************************

void
ViewerFileServer::StartServer(const std::string &host, const stringVector &args)
{
    // If the server already exists, return.
    if(servers.find(host) != servers.end())
        return;

    ViewerConnectionProgressDialog *dialog = 0;

    // Create a new MD server on the remote machine.
    MDServerProxy *newServer = new MDServerProxy;
    TRY
    {
        // Add arguments from a matching host profile to the mdserver proxy.
        AddProfileArguments(newServer, host);

        // Add regular arguments to the new mdserver proxy.
        AddArguments(newServer, args);

        // Get the client machine name options
        HostProfile::ClientHostDetermination chd;
        std::string clientHostName;
        GetClientMachineNameOptions(host, chd, clientHostName);

        // Get the ssh port options
        bool manualSSHPort;
        int  sshPort;
        GetSSHPortOptions(host, manualSSHPort, sshPort);

        // Create a connection progress dialog and hook it up to the
        // mdserver proxy.
        dialog = SetupConnectionProgressWindow(newServer, host);

        // Start the mdserver on the specified host.
        if (!ShouldShareBatchJob(host) && HostIsLocalHost(host))
        {
            newServer->Create("localhost", chd, clientHostName,
                              manualSSHPort, sshPort);
        }
        else
        {
            // Use VisIt's launcher to start the remote mdserver.
            newServer->Create(host, chd, clientHostName,
                              manualSSHPort, sshPort,
                              OpenWithLauncher, (void*)dialog, true);
        }

        // Add the information about the new server to the 
        // server map.
        servers[host] = new ServerInfo(newServer, args);
    }
    CATCH(BadHostException)
    {
        char msg[200];
        SNPRINTF(msg, 200, "VisIt cannot launch a metadata server on host %s.", 
                 host.c_str());
        Error(msg);

        delete newServer;
    }
    CATCH(IncompatibleVersionException)
    {
        char message[200];
        SNPRINTF(message, 200, "The metadata server on host %s is an "
                 "incompatible version. It cannot be used.", host.c_str());
        Error(message);
        delete newServer;
        delete dialog;

        // Re-throw the exception.
        RETHROW;
    }
    CATCH(IncompatibleSecurityTokenException)
    {
        char message[200];
        SNPRINTF(message, 200, "The metadata server on host %s did not return "
                "the proper credentials. It cannot be used.", host.c_str());
        Error(message);

        delete newServer;
        delete dialog;

        // Re-throw the exception.
        RETHROW;
    }
    CATCH(CouldNotConnectException)
    {
        char message[200];
        SNPRINTF(message, 200,
                "The metadata server on host %s could not be launched. "
                "You may want to make sure your PATH environment variable "
                "includes the program \"visit\".", host.c_str());
        Error(message);

        delete newServer;
        delete dialog;

        // Re-throw the exception.
        RETHROW;
    }
    CATCH(CancelledConnectException)
    {
        char message[200];
        SNPRINTF(message, 200, "The launch of the metadata server on "
                 "host \"%s\" has been cancelled.", host.c_str());
        Error(message);

        delete newServer;
        delete dialog;

        // Re-throw the exception.
        RETHROW;
    }
    ENDTRY

    // Delete the connection dialog
    delete dialog;
}

// ****************************************************************************
// Method: ViewerFileServer::NoFaultStartServer
//
// Purpose: 
//   Starts a server on the specified host.
//
// Arguments:
//   host : The name of the host on which to start the mdserver.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 13 10:44:27 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerFileServer::NoFaultStartServer(const std::string &host)
{
    stringVector noArgs;
    NoFaultStartServer(host, noArgs);
}

// ****************************************************************************
// Method: ViewerFileServer::NoFaultStartServer
//
// Purpose: 
//   Tries to start a server on the specified host if no server exists.
//
// Arguments:
//   host : The host to use.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 15:08:06 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Feb 28 08:02:31 PDT 2003
//   I removed a warning message.
//
// ****************************************************************************

void
ViewerFileServer::NoFaultStartServer(const std::string &host, const stringVector &args)
{
    if(servers.find(host) == servers.end())
    {
        // Try and start a metadata server on the specified host.
        TRY
        {
            StartServer(host, args);
        }
        CATCHALL(...)
        {
            // Do nothing.
        }
        ENDTRY
    }
}

// ****************************************************************************
// Method: ViewerFileServer::CloseServer
//
// Purpose: 
//   Closes the server associated the specified host name.
//
// Arguments:
//   host : The host whose mdserver we want to delete.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 24 17:06:08 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerFileServer::CloseServer(const std::string &host, bool close)
{
    ServerMap::iterator pos;
    if((pos = servers.find(host)) != servers.end()) 
    {
        // Close down the server.
        if(close)
            pos->second->proxy->Close();

        // Delete the server
        delete pos->second;

        // Remove the entry from the server map.
        servers.erase(pos);
    }
}

// ****************************************************************************
// Method: ViewerFileServer::ConnectServer
//
// Purpose: 
//   Tells the metadata server running on "mdServerHost" to connect to the
//   program running on "progHost" on port "port".
//
// Arguments:
//   mdServerHost : The host where the metadata server is running.
//   progHost     : The host where the program that we want to connect to
//                  is running.
//   port         : The port the program we're connecting to is listening on.
//   nread        : The number of read sockets to create.
//   nwrite       : The number of write sockets to create.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 16 16:37:06 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Apr 24 17:11:29 PST 2001
//   Added code to detect whether or not the mdserver is alive if is was
//   already in the server map. If it was in the server map, but was dead,
//   it is closed and re-started.
//
//   Brad Whitlock, Fri Apr 27 11:07:51 PDT 2001
//   Added code to handle a version exception.
//
//   Jeremy Meredith, Fri Apr 27 15:45:10 PDT 2001
//   Added catching of CouldNotConnectException.
//
//   Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//   Changed the exception keywords to macros.
//
//   Brad Whitlock, Fri Sep 27 15:45:59 PST 2002
//   I added code to catch CancelledConnectException.
//
//   Brad Whitlock, Thu Dec 26 15:46:12 PST 2002
//   I added support for security checking.
//
//   Brad Whitlock, Mon May 5 14:23:10 PST 2003
//   I changed the code so it parses out the arguments that it needs from the
//   argument vector.
//
//   Brad Whitlock, Mon Jun 16 13:06:43 PST 2003
//   I made it capable of using pipes.
//
// ****************************************************************************

void
ViewerFileServer::ConnectServer(const std::string &mdServerHost,
    const stringVector &args)
{
    // If a server exists, get the current directory to make sure it is
    // still alive.
    stringVector startArgs;
    if(servers.find(mdServerHost) != servers.end())
    {
        TRY
        {
            // Get the directory, it is relatively cheap to do this and it
            // also proves that the mdserver is still alive.
            servers[mdServerHost]->proxy->GetDirectory();
        }
        CATCH(LostConnectionException)
        {
            startArgs = servers[mdServerHost]->arguments;
            // Close the connection to the server because it is dead anyway.
            CloseServer(mdServerHost, false);
        }
        ENDTRY
    }

    // Part of a message that we print to the debug logs if we're terminating
    // the connection to an mdserver client.
    static const char *termString = "ViewerFileServer::ConnectServer: "
        "Terminating client mdserver connection because ";

    // Start a server if one has not already been started.
    TRY
    {
        StartServer(mdServerHost, startArgs);
    }
    CATCH(IncompatibleVersionException)
    {
        // Open a connection back to the process that initiated the request
        // and send it a reason to quit.
        debug1 << termString << "of incompatible versions." << endl;
        TerminateConnectionRequest(args, 1);
    }
    CATCH(IncompatibleSecurityTokenException)
    {
        // Open a connection back to the process that initiated the request
        // and send it a reason to quit.
        debug1 << termString << "of incompatible security tokens." << endl;
        TerminateConnectionRequest(args, 2);
    }
    CATCH(CouldNotConnectException)
    {
        // Open a connection back to the process that initiated the request
        // and send it a reason to quit.
        debug1 << termString << "we could not connect." << endl;
        TerminateConnectionRequest(args, 3);
    }
    CATCH(CancelledConnectException)
    {
        // Open a connection back to the process that initiated the request
        // and send it a reason to quit.
        debug1 << termString << "the connection was cancelled by the user."
               << endl;
        TerminateConnectionRequest(args, 4);
    }
    ENDTRY

    // If the remote host is in the server list, tell it to connect
    // to another program.
    if(servers.find(mdServerHost) != servers.end())
    {
        servers[mdServerHost]->proxy->Connect(args);
    }
}

// ****************************************************************************
// Method: ViewerFileServer::TerminateConnectionRequest
//
// Purpose: 
//   Connects back to the process that requested the connection and sends it
//   a bad version number so the other process will know there was a version
//   problem with the mdserver.
//
// Arguments:
//   progHost : The host that we're connecting back to.
//   port     : The port that we're connecting back to.
//   nread    : The number of read sockets to create.
//   nwrite   : The number of write sockets to create.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 26 15:50:02 PST 2001
//
// Modifications:
//   Jeremy Meredith, Fri Apr 27 15:40:09 PDT 2001
//   Added fail bit.
//
//   Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//   Changed the exception keywords to macros.
//
//   Brad Whitlock, Thu Dec 26 15:46:38 PST 2002
//   I added support for security checking.
//
//   Brad Whitlock, Mon Jun 16 13:36:38 PST 2003
//   I simplified the code so it does not matter which sorts of connections
//   are used.
//
//   Brad Whitlock, Tue Jul 29 11:40:07 PDT 2003
//   Changed interface to ParentProcess::Connect.
//
// ****************************************************************************

void
ViewerFileServer::TerminateConnectionRequest(const stringVector &args, int failCode)
{
    int  argc = args.size();
    char **argv = new char *[args.size() + 1];

    // Create an argv array out of the args string vector.
    for(int i = 0; i < argc; ++i)
        argv[i] = (char *)args[i].c_str();
    argv[argc] = 0;

    // Try and connect back to the process that initiated the request and
    // send it a non-zero fail code so it will terminate the connection.
    TRY
    {
        ParentProcess killer;

        // Connect back to the process.
        killer.Connect(1, 1, &argc, &argv, true, failCode);
    }
    CATCHALL(...)
    {
        // We know that we're going to get here, but no action is required.
    }
    ENDTRY

    delete [] argv;
}

// ****************************************************************************
// Method: ViewerFileServer::ClearFile
//
// Purpose: 
//   Clears information about the specified file.
//
// Arguments:
//   fullName : The full name of the file we want to clear.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 29 15:28:25 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerFileServer::ClearFile(const std::string &fullName)
{
    // Clear the metadata.
    FileMetaDataMap::iterator mpos;
    if((mpos = fileMetaData.find(fullName)) != fileMetaData.end())
    {
        delete mpos->second;
        fileMetaData.erase(mpos);
    }

    // Clear the SIL.
    FileSILMap::iterator spos;
    if((spos = fileSIL.find(fullName)) != fileSIL.end())
    {
        delete spos->second;
        fileSIL.erase(spos);
    }
}

// ****************************************************************************
// Method: ViewerFileServer::CloseFile
//
// Purpose: 
//   Tells the mdserver running on the specified host to close its open
//   file so that we can re-read it.
//
// Arguments:
//   host : The host on which the mdserver is running.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 14:32:14 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerFileServer::CloseFile(const std::string &host)
{
    ServerMap::iterator pos;
    if((pos = servers.find(host)) != servers.end()) 
    {
        TRY
        {
            pos->second->proxy->CloseDatabase();
        }
        CATCH(LostConnectionException)
        {
            CloseServer(host, false);
        }
        ENDTRY
    }
}


ViewerFileServer::ServerInfo::ServerInfo(MDServerProxy *p, const stringVector &args) : arguments(args)
{
    proxy = p;
}

ViewerFileServer::ServerInfo::ServerInfo(const ServerInfo &b)
{
    proxy = 0;
    arguments = b.arguments;
}

ViewerFileServer::ServerInfo::~ServerInfo()
{
    delete proxy;
}
