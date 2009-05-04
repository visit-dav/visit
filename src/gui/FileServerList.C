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

#include <FileServerList.h>
#include <BadHostException.h>
#include <CancelledConnectException.h>
#include <ChangeDirectoryException.h>
#include <CouldNotConnectException.h>
#include <GetFileListException.h>
#include <GetMetaDataException.h>
#include <LostConnectionException.h>
#include <IncompatibleVersionException.h>
#include <IncompatibleSecurityTokenException.h>
#include <MessageAttributes.h>
#include <avtDatabaseMetaData.h>
#include <avtSIL.h>
#include <DataNode.h>

#include <stdio.h>
#include <snprintf.h>
#include <visit-config.h>
#include <DebugStream.h>
#include <Utility.h>

#include <string>
#include <vector>
using std::string;
using std::vector;

// Some static constants.
static const int FILE_NOACTION = 0;
static const int FILE_OPEN = 1;
static const int FILE_REPLACE = 2;
static const int FILE_OVERLAY = 3;
static const int FILE_CLOSE = 4;
const bool FileServerList::ANY_STATE = true; 
const bool FileServerList::GET_NEW_MD = false;

// ****************************************************************************
// Function: MDCacheKeys
//
// Purpose: Build vector of two possible keys for cached md, one that does not
//          include state and the other that does
//
// Programmer: Mark C. Miller 
// Creation:   Tue Jul 25 07:58:00 PDT 2006 
// ****************************************************************************

static vector<string>
MDCacheKeys(const string& stateLessName, int timeState)
{
    vector<string> result;

    // state-less name takes highest priority
    result.push_back(stateLessName);

    // build an alternative key with time state appended
    char timeStateStr[32];
    sprintf(timeStateStr, "%08d", timeState);
    string stateFullName = stateLessName + string(timeStateStr);

    // put the alternative key in second position
    result.push_back(stateFullName);

    return result;
}

// ****************************************************************************
// Method: FileServerList::FileServerList
//
// Purpose: 
//   Constructor for the FileServerList class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 21 14:38:05 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Aug 29 17:17:27 PST 2000
//   I added code to catch a GetFileListException.
//
//   Brad Whitlock, Wed Apr 25 13:41:46 PST 2001
//   Added messageAtts.
//
//   Brad Whitlock, Fri Jul 26 13:50:05 PST 2002
//   I initialized the useCurrentDirectoryFlag member.
//
//   Brad Whitlock, Mon Aug 19 11:10:09 PDT 2002
//   I added an attribute to the format string.
//
//   Brad Whitlock, Thu Sep 5 16:16:03 PST 2002
//   I made the "useCurrentDirectory" setting turned off by default on
//   the Windows platform.
//
//   Brad Whitlock, Mon Sep 30 08:09:00 PDT 2002
//   I initialized some callback functions.
//
//   Brad Whitlock, Thu Mar 27 09:19:05 PDT 2003
//   I initialized automaticFileGroupingFlag.
//
//   Brad Whitlock, Mon Oct 13 10:01:07 PDT 2003
//   I initialized recentPathsFlag.
//
//   Jeremy Meredith, Wed Jul  7 17:04:03 PDT 2004
//   I made the filter be global to all hosts.
//
//   Brad Whitlock, Thu Jul 29 13:41:07 PST 2004
//   I added the notion of smart file grouping.
//
//   Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//   Added forceReadAllCyclesTimes
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Initialized openFileTimeState 
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//   Added flags for creation of MeshQuality and TimeDerivative expressions.
//
//   Cyrus Harrison, Thu Nov 29 08:12:53 PST 2007
//   Added flags for creation of vector magnitude expressions.
//
// ****************************************************************************

FileServerList::FileServerList() : AttributeSubject("bbbbbibbbb"), servers(),
    activeHost("localhost"), fileList(), appliedFileList(), openFile(),
    fileMetaData(), recentPaths()
{
    hostFlag = pathFlag = filterFlag = false;
    fileListFlag = appliedFileListFlag = false;
    fileAction = FILE_NOACTION;
#if defined(_WIN32)
    useCurrentDirectoryFlag = false;
#else
    useCurrentDirectoryFlag = true;
#endif
    automaticFileGroupingFlag = true;
    smartFileGroupingFlag = true;
    recentPathsFlag = false;
    connectingServer = false;
    filter = "*";

    openFileTimeState = -1;

    forceReadAllCyclesTimes = false;
    treatAllDBsAsTimeVarying = false;

    createMeshQualityExpressions = true;
    createTimeDerivativeExpressions = true;
    createVectorMagnitudeExpressions = true;

    // Initialize some callback functions.
    connectCallback = 0;
    connectCallbackData = 0;
    progressCallback = 0;
    progressCallbackData = 0;

    // Create the message attributes.
    messageAtts = new MessageAttributes;
}

// ****************************************************************************
// Method: FileServerList::~FileServerList
//
// Purpose: 
//   Destructor for the FileServerList class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 21 14:38:45 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 19 15:40:44 PST 2001
//   I removed the code that told the mdserver to quit. This is no
//   longer the GUI's responsibility since the viewer now launches the
//   mdservers.
//
//   Brad Whitlock, Wed Apr 25 13:43:08 PST 2001
//   Added messageAtts.
//
//   Eric Brugger, Thu Nov 29 12:02:21 PST 2001
//   Added caching of SILs.
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Made fileMetaData and SILData MRUCache's
// ****************************************************************************

FileServerList::~FileServerList()
{
    // Iterate through the server list, close and destroy each
    // server and remove the server from the map.
    ServerMap::iterator pos;
    for(pos = servers.begin(); pos != servers.end(); ++pos)
    {
        // Delete the server
        delete pos->second->server;

        // Delete the ServerInfo item.
        delete pos->second;
    }
    servers.clear();

    fileMetaData.clear();
    SILData.clear();

    // delete the message attributes.
    delete messageAtts;
}

// ****************************************************************************
// Method: FileServerList::SelectAll
//
// Purpose: 
//   Selects all of the attributes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 21 14:39:08 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Aug 19 11:02:19 PDT 2002
//   I made useCurrentDirectoryFlag into an attribute.
//
//   Brad Whitlock, Thu Mar 27 09:19:54 PDT 2003
//   I added automaticFileGroupingFlag.
//
//   Brad Whitlock, Mon Oct 13 10:01:38 PDT 2003
//   I added recentPathsFlag.
//
//   Brad Whitlock, Thu Jul 29 13:41:33 PST 2004
//   I added smartFileGroupingFlag.
//
//   Brad Whitlock, Fri Dec 14 17:10:19 PST 2007
//   Made it use ids.
//
// ****************************************************************************

void
FileServerList::SelectAll()
{
    Select(ID_hostFlag,                  (void *)&hostFlag);
    Select(ID_pathFlag,                  (void *)&pathFlag);
    Select(ID_filterFlag,                (void *)&filterFlag);
    Select(ID_fileListFlag,              (void *)&fileListFlag);
    Select(ID_appliedFileListFlag,       (void *)&appliedFileListFlag);
    Select(ID_fileAction,                (void *)&fileAction);
    Select(ID_useCurrentDirectoryFlag,   (void *)&useCurrentDirectoryFlag);
    Select(ID_automaticFileGroupingFlag, (void *)&automaticFileGroupingFlag);
    Select(ID_recentPathsFlag,           (void *)&recentPathsFlag);
    Select(ID_smartFileGroupingFlag,     (void *)&smartFileGroupingFlag);
}

// *************************************************************************************
// Method: FileServerList::Initialize
//
// Purpose: 
//   Connects to an mdserver running on localhost. This method must be called
//   before doing anything with the FileServerList object.
//
// Notes:      This method could throw an IncompatibleVersionException object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 21 14:01:18 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Apr 27 11:05:01 PDT 2001
//   Caught IncompatibleVersionException.
//
//   Jeremy Meredith, Fri Apr 27 15:40:09 PDT 2001
//   Made it catch VisItExceptions from StartServer.
//
//   Brad Whitlock, Mon Oct 22 18:25:42 PST 2001
//   Changed the exception handling keywords to macros.
//
//   Brad Whitlock, Mon Aug 19 11:34:12 PDT 2002
//   I made fileList selected.
//
//   Brad Whitlock, Thu Mar 27 09:23:47 PDT 2003
//   I made it use the file grouping flag.
//
//   Jeremy Meredith, Wed Jul  7 17:04:03 PDT 2004
//   I made the filter be global to all hosts.
//
//   Brad Whitlock, Thu Jul 29 13:42:24 PST 2004
//   I added smartFileGroupingFlag.
//
// *************************************************************************************

void
FileServerList::Initialize()
{
    // Start a MetaData server on the local machine.
    TRY
    {
        StartServer(activeHost);
    }
    CATCH(VisItException)
    {
        // Do nothing.
    }
    ENDTRY

    // Get the remote file list and copy it to the local file list.
    ServerMap::iterator info = servers.find(activeHost);
    if(info != servers.end())
    {
        TRY
        {
            // Try to get the file list from the MD Server.
            fileList = *(info->second->server->GetFileList(filter,
                automaticFileGroupingFlag, smartFileGroupingFlag));
            Select(ID_fileListFlag, (void *)&fileListFlag);

            // Copy virtual files from the fileList into the
            // virtualFiles map.
            DefineVirtualFiles();

            // Generate the applied file list from the file list.
            appliedFileList = GetFilteredFileList();

            // Select the applied file list.
            Select(ID_appliedFileListFlag, (void *)&appliedFileListFlag);
        }
        CATCH(GetFileListException)
        {
            // We got a GetFileListException exception.
            fileList.Clear();
        }
        ENDTRY
    }
}

