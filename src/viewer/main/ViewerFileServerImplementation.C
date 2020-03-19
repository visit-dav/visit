// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <visit-config.h>
#include <ViewerFileServerImplementation.h>

#include <BadHostException.h>
#include <CancelledConnectException.h>
#include <ChangeUsernameException.h>
#include <CouldNotConnectException.h>
#include <DBPluginInfoAttributes.h>
#include <DataNode.h>
#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <DebugStream.h>
#include <FileFunctions.h>
#include <FileOpenOptions.h>
#include <GetMetaDataException.h>
#include <GlobalAttributes.h>
#include <HostProfileList.h>
#include <IncompatibleSecurityTokenException.h>
#include <IncompatibleVersionException.h>
#include <LauncherProxy.h>
#include <LostConnectionException.h>
#include <MDServerManager.h>
#include <MDServerProxy.h>
#include <MachineProfile.h>
#include <ParentProcess.h>
#include <ParsingExprList.h>
#include <SILAttributes.h>
#include <TimingsManager.h>
#include <Utility.h>

#include <avtDatabaseMetaData.h>
#include <avtExpressionTypeConversions.h>
#include <avtSIL.h>
#include <avtSimulationInformation.h>

#include <ViewerChangeUsername.h>
#include <ViewerConnectionProgress.h>
#include <ViewerFactory.h>
#include <ViewerMessaging.h>
#include <ViewerProperties.h>
#include <ViewerState.h>
#include <ViewerText.h>

#include <stdlib.h>
#include <algorithm>
#include <string>
#include <vector>

// A static pointer to the one and only instance of ViewerFileServerImplementation
ViewerFileServerImplementation *ViewerFileServerImplementation::instance = NULL;

const int ViewerFileServerImplementation::ANY_STATE = -1;

// ****************************************************************************
// Method: ViewerFileServerImplementation::ViewerFileServerImplementation
//
// Purpose: 
//   Constructor for the ViewerFileServerImplementation class.
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
//   Added GetViewerState()->GetDBPluginInfoAttributes().
//
//   Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//   Added initialization of tryHarderCyclesTimes
//
//   Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//   Replaced tryHarderCyclesTimes and treatAllDBsAsTimeVarying with
//   static functions calling VWM
//
//   Jeremy Meredith, Wed Jan 23 16:25:45 EST 2008
//   Store the current default file opening options.
//
//    Kathleen Bonnell, Thu Aug 14 16:13:41 PDT 2008 
//    Added shouldSendFileOpenOptions.
//
// ****************************************************************************

