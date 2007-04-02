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

#include <stdio.h>   // for sprintf
#include <stdlib.h>
#include <snprintf.h>
#include <ViewerFileServer.h>

#include <avtDatabaseMetaData.h>
#include <avtSimulationInformation.h>
#include <avtSIL.h>
#include <BadHostException.h>
#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <DataNode.h>
#include <DBPluginInfoAttributes.h>
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
#include <ParsingExprList.h>
#include <SILAttributes.h>
#include <ViewerConnectionProgressDialog.h>
#include <ViewerWindowManager.h>

#include <DebugStream.h>

#include <algorithm>

#define ANY_STATE -1

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
//   Brad Whitlock, Tue Apr 13 23:19:08 PST 2004
//   I added declinedFiles and declinedFilesLength.
//
//   Hank Childs, Wed May 25 10:51:15 PDT 2005
//   Added dbPluginInfoAtts.
//
//   Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//   Added initialization of tryHarderCyclesTimes
// ****************************************************************************

ViewerFileServer::ViewerFileServer() : ViewerServerManager(), servers(),
    fileMetaData(), fileSIL(), declinedFiles(), declinedFilesLength()
{
    databaseCorrelationList = new DatabaseCorrelationList;
    dbPluginInfoAtts = new DBPluginInfoAttributes;
    tryHarderCyclesTimes = false;
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
//   Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//   Added initialization of tryHarderCyclesTimes
// ****************************************************************************

ViewerFileServer::ViewerFileServer(const ViewerFileServer &) : ViewerServerManager()
{
    tryHarderCyclesTimes = false;
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
// Modifications:
//   Brad Whitlock, Fri Mar 26 10:47:51 PDT 2004
//   I rewrote the method so it uses GetMetaDataForState.
//
// ****************************************************************************

bool
ViewerFileServer::MetaDataIsInvariant(const std::string &host, 
    const std::string &filename, int state)
{
    //
    // Get the metadata for the specified state and then return whether it
    // is invariant.
    //
    const avtDatabaseMetaData *md = GetMetaDataForState(host, filename, state);
    return (md != 0) ? (!md->GetMustRepopulateOnStateChange()) : true;
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
//   Brad Whitlock, Fri Mar 26 11:48:56 PDT 2004
//   Moved the bulk of the code into GetMetaDataHelper.
//
//   Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//   Added arg forceReadAllCyclesAndTimes and logic to ignore cached
//   values if we are forcing reading of all cycles and times and
//   the cached metadata doesn't have all accurate cycles and times
// ****************************************************************************

const avtDatabaseMetaData *
ViewerFileServer::GetMetaData(const std::string &host, 
                              const std::string &db,
                              const bool forceReadAllCyclesAndTimes)
{
    //
    // We don't care about the time state so look for any cached metadata
    // having the right host and database name and return it.
    //
    std::string dbName(ComposeDatabaseName(host, db));
    for(FileMetaDataMap::const_iterator pos = fileMetaData.begin();
        pos != fileMetaData.end();
        ++pos)
    {
        // Split the metadata key into name and ts components so we only
        // add a file once in the case that it is time-varying and there are
        // multiple cached metadata objects for different time states.
        std::string name;
        int         ts;
        SplitKey(pos->first, name, ts);

        bool cyclesAndTimesAreGood = pos->second->AreAllTimesAccurateAndValid() &&
                                     pos->second->AreAllCyclesAccurateAndValid();

        if (name == dbName && (!forceReadAllCyclesAndTimes || cyclesAndTimesAreGood))
            return pos->second;
    }

    // if we're here because we've forced reading all cycles and times
    // then we can't use ANY_STATE because we are potentially ALSO making this
    // call on a database that is different from the one the mdserver thinks
    // is current. Our only choice is to use state zero
    int state = forceReadAllCyclesAndTimes ? 0 : ANY_STATE;

    return GetMetaDataHelper(host, db, state, forceReadAllCyclesAndTimes);

}

// ****************************************************************************
// Method: ViewerFileServer::GetMetaDataForState
//
// Purpose: 
//   This method returns the metadata for database at the specified time state.
//
// Arguments:
//   host      : The host where the database is located.
//   db        : The name of the database.
//   timeState : The time state for which we want metadata.
//
// Returns:    
//
// Note:       The timeState argument is only used if we have to read new
//             metadata or if the metadata is not invariant. We ignore the
//             time state argument in the cache lookup if the metadata is
//             invariant because invariant metadata never has the time state
//             stored as part of its cache key.
//
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 26 11:49:13 PDT 2004
//
// Modifications:
//
//   Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//   Added bool argument to force reading of all cycles and times
//   to call to GetMetaDataHelper
//   
//   Jeremy Meredith, Mon Aug 28 16:55:01 EDT 2006
//   Added ability to force using a specific plugin when opening a file.
//
// ****************************************************************************

const avtDatabaseMetaData *
ViewerFileServer::GetMetaDataForState(const std::string &host, 
    const std::string &db, int timeState,
    const std::string &forcedFileType)
{
    //
    // Make sure a valid state was passed.
    //
    if(timeState < 0)
    {
        Error("GetMetaDataForState called with ANY_STATE. That is "
              "not allowed so VisIt will instead use time state 0.");
        timeState = 0;
    }

    //
    // First check to see if the metadata is time-varying. If the metadata
    // does not vary over time then return the first metadata that we
    // find. If it does vary over time and that metadata is already cached,
    // return that metadata.
    //
    std::string dbName(ComposeDatabaseName(host, db));
    for(FileMetaDataMap::const_iterator pos = fileMetaData.begin();
        pos != fileMetaData.end();
        ++pos)
    {
        // Split the metadata key into name and ts components so we only
        // add a file once in the case that it is time-varying and there are
        // multiple cached metadata objects for different time states.
        std::string name;
        int         ts;
        SplitKey(pos->first, name, ts);

        if(name == dbName)
        {
            //
            // If the metadata does not change over time or if it does and
            // the time states match then return what we found.
            //
            if(!pos->second->GetMustRepopulateOnStateChange() ||
               ts == timeState)
            {
                return pos->second;
            }
        }
    }

    const bool forceReadAllCyclesAndTimes = false;
    return GetMetaDataHelper(host, db, timeState, forceReadAllCyclesAndTimes,
                             forcedFileType);
}

// ****************************************************************************
// Method: ViewerFileServer::GetMetaDataHelper
//
// Purpose: 
//   This method is used to help read metadata from the mdserver.
//
// Arguments:
//   host      : The host where the database is located.
//   db        : The name of the database.
//   timeState : The time state for which we want metadata.
//
// Returns:    
//
// Note:       The bulk of this code was moved from GetMetaData. The timeState
//             argument can be set to a valid time state or it can be set to
//             ANY_STATE if we want to allow the mdserver to give back the
//             metadata for whatever state it prefers.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 26 11:52:33 PDT 2004
//
// Modifications:
//   
//   Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//   Added bool arg forceReadAllCyclesAndTimes
//
//   Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//   Added use of tryHarderCyclesTimes in call to GetMetaData
//
//   Jeremy Meredith, Mon Aug 28 16:55:01 EDT 2006
//   Added ability to force using a specific plugin when opening a file.
//
//   Hank Childs, Fri Jan 12 09:12:11 PST 2007
//   Clean up error message a bit.
//
// ****************************************************************************

const avtDatabaseMetaData *
ViewerFileServer::GetMetaDataHelper(const std::string &host, 
    const std::string &db, int timeState, bool forceReadAllCyclesAndTimes,
    const std::string &forcedFileType)
{
    // Try and start a server if one does not exist.
    NoFaultStartServer(host);

    // Initialize the return value.
    const avtDatabaseMetaData *retval = NULL;

    // If a server exists, get the metadata.
    if(servers.find(host) != servers.end())
    {
        int numAttempts = 0;
        bool tryAgain = false;

        //
        // We have a server from which we can get the metadata for the 
        // specified filename so let's try and get it now and put it in the
        // metadata map.
        //
        do
        {
            tryAgain = false;

            TRY
            {
                const avtDatabaseMetaData *md =
                    servers[host]->proxy->GetMetaData(db, timeState,
                                              forceReadAllCyclesAndTimes ||
                                              tryHarderCyclesTimes,
                                              forcedFileType);

                if(md != NULL)
                {
                    avtDatabaseMetaData *mdCopy = new avtDatabaseMetaData(*md);

                    //
                    // If the meta-data changes for each state, then cache
                    // the meta-data on a per state basis.  This is done by
                    // encoding the state into the name. Don't encode the state
                    // into the name though if we got it using ANY_STATEs
                    //
                    std::string key(ComposeDatabaseName(host, db));
                    if (mdCopy->GetMustRepopulateOnStateChange() &&
                        timeState != ANY_STATE)
                    {
                        char timeStateString[20];
                        SNPRINTF(timeStateString, 20, ":%d", timeState);
                        key += timeStateString;
                    }

                    //
                    // Add the metadata copy to the cache.
                    //
                    fileMetaData[key] = mdCopy;

                    retval = mdCopy;
                }
            }
            CATCH2(GetMetaDataException, gmde)
            {
                char msg[1000];
                if (host == "localhost")
                {
                    SNPRINTF(msg, 1000, "VisIt could not read from the file "
                             "\"%s\".\n\nThe generated error message"
                             " was:\n\n%s", db.c_str(),
                            gmde.Message().c_str());
                }
                else
                {
                    SNPRINTF(msg, 1000, "VisIt could not read from the file "
                             "\"%s\" on host %s.\n\nThe generated error message"
                             " was:\n\n%s", db.c_str(),
                            host.c_str(), gmde.Message().c_str());
                }
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
                int numTries = 2;
                tryAgain = (numAttempts < numTries);
                if (numAttempts == numTries)
                {
                    char message[512];
                    SNPRINTF(message, 512, "VisIt was unable to open \"%s\"."
                              "  Each attempt to open it caused VisIt's "
                              "metadata server to crash.  This can occur when "
                              "the file is corrupted, or when the underlying "
                              "file format has changed and VisIt's readers "
                              "have not been updated yet, or when the reader "
                              "VisIt is using"
                              " for your file format is not robust.  Please "
                              "check whether the file is corrupted and, if "
                              "not, contact a VisIt developer.", db.c_str());
                    Error(message);
                }

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
//   Brad Whitlock, Fri Mar 26 12:27:00 PDT 2004
//   I rewrote the method so it will return the SIL for any time state that
//   it finds.
//
// ****************************************************************************

const avtSIL *
ViewerFileServer::GetSIL(const std::string &host, 
    const std::string &db)
{
    //
    // We don't care about the time state so look for any cached metadata
    // having the right host and database name and return it.
    //
    std::string dbName(ComposeDatabaseName(host, db));
    for(FileSILMap::const_iterator pos = fileSIL.begin();
        pos != fileSIL.end();
        ++pos)
    {
        // Split the metadata key into name and ts components so we only
        // add a file once in the case that it is time-varying and there are
        // multiple cached metadata objects for different time states.
        std::string name;
        int         ts;
        SplitKey(pos->first, name, ts);

        if(name == dbName)
            return pos->second;
    }

    return GetSILHelper(host, db, ANY_STATE);
}

// ****************************************************************************
// Method: ViewerFileServer::GetSILForState
//
// Purpose: 
//   Gets the SIL at the specified time state.
//
// Arguments:
//   host      : The host where the database is located.
//   db        : The database for which we want the SIL.
//   timeState : The time state for which we want the SIL.
//
// Returns:    A pointer to a SIL.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 26 12:27:31 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

const avtSIL *
ViewerFileServer::GetSILForState(const std::string &host, 
    const std::string &db, int timeState)
{
    //
    // Make sure a valid state was passed.
    //
    if(timeState < 0)
    {
        Error("GetSILForState called with ANY_STATE. That is "
              "not allowed so VisIt will instead use time state 0.");
        timeState = 0;
    }

    //
    // Determine whether the metadata (and the SIL) are invariant.
    //
    bool invariantMetaData = MetaDataIsInvariant(host, db, timeState);

    //
    // First check to see if the metadata is time-varying. If the metadata
    // does not vary over time then return the first metadata that we
    // find. If it does vary over time and that metadata is already cached,
    // return that metadata.
    //
    std::string dbName(ComposeDatabaseName(host, db));
    for(FileSILMap::const_iterator pos = fileSIL.begin();
        pos != fileSIL.end();
        ++pos)
    {
        // Split the metadata key into name and ts components so we only
        // add a file once in the case that it is time-varying and there are
        // multiple cached metadata objects for different time states.
        std::string name;
        int         ts;
        SplitKey(pos->first, name, ts);

        if(name == dbName)
        {
            //
            // If the metadata does not change over time or if it does and
            // the time states match then return what we found.
            //
            if(invariantMetaData || ts == timeState)
            {
                return pos->second;
            }
        }
    }

    return GetSILHelper(host, db, timeState);
}

// ****************************************************************************
// Method: ViewerFileServer::GetSILHelper
//
// Purpose: 
//   Gets the SIL at the specified time state.
//
// Arguments:
//   host      : The host where the database is located.
//   db        : The database for which we want the SIL.
//   timeState : The time state for which we want the SIL.
//
// Returns:    A pointer to a SIL.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 26 12:27:31 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

const avtSIL *
ViewerFileServer::GetSILHelper(const std::string &host, const std::string &db,
    const int timeState)
{
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
                //
                // Create a key to use when storing the SIL in the map.
                //
                std::string key(ComposeDatabaseName(host, db));
                if(timeState != ANY_STATE &&
                   !MetaDataIsInvariant(host, db, timeState))
                {
                    char state_str[64];
                    SNPRINTF(state_str, 64, ":%d", timeState);
                    key += state_str;
                }

                //
                // Get the SIL from the mdserver
                //
                const SILAttributes *atts =
                    servers[host]->proxy->GetSIL(db, timeState);

                if(atts != NULL)
                {
                    avtSIL *silCopy = new avtSIL(*atts);
                    fileSIL[key] = silCopy;
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
                         "the following message:\n\n%s", db.c_str(),
                         host.c_str(), gmde.Message().c_str());
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
// Method: ViewerFileServer::ExpandDatabaseName
//
// Purpose: 
//   Expands a hostDBName into a host, db, and host+db name.
//
// Arguments:
//   hostDBName : The only input that is required. The value must be a
//                host:database.
//   host       : Returns the name of the host.
//   db         : Returns the expanded database name.
//
// Note:       hostDBName is modified to be the new host-qualified
//             expanded name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 25 15:03:48 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerFileServer::ExpandDatabaseName(std::string &hostDBName,
    std::string &host, std::string &db)
{
    // Split the host database name into host + db.
    SplitHostDatabase(hostDBName, host, db);

    //
    // Expand the file name so it does not contain wildcards or
    // relatve paths.
    //
    db = ExpandedFileName(host, db);

    //
    // Compose the new filename.
    //
    hostDBName = ComposeDatabaseName(host, db);
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
//   Brad Whitlock, Wed Mar 31 09:55:20 PDT 2004
//   Added a call to ExpansionRequired so we can prevent calls to the
//   mdserver if they are not really needed.
//
// ****************************************************************************

std::string
ViewerFileServer::ExpandedFileName(const std::string &host,
    const std::string &filename)
{
    std::string retval(filename);

    //
    // If filename expansion is required, expand the filename.
    //
    if(ExpansionRequired(filename))
    {
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
    }

    return retval;
}

// ****************************************************************************
// Method: ViewerFileServer::ExpansionRequired
//
// Purpose: 
//   Returns whether a filename needs to be expanded my the mdserver.
//
// Arguments:
//   filename : The filename that we're checking for proper expansion.
//
// Returns:    True if the filename needs to be expanded; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 31 10:00:43 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerFileServer::ExpansionRequired(const std::string &filename) const
{
    if(filename.size() > 0)
    {
#if defined(_WIN32)
        // Look for some drive punctuation
        if(filename.find(":\\") != std::string::npos)
            return true;
#else
        // Make sure that we have an absolute path.
        if(filename[0] != '/')
            return true;
#endif
        if(filename.find("~") != std::string::npos)
            return true;

        if(filename.find("..") != std::string::npos)
            return true;
    }

    return false;
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
//    Brad Whitlock, Wed Feb 2 14:25:31 PST 2005
//    Improved the error message for "Could not connect" exception.
//
//    Brad Whitlock, Fri Mar 17 10:56:20 PDT 2006
//    Improved the "Cound not connect" exception error message even more.
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
        char *message = new char[10000];
        const char *h = host.c_str();
        SNPRINTF(message, 10000,
            "The metadata server on host %s could not be launched or it "
            "could not connect back to your local computer. This can "
            "happen for a variety of reasons.\n\n"

            "It is possible that SSH was unable to launch VisIt on %s. "
            "If you want to verify this, run "
            "\"visit -debug 5\" and then check to see if any vcl, "
            "mdserver, or engine log files are present on %s in your "
            "home directory. If no log files were created then SSH was "
            "probably not able to launch VisIt components on %s. In that "
            "case, check that you can SSH to %s and check your local "
            "VisIt installation's Host profiles to make sure the path "
            "to VisIt on %s is specified. Alternatively, you set the "
            "PATH environment variable on %s so it contains the "
            "path to the program \"visit\".\n\n"

            "If there were no debug logs to be found on %s and your local "
            "computer runs a newer version of Linux then quit VisIt and "
            "try running \"visit -nopty -debug 5\". The \"-nopty\" option "
            "tells VisIt not to allocate a pseudoterminal in which to "
            "run SSH. When you run with the \"-nopty\" option, VisIt's "
            "password window will not be used. Instead, look for an SSH "
            "prompt in the terminal window where you ran VisIt. You should "
            "be able to enter your password at that prompt. If successful, "
            "SSH should continue trying to launch VisIt on %s. If VisIt "
            "still cannot connect after SSH launches VisIt's remote "
            "components, check for debug logs on %s to see if VisIt "
            "was at least able to launch there.\n\n"

            "If you found debug log files on %s but VisIt still can't "
            "connect then it's possible that %s cannot connect to your "
            "local computer. Some desktop computers do not provide a "
            "valid network name when VisIt asks for one. If you suspect "
            "that this could be the cause of the launch failure, try "
            "using \"Parse from SSH_CLIENT\" in your host profile for "
            "host %s. If that does not work and if you are using VPN "
            "then you should try manually setting the local host name "
            "VisIt will use when telling its remote components to connect "
            "back to your local computer. Open the Host profiles window "
            "and go to the Advanced options tab. Click the \"Specify manually\" "
            "radio button and type in the IP address of your VPN session "
            "into the adjacent text field before you try connecting again.\n\n"

            "If changing the above settings still does not allow you to "
            "connect then you may have a local firewall blocking "
            "ports 5600-5609, which are the ports that VisIt uses to "
            "listen for incoming connections (when they are expected) "
            "from remote VisIt components. If you've tried the previous "
            "suggestions and none of them worked then you may have a "
            "firewall denying VisIt access to local computer. Try turning "
            "the firewall off or allowing ports 5600-5609 and run VisIt "
            "again. If you do not know how to enable ports for your "
            "firewall or if you do not have the required privileges, "
            "contact your system administrator.\n\n"

            "If none of these suggestions allow you to successfully "
            "connect to %s then contact visit-help@llnl.gov and provide "
            "information about how you are trying to connect. Be sure to "
            "include the VisIt version and platform on which you are "
            "running.", h,h,h,h,h,h,h,h,h,h,h,h,h,h);
        Error(message);
        delete [] message;

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
// Method: ViewerFileServer::SendKeepAlives
//
// Purpose: 
//   Sends keep alive signals to all of the mdservers.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 12:07:13 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerFileServer::SendKeepAlives()
{
    ServerMap::iterator pos;
    for(pos = servers.begin(); pos != servers.end();)
    {
        TRY
        {
            debug2 << "Sending keep alive signal to mdserver on "
                   << pos->first.c_str() << endl;
            pos->second->proxy->SendKeepAlive();
            ++pos;
        }
        CATCHALL(...)
        {
            debug2 << "Could not send keep alive signal to mdserver on "
                   << pos->first.c_str() << " so that mdserver will be closed."
                   << endl;
            delete pos->second->proxy;
            delete pos->second;
            pos->second = 0;
            servers.erase(pos++);
        }
        ENDTRY
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
// Method: ViewerFileServer::UpdateDBPluginInfo
//
// Purpose:
//   Tells the mdserver to update the database plugin information.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// ****************************************************************************
void
ViewerFileServer::UpdateDBPluginInfo(const std::string &host)
{
    if(servers.find(host) != servers.end())
    {
        *dbPluginInfoAtts = *(servers[host]->proxy->GetDBPluginInfo());
        dbPluginInfoAtts->Notify();
    }
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
//   Brad Whitlock, Fri Feb 27 12:25:13 PDT 2004
//   I changed the code so it can actually remove metadata and SILs for
//   time-varying metadata. I also added code to delete the default database
//   correlation for the database, if one exists.
//
// ****************************************************************************

void
ViewerFileServer::ClearFile(const std::string &fullName)
{
    debug4 << "ViewerFileServer::Clearfile" << endl;

    // Clear the metadata.
    for(FileMetaDataMap::iterator mpos = fileMetaData.begin();
        mpos != fileMetaData.end();)
    {
        // Split the key into host+database and time state components.
        std::string hdb;
        int ts;
        SplitKey(mpos->first, hdb, ts);

        // If the name is a file that we're deleting then remove the metadata.
        if(hdb == fullName)
        {
            debug4 << "\tDeleted metadata for " << hdb.c_str() << endl;
            delete mpos->second;
            fileMetaData.erase(mpos++);
        }
        else
            ++mpos;
    }

    // Clear the SIL.
    for(FileSILMap::iterator spos = fileSIL.begin(); spos != fileSIL.end();)
    {
        // Split the key into host+database and time state components.
        std::string hdb;
        int ts;
        SplitKey(spos->first, hdb, ts);

        // If the name is a file that we're deleting then remove the SIL.
        if(hdb == fullName)
        {
            debug4 << "\tDeleted SIL for " << hdb.c_str() << endl;
            delete spos->second;
            fileSIL.erase(spos++);
        }
        else
            ++spos;
    }

    // Remove the correlation
    if(databaseCorrelationList->RemoveCorrelation(fullName))
    {
        debug4 << "\tDeleted database correlation for " << fullName.c_str() << endl;
        databaseCorrelationList->Notify();
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
//   db   : The database to close. If it's not provided then the mdserver
//          closes its current database.
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

void
ViewerFileServer::CloseFile(const std::string &host, const std::string &db)
{
    ServerMap::iterator pos;
    if((pos = servers.find(host)) != servers.end()) 
    {
        TRY
        {
            pos->second->proxy->CloseDatabase(db);
        }
        CATCH(LostConnectionException)
        {
            CloseServer(host, false);
        }
        ENDTRY
    }
}

// ****************************************************************************
// Method: ViewerFileServer::GetDatabaseCorrelationList
//
// Purpose: 
//   Returns the list of database correlations.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 23 17:15:45 PST 2004
//
// Modifications:
//   
// ****************************************************************************

DatabaseCorrelationList *
ViewerFileServer::GetDatabaseCorrelationList()
{
    return databaseCorrelationList;
}

// ****************************************************************************
// Method: ViewerFileServer::CreateDatabaseCorrelation
//
// Purpose: 
//   Creates and returns a database correlation that contains the specified
//   databases.
//
// Arguments:
//   name    : The name of the new correlation.
//   dbs     : The databases to include in the correlation.
//   method  : The correlation method.
//   nStates : The number of states (optional)
//
// Returns:    A pointer to a new database correlation or 0 if one could
//             not be created.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 22:58:13 PST 2004
//
// Modifications:
//   Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//   Added logic to decide if we need to force reading of all cycles
//   and times when getting meta data
//
// ****************************************************************************

DatabaseCorrelation *
ViewerFileServer::CreateDatabaseCorrelation(const std::string &name,
    const stringVector &dbs, int method, int nStates)
{
    if(dbs.size() < 1)
    {
        Error("VisIt cannot create a database correlation that does "
              "not use any databases.");
        return 0;
    }

    //
    // Create a new correlation and set its basic attributes.
    //
    DatabaseCorrelation *correlation = new DatabaseCorrelation;
    correlation->SetName(name);
    DatabaseCorrelation::CorrelationMethod m =
        (DatabaseCorrelation::CorrelationMethod)method;
    correlation->SetMethod(m);
    if(nStates != -1 &&
       (m == DatabaseCorrelation::IndexForIndexCorrelation ||
        m == DatabaseCorrelation::StretchedIndexCorrelation))
    {
        correlation->SetNumStates(nStates);
    }

    // Add the different databases to the correlation.
    char msg[200];
    for(int i = 0; i < dbs.size(); ++i)
    {
        //
        // Split the database name into host and database components
        // and expand it too.
        //
        std::string host, db;
        std::string correlationDB(dbs[i]);
        ExpandDatabaseName(correlationDB, host, db);

        //
        // Get the metadata for the database.
        //
        const avtDatabaseMetaData *md = GetMetaData(host, db);

        //
        // We might need to re-acqure metadata if we are doing
        // time or cycle correlations and times or cycles are not
        // all accurate and valid
        //
        if(md)
        {
            const bool forceReadAllCyclesAndTimes = true;

            if ((m == DatabaseCorrelation::TimeCorrelation &&
                 md->AreAllTimesAccurateAndValid() == false) ||
                (m == DatabaseCorrelation::CycleCorrelation &&
                 md->AreAllCyclesAccurateAndValid() == false))
                md = GetMetaData(host, db, forceReadAllCyclesAndTimes);
        }

        if(md)
        {
            //
            // Issue warning messages if we're doing time or cycle
            // correlations and the metadata cannot be trusted.
            //
            if(m == DatabaseCorrelation::TimeCorrelation)
            {
                bool accurate = true;
                for(int j = 0; j < md->GetNumStates() && accurate; ++j)
                    accurate &= md->IsTimeAccurate(j);

                if(!accurate)
                {
                    SNPRINTF(msg, 200, "The times for %s may not be "
                             "accurate so the new correlation %s might "
                             "not work as expected.", correlationDB.c_str(),
                             name.c_str());
                    Warning(msg);
                }
            }
            else if(m == DatabaseCorrelation::CycleCorrelation)
            {
                bool accurate = true;
                for(int j = 0; j < md->GetNumStates() && accurate; ++j)
                    accurate &= md->IsCycleAccurate(j);

                if(!accurate)
                {
                    SNPRINTF(msg, 200, "The cycles for %s may not be "
                             "accurate so the new correlation %s might "
                             "not work as expected.", correlationDB.c_str(),
                             name.c_str());
                    Warning(msg);
                }
            }

            //
            // Add the database to the new correlation.
            //
            correlation->AddDatabase(correlationDB, md->GetNumStates(),
                md->GetTimes(), md->GetCycles()); 
        }
        else
        {
            delete correlation; correlation = 0;
            SNPRINTF(msg, 200, "VisIt could not retrieve metadata "
                     "for %s so the correlation %s could not be "
                     "created.", correlationDB.c_str(),
                     name.c_str());
            Error(msg);
            break;
        }
    }

    return correlation;
}

// ****************************************************************************
// Method: ViewerFileServer::UpdateDatabaseCorrelation
//
// Purpose: 
//   Updates a the named database correlation by recreating it using its
//   input databases and storing the result over the old database correlation.
//
// Arguments:
//   cName : The name of the database correlation to update.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 27 10:36:35 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerFileServer::UpdateDatabaseCorrelation(const std::string &cName)
{
    DatabaseCorrelation *c = databaseCorrelationList->FindCorrelation(cName);
    if(c != 0)
    {
        DatabaseCorrelation *replacementCorrelation = CreateDatabaseCorrelation(
            c->GetName(), c->GetDatabaseNames(), (int)c->GetMethod());
        if(replacementCorrelation != 0)
        {
            // Copy over the old database correlation.
            *c = *replacementCorrelation;
        }
    }
}

// ****************************************************************************
// Method: ViewerFileServer::GetMostSuitableCorrelation
//
// Purpose: 
//   Returns a pointer to the correlation that most matches the list of 
//   databases.
//
// Arguments:
//   dbs : The list of databases for which we want a correlation.
//
// Returns:    A pointer to the most suitable correlation or 0 if there is
//             no suitable correlation.
//
// Note:       This method will not return a pointer to a trivial correlation
//             unless that is the only database in the dbs list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 16 08:57:49 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

DatabaseCorrelation *
ViewerFileServer::GetMostSuitableCorrelation(const stringVector &dbs) const
{
    //
    // Score the correlations as to how many of the input databases they
    // contain.
    //
    StringIntMap scores;
    for(int i = 0; i < databaseCorrelationList->GetNumDatabaseCorrelations();
        ++i)
    {
        const DatabaseCorrelation &c = databaseCorrelationList->
            GetDatabaseCorrelation(i);
        scores[c.GetName()] = 0;
        for(int j = 0; j < dbs.size(); ++j)
        {
            if(c.UsesDatabase(dbs[j]))
                ++scores[c.GetName()];
        }
    }

#if 0
    debug3 << "Scores: " << endl;
    for(StringIntMap::const_iterator sIt = scores.begin(); sIt != scores.end(); ++sIt)
        debug3 << "\t" << sIt->first << ", score= " << sIt->second << endl;
#endif

    //
    // Look for any databases with a score of dbs.size() and then go down
    // to the correlation with the next highest score.
    //
    std::string correlationName;
    int score = 0;
    for(int desiredScore = dbs.size(); desiredScore > 1 && score == 0;
        --desiredScore)
    {
        for(StringIntMap::const_iterator pos = scores.begin();
            pos != scores.end(); ++pos)
        {
            if(pos->second == desiredScore)
            {
                score = desiredScore;
                correlationName = pos->first;
                break;
            }
        }
    }

    //
    // If the score matches the number of databases then return that
    // correlation. It's okay to return that correlation if score > 1
    // since it means that it is not a trivial correlation.
    //
    DatabaseCorrelation *retval = 0;
    if(dbs.size() == score || score > 1)
        retval = databaseCorrelationList->FindCorrelation(correlationName);

    return retval;
}

// ****************************************************************************
// Method: ViewerFileServer::CreateNewCorrelationName
//
// Purpose: 
//   Gets the next correlation name in the series.
//
// Returns:    The next automatically generated name for a database correlation.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 16 08:56:09 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

std::string
ViewerFileServer::CreateNewCorrelationName() const
{
    int index = 1;
    char newName[100];
    do
    {
        SNPRINTF(newName, 100, "Correlation%02d", index);
    } while(databaseCorrelationList->FindCorrelation(newName) != 0);

    return std::string(newName);
}

// ****************************************************************************
// Method: ViewerFileServer::PreviouslyDeclinedCorrelationCreation
//
// Purpose: 
//   Returns whether the user previously declined to create a database 
//   correlation for a specified set of files.
//
// Arguments:
//   dbs : The list of files that are in the correlation.
//
// Returns:    True if the user did not want to create a correlation; false
//             if they never declined correlation creation.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 13 23:27:53 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerFileServer::PreviouslyDeclinedCorrelationCreation(
    const stringVector &dbs) const
{
    int index = 0;
    for(int fileSet = 0; fileSet < declinedFilesLength.size();
        ++fileSet)
    {
        if(declinedFilesLength[fileSet] == dbs.size())
        {
            bool same = true;
            for(int i = 0; i < declinedFilesLength[fileSet] && same; ++i, ++index)
                same &= (std::find(dbs.begin(), dbs.end(), declinedFiles[index])
                         != dbs.end());

            if(same)
                return true;
        }
    }

    return false;
}

// ****************************************************************************
// Method: ViewerFileServer::DeclineCorrelationCreation
//
// Purpose: 
//   Tell VisIt that it should not allow automatic correlations containing
//   the files in the passed in list of files.
//
// Arguments:
//   dbs : The list of files.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 13 23:29:32 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerFileServer::DeclineCorrelationCreation(const stringVector &dbs)
{
    if(dbs.size() > 0)
    {
        for(int i = 0; i < dbs.size(); ++i)
            declinedFiles.push_back(dbs[i]);
        declinedFilesLength.push_back(dbs.size());
    }
}

// ****************************************************************************
// Method: ViewerFileServer::IsDatabase
//
// Purpose: 
//   Returns whether or not the name is an open database.
//
// Arguments:
//   fullname : The whole host+database filename.
//
// Returns:    True if the fullname is an open file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 23:43:20 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerFileServer::IsDatabase(const std::string &fullname) const
{
    stringVector f(GetOpenDatabases());
    return std::find(f.begin(), f.end(), fullname) != f.end();
}

// ****************************************************************************
// Method: ViewerFileServer::SplitHostDatabase
//
// Purpose: 
//   Splits a hostDB into host and database names.
//
// Arguments:
//   hostDB : The entire host and db name.
//   host   : The host that was split out of the hostDB.
//   db     : The db that was split out of the hostDB.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 25 15:40:48 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerFileServer::SplitHostDatabase(const std::string &hostDB, 
    std::string &host, std::string &db)
{
    std::string hdb(hostDB);
    //
    // If we found what looks like drive punctuation (on Windows), replace
    // the : in the drive with something else so we can still check
    // for a host without getting it wrong.
    //
    bool foundDrive = false;
    std::string::size_type driveColon = hdb.find(":\\");
    if(driveColon != std::string::npos)
    {
        hdb.replace(driveColon, 1, "]");
        foundDrive = true;
    }

    // Look for the host colon.
    std::string::size_type hostColon = hdb.find(':');

    //
    // Now that the host colon was looked for, replace the drive if
    // we found one previously.
    //
    if(foundDrive)
        hdb.replace(driveColon, 1, ":");

    //
    // If the database string doesn't have a ':' in it then assume that
    // the host name is "localhost" and the database name is the entire
    // string.
    //
    if (hostColon == std::string::npos)
    {
        host = "localhost";
        db = hdb;
    }
    else
    {
        //
        // If the database string does have a ':' in it then the part before
        // it is the host name and the part after it is the database name.
        //
        host = hdb.substr(0, hostColon);
        db = hdb.substr(hostColon + 1);
    }
}

// ****************************************************************************
// Method: ViewerFileServer::ComposeDatabaseName
//
// Purpose: 
//   Composes a database name from host and file names.
//
// Arguments:
//   host : The name of the host where the database is stored.
//   db   : The name of the database.
//
// Returns:    The host + database name.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 25 15:05:40 PST 2004
//
// Modifications:
//   
// ****************************************************************************

std::string
ViewerFileServer::ComposeDatabaseName(const std::string &host,
    const std::string &db)
{
    std::string h(host);

    if(h == "")
        h = "localhost";

    return h + ":" + db;
}

// ****************************************************************************
// Method: ViewerFileServer::SplitKey
//
// Purpose: 
//   Splits a key into name and time state components.
//
// Arguments:
//   name : The key to split.
//   hdb  : The host + database part of the key.
//   ts   : The ts value of the key or -1 if no ts value was in the key.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 1 09:23:15 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerFileServer::SplitKey(const std::string &key, std::string &hdb,
    int &ts) const
{
    hdb = key;
    ts = -1;

    std::string::size_type drive = hdb.find(":\\");
        
    // If we found what looks like drive punctuation (on Windows), replace
    // the : in the drive with something else so we can still check
    // for a state suffix without getting it wrong.
    if(drive != std::string::npos)
        hdb.replace(drive, 1, "]");

    // Search for colon from both ends of the string so we can see if
    // there is more than one. If so, then asume that we have a state
    // suffix.
    std::string::size_type lsearch = hdb.find(":");
    std::string::size_type rsearch = hdb.rfind(":");

    // If we replaced the drive punctuation, restore it now.
    if(drive != std::string::npos)
        hdb.replace(drive, 1, ":");

    // The key has the time state in it. Remove it.
    if(lsearch != rsearch)
    {
        hdb = hdb.substr(0, rsearch);
        std::string tsString = key.substr(rsearch+1, key.size() - rsearch-1);
        int tmp;
        if(sscanf(tsString.c_str(), "%d", &tmp) == 1 && tmp >= 0)
            ts = tmp;
    }
}

// ****************************************************************************
// Method: ViewerFileServer::GetOpenDatabases
//
// Purpose: 
//   Returns a vector of sorted open database names.
//
// Returns:    A vector containing the open file names.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 23:42:35 PST 2004
//
// Modifications:
//   
// ****************************************************************************

stringVector
ViewerFileServer::GetOpenDatabases() const
{
    stringVector openFiles;
    for(FileMetaDataMap::const_iterator pos = fileMetaData.begin();
        pos != fileMetaData.end();
        ++pos)
    {
        // Split the metadata key into name and ts components so we only
        // add a file once in the case that it is time-varying and there are
        // multiple cached metadata objects for different time states.
        std::string name;
        int         ts;
        SplitKey(pos->first, name, ts);

        // If the name is not already in the list, add it to the list.
        if(std::find(openFiles.begin(), openFiles.end(), name) == openFiles.end())
            openFiles.push_back(name);
    }
 
    // Sort the list of sources.
    std::sort(openFiles.begin(), openFiles.end());

    return openFiles;
}

// ****************************************************************************
// Method: ViewerFileServer::CreateNode
//
// Purpose: 
//   Tells the database correlation list to save its settings to the data
//   node that was passed in.
//
// Arguments:
//   parentNode : the node on which to save the settings.
//   detailed   : Whether we're saving a session file.
//
// Note:       Correlations are only saved in session files.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 25 16:50:05 PST 2004
//
// Modifications:
//   Brad Whitlock, Thu Nov 9 16:36:16 PST 2006
//   I added code to make sure that database correlations use the source
//   ids instead of databases when their information is saved. We will fill
//   in the blanks for the correlation when we read it back in.
//
// ****************************************************************************

void
ViewerFileServer::CreateNode(DataNode *parentNode, 
    const std::map<std::string, std::string> &dbToSource, bool detailed)
{
    // Create a copy of the database correlation list.
    DatabaseCorrelationList dbcl(*databaseCorrelationList);
    dbcl.ClearDatabaseCorrelations();
    for(int i = 0; 
        i < databaseCorrelationList->GetNumDatabaseCorrelations(); ++i)
    {
        const DatabaseCorrelation &corr = 
            databaseCorrelationList->GetDatabaseCorrelation(i);
        // Let's only save out correlations that have more than 1 db.
        if(corr.GetNumDatabases() > 1)
        {
            // Map database names to source names.
            const stringVector &dbNames = corr.GetDatabaseNames();
            stringVector sourceIds;
            for(int j = 0; j < dbNames.size(); ++j)
            {
                std::map<std::string, std::string>::const_iterator pos =
                    dbToSource.find(dbNames[j]);
                if(pos == dbToSource.end())
                    sourceIds.push_back(dbNames[j]);
                else
                    sourceIds.push_back(pos->second);
            }

            // Create a copy of the correlation but override its database
            // names with source ids. Also, note that we're not using a
            // copy constructor because we want most of the correlation
            // information to be absent so we can repopulate it on session read
            // in case a new database is chosen.
            DatabaseCorrelation modCorr;                       
            if(corr.GetMethod() != DatabaseCorrelation::UserDefinedCorrelation)
            {
                modCorr.SetName(corr.GetName());
                modCorr.SetNumStates(corr.GetNumStates());
                modCorr.SetMethod(corr.GetMethod());
            }
            else
            {
               // A user-defined correlation should be saved mostly as-is.
               modCorr = corr;
            }
            modCorr.SetDatabaseNames(sourceIds);

            // Add the modified correlation to the list.
            dbcl.AddDatabaseCorrelation(modCorr);
        }
    }

    // Add the database correlation list information to the session.
    dbcl.CreateNode(parentNode, detailed, false);
}

// ****************************************************************************
// Method: ViewerFileServer::SetFromNode
//
// Purpose: 
//   Initializes the database correlation list.
//
// Arguments:
//   parentNode : The node on which to look for attribute nodes.
//   sourceToDB : The map of source ids -> database names.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 25 16:49:01 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri Nov 10 13:44:40 PST 2006
//   I rewrote the method to handle creation of database correlations 
//   using replacement sources.
//
// ****************************************************************************

void
ViewerFileServer::SetFromNode(DataNode *parentNode,
    const std::map<std::string,std::string> &sourceToDB)
{
    DataNode *cLNode = parentNode->GetNode("DatabaseCorrelationList");
    if(cLNode != 0)
    {
        // Read the database correlation list into a temporary.
        DatabaseCorrelationList dbcl;
        dbcl.SetFromNode(parentNode);

        // Since the database correlations in the temporary may
        // not have complete information, let's recreate the 
        // database correlations.
        databaseCorrelationList->ClearDatabaseCorrelations();
        for(int i = 0; i < dbcl.GetNumDatabaseCorrelations(); ++i)
        {
            const DatabaseCorrelation &corr = dbcl[i];
            if(corr.GetMethod() != DatabaseCorrelation::UserDefinedCorrelation)
            {
                // Translate the source names in the database correlation into
                // database names using the sourceToDB map.
                stringVector dbNames;
                const stringVector &sourceNames = corr.GetDatabaseNames();
                for(int j = 0; j < sourceNames.size(); ++j)
                {
                    std::map<std::string,std::string>::const_iterator pos =
                        sourceToDB.find(sourceNames[j]);
                    if(pos != sourceToDB.end())
                        dbNames.push_back(pos->second);
                    else
                        dbNames.push_back(sourceNames[j]);
                }

                // Create a new database correlation based on the inputs for the
                // one that we read from the session file.
                DatabaseCorrelation *newCorr = CreateDatabaseCorrelation(
                    corr.GetName(), dbNames, corr.GetMethod(), -1);

                // If we were able to create a database correlation then add it
                // to the database correlation list.
                if(newCorr != 0)
                {
                    databaseCorrelationList->AddDatabaseCorrelation(*newCorr);
                    delete newCorr;
                }
            }
            else
            {
                // We read in a user-defined database correlation. 
                // Don't mess with it.
                databaseCorrelationList->AddDatabaseCorrelation(corr);
            }
        }
              
        //
        // Now that the correlation list contains valid correlations,
        // notify clients.
        //
        databaseCorrelationList->Notify();
    }
}

// ****************************************************************************
// Method: ViewerFileServer::DetermineVarType
//
// Purpose: 
//   Determines the AVT variable type for the specified variable.
//
// Arguments:
//   host  : The host where the file resides.
//   db    : The database.
//   var   : The variable that we want.
//   state : The state at which we want information about the variable.
//
// Returns:    
//
// Note:       This code was moved out of ViewerPlot and ViewerQueryManager
//             since the code in those two classes was identical. Normally
//             code like this would not be here but it seems like a fairly
//             convenient place since we have no base class for viewer objects.
//
//             We must pass the state because of variables that can exist
//             at some time states and not at others.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 26 10:18:45 PDT 2004
//
// Modifications:
//   Brad Whitlock, Fri Feb 18 10:00:35 PDT 2005
//   Changed the code so it uses an expression list that is correct for the
//   specified database instead of just assuming that ParsingExprList will
//   contain the right expression list. Note that someday when we have support
//   for expressions from multiple databases, and ParsingExprList contains
//   the list of expressions from all open sources then it will be okay to
//   use ParsingExprList again.
//
// ****************************************************************************

avtVarType
ViewerFileServer::DetermineVarType(const std::string &host,
    const std::string &db, const std::string &var, int state)
{
    avtVarType retval = AVT_UNKNOWN_TYPE;

    // Check if the variable is an expression.
    ExpressionList expressionList;
    GetAllExpressions(expressionList, host, db, state);
    Expression *exp = expressionList[var.c_str()];

    if (exp != NULL)
    {
        retval = ParsingExprList::GetAVTType(exp->GetType());
    }
    else
    {
        const avtDatabaseMetaData *md = GetMetaDataForState(host, db, state);
        if (md != 0)
        {
            // 
            // Get the type for the variable.
            //
            TRY
            {
                avtDatabaseMetaData *ncmd = (avtDatabaseMetaData *) md;
                retval = ncmd->DetermineVarType(var);
            }
            CATCH(VisItException)
            {
                std::string message("VisIt was unable to determine "
                    "the variable type for ");
                message += host; 
                message += ":";
                message += db;
                message += "'s ";
                message += var;
                message += " variable.";
                Error(message.c_str());
                debug1 << "ViewerFileServer::DetermineVarType: Caught an "
                          "exception!" << endl;
                retval = AVT_UNKNOWN_TYPE;
            }
            ENDTRY
        }
    }

    return retval;
}


// ****************************************************************************
// Method: ViewerFileServer::DetermineRealVarType
//
// Purpose: 
//   Determines the AVT variable type for the 'real' variable behind
//   the specified variable.
//
// Arguments:
//   host  : The host where the file resides.
//   db    : The database.
//   var   : The variable that we want.
//   state : The state at which we want information about the variable.
//
// Returns:    
//
// Programmer: Kathleen Bonnell 
// Creation:   July 5, 2005 
//
// Modifications:
//
// ****************************************************************************

avtVarType
ViewerFileServer::DetermineRealVarType(const std::string &host,
    const std::string &db, const std::string &var, int state)
{
    avtVarType retval = AVT_UNKNOWN_TYPE;

    // Check if the variable is an expression.
    ExpressionList expressionList;
    GetAllExpressions(expressionList, host, db, state);
    Expression *exp = expressionList[var.c_str()];
    string realVar = var;
    if (exp != NULL)
    {
        string realVar = ParsingExprList::GetRealVariable(var);
        if (realVar != var)
            exp = NULL;
    }
    if (exp != NULL)
    {
        retval = ParsingExprList::GetAVTType(exp->GetType());
    }
    else
    {
        const avtDatabaseMetaData *md = GetMetaDataForState(host, db, state);
        if (md != 0)
        {
            // 
            // Get the type for the variable.
            //
            TRY
            {
                avtDatabaseMetaData *ncmd = (avtDatabaseMetaData *) md;
                retval = ncmd->DetermineVarType(realVar);
            }
            CATCH(VisItException)
            {
                std::string message("VisIt was unable to determine "
                    "the real variable type for ");
                message += host; 
                message += ":";
                message += db;
                message += "'s ";
                message += var;
                message += " variable.";
                Error(message.c_str());
                debug1 << "ViewerFileServer::DetermineVarType: Caught an "
                          "exception!" << endl;
                retval = AVT_UNKNOWN_TYPE;
            }
            ENDTRY
        }
    }

    return retval;
}


// ****************************************************************************
// Method: ViewerFileServer::GetUserExpressions
//
// Purpose: 
//   Gets the expressions that were defined by the user.
//
// Arguments:
//   newList : The return list for the user-defined expressions.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 18 09:44:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerFileServer::GetUserExpressions(ExpressionList &newList)
{
    ExpressionList *exprList = ParsingExprList::Instance()->GetList();

    //
    // Create a new expression list that contains all of the expressions
    // from the main expression list that are not expressions that come
    // from databases.
    //
    for(int i = 0; i < exprList->GetNumExpressions(); ++i)
    {
        const Expression &expr = exprList->GetExpression(i);
        if(!expr.GetFromDB())
            newList.AddExpression(expr);
    }
}

// ****************************************************************************
// Method: ViewerFileServer::GetDatabaseExpressions
//
// Purpose: 
//   Gets the database expressions for the specified database.
//
// Arguments:
//   newList : The return list for the database expressions.
//   host    : The host where the database is located.
//   db      : The database name.
//   state   : The database time state.
//
// Returns:    The list of database expressions.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 18 09:45:27 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerFileServer::GetDatabaseExpressions(ExpressionList &newList,
    const std::string &host, const std::string &db, int state)
{
    // Store all of the specified database's expressions in the 
    // new list.
    if(host.size() > 0 && db.size() > 0)
    {
        const avtDatabaseMetaData *md = GetMetaDataForState(host, db, state);
        if (md != 0)
        {
            // Add the expressions for the database.
            for (int j = 0 ; j < md->GetNumberOfExpressions(); ++j)
                newList.AddExpression(*(md->GetExpression(j)));
        }
    }
}

// ****************************************************************************
// Method: ViewerFileServer::GetAllExpressions
//
// Purpose: 
//   Gets user-defined expressions and the expressions for the specified
//   database.
//
// Arguments:
//   newList : The return list for the database expressions.
//   host    : The host where the database is located.
//   db      : The database name.
//   state   : The database time state.
//
// Returns:    A list of expressions.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 18 09:46:48 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerFileServer::GetAllExpressions(ExpressionList &newList,
    const std::string &host, const std::string &db, int state)
{
    // Store all of the expressions that are not database expressions
    // in the new list.
    GetUserExpressions(newList);
    GetDatabaseExpressions(newList, host, db, state);
}

// ****************************************************************************
//  Method:  ViewerFileServer::SetSimulationMetaData
//
//  Purpose:
//    Store updated metadata for a simulation.  This happens when
//    (1) we first start a simulation, it has complete information
//    compared to the mdserver's metadata, and (2) when the simulation
//    changes time steps, it sends new metadata.
//
//  Arguments:
//    host,filename   the identifiers for the simulation
//    md              the new metadata
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon Apr  4 17:11:27 PDT 2005
//    Simulation commands are part of the simulation info, but unlike the
//    other siminfo stuff, the new ones should be accepted.
//
//    Jeremy Meredith, Thu Apr 28 17:51:52 PDT 2005
//    It wound up being simpler and easier run to just keep only what
//    needed keeping.
//
// ****************************************************************************
void
ViewerFileServer::SetSimulationMetaData(const std::string &host,
                                        const std::string &filename,
                                        const avtDatabaseMetaData &md)
{
    std::string dbName(ComposeDatabaseName(host, filename));
    for(FileMetaDataMap::const_iterator pos = fileMetaData.begin();
        pos != fileMetaData.end();
        ++pos)
    {
        // Split the metadata key into name and ts components so we only
        // add a file once in the case that it is time-varying and there are
        // multiple cached metadata objects for different time states.
        std::string name;
        int         ts;
        SplitKey(pos->first, name, ts);

        if(name == dbName)
        {
            string host = pos->second->GetSimInfo().GetHost();
            int    port = pos->second->GetSimInfo().GetPort();
            string key  = pos->second->GetSimInfo().GetSecurityKey();
            vector<string> onames = pos->second->GetSimInfo().GetOtherNames();
            vector<string> ovalues= pos->second->GetSimInfo().GetOtherValues();

            *(pos->second) = md;
            pos->second->GetSimInfo().SetHost(host);
            pos->second->GetSimInfo().SetPort(port);
            pos->second->GetSimInfo().SetSecurityKey(key);
            pos->second->GetSimInfo().SetOtherNames(onames);
            pos->second->GetSimInfo().SetOtherValues(ovalues);
        }
    }
}

// ****************************************************************************
//  Method:  ViewerFileServer::SetSimulationSILAtts
//
//  Purpose:
//    Store updated metadata for a simulation.  This happens when
//    (1) we first start a simulation, it has complete information
//    compared to the mdserver's metadata, and possibly (2) when the
//    simulation changes time steps, it sends new metadata.
//
//  Arguments:
//    host,filename   the identifiers for the simulation
//    silAtts         the new SIL attributes
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
// ****************************************************************************
void
ViewerFileServer::SetSimulationSILAtts(const std::string &host,
                                       const std::string &filename,
                                       const SILAttributes &silAtts)
{
    avtSIL *sil = new avtSIL(silAtts);
    std::string dbName(ComposeDatabaseName(host, filename));
    for(FileSILMap::const_iterator pos = fileSIL.begin();
        pos != fileSIL.end();
        ++pos)
    {
        // Split the metadata key into name and ts components so we only
        // add a file once in the case that it is time-varying and there are
        // multiple cached metadata objects for different time states.
        std::string name;
        int         ts;
        SplitKey(pos->first, name, ts);

        if(name == dbName)
        {
            *(pos->second) = *sil;
        }
    }
    delete sil;
}

// ****************************************************************************
//  Method:  ViewerFileServer::GetPluginErrors
//
//  Purpose:
//    Gets the errors a particular mdserver found during plugin initialization.
//
//  Arguments:
//    host       the host where the appropriate mdserver is running
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  7, 2005
//
// ****************************************************************************
std::string
ViewerFileServer::GetPluginErrors(const std::string &host)
{
    if(servers.find(host) != servers.end())
    {
        return servers[host]->proxy->GetPluginErrors();
    }
    return "";
}

//
// ViewerFileServer::ServerInfo methods
//

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