// ****************************************************************************
// Method: FileServerList::Notify
//
// Purpose: 
//   Does some RPC calls to talk to the MetaData server. This can
//   update the file list. The subject's observers are then called
//   to respond to the changes in the file list. It's done this way
//   because of the potentially high cost of RPC's and the observers
//   should not have to worry where their file list came from.
//
// Notes:
//   This routine can throw exceptions. When SetPath was called before
//   this routine, GetFileListException or ChangeDirectoryException
//   can be thrown. When SetHost is called before this routine,
//   GetFileListException can be thrown.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 23 11:39:11 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Jun 20 17:34:08 PST 2003
//   I added code to prevent the mdserver's CloseDatabase method from
//   being called if we have no open file.
//
//   Brad Whitlock, Mon Oct 13 11:12:44 PDT 2003
//   Added recentPathsFlag.
//
// ****************************************************************************

void
FileServerList::Notify()
{
    //
    // Do a silent notify, which does the real work but does not tell the
    // observers that anything happened.
    //
    SilentNotify();

    //
    // Call the base class's Notify method to tell the observers about
    // new information.
    //
    AttributeSubject::Notify();

    // Reset the flags for future operations.
    hostFlag = pathFlag = filterFlag = false;
    fileListFlag = appliedFileListFlag = false;
    recentPathsFlag = false;
    fileAction = FILE_NOACTION;
}

// ****************************************************************************
// Method: FileServerList::SilentNotify
//
// Purpose: 
//   Does some RPC calls to talk to the MetaData server. This can
//   update the file list. The subject's observers are then called
//   to respond to the changes in the file list. It's done this way
//   because of the potentially high cost of RPC's and the observers
//   should not have to worry where their file list came from.
//
// Notes:
//   This routine can throw exceptions. When SetPath was called before
//   this routine, GetFileListException or ChangeDirectoryException
//   can be thrown. When SetHost is called before this routine,
//   GetFileListException can be thrown.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 21 14:21:02 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Aug 29 17:29:59 PST 2000
//   I added exception handlers.
//
//   Brad Whitlock, Wed Aug 30 15:36:42 PST 2000
//   I changed the exception handlers so they rethrow the exception
//   so the routine that called Notify can use the exception to
//   let the user know there were problems. I changed it because
//   if there were problems, I don't want the other observers to
//   be notified.
//
//   Brad Whitlock, Wed Oct 4 16:40:13 PST 2000
//   I changed the logic so the host and the path can be set in the
//   same call to this method.
//
//   Brad Whitlock, Tue Apr 24 18:11:46 PST 2001
//   Added code to try and restart the mdserver if it was determined that it
//   is no longer alive.
//
//   Brad Whitlock, Wed Apr 25 12:52:29 PDT 2001
//   Added code to catch version errors.
//
//   Brad Whitlock, Fri Apr 27 13:58:46 PST 2001
//   Fixed a memory problem.
//
//   Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//   Changed the exception keywords to macros.
//
//   Brad Whitlock, Wed Feb 13 12:57:03 PDT 2002
//   Added code to add a new path to the recently used path list.
//
//   Brad Whitlock, Tue Jul 30 11:31:58 PDT 2002
//   I changed the routine so that if the fileAction variable is FILE_CLOSE,
//   we make a CloseDatabaseRPC to the active mdserver.
//
//   Brad Whitlock, Mon Aug 26 15:06:44 PST 2002
//   I changed the code so it always gets the name of the current directory
//   after changing to a new directory. This filters stuff out of the path
//   name since the MDServer now is responsible for filtering the pathname.
//
//   Brad Whitlock, Mon Mar 24 14:26:40 PST 2003
//   I passed the filter to the server's GetFileList rpc.
//
//   Brad Whitlock, Mon Jun 23 11:37:03 PDT 2003
//   I renamed the method and removed some code.
//
//   Jeremy Meredith, Wed Jul  7 17:04:03 PDT 2004
//   I made the filter be global to all hosts.
//
//   Brad Whitlock, Thu Jul 29 13:43:04 PST 2004
//   I added smartFileGroupingFlag.
//
// ****************************************************************************

void
FileServerList::SilentNotify()
{
    // Return if there is no mdserver associated with the active host.
    ServerMap::iterator pos;
    if((pos = servers.find(activeHost)) == servers.end())
        return;

    // Get a pointer to the mdserver for the active host.
    ServerInfo *info = servers[activeHost];
    int        numAttempts = 0;
    bool       tryAgain = false;

    // Try and do the specified operations.
    do
    {
        TRY
        {
            // If the fileAction is to close the file, close it. Otherwise,
            // try and change the host or path, etc.
            if(fileAction == FILE_CLOSE)
            {
                if(!openFile.Empty())
                    info->server->CloseDatabase();
            }
            else
            {
                if(hostFlag)
                {
                    if(!pathFlag)
                    {
                        // Get the file list from the MDServerProxy.
                        const MDServerProxy::FileList *fl =
                            info->server->GetFileList(filter,
                                                      automaticFileGroupingFlag,
                                                      smartFileGroupingFlag);
                        // copy the file list into the local file list.
                        fileList = *fl;
                        Select(ID_fileListFlag, (void *)&fileListFlag);

                        // Copy virtual files from the fileList into the
                        // virtualFiles map.
                        DefineVirtualFiles();
                    }

                    // Changing hosts requires the path, filter, and file list
                    // to be updated too. Select them.
                    Select(ID_pathFlag, (void *)&pathFlag);
                    Select(ID_filterFlag, (void *)&filterFlag);
                }

                if(pathFlag || (filterFlag && automaticFileGroupingFlag) ||
                   IsSelected(ID_automaticFileGroupingFlag) ||
                   IsSelected(ID_smartFileGroupingFlag))
                {
                    TRY
                    {
                        // Change to the new path.
                        if(pathFlag)
                        {
                            info->server->ChangeDirectory(info->path);
                            info->path = info->server->GetDirectory();
                        }

                        // Copy the file list into the local file list.
                        const MDServerProxy::FileList *fl =
                            info->server->GetFileList(filter,
                                                      automaticFileGroupingFlag,
                                                      smartFileGroupingFlag);
                        fileList = *fl;
                        Select(ID_fileListFlag, (void *)&fileListFlag);

                        // Copy virtual files from the fileList into the
                        // virtualFiles map.
                        DefineVirtualFiles();

                        // The path must be valid to get to this point. Since
                        // that must be true, add the path to the recently
                        // visited path list.
                        AddPathToRecentList(activeHost, info->path);
                    }
                    CATCH(ChangeDirectoryException)
                    {
                        // Set the directory back to the previous directory name.
                        info->path = info->server->GetDirectory();
                        // Reset the flags for future operations and rethrow.
                        hostFlag = pathFlag = filterFlag = false;
                        fileListFlag = appliedFileListFlag = false;
                        fileAction = FILE_NOACTION;

                        UnSelectAll();
                        RETHROW;
                    }
                    ENDTRY
                }
            }

            // Indicate that we do not need to try the loop again.
            tryAgain = false;
        }
        CATCH(GetFileListException)
        {
            // We could not get a file list back from the MDServerProxy
            // so we should clear the list since we changed to a new
            // directory.
            fileList.Clear();

            // Reset the flags for future operations and rethrow.
            hostFlag = pathFlag = filterFlag = false;
            fileListFlag = appliedFileListFlag = false;
            fileAction = FILE_NOACTION;

            UnSelectAll();
            RETHROW;
        }
        CATCH(LostConnectionException)
        {
            ++numAttempts;
            tryAgain = (numAttempts < 2);

            TRY
            {
                // Save the old path
                string oldPath(info->path);

                // Try and restart the mdserver.
                CloseServer(activeHost);
                StartServer(activeHost);

                // If the server was added to the server map, reset the info
                // pointer and set the path
                if((pos = servers.find(activeHost)) != servers.end())
                {
                    info = servers[activeHost];

                    // Set the path
                    SetPath(oldPath);
                    hostFlag = false;
                }
                else
                    info = 0;
            }
            CATCHALL(...)
            {
                info = 0;
                tryAgain = false;
            }
            ENDTRY
        }
        ENDTRY
    } while(tryAgain);
}

// ****************************************************************************
// Method: FileServerList::SetHost
//
// Purpose: 
//   Sets the current host. If the host is not in the server map, a
//   new mdserver is created. When Notify() is called, 
//
// Notes:
//   BadHostException is thrown when calling this function with a
//   bad host name.
//
// Arguments:
//   host : The name of the new active host.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 21 14:39:45 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Apr 26 16:21:12 PST 2001
//   Added code to catch IncompatibleVersionException.
//
//   Jeremy Meredith, Fri Apr 27 15:40:09 PDT 2001
//   Made it catch VisItExceptions from StartServer.
//
//   Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//   Changed the exception keywords to macros.
//
//   Brad Whitlock, Thu Oct 25 15:22:48 PST 2001
//   Added code to rethrow BadHostException.
//
//   Brad Whitlock, Wed Feb 13 15:01:15 PST 2002
//   Added code to rethrow CouldNotConnectException.
//
//   Jeremy Meredith, Thu Feb 14 15:10:07 PST 2002
//   Look up the fully qualified host name so it doesn't create duplicate
//   qualified and unqualified ones.
//
//   Brad Whitlock, Fri Feb 15 15:22:56 PST 2002
//   Added code to migrate an existing mdserver to a qualified server name.
//
//   Jeremy Meredith, Wed Feb 20 10:21:35 PST 2002
//   Added code to only resolve up the host name if we are not trying to
//   look up locahost.
//
//   Jeremy Meredith, Sun Mar  3 21:55:17 PST 2002
//   Removed the code to resolve to a fully qualified host name.  The problem
//   is that although correct, it forced users to have the fully qualified
//   host name in their .ssh/known_hosts, where if they have never used it
//   it will not exist.  By postponing the FQ lookup until trying to match a
//   host profile, this lets ssh go to host names the users actually entered.
//
//   Jeremy Meredith, Wed Jul  7 17:04:03 PDT 2004
//   I made the filter be global to all hosts.
//
//   Brad Whitlock, Thu Oct 27 14:45:52 PST 2005
//   I made it throw cancelledconnect exception instead of catching it.
//
// ****************************************************************************