ViewerFileServerImplementation::ViewerFileServerImplementation() : ViewerServerManager(),
    fileMetaData(), fileSIL()
{
    shouldSendFileOpenOptions = false;
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::ViewerFileServerImplementation
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
//
//   Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//   Replaced tryHarderCyclesTimes and treatAllDBsAsTimeVarying with
//   static functions calling VWM
//   
//   Kathleen Bonnell, Thu Aug 14 16:13:41 PDT 2008 
//   Added shouldSendFileOpenOptions.
//
// ****************************************************************************

ViewerFileServerImplementation::ViewerFileServerImplementation(const ViewerFileServerImplementation &) : ViewerServerManager()
{
    shouldSendFileOpenOptions = false;
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::~ViewerFileServerImplementation
//
// Purpose:
//   Destructor for the ViewerFileServerImplementation class.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 21 12:25:37 PDT 2000
//
// Modifications:
//
// ****************************************************************************

ViewerFileServerImplementation::~ViewerFileServerImplementation()
{
    CloseServers();
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::CloseServers
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
//   Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL.
//
//   Jeremy Meredith, Fri Jan 29 10:27:29 EST 2010
//   Added knowledge of what plugin was used to open a file.  Clear it here.
//
//   Eric Brugger, Tue Nov 26 12:53:36 PST 2019
//   Added code to delete pos->second->proxy before deleting pos->second to
//   eliminate a memory leak.
//
// ****************************************************************************

void
ViewerFileServerImplementation::CloseServers()
{
    // Iterate through the server list, close and destroy each
    // server and remove the server from the map.
    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
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
        CATCHALL
        {
            debug1 << "Caught an exception while closing the server."
                   << endl;
        }
        ENDTRY

        // Delete the server
        delete pos->second->proxy;
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
    filePlugins.clear();
    FileSILMap::iterator fpos2;
    for (fpos2 = fileSIL.begin() ; fpos2 != fileSIL.end() ; fpos2++)
    {
        // Delete the SIL
        delete fpos2->second;
    }
    fileSIL.clear();      
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::Instance
//
// Purpose: 
//   Returns a pointer to the only ViewerFileServerImplementation object that can be created.
//
// Returns:    A pointer to the only ViewerFileServerImplementation object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 21 12:27:31 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

ViewerFileServerImplementation *
ViewerFileServerImplementation::Instance()
{
    // If the sole instance hasn't been instantiated, then instantiate it.
    if(instance == NULL)
    {
        instance = new ViewerFileServerImplementation;
    }

    return instance;
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::MetaDataIsInvariant
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
//   Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//   Added logic for treatAllDBsAsTimeVarying
// ****************************************************************************

bool
ViewerFileServerImplementation::MetaDataIsInvariant(const std::string &host, 
    const std::string &filename, int state)
{
    //
    // Get the metadata for the specified state and then return whether it
    // is invariant.
    //
    if (GetViewerState()->GetGlobalAttributes()->GetTreatAllDBsAsTimeVarying())
        return false;
    const avtDatabaseMetaData *md = GetMetaDataForState(host, filename, state);
    return (md != 0) ? (!md->GetMustRepopulateOnStateChange()) : true;
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::GetMetaData
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
ViewerFileServerImplementation::GetMetaData(const std::string &host, 
                              const std::string &db,
                              const bool forceReadAllCyclesAndTimes)
{
    //
    // We don't care about the time state so look for any cached metadata
    // having the right host and database name and return it.
    //
    std::string dbName(FileFunctions::ComposeDatabaseName(host, db));
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

        bool cyclesAndTimesAreGood = 
                 pos->second->AreAllTimesAccurateAndValid() &&
                 pos->second->AreAllCyclesAccurateAndValid();

        if (name == dbName && 
            (!forceReadAllCyclesAndTimes || cyclesAndTimesAreGood))
        {
            return pos->second;
        }
    }

    // if we're here because we've forced reading all cycles and times
    // then we can't use ANY_STATE because we are potentially ALSO making this
    // call on a database that is different from the one the mdserver thinks
    // is current. Our only choice is to use state zero
    int state = forceReadAllCyclesAndTimes ? 0 : ANY_STATE;

    return GetMetaDataHelper(host, db, state, forceReadAllCyclesAndTimes);

}

// ****************************************************************************
// Method: ViewerFileServerImplementation::GetMetaDataForState
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
//   Brad Whitlock, Wed Mar 14 20:33:03 PST 2007
//   Added forceReadAllCyclesAndTimes argument.
//
//   Brad Whitlock, Tue Apr 29 14:49:24 PDT 2008
//   Added tr().
//
// ****************************************************************************

const avtDatabaseMetaData *
ViewerFileServerImplementation::GetMetaDataForState(const std::string &host, 
    const std::string &db, int timeState,
    const std::string &forcedFileType)
{
    bool forceReadAllCyclesAndTimes = false;
    return GetMetaDataForState(host, db, timeState, 
                               forceReadAllCyclesAndTimes, forcedFileType);
}

const avtDatabaseMetaData *
ViewerFileServerImplementation::GetMetaDataForState(const std::string &host, 
    const std::string &db, int timeState, bool forceReadAllCyclesAndTimes,
    const std::string &forcedFileType)
{
    //
    // Make sure a valid state was passed.
    //
    if(timeState <= ANY_STATE)
    {
        GetViewerMessaging()->Error(
            TR("GetMetaDataForState called with ANY_STATE. That is "
               "not allowed so VisIt will instead use time state 0."));
        timeState = 0;
    }

    //
    // First check to see if the metadata is time-varying. If the metadata
    // does not vary over time then return the first metadata that we
    // find. If it does vary over time and that metadata is already cached,
    // return that metadata.
    //
    std::string dbName(FileFunctions::ComposeDatabaseName(host, db));
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
            if((!GetViewerState()->GetGlobalAttributes()->GetTreatAllDBsAsTimeVarying() && 
                !pos->second->GetMustRepopulateOnStateChange()) ||
               ts == timeState)
            {
                return pos->second;
            }
        }
    }

    return GetMetaDataHelper(host, db, timeState, forceReadAllCyclesAndTimes,
                             forcedFileType);
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::GetMetaDataHelper
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
//   Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//   Changed refernce to treatAllDBsAsTimeVarying as a function call to
//   the static method here. Likewise for TryHarderCyclesTimes.
//
//   Kathleen Bonnell, Tue Oct  9 16:59:39 PDT 2007 
//   Added GetCreateMeshQuality/TimeDerivativeExpressions to GetMetaData call. 
//
//   Hank Childs, Tue Dec 11 16:35:25 PST 2007
//   Added timings of getting meta-data.
//
//   Brad Whitlock, Tue Apr 29 14:52:33 PDT 2008
//   Support for internationalization.
//
//   Kathleen Bonnell, Thu Aug 14 16:13:41 PDT 2008 
//   Added call to SendFileOpenOptions after StartServer.
//
//   Hank Childs, Wed Dec  3 10:56:10 PST 2008
//   Fix typo in generating error message for opening bad file when running
//   client-server.
//
//   Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL.
//
//   Mark C. Miller, Mon Jun 22 15:16:05 PDT 2009
//   Replaced typo of '%`' with '%1' in in an arg to QObject::tr().
//
//   Jeremy Meredith, Fri Jan 29 10:27:29 EST 2010
//   Added knowledge of what plugin was used to open a file.  Use it to
//   set a forced file type if one is not already set.
//
//   Jeremy Meredith, Fri Jan 29 11:35:14 EST 2010
//   If opening the plugin fails, we want to make sure we don't continue
//   to force its usage.
//
// ****************************************************************************

const avtDatabaseMetaData *
ViewerFileServerImplementation::GetMetaDataHelper(const std::string &host, 
    const std::string &db, int timeState, bool forceReadAllCyclesAndTimes,
    const std::string &forcedFileType_)
{
    // Try and start a server if one does not exist.
    NoFaultStartServer(host, stringVector());

    // Initialize the return value.
    const avtDatabaseMetaData *retval = NULL;

    // If a server exists, get the metadata.
    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
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
                std::string key(FileFunctions::ComposeDatabaseName(host, db));
                std::string forcedFileType(forcedFileType_);

                // If we aren't told which type to use, override it
                // with any save plugin name.
                if (forcedFileType == "" && filePlugins.count(key) > 0)
                    forcedFileType = filePlugins[key];

                // Now forget which plugin was used to open it.
                // If it was somehow wrong, we want to let them pick another.
                // And if it was correct, it will get re-set momentarily....
                filePlugins.erase(key);

                int t0 = visitTimer->StartTimer();
                const avtDatabaseMetaData *md =
                    servers[host]->proxy->GetMDServerMethods()->GetMetaData(db, timeState,
                                        forceReadAllCyclesAndTimes ||
                                        GetViewerState()->GetGlobalAttributes()->GetTryHarderCyclesTimes(),
                                        forcedFileType,
                                        GetViewerState()->GetGlobalAttributes()->GetTreatAllDBsAsTimeVarying(),
                                        GetViewerState()->GetGlobalAttributes()->GetCreateMeshQualityExpressions(),
                                        GetViewerState()->GetGlobalAttributes()->GetCreateTimeDerivativeExpressions());
                visitTimer->StopTimer(t0, "Getting meta data from mdserver");

                if(md != NULL)
                {
                    // Cache the format we used for this file.
                    // (Don't add time state information to its name yet.)
                    filePlugins[key] = md->GetFileFormat();

                    //
                    // If the meta-data changes for each state, then cache
                    // the meta-data on a per state basis.  This is done by
                    // encoding the state into the name. Don't encode the state
                    // into the name though if we got it using ANY_STATEs
                    //
                    if ((md->GetMustRepopulateOnStateChange() ||
                         GetViewerState()->GetGlobalAttributes()->GetTreatAllDBsAsTimeVarying()) &&
                        timeState != ANY_STATE)
                    {
                        char timeStateString[20];
                        snprintf(timeStateString, 20, ":%d", timeState);
                        key += timeStateString;
                    }

                    //
                    // Add the metadata copy to the cache.
                    //
                    avtDatabaseMetaData *mdCopy = new avtDatabaseMetaData(*md);
                    fileMetaData[key] = mdCopy;

                    retval = mdCopy;
                }
            }
            CATCH2(GetMetaDataException, gmde)
            {
                ViewerText msg;
                if (host == "localhost")
                {
                    msg = TR("VisIt could not read from the file "
                             "\"%1\".\n\nThe generated error message"
                             " was:\n\n%2").
                          arg(db).
                          arg(gmde.Message());
                }
                else
                {
                    msg = TR("VisIt could not read from the file "
                             "\"%1\" on host %2.\n\nThe generated error message"
                             " was:\n\n%3").
                          arg(db).
                          arg(host).
                          arg(gmde.Message());
                }
                GetViewerMessaging()->Error(msg);
            }
            CATCH(LostConnectionException)
            {
                // Tell the GUI that the mdserver is dead.
                if(numAttempts == 0)
                {
                    GetViewerMessaging()->Warning(
                        TR("The metadata server running on host %1 has exited "
                           "abnormally. VisIt is trying to restart it.").
                        arg(host));
                }

                ++numAttempts;
                int numTries = 2;
                tryAgain = (numAttempts < numTries);
                if (numAttempts == numTries)
                {
                    GetViewerMessaging()->Error(
                        TR("VisIt was unable to open \"%1\"."
                           "  Each attempt to open it caused VisIt's "
                           "metadata server to crash.  This can occur when "
                           "the file is corrupted, or when the underlying "
                           "file format has changed and VisIt's readers "
                           "have not been updated yet, or when the reader "
                           "VisIt is using"
                           " for your file format is not robust.  Please "
                           "check whether the file is corrupted and, if "
                           "not, contact a VisIt developer.").
                        arg(db));
                }

                TRY
                {
                    stringVector startArgs(servers[host]->arguments);
                    CloseServer(host, false);
                    StartServer(host, startArgs);
                    SendFileOpenOptions(host);
                }
                CATCHALL
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
// Method: ViewerFileServerImplementation::GetSIL
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
ViewerFileServerImplementation::GetSIL(const std::string &host, 
    const std::string &db)
{
    //
    // We don't care about the time state so look for any cached metadata
    // having the right host and database name and return it.
    //
    std::string dbName(FileFunctions::ComposeDatabaseName(host, db));
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
// Method: ViewerFileServerImplementation::GetSILForState
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
//   Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//   Changed reference to treatAllDBsAsTimeVarying to a call to a static
//   function.
//
//   Brad Whitlock, Tue Apr 29 14:53:03 PDT 2008
//   Added tr().
//
// ****************************************************************************

const avtSIL *
ViewerFileServerImplementation::GetSILForState(const std::string &host, 
    const std::string &db, int timeState)
{
    //
    // Make sure a valid state was passed.
    //
    if(timeState < 0)
    {
        GetViewerMessaging()->Error(
            TR("GetSILForState called with ANY_STATE. That is "
               "not allowed so VisIt will instead use time state 0."));
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
    std::string dbName(FileFunctions::ComposeDatabaseName(host, db));
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
            if ((invariantMetaData || ts == timeState) && 
                !GetViewerState()->GetGlobalAttributes()->GetTreatAllDBsAsTimeVarying())
            {
                return pos->second;
            }
        }
    }

    return GetSILHelper(host, db, timeState);
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::GetSILHelper
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
//   Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//   Changed reference to treatAllDBsAsTimeVarying to a call to a static
//   function.
//
//   Brad Whitlock, Tue Apr 29 14:54:17 PDT 2008
//   Support for internationalization.
//
//   Kathleen Bonnell, Thu Aug 14 16:13:41 PDT 2008 
//   Added call to SendFileOpenOptions after StartServer.
//
//   Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL.
// ****************************************************************************

const avtSIL *
ViewerFileServerImplementation::GetSILHelper(const std::string &host, const std::string &db,
    const int timeState)
{
    // Try and start a server if one does not exist.
    NoFaultStartServer(host, stringVector());

    // Initialize the return value.
    const avtSIL *retval = NULL;

    // If a server exists, get the metadata.
    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
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
                std::string key(FileFunctions::ComposeDatabaseName(host, db));
                if(GetViewerState()->GetGlobalAttributes()->GetTreatAllDBsAsTimeVarying() ||
                    (timeState != ANY_STATE &&
                    !MetaDataIsInvariant(host, db, timeState)))
                {
                    char state_str[64];
                    snprintf(state_str, 64, ":%d", timeState);
                    key += state_str;
                }

                //
                // Get the SIL from the mdserver
                //
                const SILAttributes *atts =
                    servers[host]->proxy->GetMDServerMethods()->GetSIL(db, timeState);

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
                GetViewerMessaging()->Error(
                    TR("VisIt cannot read the SIL for the file "
                       "\"%1\" on host %2.\n\nThe metadata server returned "
                       "the following message:\n\n%3").
                    arg(db).
                    arg(host).
                    arg(gmde.Message()));
            }
            CATCH(LostConnectionException)
            {
                // Tell the GUI that the mdserver is dead.
                if(numAttempts == 0)
                {
                    GetViewerMessaging()->Warning(
                        TR("The metadata server running on host "
                           "%1 has exited abnormally. VisIt is trying to "
                           "restart it.").
                        arg(host));
                }

                ++numAttempts;
                tryAgain = (numAttempts < 2);

                TRY
                {
                    stringVector startArgs(servers[host]->arguments);
                    CloseServer(host, false);
                    StartServer(host, startArgs);
                    SendFileOpenOptions(host);
                }
                CATCHALL
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
// Method: ViewerFileServerImplementation::ExpandDatabaseName
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
ViewerFileServerImplementation::ExpandDatabaseName(std::string &hostDBName,
    std::string &host, std::string &db)
{
    // Split the host database name into host + db.
    FileFunctions::SplitHostDatabase(hostDBName, host, db);

    //
    // Expand the file name so it does not contain wildcards or
    // relatve paths.
    //
    db = ExpandedFileName(host, db);

    //
    // Compose the new filename.
    //
    hostDBName = FileFunctions::ComposeDatabaseName(host, db);
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::ExpandedFileName
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
//   Brad Whitlock, Tue Apr 29 14:55:30 PDT 2008
//   Support for internationalization.
//
//   Kathleen Bonnell, Thu Aug 14 16:13:41 PDT 2008 
//   Added call to SendFileOpenOptions after StartServer.
//
//   Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL.
// ****************************************************************************

std::string
ViewerFileServerImplementation::ExpandedFileName(const std::string &host,
    const std::string &filename)
{
    std::string retval(filename);

    //
    // If filename expansion is required, expand the filename.
    //
    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
    if(ExpansionRequired(filename))
    {
        // Try and start a server.
        NoFaultStartServer(host, stringVector());

        if(servers.find(host) != servers.end())
        {
            int  numAttempts = 0;
            bool tryAgain = false;

            do
            {
                TRY
                {
                    retval = servers[host]->proxy->GetMDServerMethods()->ExpandPath(filename);
                    tryAgain = false;
                }
                CATCH(LostConnectionException)
                {
                    // Tell the GUI that the mdserver is dead.
                    if(numAttempts == 0)
                    {
                        GetViewerMessaging()->Warning(
                            TR("The metadata server running on host %1 has "
                               "exited abnormally. VisIt is trying to "
                               "restart it.").
                            arg(host));
                    }

                    ++numAttempts;
                    tryAgain = (numAttempts < 2);

                    TRY
                    {
                        stringVector startArgs(servers[host]->arguments);
                        CloseServer(host, false);
                        StartServer(host, startArgs);
                        SendFileOpenOptions(host);
                    }
                    CATCHALL
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
// Method: ViewerFileServerImplementation::ExpansionRequired
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
//   Kathleen Biagas, Thu Jan 23 10:55:41 MST 2014
//   Return 'true' if we DON'T find the drive punctuation on Windows.
//
// ****************************************************************************

bool
ViewerFileServerImplementation::ExpansionRequired(const std::string &filename) const
{
    if(filename.size() > 0)
    {
#if defined(_WIN32)
        // Look for some drive punctuation
        if(filename.find(":\\") == std::string::npos)
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
// Method: ViewerFileServerImplementation::StartServer
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
//    Jeremy Meredith, Thu May 24 10:33:27 EDT 2007
//    Added SSH tunneling option to MDServerProxy::Create, and set it to false.
//    If we need to tunnel, the VCL will do the host/port translation for us.
//
//    Jeremy Meredith, Mon Jul 16 17:11:15 EDT 2007
//    Added call to UpdateDBPluginInfo after the server is started just to
//    get an initial set of DB plugin info.  It reverts to the previous
//    active one after we've gotten the initial set.
//
//    Hank Childs, Sun Nov 11 22:21:55 PST 2007
//    Clean up gracefully for any exception type.
//
//    Jeremy Meredith, Wed Jan 23 16:25:45 EST 2008
//    Tell new mdservers what the current default file opening options are.
//
//    Brad Whitlock, Tue Apr 29 14:59:18 PDT 2008
//    Support for internationalization.
//
//    Kathleen Bonnell, Thu Aug 14 16:18:21 PDT 2008
//    Moved UpdateDBPluginInfo and setting of default file open options to
//    SendFileOpenOptions, and call that method after this one completes. 
//
//    Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL.
//
//    Hank Childs, Thu Nov  5 18:42:45 PST 2009
//    Beef up error message when you cannot connect.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.
//
//    Eric Brugger, Mon May  2 17:09:13 PDT 2011
//    I added the ability to use a gateway machine when connecting to a
//    remote host.
//
//    Brad Whitlock, Tue Nov 29 16:41:35 PST 2011
//    Launch the mdserver through the engine if they should share the same
//    batch job.
//
//    Brad Whitlock, Tue Jun  5 17:16:31 PDT 2012
//    Pass a MachineProfile down into the proxy's Create.
//
//    Brad Whitlock, Tue Jan 15 14:11:07 PST 2013
//    Check the host for validity.
//
//    Brad Whitlock, Tue Sep  2 13:21:54 PDT 2014
//    Turn OpenWithEngine into a callback.
//
// ****************************************************************************

void
ViewerFileServerImplementation::StartServer(const std::string &host, const stringVector &args)
{
    const char *mName = "ViewerFileServerImplementation::StartServer: ";

    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
    // If the server already exists, return.
    if(servers.find(host) != servers.end())
        return;

    ViewerConnectionProgress *progress = NULL;

    // Create a new MD server on the remote machine.
    debug4 << mName << "start" << endl;
    MDServerProxy *newServer = new MDServerProxy;
    TRY
    {
        // Add regular arguments to the new mdserver proxy.
        AddArguments(newServer, args);

        MachineProfile profile = GetMachineProfile(host);

        // Determine which host we'll be connecting to and see if that host is
        // valid. We might be connecting via a gateway.
        std::string connectionHost(host);
        if(profile.GetUseGateway() && !profile.GetGatewayHost().empty())
            connectionHost = profile.GetGatewayHost();
        if(!CheckHostValidity(connectionHost))
        {
            // We'll catch this exception in this method and delete newServer then.
            EXCEPTION1(BadHostException, connectionHost);
        }

        // We don't set up tunnels when launching an MD server, just the VCL
        profile.SetTunnelSSH(false);

        // We don't use a gateway when launching an MD server, just the VCL
        profile.SetUseGateway(false);
        profile.SetGatewayHost("");

        // Add arguments from a matching host profile to the mdserver proxy.
        newServer->AddProfileArguments(profile, false);

        // Create a connection progress object and hook it up to the
        // mdserver proxy.
        progress = CreateConnectionProgress(host);
        SetupConnectionProgress(newServer, progress);

        // Start the mdserver on the specified host.
        if (HostIsLocalHost(host))
        {
            debug1 << mName << "Creating on localhost" << endl;
            profile.SetHost("localhost");
            newServer->Create(profile);
        }
        else if(profile.GetShareOneBatchJob())
        {
            debug1 << mName << "Sharing connection with engine." << endl;

            // Use VisIt's engine to start the remote mdserver.
            newServer->Create(profile, OpenWithEngineCB, (void*)progress, true);
        }
        else
        {
            debug1 << mName << "Creating on host " << host << endl;
            // Use VisIt's launcher to start the remote mdserver.
            newServer->Create(profile, OpenWithLauncher, (void*)progress, true);
        }

        // Add the information about the new server to the 
        // server map.
        servers[host] = new MDServerManager::ServerInfo();
        servers[host]->server = newServer;
        servers[host]->arguments = args;

        shouldSendFileOpenOptions = true;
    }
    CATCH(BadHostException)
    {
        GetViewerMessaging()->Error(
            TR("VisIt cannot launch a metadata server on host %1.").
            arg(host));

        delete newServer;
    }
    CATCH(IncompatibleVersionException)
    {
        GetViewerMessaging()->Error(
            TR("The metadata server on host %1 is an incompatible version. "
               "It cannot be used.").
            arg(host));
        delete newServer;
        delete progress;

        // Re-throw the exception.
        RETHROW;
    }
    CATCH(IncompatibleSecurityTokenException)
    {
        GetViewerMessaging()->Error(
            TR("The metadata server on host %1 did not return "
               "the proper credentials. It cannot be used.").
            arg(host));

        delete newServer;
        delete progress;

        // Re-throw the exception.
        RETHROW;
    }
    CATCH2(CouldNotConnectException, e)
    {
        std::string err; 
        if(!e.Message().empty())
        {
            err = ViewerText(TR("The specific error was: \"%1\"\n\n").
                             arg(e.Message())).toStdString();
        }

        ViewerText msg = TR(
            "The metadata server on host %1 could not be launched or it "
            "could not connect back to your local computer. This can "
            "happen for a variety of reasons.\n\n%2"

            "It is possible that SSH was unable to launch VisIt on %1. "
            "If you want to verify this, run "
            "\"visit -debug 5\" and then check to see if any vcl, "
            "mdserver, or engine log files are present on %1 in your "
            "home directory. If no log files were created then SSH was "
            "probably not able to launch VisIt components on %1. In that "
            "case, check that you can SSH to %1 and check your local "
            "VisIt installation's Host profiles to make sure the path "
            "to VisIt on %1 is specified. Alternatively, you set the "
            "PATH environment variable on %1 so it contains the "
            "path to the program \"visit\".\n\n"

            "If there were no debug logs to be found on %1 and your local "
            "computer runs a newer version of Linux then quit VisIt and "
            "try running \"visit -nopty -debug 5\". The \"-nopty\" option "
            "tells VisIt not to allocate a pseudoterminal in which to "
            "run SSH. When you run with the \"-nopty\" option, VisIt's "
            "password window will not be used. Instead, look for an SSH "
            "prompt in the terminal window where you ran VisIt. You should "
            "be able to enter your password at that prompt. If successful, "
            "SSH should continue trying to launch VisIt on %1. If VisIt "
            "still cannot connect after SSH launches VisIt's remote "
            "components, check for debug logs on %1 to see if VisIt "
            "was at least able to launch there.\n\n"

            "\"vglrun\": If you do not know what \"vglrun\" is, you can ignore this paragraph."
            "  If there were no debug logs to be found on %1 and you are using "
            "vglrun, then vglrun may be causing VisIt to fail. Some versions "
            "of vglrun cause the ssh program to fail.  If you are running "
            "VisIt in conjunction with vglrun, this may be causing your "
            "failure.  (You can test this by running \"vglrun ssh\" and seeing "
            "if it cores.)\n\n"

            "If you found debug log files on %1 but VisIt still can't "
            "connect then it's possible that %1 cannot connect to your "
            "local computer. Some desktop computers do not provide a "
            "valid network name when VisIt asks for one. If you suspect "
            "that this could be the cause of the launch failure, try "
            "using \"Parse from SSH_CLIENT\" in your host profile for "
            "host %1. If that does not work and if you are using VPN "
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
            "connect to %1 then contact visit-users@ornl.gov and provide "
            "information about how you are trying to connect. Be sure to "
            "include the VisIt version and platform on which you are "
            "running.").arg(host).arg(err);
        GetViewerMessaging()->Error(msg);

        delete newServer;
        delete progress;

        // Re-throw the exception.
        RETHROW;
    }
    CATCH(CancelledConnectException)
    {
        GetViewerMessaging()->Error(
            TR("The launch of the metadata server on host \"%1\" has been "
               "cancelled.").
            arg(host));

        delete newServer;
        delete progress;

        // Re-throw the exception.
        RETHROW;
    }
    CATCHALL
    {
        delete newServer;
        delete progress;

        // Re-throw the exception.
        RETHROW;
    }
    ENDTRY

    // Delete the connection progress object
    delete progress;

    debug4 << mName << "end" << endl;
}


// ****************************************************************************
// Method: ViewerFileServerImplementation::SendFileOpenOptions
//
// Purpose: 
//   Updates DBPluginInfo and FileOpenOptions for the specified host.
//
// Arguments:
//   host : The name of the host on which to update.
//
// Programmer: Kathleen Bonnell 
// Creation:   August 14, 2008
//
// Modifications:
//   
// ****************************************************************************
//
void
ViewerFileServerImplementation::SendFileOpenOptions(const std::string &host)
{
    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
    if(shouldSendFileOpenOptions && servers.find(host) != servers.end())
    {
        // Once we start a new server, we need to update the current
        // DB plugin list with data from this machine.  Revert to the
        // old one after we have the initial set.

        std::string oldhost = GetViewerState()->GetDBPluginInfoAttributes()->GetHost();
        UpdateDBPluginInfo(host);
        if (oldhost != "")
            UpdateDBPluginInfo(oldhost);

        // Send the new server our current options for opening files.
        servers[host]->proxy->GetMDServerMethods()->
            SetDefaultFileOpenOptions(*GetViewerState()->GetFileOpenOptions());
    }
    shouldSendFileOpenOptions = false;
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::NoFaultStartServer
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
//   Kathleen Bonnell, Thu Aug 14 16:13:41 PDT 2008 
//   Added call to SendFileOpenOptions after StartServer.
//
//   Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL.
// ****************************************************************************

void
ViewerFileServerImplementation::NoFaultStartServer(const std::string &host, 
    const stringVector &args)
{
    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
    if(servers.find(host) == servers.end())
    {
        // Try and start a metadata server on the specified host.
        TRY
        {
            StartServer(host, args);
            SendFileOpenOptions(host);
        }
        CATCHALL
        {
            // Do nothing.
        }
        ENDTRY
    }
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::CloseServer
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
ViewerFileServerImplementation::CloseServer(const std::string &host, bool close)
{
    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
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
// Method: ViewerFileServerImplementation::SendKeepAlives
//
// Purpose: 
//   Sends keep alive signals to all of the mdservers.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 12:07:13 PDT 2004
//
// Modifications:
//   
//   Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL.
// ****************************************************************************

void
ViewerFileServerImplementation::SendKeepAlives()
{
    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
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
        CATCHALL
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
// Method: ViewerFileServerImplementation::ConnectServer
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
//   Thomas R. Treadway, Mon Oct  8 13:27:42 PDT 2007
//   Backing out SSH tunneling on Panther (MacOS X 10.3)
//
//   Hank Childs, Sun Nov 11 22:21:55 PST 2007
//   Add support for changing the username.
//
//   Kathleen Bonnell, Thu Aug 14 16:21:57 PDT 2008
//   Added call to SendFileOpenOptions.
//
//   Brad Whitlock, Wed Oct 12 15:39:39 PDT 2011
//   Remove TerminateConnectionRequest for when we want to change usernames
//   since that sends a message to the gui that stops it from waiting for
//   a connection from the mdserver. We want it to keep waiting so when we
//   change the username, the mdserver connects.
//
// ****************************************************************************

void
ViewerFileServerImplementation::ConnectServer(const std::string &mdServerHost,
    const stringVector &args)
{
    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
    // If a server exists, get the current directory to make sure it is
    // still alive.
    stringVector startArgs;
    if(servers.find(mdServerHost) != servers.end())
    {
        TRY
        {
            // Get the directory, it is relatively cheap to do this and it
            // also proves that the mdserver is still alive.
            servers[mdServerHost]->proxy->GetMDServerMethods()->GetDirectory();
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
    static const char *termString = "ViewerFileServerImplementation::ConnectServer: "
        "Terminating client mdserver connection because ";

    bool keepGoing = true;
    while (keepGoing)
    {
        // Start a server if one has not already been started.
        TRY
        {
            keepGoing = false;
            StartServer(mdServerHost, startArgs);
        }
        CATCH(ChangeUsernameException)
        {
            // set up a new username
            debug1 << "Asked to pick a new username" << endl;
            ViewerChangeUsername *obj = GetViewerFactory()->CreateChangeUsername();
            std::string newUsername;
            obj->ChangeUsername(mdServerHost, newUsername);
            delete obj;
            keepGoing = true;
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
    }

    // If the remote host is in the server list, tell it to connect
    // to another program.
    if(servers.find(mdServerHost) != servers.end())
    {
        // If we're doing ssh tunneling, map the local host/port to the
        // remote one.
        std::map<int,int> portTunnelMap = GetPortTunnelMap(mdServerHost);
        if (!portTunnelMap.empty())
        {
            stringVector newargs(args);
            bool success = ConvertArgsToTunneledValues(portTunnelMap, newargs);
            if (!success)
            {
                debug1 << termString
                       << "tunneling was requested and no remote port existed"
                       << " to map to the desired local port. Try "
                       << "increasing the number of tunneled ports.";
                TerminateConnectionRequest(args, 5);
            }
            servers[mdServerHost]->proxy->GetMDServerMethods()->Connect(newargs);
        }
        else
        {
            // Not tunneling through SSH; just go ahead and connect
            servers[mdServerHost]->proxy->GetMDServerMethods()->Connect(args);
        }
    }
    // We only want to do this after the mdserver has connected back
    SendFileOpenOptions(mdServerHost);
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::TerminateConnectionRequest
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
//   Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL.
// ****************************************************************************

void
ViewerFileServerImplementation::TerminateConnectionRequest(const stringVector &args, int failCode)
{
    int  argc = (int)args.size();
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
    CATCHALL
    {
        // We know that we're going to get here, but no action is required.
    }
    ENDTRY

    delete [] argv;
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::UpdateDBPluginInfo
//
// Purpose:
//   Tells the mdserver to update the database plugin information.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// Modifications:
//    Jeremy Meredith, Mon Jul 16 17:11:49 EDT 2007
//    Added a "host" member to the DBPluginInfoAttributes, so observers
//    can tell which host the attributes apply to if they weren't the
//    ones that requested the update.
//
//    Jeremy Meredith, Wed Jan 23 16:28:21 EST 2008
//    When we get new database plugin info, merge that into what we know
//    about existing file opening options.  These will not override, but
//    instead append to, existing options (like ones saved in a config file).
//
//    Jeremy Meredith, Fri Mar 26 10:40:40 EDT 2010
//    Though we do not need to use the command line to specify assumed and
//    fallback formats anymore, such usage still has some conveniences.
//    Added support to munge the preferred list when given those options.
//
// ****************************************************************************
void
ViewerFileServerImplementation::UpdateDBPluginInfo(const std::string &host)
{
    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
    if(servers.find(host) != servers.end())
    {
        *GetViewerState()->GetDBPluginInfoAttributes() = *(servers[host]->proxy->GetMDServerMethods()->GetDBPluginInfo());
        GetViewerState()->GetDBPluginInfoAttributes()->SetHost(host);
        GetViewerState()->GetDBPluginInfoAttributes()->Notify();
        GetViewerState()->GetFileOpenOptions()->MergeNewFromPluginInfo(GetViewerState()->GetDBPluginInfoAttributes());
        GetViewerState()->GetFileOpenOptions()->AddAssumedFormatsToPreferred(GetViewerProperties()->GetAssumedFormats());
        GetViewerState()->GetFileOpenOptions()->AddFallbackFormatsToPreferred(GetViewerProperties()->GetFallbackFormats());
        GetViewerState()->GetFileOpenOptions()->Notify();
    }
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::ClearFile
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
//   Jeremy Meredith, Fri Jan 29 10:25:16 EST 2010
//   Added extra flag to tell ClearFile whether or not we want to 
//   forget which plugin was used to open it.
//
// ****************************************************************************

void
ViewerFileServerImplementation::ClearFile(const std::string &fullName, bool forgetPlugin)
{
    debug4 << "ViewerFileServerImplementation::Clearfile" << endl;

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

    // Clear the plugin used for this file, if needed
    if (forgetPlugin)
    {
        filePlugins.erase(fullName);
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
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::CloseFile
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
ViewerFileServerImplementation::CloseFile(const std::string &host, const std::string &db)
{
    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
    ServerMap::iterator pos;
    if((pos = servers.find(host)) != servers.end()) 
    {
        TRY
        {
            if(db.empty())
                pos->second->proxy->GetMDServerMethods()->CloseDatabase();
            else
                pos->second->proxy->GetMDServerMethods()->CloseDatabase(db);
        }
        CATCH(LostConnectionException)
        {
            CloseServer(host, false);
        }
        ENDTRY
    }
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::IsDatabase
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
ViewerFileServerImplementation::IsDatabase(const std::string &fullname) const
{
    stringVector f(GetOpenDatabases());
    return std::find(f.begin(), f.end(), fullname) != f.end();
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::SplitKey
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
ViewerFileServerImplementation::SplitKey(const std::string &key, std::string &hdb,
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
// Method: ViewerFileServerImplementation::GetOpenDatabases
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
ViewerFileServerImplementation::GetOpenDatabases() const
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
// Method: ViewerFileServerImplementation::CreateNode
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
//
// ****************************************************************************

void
ViewerFileServerImplementation::CreateNode(DataNode *parentNode)
{
    // Create a map of source ids to plugin ids and also store
    // that information into the session.
    DataNode *sourcePluginMapNode = new DataNode("SourcePlugins");
    for(StringStringMap::iterator i=filePlugins.begin() ; 
        i != filePlugins.end(); i++)
    {
        sourcePluginMapNode->AddNode(new DataNode(i->first, i->second));
    }
    parentNode->AddNode(sourcePluginMapNode);
}

// ****************************************************************************
// Method: ViewerFileServerImplementation::SetFromNode
//
// Purpose: 
//   Initializes the database correlation list.
//
// Arguments:
//   parentNode : The node on which to look for attribute nodes.
//   sourceToDB : The map of source ids -> database names.
//   configVersion : The version from the config file.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 25 16:49:01 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri Nov 10 13:44:40 PST 2006
//   I rewrote the method to handle creation of database correlations 
//   using replacement sources.
//
//   Brad Whitlock, Fri Oct 23 16:11:33 PDT 2009
//   I added code to remove all metadata, SILs, and database correlations.
//
//   Jeremy Meredith, Fri Jan 29 10:27:29 EST 2010
//   Added knowledge of what plugin was used to open a file.  Clear it here.
//
//   Jeremy Meredith, Fri Jan 29 11:33:45 EST 2010
//   Load the database->plugin map from the node.
//
// ****************************************************************************

void
ViewerFileServerImplementation::SetFromNode(DataNode *parentNode)
{
    // Clear out all of the metadata, SILs, and correlations.
    debug4 << "Clearing all stored metadata, SILs, and correlations." << endl;
    FileMetaDataMap::iterator fpos;
    for(fpos = fileMetaData.begin(); fpos != fileMetaData.end(); ++fpos)
    {
        // Delete the metadata
        delete fpos->second;
    }
    fileMetaData.clear();
    filePlugins.clear();
    FileSILMap::iterator fpos2;
    for (fpos2 = fileSIL.begin() ; fpos2 != fileSIL.end() ; fpos2++)
    {
        // Delete the SIL
        delete fpos2->second;
    }
    fileSIL.clear();

    // Load the file-to-plugin map
    DataNode *pluginMapNode = parentNode->GetNode("SourcePlugins");
    if (pluginMapNode != 0)
    {
        int n = pluginMapNode->GetNumChildren();
        DataNode **pluginMapValues = pluginMapNode->GetChildren();
        for (int i=0; i<n; i++)
        {
            std::string db(pluginMapValues[i]->GetKey());
            std::string plugin(pluginMapValues[i]->AsString());
            filePlugins[db] = plugin;
        }
    }
}

// ****************************************************************************
//  Method:  ViewerFileServerImplementation::SetSimulationMetaData
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
ViewerFileServerImplementation::SetSimulationMetaData(const std::string &host,
                                        const std::string &filename,
                                        const avtDatabaseMetaData &md)
{
    std::string dbName(FileFunctions::ComposeDatabaseName(host, filename));
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
            std::string host = pos->second->GetSimInfo().GetHost();
            int    port = pos->second->GetSimInfo().GetPort();
            std::string key  = pos->second->GetSimInfo().GetSecurityKey();
            std::vector<std::string> onames = pos->second->GetSimInfo().GetOtherNames();
            std::vector<std::string> ovalues= pos->second->GetSimInfo().GetOtherValues();

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
//  Method:  ViewerFileServerImplementation::SetSimulationSILAtts
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
//  Modifications:
//    Brad Whitlock, Tue Mar 16 11:51:51 PDT 2010
//    Add the SIL if it has not been added before. Otherwise, the mdserver
//    will be queried for the SIL and that's useless for simulations.
//
// ****************************************************************************

void
ViewerFileServerImplementation::SetSimulationSILAtts(const std::string &host,
                                       const std::string &filename,
                                       const SILAttributes &silAtts)
{
    avtSIL *sil = new avtSIL(silAtts);
    std::string dbName(FileFunctions::ComposeDatabaseName(host, filename));
    bool added = false;
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
            added = true;
            *(pos->second) = *sil;
        }
    }
    if(!added)
    {
        // If we have not added the simuation SIL before, add it now.
        debug5 << "Adding simulation SIL for " << dbName << endl;
        fileSIL[dbName] = sil;
    }
    else
    { 
        delete sil;
    }
}

// ****************************************************************************
//  Method:  ViewerFileServerImplementation::GetPluginErrors
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
ViewerFileServerImplementation::GetPluginErrors(const std::string &host)
{
    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
    if(servers.find(host) != servers.end())
    {
        return servers[host]->proxy->GetMDServerMethods()->GetPluginErrors();
    }
    return "";
}

//
// ViewerFileServerImplementation::ServerInfo methods
//

//ViewerFileServerImplementation::ServerInfo::ServerInfo(MDServerProxy *p, const stringVector &args) : arguments(args)
//{
//    proxy = p;
//}

//ViewerFileServerImplementation::ServerInfo::ServerInfo(const ServerInfo &b)
//{
//    proxy = 0;
//    arguments = b.arguments;
//}

//ViewerFileServerImplementation::ServerInfo::~ServerInfo()
//{
//    delete proxy;
//}

// ****************************************************************************
//  Method:  ViewerFileServerImplementation::BroadcastUpdatedFileOpenOptions
//
//  Purpose:
//    The default file opening options have changed; let all the existin
//    mdservers know about them.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2008
//
// ****************************************************************************
void
ViewerFileServerImplementation::BroadcastUpdatedFileOpenOptions()
{
    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
    ServerMap::iterator it;
    for (it = servers.begin(); it != servers.end(); it++)
    {
        it->second->proxy->GetMDServerMethods()->
            SetDefaultFileOpenOptions(*GetViewerState()->GetFileOpenOptions());
    }
}

// ****************************************************************************
//  Method:  ViewerFileServerImplementation::SaveSession
//
//  Purpose:
//     Save current session on host.
//
//  Arguments:
//    host     - Host system to save file on.
//    filename - Filename to save session file contents.
//    contents - Contents for session file.
//
//  Programmer:  David Camp
//  Creation:    Thu Aug 27 09:40:00 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

void
ViewerFileServerImplementation::SaveSession(const std::string &host,
                                            const std::string &filename,
                                            const std::string &contents)
{
    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
    ServerMap::iterator info = servers.find(host);
    if(info != servers.end())
    {
        info->second->proxy->GetMDServerMethods()->SaveSession(filename, contents);
    }
}

// ****************************************************************************
//  Method:  ViewerFileServerImplementation::RestoreSession
//
//  Purpose:
//     Restore session from host.
//
//  Arguments:
//    host     - Host system to save file on.
//    filename - Filename to save session file contents.
//    contents - Contents from session file, which is load from host filename.
//
//  Programmer:  David Camp
//  Creation:    Thu Aug 27 09:40:00 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

void
ViewerFileServerImplementation::RestoreSession(const std::string &host, 
                                               const std::string &filename, 
                                               std::string &contents)
{
    typedef MDServerManager::ServerMap ServerMap;
    ServerMap& servers = MDServerManager::Instance()->GetServerMap();
    ServerMap::iterator info = servers.find(host);
    if(info != servers.end())
    {
        info->second->proxy->GetMDServerMethods()->RestoreSession(filename, contents);
    }
}