void
FileServerList::SetHost(const string &host)
{
    // If the fully qualified host name is not the same as the regular
    // hostname and we have a server under the regular hostname, migrate
    // the server to the new hostname.
    ServerMap::iterator pos = servers.find(host);
    if(pos != servers.end())
    {
        ServerInfo *oldServer = pos->second;
        servers.erase(pos);
        servers[host] = oldServer;

        StringStringVectorMap::iterator pos2 = recentPaths.find(host);
        if(pos2 != recentPaths.end())
        {
            stringVector oldRecentPaths(pos2->second);
            recentPaths.erase(pos2);
            recentPaths[host] = oldRecentPaths;
        }
    }

    // If the host is not in the map, we need to create a new
    // server on that host.
    TRY
    {
        if((pos = servers.find(host)) == servers.end())
        {
            StartServer(host);
        }

        // Set the new active host.
        hostFlag = true;
        activeHost = host;
        Select(ID_hostFlag, (void *)&hostFlag);

        // Set the file action to none.
        fileAction = FILE_NOACTION;
        Select(ID_fileAction, (void *)&fileAction);

        // Use the path for the new host.
        SetPath(servers[host]->path);
    }
    CATCH(BadHostException)
    {
        // Rethrow the exception.
        RETHROW;
    }
    CATCH(CouldNotConnectException)
    {
        // Rethrow the exception.
        RETHROW;
    }
    CATCH(CancelledConnectException)
    {
        debug1 << "FileServerList::SetHost: activeHost="
               << activeHost.c_str() << endl;

        // Rethrow the exception.
        RETHROW;
    }
    CATCH(VisItException)
    {
        // Do nothing.
    }
    ENDTRY
}

// ****************************************************************************
// Method: FileServerList::StartServer
//
// Purpose: 
//   Starts a MetaData server on the specified host.
//
// Arguments:
//   host : The name of the host where the server will be started.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 22 09:50:46 PDT 2000
//
// Modifications:
//   Brad Whitlock, Fri Aug 25 14:34:30 PST 2000
//   I added a clean-up handler that deletes the new server when
//   an exception is thrown.
//
//   Eric Brugger, Wed Oct 25 15:35:34 PDT 2000
//   I modified the routine to match a change to the MDServerProxy Create
//   method.
//
//   Brad Whitlock, Tue Nov 21 13:20:18 PST 2000
//   I passed a callback to the MDServerProxy::Create method.
//
//   Brad Whitlock, Thu Apr 26 11:46:26 PDT 2001
//   Added handler for IncompatibleVersionException.
//
//   Jeremy Meredith, Fri Apr 27 15:40:39 PDT 2001
//   Added catch for CouldNotConnectException.
//
//   Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//   Changed the exception keywords to macros.
//
//   Brad Whitlock, Thu Feb 7 09:56:33 PDT 2002
//   Fixed the hostname used in the error messages.
//
//   Brad Whitlock, Wed Feb 13 13:51:45 PST 2002
//   Added code that appends the start directory to the recent paths.
//
//   Brad Whitlock, Fri Feb 15 16:59:24 PST 2002
//   Added code to delete the mdserver proxy when an exception is thrown.
//
//   Brad Whitlock, Mon Sep 30 08:12:51 PDT 2002
//   Added code to set the mdserver's progress callback.
//
//   Brad Whitlock, Mon Dec 16 16:36:47 PST 2002
//   I added support for IncompatibleSecurityTokenException.
//
//   Jeremy Meredith, Thu Oct  9 15:46:23 PDT 2003
//   Added ability to manually specify a client host name or to have it
//   parsed from the SSH_CLIENT (or related) environment variables.  Added
//   ability to specify an SSH port.
//
//   Brad Whitlock, Fri Mar 12 14:26:41 PST 2004
//   Added connectingServer member.
//
//   Jeremy Meredith, Wed Jul  7 17:04:03 PDT 2004
//   I made the filter be global to all hosts.
//
//   Jeremy Meredith, Thu May 24 10:35:14 EDT 2007
//   Added SSH tunneling option to MDServerProxy::Create, and set it to false.
//   If we need to tunnel, the VCL will do the host/port translation for us.
//
// ****************************************************************************

void
FileServerList::StartServer(const string &host)
{
    ServerInfo *info = new ServerInfo();
    info->server = 0;

    // Create a new MD server on the remote machine.
    TRY
    {
        connectingServer = true;
        info->server = new MDServerProxy();
        info->server->SetProgressCallback(progressCallback,
            progressCallbackData);
        info->server->Create(host,
                             HostProfile::MachineName, "", false, 0, false,
                             connectCallback, connectCallbackData);
        connectingServer = false;

        // Get the current directory from the server
        info->path = info->server->GetDirectory();

        // Add the information about the new server to the 
        // server map.
        servers[host] = info;

        // Add the default path to the recent path list.
        AddPathToRecentList(host, info->path);
    }
    CATCHALL(...) // Clean-up handler
    {
        connectingServer = false;
        delete info->server;
        delete info;
        // re-throw the exception
        RETHROW;
    }
    ENDTRY
}

// ****************************************************************************
// Method: FileServerList::CloseServer
//
// Purpose: 
//   Closes the mdserver associated with the specified host.
//
// Arguments:
//   host : The hostname for the mdserver we want to close.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 24 16:50:58 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
FileServerList::CloseServer(const string &host)
{
    ServerMap::iterator pos;
    if((pos = servers.find(host)) != servers.end()) 
    {
        // Delete the server
        delete pos->second->server;

        // Delete the ServerInfo item.
        delete pos->second;

        // Remove the entry from the server map.
        servers.erase(pos);
    }
}

// ****************************************************************************
// Method: FileServerList::SetPath
//
// Purpose: 
//   Sets a new path. When Notify() is called, the file list gets
//   updated.
//
// Arguments:
//   path : The name of the new path.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 21 14:40:59 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Aug 19 13:34:40 PST 2002
//   Set the file action to no action.
//
// ****************************************************************************

void
FileServerList::SetPath(const string &path)
{
    // If the activeHost is in the server map, set its path.
    ServerMap::iterator pos;
    if((pos = servers.find(activeHost)) != servers.end())
    {
        if(pos->second->path != path)
        {
            // Set the file action to none.
            fileAction = FILE_NOACTION;
            Select(ID_fileAction, (void *)&fileAction);

            pathFlag = true;
            pos->second->path = path;
            Select(ID_pathFlag, (void *)&pathFlag);
        }
    }
}

// ****************************************************************************
// Method: FileServerList::LoadPlugins
//
// Purpose: 
//     Tell the mdserver for the current host to load its plugins.  It will
//     do this automatically when you open a file; the only reason to make
//     this call is for efficiency.
//
// Programmer: Hank Childs
// Creation:   January 24, 2004
//
// Modifications:
//
// ****************************************************************************

void
FileServerList::LoadPlugins()
{
    // If the activeHost is in the server map, set its path.
    ServerMap::iterator pos;
    if((pos = servers.find(activeHost)) != servers.end())
    {
        // Now that we have what we need, tell the mdserver to load its
        // plugins.
        pos->second->server->LoadPlugins();
    }
}

// ****************************************************************************
// Method: FileServerList::SendKeepAlives
//
// Purpose: 
//   This method sends keep alive signals to all of the mdservers.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 14:24:53 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
FileServerList::SendKeepAlives()
{
    if(!connectingServer)
    {
        ServerMap::iterator pos;
        for(pos = servers.begin(); pos != servers.end();)
        {
            TRY
            {
                debug2 << "Sending keep alive signal to mdserver on "
                       << pos->first.c_str() << endl;
                pos->second->server->SendKeepAlive();
                ++pos;
            }
            CATCHALL(...)
            {
                debug2 << "Could not send keep alive signal to mdserver on "
                       << pos->first.c_str() << " so that mdserver will be closed."
                       << endl;
                delete pos->second->server;
                delete pos->second;
                pos->second = 0;
                servers.erase(pos++);
            }
            ENDTRY
        }
    }
}

// ****************************************************************************
// Method: FileServerList::SetFilter
//
// Purpose: 
//   Sets a new file filter.
//
// Arguments:
//   filter : The new filter to use.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 21 14:42:17 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Aug 19 13:34:40 PST 2002
//   Set the file action to no action.
//   
//   Jeremy Meredith, Wed Jul  7 17:04:03 PDT 2004
//   I made the filter be global to all hosts.
//
// ****************************************************************************

void
FileServerList::SetFilter(const string &newFilter)
{
    if(newFilter != filter)
    {
        // Set the file action to none.
        fileAction = FILE_NOACTION;
        Select(ID_fileAction, (void *)&fileAction);

        filterFlag = true;
        filter = newFilter;
        Select(ID_filterFlag, (void *)&filterFlag);
    }
}

// ****************************************************************************
// Method: FileServerList::SetAppliedFileList
//
// Purpose: 
//   Sets the selected file list.
//
// Arguments:
//   newFiles : This is a vector of strings representing the selected
//              file list. It is a vector of strings that contain
//              qualified file names.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:29:22 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Aug 19 13:34:40 PST 2002
//   Set the file action to no action.
//
//   Brad Whitlock, Wed Apr 2 10:14:19 PDT 2003
//   I made the open file reopen if it is in the list and it is a virtual file.
//   This makes sure that we get new metadata for the virtual file in case
//   its contents changed.
//
//   Brad Whitlock, Mon Dec 29 13:17:00 PST 2003
//   I added code to open and get new metadata for virtual databases that
//   have more time states than files so they are displayed correctly in the
//   file panel.
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interfaces to GetMetaData and GetSIL. Added timeStates arg.
//
//   Kathleen Bonnell, Wed Oct 29 12:50:17 PDT 2008
//   Added a work-around for bad indexing into timeStates.  This fixes a crash
//   on windows, but the use of timeStates here should be reworked.
// 
// ****************************************************************************

void
FileServerList::SetAppliedFileList(const QualifiedFilenameVector &newFiles,
    const vector<int>& timeStates)
{
    QualifiedFilename oldOpenFile(openFile);
    int               oldOpenFileTimeState = openFileTimeState;

    // Remove the metadata and SIL for any virtual files.
    for(int i = 0; i < appliedFileList.size(); ++i)
    {
        if(appliedFileList[i].IsVirtual())
        {
            //
            // If the virtual database has more time states than files, 
            // make sure we reopen it.
            //
            bool forceReopen = false;
        
            // Work around possible bad indexing into timeStates, causing
            // crash on Windows. 
            // Old method for calculating time step to send with 
            // GetMetaData and OpenFile calls:
            // timeState.size() ? timeStates[i] : 0
            // timeStates may not be same size as appliedFileList which we
            // are looping on, so we cannot always index by the same value.
            // Use of timeStates in this manner should be addressed.

            int ts = (timeStates.size() && timeStates.size() > i) ? 
                     timeStates[i] : 0;

            const avtDatabaseMetaData *md = GetMetaData(appliedFileList[i],
                                                ts,
                                                ANY_STATE,
                                                !GET_NEW_MD);
            if(md != 0 &&
               (md->GetNumStates() != GetVirtualFileDefinitionSize(appliedFileList[i])))
            {
                forceReopen = true;
            }

            // Free the metadata and SIL for the file.
            ClearFile(appliedFileList[i]);

            // If the file happens to be the open file, reopen it so we
            // get new cached metadata for later.
            if(openFile == appliedFileList[i] || forceReopen)
            {
                TRY
                {
                    CloseFile();
                    OpenFile(appliedFileList[i], ts);
                }
                CATCH(VisItException)
                {
                }
                ENDTRY
            }
        }
    }

    //
    // If we opened a file other than the open file so we could get its
    // metadata, we need to restore the open file.
    //
    if(oldOpenFile != openFile)
    {
        TRY
        {
            OpenFile(oldOpenFile, oldOpenFileTimeState);
        }
        CATCH(VisItException)
        {
        }
        ENDTRY
    }

    // Set the file action to none.
    fileAction = FILE_NOACTION;
    Select(ID_fileAction, (void *)&fileAction);

    // Overwrite the applied file list with a new one.
    appliedFileList = newFiles;
    appliedFileListFlag = true;
    Select(ID_appliedFileListFlag, (void *)&appliedFileListFlag);
}

// ****************************************************************************
// Method: FileServerList::SetUseCurrentDirectory
//
// Purpose: 
//   Sets the flag that tells VisIt to start up in the current directory.
//
// Arguments:
//   val : The new value of the flag.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 26 13:54:32 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Aug 19 11:09:36 PDT 2002
//   I made useCurrentDirectoryFlag into an attribute.
//
// ****************************************************************************

void
FileServerList::SetUseCurrentDirectory(bool val)
{
    useCurrentDirectoryFlag = val;
    Select(ID_useCurrentDirectoryFlag, (void *)&useCurrentDirectoryFlag);
}

// ****************************************************************************
// Method: FileServerList::SetAutomaticFileGrouping
//
// Purpose: 
//   Sets the flag that tells VisIt to use automatic file grouping.
//
// Arguments:
//   val : The new value of the flag.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 27 09:35:32 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
FileServerList::SetAutomaticFileGrouping(bool val)
{
    automaticFileGroupingFlag = val;
    Select(ID_automaticFileGroupingFlag, (void *)&automaticFileGroupingFlag);
}

// ****************************************************************************
// Method: FileServerList::SetSmartFileGrouping
//
// Purpose: 
//   Sets the flag that tells VisIt to use smart automatic file grouping.
//
// Arguments:
//   val : The new value of the flag.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 29 13:45:25 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
FileServerList::SetSmartFileGrouping(bool val)
{
    smartFileGroupingFlag = val;
    Select(ID_smartFileGroupingFlag, (void *)&smartFileGroupingFlag);
}

// ****************************************************************************
// Method: FileServerList::SetForceReadAllCyclesTimes
//
// Purpose: Set flag indicating if all cycles/times should be read during
// GetMetaData calls
//
// Programmer: Mark C. Miller 
// Creation:   May 25, 2005 
//   
// ****************************************************************************

void
FileServerList::SetForceReadAllCyclesTimes(bool set)
{
    forceReadAllCyclesTimes = set;
}

// ****************************************************************************
// Method: FileServerList::SetTreatAllDBsAsTimeVarying
//
// Purpose: Set flag indicating if all databases should be treated as time
// varying
//
// Programmer: Mark C. Miller 
// Creation:   June 11, 2007 
//   
// ****************************************************************************

void
FileServerList::SetTreatAllDBsAsTimeVarying(bool set)
{
    treatAllDBsAsTimeVarying = set;
}


// ****************************************************************************
// Method: FileServerList::SetCreateMeshQualityExpressions
//
// Purpose: Set flag indicating if mesh quality expressions should be 
// automatically created during GetMetaData calls
//
// Programmer: Kathleen Bonnell
// Creation:   October 8, 2007
//   
// ****************************************************************************

void
FileServerList::SetCreateMeshQualityExpressions(bool set)
{
    createMeshQualityExpressions = set;
}

// ****************************************************************************
// Method: FileServerList::SetCreateTimeDerivativeExpressions
//
// Purpose: Set flag indicating if time derivative expressions should be 
// automatically created during GetMetaData calls
//
// Programmer: Kathleen Bonnell
// Creation:   October 8, 2007
//   
// ****************************************************************************

void
FileServerList::SetCreateTimeDerivativeExpressions(bool set)
{
    createTimeDerivativeExpressions = set;
}

// ****************************************************************************
// Method: FileServerList::SetCreateVectorMagnitudeExpressions
//
// Purpose: Set flag indicating if vector magnitude expressions should be
//          auto generated 
//
// Programmer: Cyrus Harrison
// Creation:   November 28, 2007
//   
// ****************************************************************************

void
FileServerList::SetCreateVectorMagnitudeExpressions(bool set)
{
    createVectorMagnitudeExpressions = set;
}

// ****************************************************************************
// Method: FileServerList::OpenFile
//
// Purpose: 
//   Opens the specified file.
//
// Arguments:
//   filename  : The filename to open.
//   timeState : The time state to open.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 22:29:13 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue May 13 15:14:29 PST 2003
//   I added the timeState argument.
//
// ****************************************************************************

void
FileServerList::OpenFile(const QualifiedFilename &filename, int timeState)
{
    OpenAndGetMetaData(filename, timeState, FILE_OPEN);
}

// ****************************************************************************
// Method: FileServerList::ReplaceFile
//
// Purpose: 
//   Replaces the current file with the specified file.
//
// Arguments:
//   filename : The filename to open.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 22:29:13 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu May 15 12:52:39 PDT 2003
//   Changed interface to OpenAndGetMetaData.
//
// ****************************************************************************

void
FileServerList::ReplaceFile(const QualifiedFilename &filename)
{
    OpenAndGetMetaData(filename, 0, FILE_REPLACE);
}

// ****************************************************************************
// Method: FileServerList::OverlayFile
//
// Purpose: 
//   Overlays the specified file on the current file.
//
// Arguments:
//   filename : The filename to open.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 22:29:13 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu May 15 12:52:39 PDT 2003
//   Changed interface to OpenAndGetMetaData.
//   
// ****************************************************************************

void
FileServerList::OverlayFile(const QualifiedFilename &filename)
{
    OpenAndGetMetaData(filename, 0, FILE_OVERLAY);
}

// ****************************************************************************
// Method: FileServerList::CloseFile
//
// Purpose: 
//   Closes the open file and clears the active file.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 11:00:27 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Apr 2 09:04:42 PDT 2003
//   Actually try to close the file on the mdserver so we get new metadata
//   the next time we open the file.
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   reset openFileTimeState
// ****************************************************************************

void
FileServerList::CloseFile()
{
    // Try to close the file on the mdserver.
    if(servers.find(openFile.host) != servers.end())
    {
        TRY
        {
            servers[openFile.host]->server->CloseDatabase();
        }
        CATCH(LostConnectionException)
        {
            CloseServer(openFile.host);
        }
        ENDTRY
    }

    openFile = QualifiedFilename();
    openFileTimeState = -1;
    fileAction = FILE_CLOSE;
    Select(ID_fileAction, (void *)&fileAction);
}

// ****************************************************************************
// Method: FileServerList::OpenAndGetMetaData
//
// Purpose: 
//   Opens specified file and makes it the current file.
//
// Arguments:
//   filename  : The filename to open.
//   timeState : The timestate to open.
//   action    : How we're opening the file (FILE_OPEN, FILE_REPLACE,
//               FILE_OVERLAY) 
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 11:31:10 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Apr 24 16:35:56 PST 2001
//   Added code that attempts to restart an mdserver if it has been determined
//   that the mdserver it thought was there is dead.
//
//   Brad Whitlock, Fri May 18 09:51:31 PDT 2001
//   I put in code to handle the case where the mdserver repeatedly crashes
//   right after being re-launched.
//
//   Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//   Changed the exception keywords to macros.
//
//   Eric Brugger, Thu Nov 29 12:02:21 PST 2001
//   Added caching of SILs.
//
//   Brad Whitlock, Thu Feb 7 11:32:20 PDT 2002
//   Modified the exception handling code so it can give a reason as to
//   why the exception is happening.
//
//   Brad Whitlock, Tue May 13 15:10:19 PST 2003
//   I added the timeState argument so we can request a specific timestate.
//
//   Brad Whitlock, Thu May 15 12:53:31 PDT 2003
//   I changed the order of the arguments.
//
//   Brad Whitlock, Fri Feb 4 15:31:15 PST 2005
//   Added code to overwrite the local virtual file definition with the 
//   definition from the metadata. This takes care of making the virtual
//   file definition have the right number of states without having to
//   reread the file list.
//
//   Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//   Added use of forceReadAllCyclesTimes in call to GetMetaData
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interfaces to GetMetaData and GetSIL
//
//   Mark C. Miller, Thu Aug  3 13:33:20 PDT 2006
//   Fixed missing line to set readFileFailed to false
// ****************************************************************************

void
FileServerList::OpenAndGetMetaData(const QualifiedFilename &filename,
    int timeState, int action)
{
    // If the metadata has been seen before, indicate that we have it.
    // Otherwise, read it from the MetaData Server.
    if (GetMetaData(filename, timeState, !ANY_STATE, !GET_NEW_MD))
    {
        // The metadata has been read previously.
        fileAction = action;
        Select(ID_fileAction, (void *)&fileAction);
        // We made it to here then really set the open file.
        openFile = filename;
        openFileTimeState = timeState;
    }
    else
    {
        bool changedHosts = false;

        // If the specified MDServer is not in the list, try and
        // start one. This could throw a BadHostException.
        if(servers.find(filename.host) == servers.end())
        {
            changedHosts = true;
            SetHost(filename.host);
            // maybe set the path?
        }

        // There is a loop around this code to open the file and get its
        // metadata because we initially do not know whether or not the
        // mdserver we're talking to is still alive. If it is not alive then
        // we re-start it and try to open the file again.
        int         numAttempts = 0;
        bool        tryAgain;
        bool        readFileFailed = true;
        string reason;
        do
        {
            tryAgain = false;

            TRY
            {
                // Do a GetMetaData RPC on the MD Server.
                const avtDatabaseMetaData *newMetaData =
                    GetMetaData(filename, timeState, !ANY_STATE, GET_NEW_MD);

                const avtSIL *newSIL = 
                    GetSIL(filename, timeState, !ANY_STATE, GET_NEW_MD);

                readFileFailed = false;

                // Assume the metadata is more complete than the virtual file
                // definition, which is only updated when we change the selected
                // files. Copy the time step names over the virtual file
                // definition.
                if(newMetaData != 0 && newMetaData->GetIsVirtualDatabase())
                {
                    const stringVector &states = newMetaData->GetTimeStepNames();
                    virtualFiles[filename.FullName()] = states;
                    debug4 << "Overwriting virtual file definition for "
                           << filename.FullName().c_str() << " with:" << endl;
                    for(int i = 0; i < states.size(); ++i)
                        debug4 << "\t" << states[i].c_str() << endl;
                }

                // We made it to here then really set the open file.
                openFile = filename;
                openFileTimeState = timeState;
                fileAction = action;
                Select(ID_fileAction, (void *)&fileAction);
            }
            CATCH2(GetMetaDataException, gmde)
            {
                // Save the reason of why the metadata could be read.
                reason = gmde.Message();

                //Is there much to do here?
                if(changedHosts)
                    Notify();
            }
            CATCH2(LostConnectionException, lce)
            {
                // Save the reason of why the connection was lost.
                reason = lce.Message();

                ++numAttempts;
                tryAgain = (numAttempts < 2);

                TRY
                {
                    CloseServer(filename.host);
                    StartServer(filename.host);
                }
                CATCHALL(...)
                {
                    tryAgain = false;
                }
                ENDTRY
            }
            ENDTRY
        } while(tryAgain);

        // If we could not read the file, throw an exception.
        if(readFileFailed)
        {
            EXCEPTION1(GetMetaDataException, reason);
        }
    }
}

// ****************************************************************************
// Method: FileServerList::ClearFile
//
// Purpose: 
//   Removes metadata and SIL information for the specified file from the
//   internal caches so that it must be re-read.
//
// Arguments:
//   filename : The file for which we're removing information.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 29 14:51:59 PST 2002
//
// Modifications:
//   
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Made fileMetaData and SILData MRUCache's
//
//   Mark C. Miller, Mon Sep 18 16:24:12 PDT 2006
//   Changed to build list of keys to remove beforing removing keyed values.
//   Because gnu's compare(0,n,string) is buggy, use compare(0,n,string,0,n)
// ****************************************************************************

void
FileServerList::ClearFile(const QualifiedFilename &filename)
{
    const string& fullName = filename.FullName();
    const int n = fullName.size();
    vector<string> keysToRemove;
    int i;

    FileMetaDataMap::iterator mdit;
    for (mdit = fileMetaData.begin(); mdit != fileMetaData.end(); mdit++)
    {
        if (fullName.compare(0, n, mdit->first, 0, n) == 0)
            keysToRemove.push_back(mdit->first);
    }
    for (i = 0; i < keysToRemove.size(); i++)
        fileMetaData.remove(keysToRemove[i]);

    keysToRemove.clear();
    SILMap::iterator sit;
    for (sit = SILData.begin(); sit != SILData.end(); sit++)
    {
        if (fullName.compare(0, n, sit->first, 0, n) == 0)
            keysToRemove.push_back(sit->first);
    }
    for (i = 0; i < keysToRemove.size(); i++)
        SILData.remove(keysToRemove[i]);
}

// ****************************************************************************
// Method: FileServerList::HaveOpenedFile
//
// Purpose: 
//   Returns whether we have opened the file in the past.
//
// Arguments:
//   filename : The name of the file to check.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 28 08:25:17 PDT 2003
//
// Modifications:
//   
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interfaces to GetMetaData and GetSIL
// ****************************************************************************

bool
FileServerList::HaveOpenedFile(const QualifiedFilename &filename)
{
    return GetMetaData(filename, 0, ANY_STATE, !GET_NEW_MD) != 0;
}

// ****************************************************************************
// Method: FileServerList::GetFileIndex
//
// Purpose: 
//   Returns the index of the filename in the applied file list.
//
// Arguments:
//   fileName : This is a NON-QUALIFIED filename of the form [path/]file.
//
// Returns:    
//   The index of the file is returned. If it is not found, -1
//   is returned.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 16:59:38 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
FileServerList::GetFileIndex(const QualifiedFilename &fileName)
{
    int index = 0;
    bool still_looking = true;
    string fullName(fileName.FullName());
    QualifiedFilenameVector::iterator pos;
    for(pos = appliedFileList.begin();
        pos != appliedFileList.end() && still_looking; ++pos, ++index)
    {
        // Compare the complete filenames.
        still_looking = (pos->FullName() != fullName);
    }

    return still_looking ? -1 : index;
}

// ****************************************************************************
// Method: FileServerList::QualifiedFilename
//
// Purpose: 
//   Prepends the hostname and path of the current MD Server to the 
//   given filename. The resulting qualified filename is of the form:
//   host:path:filename.
//
// Arguments:
//   fileName : The filename string to use.
//
// Returns:    
//   A qualified filename.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:27:39 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QualifiedFilename
FileServerList::QualifiedName(const string &fileName)
{
    return QualifiedFilename(activeHost, servers[activeHost]->path,
                             fileName);
}

// ****************************************************************************
// Method: FileServerList::SetConnectCallback
//
// Purpose: 
//   Sets the callback function used to tell the viewer to launch an
//   mdserver.
//
// Arguments:
//   cb   : The address of the callback function.
//   data : Callback data.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 21 13:16:45 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
FileServerList::SetConnectCallback(ConnectCallback *cb, void *data)
{
    connectCallback = cb;
    connectCallbackData = data;
}

// ****************************************************************************
// Method: FileServerList::SetProgressCallback
//
// Purpose: 
//   Sets the progress callback that is called while we launch a new mdserver.
//
// Arguments:
//   cb   : The address of the callback function.
//   data : Callback data.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 30 08:10:14 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
FileServerList::SetProgressCallback(bool (*cb)(void *, int), void *data)
{
    progressCallback = cb;
    progressCallbackData = data;
}

// ****************************************************************************
// Method: FileServerList::GetFilteredFileList
//
// Purpose: 
//   Compares all of the files in fileList against the filter list
//   and returns a list of qualified filenames that match any of the
//   filters. 
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 4 15:20:28 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Mar 27 09:28:57 PDT 2003
//   I made the routine do different things based on the flag for automatic
//   file grouping.
//
//   Brad Whitlock, Tue Apr 29 12:35:58 PDT 2003
//   I made it preserve the access flag for files when automatic file grouping
//   is turned off so the file selection window displays files that can't be
//   accessed in the right color.
//
// ****************************************************************************

QualifiedFilenameVector
FileServerList::GetFilteredFileList()
{
    QualifiedFilenameVector retval;

    if(automaticFileGroupingFlag)
    {
        // Go through each file in the file and virtualFiles list and add them
        // all to the returned list.
        MDServerProxy::FileEntryVector::const_iterator pos;
        for(pos = fileList.files.begin();
            pos != fileList.files.end(); ++pos)
        {
            // Add the host qualified filename to the applied file list.
            QualifiedFilename f(activeHost, servers[activeHost]->path,
                                pos->name, pos->CanAccess(), pos->IsVirtual());
            retval.push_back(f);
        }
    }
    else
    {
        // Parse the filter string into a list of filter strings.
        stringVector filterList;
        ParseFilterString(GetFilter(), filterList);

        // Make sure we have at least one filter.
        if(filterList.size() == 0)
            filterList.push_back("*");

        // Go through each file in the file list and
        MDServerProxy::FileEntryVector::const_iterator pos;
        for(pos = fileList.files.begin();
            pos != fileList.files.end(); ++pos)
        {
            if(FileMatchesFilterList(pos->name, filterList))
            {
                // Add the host qualified filename to the applied file list.
                QualifiedFilename f(activeHost, servers[activeHost]->path,
                                    pos->name, pos->CanAccess(), false);
                retval.push_back(f);
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::ParseFilterString
//
// Purpose: 
//   Given a string that possibly contains multiple filters, this
//   function parses the string into a list of filters and stores
//   them in the filter list.
//
// Arguments:
//   filter     : The string containing the filters.
//   filterList : The list of filters that is returned.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 15:15:47 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
FileServerList::ParseFilterString(const string &filter,
    stringVector &filterList)
{
    // Create a temporary C string since strtok messes up the string.
    char *temp  = new char[filter.size() + 1];
    char *ptr, *temp2 = temp;
    memcpy((void *)temp, (void *)filter.c_str(), filter.size() + 1);

    // Parse the filter string and store the result in filterList.
    filterList.clear();
    while((ptr = strtok(temp2, " ")) != NULL)
    {
        filterList.push_back(string(ptr));
        temp2 = NULL;
    }
    delete[] temp;
}

// ****************************************************************************
// Method: FileServerList::FileMatchesFilterList
//
// Purpose: 
//   Checks a filename against a list of filters to see if it matches
//   any of them.
//
// Arguments:
//   fileName   : The filename we're checking against the filters.
//   filterList : The list of filters we're checking for.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 4 15:19:12 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Sep 12 14:33:04 PST 2002
//   I restructured the code so it is easier to understand. I also changed it
//   to fit the new interface for FileMatchesFilter.
//
//   Jeremy Meredith, Fri Mar 19 14:46:24 PST 2004
//   I made it use WildcardStringMatch from Utility.h.
//
// ****************************************************************************

bool
FileServerList::FileMatchesFilterList(const string &fileName,
    const stringVector &filterList)
{
    // Try the filename against all the filters in the list until
    // it matches or we've tested all the filters.
    bool match = false;
    for(int i = 0; i < filterList.size() && !match; ++i)
    {
        match = WildcardStringMatch(filterList[i],fileName);
    }

    return match;
}

// ****************************************************************************
// Method: FileServerList::AddPathToRecentList
//
// Purpose: 
//   Adds the path to the recently used path list for the active host.
//
// Arguments:
//   path : The path that we're adding.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 13 13:02:15 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Aug 26 15:51:26 PST 2002
//   I removed the code to filter the path since it is now done in the 
//   mdserver.
//
//   Brad Whitlock, Tue Apr 22 13:59:48 PST 2003
//   I fixed a crash on Windows.
//
//   Brad Whitlock, Mon Oct 13 10:03:21 PDT 2003
//   I added the recentPathsFlag.
//
// ****************************************************************************

void
FileServerList::AddPathToRecentList(const string &host,
    const string &path)
{
    StringStringVectorMap::iterator pos = recentPaths.find(host);
    if(pos != recentPaths.end())
    {
        // Search the list to see if it already exists.
        bool exists = false;
        for(int i = 0; i < pos->second.size(); ++i)
        {
            if(path == pos->second[i])
            {
                exists = true;
                break;
            }
        }

        if(!exists)
        {
            pos->second.push_back(path);
            Select(ID_recentPathsFlag, (void *)&recentPathsFlag);
        }
    }
    else
    {
        stringVector tmp;
        tmp.push_back(path);
        recentPaths[host] = tmp;
        Select(ID_recentPathsFlag, (void *)&recentPathsFlag);
    }
}

// ****************************************************************************
// Method: FileServerList::ClearRecentPathList
//
// Purpose: 
//   Clears out the recent path list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 16:06:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
FileServerList::ClearRecentPathList()
{
    recentPaths.clear();
    Select(ID_recentPathsFlag, (void *)&recentPathsFlag);
}

// ****************************************************************************
// Method: FileServerList::DefineVirtualFiles
//
// Purpose: 
//   Copies the fileList's virtual files to the virtual files map.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 31 12:16:19 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr 22 13:59:21 PST 2003
//   I fixed a crash on Windows.
//
// ****************************************************************************

void
FileServerList::DefineVirtualFiles()
{
    StringStringVectorMap::const_iterator pos;
    for(pos = fileList.virtualFiles.begin();
        pos != fileList.virtualFiles.end(); ++pos)
    {
        QualifiedFilename name(activeHost, servers[activeHost]->path, pos->first);
        virtualFiles[name.FullName()] = pos->second;
#if 0
        debug4 << "Virtual file: " << name.FullName() << endl;
        for(int i = 0; i < pos->second.size(); ++i)
            debug4 << "\t" << pos->second[i] << endl;
#endif
    }
}

// ****************************************************************************
// Method: FileServerList::GetVirtualFileDefinition
//
// Purpose: 
//   Gets the list of files contained by the virtual file.
//
// Arguments:
//   name : The name of the virtual file.
//
// Returns:    A vector of strings that contains the list of files.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 31 12:26:49 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr 22 13:57:44 PST 2003
//   I fixed a crash on Windows.
//
// ****************************************************************************

stringVector
FileServerList::GetVirtualFileDefinition(const QualifiedFilename &name) const
{
    StringStringVectorMap::const_iterator pos = virtualFiles.find(name.FullName());
    if(pos != virtualFiles.end())
        return pos->second;

    stringVector retval;
    return retval;
}

// ****************************************************************************
// Method: FileServerList::GetVirtualFileDefinitionSize
//
// Purpose: 
//   Returns the size of the virtual file definition.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 29 11:15:20 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

int
FileServerList::GetVirtualFileDefinitionSize(const QualifiedFilename &name) const
{
    StringStringVectorMap::const_iterator pos = virtualFiles.find(name.FullName());
    if(pos != virtualFiles.end())
        return pos->second.size();

    return 0;
}

// ****************************************************************************
// Method: FileServerList::CreateNode
//
// Purpose: 
//   Adds FileServerList to the DataNode tree used to write the config file.
//
// Arguments:
//   parentNode : The parent node under which the file server node
//                should be added.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 4 12:51:34 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Feb 13 15:58:02 PST 2002
//   Added support for saving recent paths.
//
//   Brad Whitlock, Fri Jul 26 13:51:07 PST 2002
//   Added the useCurrentDirectoryFlag flag.
//
//   Brad Whitlock, Thu Aug 22 14:45:33 PST 2002
//   I added code to encode spaces in path names.
//
//   Brad Whitlock, Thu Mar 27 09:30:14 PDT 2003
//   I added automaticFileGroupingFlag.
//
//   Brad Whitlock, Tue Feb 24 14:42:47 PST 2004
//   I disabled VisIt's ability to save the active host. This prevents
//   VisIt from automatically logging you into a remote machine from having
//   saved your settings while visiting that remote computer.
//
//   Jeremy Meredith, Wed Jul  7 17:04:03 PDT 2004
//   I made the filter be global to all hosts.
//
//   Brad Whitlock, Thu Jul 29 13:46:46 PST 2004
//   I added smartFileGrouping.
//
// ****************************************************************************

bool
FileServerList::CreateNode(DataNode *parentNode, bool, bool)
{
    DataNode *fsNode = new DataNode("FileServerList");
    parentNode->AddNode(fsNode);

    // Add the path and filter for localhost to the fsNode.
#ifdef PERSISTENT_REMOTE_HOST_INFO
    //
    // This code was commented out on Tue Feb 24 14:38:05 PST 2004.
    // Enable this code again if we decide to allow users to save the
    // host on which they were last working so VisIt can log them in
    // automatically.
    //
    fsNode->AddNode(new DataNode("host", activeHost));
    fsNode->AddNode(new DataNode("path", servers[activeHost]->path));
#else
    //
    // Save the path and filter for localhost. If localhost is not in the
    // list of hosts then no path or filter are saved.
    //
    ServerMap::const_iterator info = servers.find("localhost");
    if(info != servers.end())
    {
        fsNode->AddNode(new DataNode("path", info->second->path));
    }
#endif
    fsNode->AddNode(new DataNode("filter", filter));
    fsNode->AddNode(new DataNode("useCurrentDir", useCurrentDirectoryFlag));
    fsNode->AddNode(new DataNode("automaticFileGrouping", automaticFileGroupingFlag));
    fsNode->AddNode(new DataNode("smartFileGrouping", smartFileGroupingFlag));

    // Add nodes for the recent paths.
    DataNode *pathNode = new DataNode("recentpaths");
    fsNode->AddNode(pathNode);
    StringStringVectorMap::const_iterator pos;
    for(pos = recentPaths.begin(); pos != recentPaths.end(); ++pos)
    {
        // Encode any spaces that might be in the path names.
        stringVector paths;
        for(int i = 0; i < pos->second.size(); ++i)
            paths.push_back(EncodePath(pos->second[i]));

        // Add a node for the paths from the current host.
        pathNode->AddNode(new DataNode(pos->first, paths));
    }

    return true;
}

// ****************************************************************************
// Method: FileServerList::SetFromNode
//
// Purpose: 
//   Sets the path and the filter from the config file.
//
// Arguments:
//   parentNode : The parent of the file server node.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 4 12:56:33 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Feb 13 16:09:22 PST 2002
//   Added support for reading in recent paths.
//
//   Brad Whitlock, Fri Jul 26 13:51:07 PST 2002
//   Added the useCurrentDirectoryFlag flag.
//
//   Brad Whitlock, Mon Aug 19 11:12:22 PDT 2002
//   I made useCurrentDirectoryFlag an attribute.
//
//   Brad Whitlock, Thu Aug 22 14:07:50 PST 2002
//   I made the recent paths be added to the recent paths so we don't lose
//   any recent paths that are not already in the list. I also change it
//   so recent paths that contained spaces are decoded.
//
//   Brad Whitlock, Thu Mar 27 09:31:55 PDT 2003
//   I added automaticFileGrouping.
//
//   Brad Whitlock, Tue Feb 24 14:45:36 PST 2004
//   I disabled VisIt's ability to set the host from a config file. This
//   means that you cannot any longer have VisIt automatically log you into
//   a remote computer.
//
//   Jeremy Meredith, Wed Jul  7 17:04:03 PDT 2004
//   I made the filter be global to all hosts.
//
//   Brad Whitlock, Thu Jul 29 13:47:20 PST 2004
//   I added smartFileGrouping.
//
// ****************************************************************************

void
FileServerList::SetFromNode(DataNode *parentNode)
{
    DataNode *fsNode = parentNode->GetNode("FileServerList");
    if(fsNode == 0)
        return;

    // See if we should use the current directory by default.
    DataNode *node;
    if((node = fsNode->GetNode("useCurrentDir")) != 0)
        SetUseCurrentDirectory(node->AsBool());

    if((node = fsNode->GetNode("automaticFileGrouping")) != 0)
        SetAutomaticFileGrouping(node->AsBool());

    if((node = fsNode->GetNode("smartFileGrouping")) != 0)
        SetSmartFileGrouping(node->AsBool());

    // If we are not using the current directory, read the default host
    // and path from the default settings.
    if(!useCurrentDirectoryFlag)
    {
#ifdef PERSISTENT_REMOTE_HOST_INFO
        // Read the filter and the path from the DataNode tree.
        if((node = fsNode->GetNode("host")) != 0)
            SetHost(node->AsString());
#endif
        if((node = fsNode->GetNode("path")) != 0)
            SetPath(node->AsString());
    }
    if((node = fsNode->GetNode("filter")) != 0)
        SetFilter(node->AsString());

    // Set the recent paths from the saved settings.
    if((node = fsNode->GetNode("recentpaths")) != 0)
    {
        DataNode **children = node->GetChildren();
        for(int i = 0; i < node->GetNumChildren(); ++i)
        {
            const stringVector &sv = children[i]->AsStringVector();
            for(int j = 0; j < sv.size(); ++j)
                AddPathToRecentList(children[i]->GetKey(), DecodePath(sv[j]));
        }
    }
}

//
// Some convenience functions. This is so the user of this class
// does not have to test whether or not an attribute is selected
// by using the ordinal.
//

bool
FileServerList::HostChanged() const
{
    return IsSelected(ID_hostFlag);
}

bool
FileServerList::PathChanged() const
{
    return IsSelected(ID_pathFlag);
}

bool
FileServerList::FilterChanged()   const
{
    return IsSelected(ID_filterFlag);
}

bool
FileServerList::FileListChanged() const
{
    return IsSelected(ID_fileListFlag);
}

bool
FileServerList::AppliedFileListChanged() const
{
    return IsSelected(ID_appliedFileListFlag);
}

bool
FileServerList::FileChanged() const
{
    return IsSelected(ID_fileAction);
}

bool
FileServerList::OpenedFile() const
{
    return (IsSelected(ID_fileAction) && (fileAction == FILE_OPEN));
}

bool
FileServerList::ReplacedFile() const
{
    return (IsSelected(ID_fileAction) && (fileAction == FILE_REPLACE));
}

bool
FileServerList::OverlayedFile() const
{
    return (IsSelected(ID_fileAction) && (fileAction == FILE_OVERLAY));
}

bool
FileServerList::ClosedFile() const
{
    return (IsSelected(ID_fileAction) && (fileAction == FILE_CLOSE));
}

bool
FileServerList::RecentPathsChanged() const
{
    return IsSelected(ID_recentPathsFlag);
}

//
// Property getting functions.
//

const string &
FileServerList::GetHost() const
{
    return activeHost;
}

const string &
FileServerList::GetPath() const
{
    static string retval("~");

    ServerMap::const_iterator pos;
    if((pos = servers.find(activeHost)) != servers.end())
        retval = pos->second->path;

    return retval;
}

// ****************************************************************************
// Method: FileServerList::GetRecentHosts
//
// Purpose: 
//   Returns a list of the recently visited hosts.
//
// Returns:    The list of recently visited hosts.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 25 15:44:16 PST 2002
//
// Modifications:
//   
// ****************************************************************************

stringVector
FileServerList::GetRecentHosts() const
{
    stringVector retval;
    for(StringStringVectorMap::const_iterator pos = recentPaths.begin();
        pos != recentPaths.end(); ++pos)
    {
        retval.push_back(pos->first);
    }

    return retval;
}

const stringVector &
FileServerList::GetRecentPaths(const string &host) const
{
    StringStringVectorMap::const_iterator pos = recentPaths.find(host);
    if(pos == recentPaths.end())
    {
        EXCEPTION1(BadHostException, host);
    }

    return pos->second;
}

// Modifications:
//
//   Jeremy Meredith, Wed Jul  7 17:04:03 PDT 2004
//   I made the filter be global to all hosts.
//
const string &
FileServerList::GetFilter() const
{
    return filter;
}

const MDServerProxy::FileList &
FileServerList::GetFileList() const
{
    return fileList;
}

const QualifiedFilenameVector &
FileServerList::GetAppliedFileList()
{
    return appliedFileList;
}

const QualifiedFilename &
FileServerList::GetOpenFile() const
{
    return openFile;
}

// ****************************************************************************
// Method: FileServerList::GetMetaData
//
// Purpose: Get metadata for specified file and time state. First, tries to
//          find it in the cache by either filename key only or filename+state
//          key. If it can't find it in the cache, requests it from the MD
//          server unless dontGetNew is set to true.
//
// Programmer: Mark C. Miller (totally re-wrote)
// Creation:   July 25, 2006 
//
// Modifications:
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//   Send flags for creation of MeshQuality and TimeDerivative expressions
//   in GetMetaData call.
//
// ****************************************************************************

const avtDatabaseMetaData *
FileServerList::GetMetaData(const QualifiedFilename &filename,
    int timeState, bool anyStateOk, bool dontGetNew, string *key)
{
    // build set of keys for cached metadata
    vector<string> mdKeys = MDCacheKeys(filename.FullName(),
                                        timeState);

    // look for cached metadata in priority order of keys
    FileMetaDataMap::const_iterator it = fileMetaData.find(mdKeys);

    if (it == fileMetaData.end())
    {// didn't find it in cache. So, get it from mdserver.

        // Since queries for any time require a brute force search,
        // we attempt those only if above failed
        if (anyStateOk)
        {
            const string& fullName = filename.FullName();
            int n = fullName.size();

            FileMetaDataMap::iterator mdit;
            for (mdit = fileMetaData.begin(); mdit != fileMetaData.end(); mdit++)
            {
                // gnu's compare(0,n,string) is buggy, so use compare(0,n,string,0,n)
                if (fullName.compare(0, n, mdit->first, 0, n) == 0)
                {
                  if (key)*key = mdit->first;
                    return mdit->second;
                }
            }
            if (dontGetNew)
                return 0;
        }
        else
        {
             if (dontGetNew)
                return 0;
        }

        // acquire new metadata from mdserver
        ServerMap::const_iterator svit = servers.find(filename.host);
        if (svit == servers.end()) 
            return 0;

        MDServerProxy *mds = svit->second->server;
        const avtDatabaseMetaData *md =
            mds->GetMetaData(filename.PathAndFile(), timeState,
                             forceReadAllCyclesTimes, "",
                             treatAllDBsAsTimeVarying,
                             createMeshQualityExpressions,
                             createTimeDerivativeExpressions,
                             createVectorMagnitudeExpressions);

        // cache what we got
        if (md)
        {
           // make a copy to cache
            avtDatabaseMetaData *newmd = new avtDatabaseMetaData;
            *newmd = *md;

            // decide on the right key
            string useKey = mdKeys[0]; // non-state-qualified key
            if (treatAllDBsAsTimeVarying || md->GetMustRepopulateOnStateChange())
                useKey = mdKeys[1];    // state-qualified key

            // cache it. Note MRU handles deletion
            fileMetaData[useKey] = newmd;

            if (key) *key = useKey;
            debug3 << "Caching metadata with key \"" << useKey << "\"" << endl;
            newmd->Print(DebugStream::Stream3());
            return fileMetaData[useKey];
        }
        else
        {
            return 0;
        }
    }
    else
    {// found it in cache, return it.
        if (key) *key = it->first;
        return fileMetaData[it->first];
    }
}

bool
FileServerList::GetUseCurrentDirectory() const
{
    return useCurrentDirectoryFlag;
}

bool
FileServerList::GetAutomaticFileGrouping() const
{
    return automaticFileGroupingFlag;
}

bool
FileServerList::GetSmartFileGrouping() const
{
    return smartFileGroupingFlag;
}

bool
FileServerList::GetForceReadAllCyclesTimes() const
{
    return forceReadAllCyclesTimes;
}

bool
FileServerList::GetTreatAllDBsAsTimeVarying() const
{
    return treatAllDBsAsTimeVarying;
}

bool
FileServerList::GetCreateMeshQualityExpressions() const
{
    return createMeshQualityExpressions;
}

bool
FileServerList::GetCreateTimeDerivativeExpressions() const
{
    return createTimeDerivativeExpressions;
}

// ****************************************************************************
// Method: FileServerList::GetCreateVectorMagnitudeExpressions
//
// Purpose: Get flag indicating if vector magnitude expressions should be 
//          auto generated
//
// Programmer: Cyrus Harrison
// Creation:   November 28, 2007
//   
// ****************************************************************************

bool
FileServerList::GetCreateVectorMagnitudeExpressions() const
{
    return createVectorMagnitudeExpressions;
}


// ****************************************************************************
// Method: FileServerList::GetSIL
//
// Purpose: 
//   Returns a pointer to a SIL for the given filename.
//
// Arguments:
//   filename : The filename for which we will try and read a SIL.
//
// Returns:    A pointer to a the SIL or NULL if the filename is bad.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:53:57 PDT 2001
//
// Modifications:
//   Eric Brugger, Thu Nov 29 12:02:21 PST 2001
//   Added caching of SILs.
//   
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Totally re-wrote to mimic metadata cache behavior 
//
//   Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//   Added use of treatAllDBsAsTimeVarying to GetSIL call to server.
// ****************************************************************************

const avtSIL *
FileServerList::GetSIL(const QualifiedFilename &filename, int timeState,
    bool anyStateOk, bool dontGetNew, string *key)
{
    // build set of keys for cached SIL
    vector<string> mdKeys = MDCacheKeys(filename.FullName(),
                                        timeState);

    // look for cached SIL in priority order of keys
    SILMap::const_iterator it = SILData.find(mdKeys);

    if (it == SILData.end())
    {// didn't find it in cache. So, get it from mdserver.

        // Since queries for any time require a brute force search,
        // we attempt those only if above failed
        if (anyStateOk)
        {
            const string& fullName = filename.FullName();
            int n = fullName.size();

            SILMap::iterator sit;
            for (sit = SILData.begin(); sit != SILData.end(); sit++)
            {
                // gnu's compare(0,n,string) is buggy, so use compare(0,n,string,0,n)
                if (fullName.compare(0, n, sit->first, 0, n) == 0)
                    return sit->second;
            }
            if (dontGetNew)
                return 0;
        }
        else
        {
            if (dontGetNew)
                return 0;
        }

        // acquire new SIL from mdserver
        ServerMap::const_iterator svit = servers.find(filename.host);
        if (svit == servers.end()) 
            return 0;

        MDServerProxy *mds = svit->second->server;
        const SILAttributes *sil = mds->GetSIL(filename.PathAndFile(),
            timeState, treatAllDBsAsTimeVarying);

        // cache what we got
        if (sil)
        {
            // Create a new SIL object and copy the one that was
            // returned from the MDServer.
            avtSIL *newSIL = new avtSIL(*sil);

            // decide on the right key
            string useKey = mdKeys[0]; // non-state-qualified key
            if (treatAllDBsAsTimeVarying ||
                GetMetaData(filename, timeState, ANY_STATE, !GET_NEW_MD, 0)->
                    GetMustRepopulateOnStateChange())
                useKey = mdKeys[1];    // state-qualified key

            // cache it. Note MRU handles deletion
            SILData[useKey] = newSIL;

            if (key) *key = useKey;
            debug3 << "Caching SIL with key \"" << useKey << "\"" << endl;
            newSIL->Print(DebugStream::Stream3());
            return SILData[useKey];
        }
        else
        {
            return 0;
        }
    }
    else
    {// found it in cache, return it.

        if (key) *key = it->first;
        return SILData[it->first];
    }
}

// ****************************************************************************
// Method: FileServerList::CreateGroupList
//
// Purpose: 
//   Tells the server to group files together in a list.  Then refreshes
//   the list of files from that server.
//
// Arguments:
//   filename:  The name of the group file to create
//   groupList: The list of files to group together.
//
// Programmer: Sean Ahern
// Creation:   Wed Feb 28 17:11:40 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 13 12:50:55 PDT 2002
//   Changed prototype.
//
// ****************************************************************************

void
FileServerList::CreateGroupList(const string &filename,
    const stringVector &groupList)
{
    ServerInfo *info = servers[activeHost];
    info->server->CreateGroupList(filename, groupList);

    // Reread the file list from the server
    pathFlag = true;
    Select(ID_pathFlag, (void *)&pathFlag);
    Notify();
}

// ****************************************************************************
// Method: FileServerList::GetHomePath
//
// Purpose: 
//   Gets the home path for the active file server.
//
// Returns:    The home path for the active file server.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 11:46:35 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

string
FileServerList::GetHomePath()
{
    // Get a pointer to the mdserver for the active host.
    ServerInfo *info = servers[activeHost];

    // Initialize the return value.
    string homePath("~");

    TRY
    {
        homePath = info->server->ExpandPath("~");
    }
    CATCH(LostConnectionException)
    {
        Error("Could not get home directory");
    }
    ENDTRY

    return homePath;
}

// ****************************************************************************
// Method: FileServerList::ExpandPath
//
// Purpose: 
//   Expands the path to a full path.
//
// Arguments:
//   p : The path to expand.
//
// Returns:    A string containing the expanded path.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 29 12:05:20 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

string
FileServerList::ExpandPath(const string &p)
{
    // Initialize the return value.
    string homePath(p);

    if(servers.find(activeHost) != servers.end())
    {
        // Get a pointer to the mdserver for the active host.
        ServerInfo *info = servers[activeHost];

        TRY
        {
            homePath = info->server->ExpandPath(p);
        }
        CATCH(VisItException)
        {
            Error("VisIt could not expand the path.");
        }
        ENDTRY
    }

    return homePath;
}

// ****************************************************************************
// Method: FileServerList::GetMessageAttributes
//
// Purpose: 
//   Returns a pointer the object's MessageAttributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 26 10:01:50 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

MessageAttributes *
FileServerList::GetMessageAttributes()
{
    return messageAtts;
}

// ****************************************************************************
// Method: FileServerList::Error
//
// Purpose: 
//   Notifies the file server's client that there was an error.
//
// Arguments:
//   message : The error message to use.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 26 10:02:13 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
FileServerList::Error(const char *message)
{
    messageAtts->SetSeverity(MessageAttributes::Error);
    messageAtts->SetText(string(message));
    messageAtts->Notify();
}

// ****************************************************************************
// Method: FileServerList::Warning
//
// Purpose: 
//   Notifies the file server's client that there was a warning.
//
// Arguments:
//   message : The warning message to use.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 26 10:02:13 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
FileServerList::Warning(const char *message)
{
    messageAtts->SetSeverity(MessageAttributes::Warning);
    messageAtts->SetText(string(message));
    messageAtts->Notify();
}

// ****************************************************************************
// Method: FileServerList::EncodePath
//
// Purpose: 
//   Replaces all of the spaces in a string with "%32".
//
// Arguments:
//   path : The input string.
//
// Returns:    A string in which all spaces have been turned to %32.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 22 14:26:39 PST 2002
//
// Modifications:
//   
// ****************************************************************************

string
FileServerList::EncodePath(const string &path)
{
    int index = 0;
    string retval(path);

    while((index = retval.find(" ", index)) != -1)
        retval.replace(index, 1, "%32");

    return retval;
}

// ****************************************************************************
// Method: FileServerList::DecodePath
//
// Purpose: 
//   Replaces all "%32" strings in the input string with spaces.
//
// Arguments:
//   path : The input string.
//
// Returns:    A string in which all "%32" strings have been turned to spaces.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 22 14:26:35 PST 2002
//
// Modifications:
//   
// ****************************************************************************

string
FileServerList::DecodePath(const string &path)
{
    int index = 0;
    string retval(path);

    while((index = retval.find("%32", index)) != -1)
        retval.replace(index, 3, " ");

    return retval;
}

// ****************************************************************************
// Method: FileServerList::GetSeparator
//
// Purpose: 
//   Returns the character used to separate directory names.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 26 15:47:44 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Jan 17 15:41:07 PST 2003
//   I made it use a wrapper that tries to restart the mdserver. This prevents
//   us from ever trying to use a NULL pointer.
//
// ****************************************************************************

#define SAFE_GET_SEPARATOR(host, func)    TRY\
    {\
        if(servers.find(host) == servers.end())\
            StartServer(host);\
        if(servers.find(host) != servers.end())\
        {   const ServerInfo *info = servers[host]; \
            sep = info->server->func();\
        }\
    }\
    CATCHALL(...)\
    {\
    }\
    ENDTRY

char
FileServerList::GetSeparator()
{
    char sep = VISIT_SLASH_CHAR;
    SAFE_GET_SEPARATOR(activeHost, GetSeparator);
    return sep;
}

char
FileServerList::GetSeparator(const string &host)
{
    char sep = VISIT_SLASH_CHAR;
    SAFE_GET_SEPARATOR(host, GetSeparator);
    return sep;
}

// ****************************************************************************
// Method: FileServerList::GetSeparatorString
//
// Purpose: 
//   Returns the string used to separate directory names.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 26 15:48:07 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Jan 17 15:41:07 PST 2003
//   I made it use a wrapper that tries to restart the mdserver. This prevents
//   us from ever trying to use a NULL pointer.
//   
// ****************************************************************************

string
FileServerList::GetSeparatorString()
{
    string sep(VISIT_SLASH_STRING);
    SAFE_GET_SEPARATOR(activeHost, GetSeparatorString);
    return sep;
}

string
FileServerList::GetSeparatorString(const string &host)
{
    string sep(VISIT_SLASH_STRING);
    SAFE_GET_SEPARATOR(host, GetSeparatorString);
    return sep;
}

// ****************************************************************************
//  Method:  FileServerList::SetOpenFileMetaData
//
//  Purpose:
//    Poke new metadata into the file server.  This is needed by
//    databases that return metadata from the Engine and not the
//    MDServer -- i.e. simulations.
//
//  Arguments:
//    md         the new metadata
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
//  Modifications:
//    Brad Whitlock, Wed May 4 14:25:27 PST 2005
//    I made it check to make sure that the file exists before trying to 
//    blindly insert the metadata into the cache.
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interfaces to GetMetaData and GetSIL
//   Made fileMetaData and SILData MRUCache's
//
//   Shelly Prevost, Fri Sept 8 12:18
//   Rolled back Mark's changes because it breaks the
//   Simulation windows ability to get updated metadata.
//   The key is not valid.
// ****************************************************************************

void
FileServerList::SetOpenFileMetaData(const avtDatabaseMetaData *md, int timeState)
{
   if(fileMetaData.find(openFile.FullName()) != fileMetaData.end())
    {
        *(fileMetaData[openFile.FullName()]) = *md;
        // hack to have it return that the file changed
        fileAction=FILE_OPEN;
        Select(ID_fileAction, (void *)&fileAction);
    }
    else
    {
        debug1 << "Attempted to insert metadata for a file that has not been "
                  "opened." << endl;
    } 
}

// ****************************************************************************
//  Method:  FileServerList::SetOpenFileSIL
//
//  Purpose:
//    Poke a new SIL into the file server.  This is needed by
//    databases that return metadata from the Engine and not the
//    MDServer -- i.e. simulations.
//
//  Arguments:
//    sil        the new SIL
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
//  Modifications:
//    Brad Whitlock, Wed May 4 14:25:27 PST 2005
//    I made it check to make sure that the file exists before trying to 
//    blindly insert the metadata into the cache.
//
// ****************************************************************************

void
FileServerList::SetOpenFileSIL(const avtSIL *sil)
{
    if(SILData.find(openFile.FullName()) != SILData.end())
    {
        *(SILData[openFile.FullName()]) = *sil;
        // hack to have it return that the file changed
        fileAction=FILE_OPEN;
        Select(ID_fileAction, (void *)&fileAction);
    }
    else
    {
        debug1 << "Attempted to insert a SIL for a file that has not been "
                  "opened." << endl;
    } 
}
