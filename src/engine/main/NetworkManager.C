#include <snprintf.h>
#include <NetworkManager.h>
#include <DataNetwork.h>
#include <ClonedDataNetwork.h>
#include <DebugStream.h>
#include <avtDatabaseFactory.h>
#include <LoadBalancer.h>
#include <MaterialAttributes.h>
#include <avtExpressionEvaluatorFilter.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <DatabaseException.h>
#include <NoInputException.h>
#include <avtPluginFilter.h>
#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>
#include <AnnotationObjectList.h>
#include <PickAttributes.h>
#include <VisualCueInfo.h>
#include <VisualCueList.h>
#include <avtCallback.h>
#include <avtColorTables.h>
#include <avtExtents.h>
#include <avtNullData.h>
#include <avtDatabaseMetaData.h>
#include <avtDataObjectQuery.h>
#include <avtMultipleInputQuery.h>
#include <avtAreaBetweenCurvesQuery.h>
#include <avtFileWriter.h>
#include <avtL2NormBetweenCurvesQuery.h>
#include <avtLocateQuery.h>
#include <avtLocateCellQuery.h>
#include <avtLocateNodeQuery.h>
#include <avtActualCoordsQuery.h>
#include <avtActualNodeCoordsQuery.h>
#include <avtActualZoneCoordsQuery.h>
#include <avtPickQuery.h>
#include <avtNodePickQuery.h>
#include <avtParallel.h>
#include <avtPickByNodeQuery.h>
#include <avtPickByZoneQuery.h>
#include <avtZonePickQuery.h>
#include <avtCurvePickQuery.h>
#include <avtSourceFromAVTImage.h>
#include <avtSourceFromImage.h>
#include <avtSourceFromNullData.h>
#include <avtTiledImageCompositor.h>
#include <avtWholeImageCompositerWithZ.h>
#include <avtWholeImageCompositerNoZ.h>
#include <avtPlot.h>
#include <avtQueryOverTimeFilter.h>
#include <avtQueryFactory.h>
#include <CompactSILRestrictionAttributes.h>
#include <VisWindow.h>
#include <ParsingExprList.h>
#include <EngineExprNode.h>
#include <ParserInterface.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#ifdef PARALLEL
#include <mpi.h>
#include <parallel.h>
#endif
#include <TimingsManager.h>

#include <set>
using std::set;

static double RenderBalance(int numTrianglesIHave);
static void   DumpImage(avtDataObject_p, const char *fmt, bool allprocs=true);
static void   DumpImage(avtImage_p, const char *fmt, bool allprocs=true);

// ****************************************************************************
//  Method: NetworkManager default constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   September 29, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Nov 20 12:35:54 PST 2001
//    Initialize requireOriginalCells. 
//
//    Hank Childs, Mon Jan  5 16:04:57 PST 2004
//    Initialize uniqueNetworkId.
//
//    Hank Childs, Thu Mar  4 09:11:46 PST 2004
//    Change instantiation of vis window, since it is now assumed to be nowin.
//
//    Mark C. Miller, Thu Apr 29 16:15:25 PDT 2004
//    Added missing initialization of dumpRenders
//
//    Mark C. Miller, Tue May 25 20:44:10 PDT 2004
//    Removed unnecessary scope. Set annotationAttributes data member
//
//    Kathleen Bonnell, Wed Jun  2 09:48:29 PDT 2004 
//    Initialize requireOriginalNodes. 
//
// ****************************************************************************
NetworkManager::NetworkManager(void) : virtualDatabases()
{
    workingNet = NULL;
    loadBalancer = NULL;
    requireOriginalCells = false;
    requireOriginalNodes = false;
    uniqueNetworkId = 0;
    dumpRenders = false;

    // stuff to support scalable rendering
    viswin = new VisWindow();

    annotationAttributes = *(viswin->GetAnnotationAtts());
    annotationAttributes.SetUserInfoFlag(false);
    annotationAttributes.SetDatabaseInfoFlag(false);
    annotationAttributes.SetLegendInfoFlag(false);
    annotationAttributes.SetTriadFlag(false);
    annotationAttributes.SetBboxFlag(false);
    annotationAttributes.SetAxesFlag(false);
    annotationAttributes.SetAxesFlag2D(false);
    viswin->SetAnnotationAtts(&annotationAttributes);

    viswin->DisableUpdates();

}

// ****************************************************************************
//  Method: NetworkManager destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   September 29, 2000
//
//  Modifications:
//    Sean Ahern, Fri May 24 11:12:25 PDT 2002
//    Got rid of deletion of the database cache, since all DBs in there
//    will be deleted by deleting the network cache.
//
//    Hank Childs, Mon Jan  5 16:30:04 PST 2004
//    Don't assume the caches are valid.
//
// ****************************************************************************
NetworkManager::~NetworkManager(void)
{
    for (int i = 0; i < networkCache.size(); i++)
        if (networkCache[i] != NULL)
            delete networkCache[i];
}

// ****************************************************************************
//  Method: NetworkManager::ClearAllNetworks
//
//  Purpose:
//      Clears out the caches.  Really only used for checking for memory leaks.
//
//  Programmer: Hank Childs
//  Creation:   January 7, 2001
//
//  Modifications:
//    Sean Ahern, Tue May 14 14:35:42 PDT 2002
//    Retrofitted for my new network management techniques.
//
//    Sean Ahern, Tue May 21 16:09:56 PDT 2002
//    Told the underlying objects to free their data.
//
//    Hank Childs, Mon Jan  5 16:30:04 PST 2004
//    Don't clear out the vectors, so this can be consistent with how the
//    ClearNetworksWithDatabase works.
//
//    Brad Whitlock, Thu Feb 26 11:58:58 PDT 2004
//    I replaced the commented out cerr with debug3.
//
//    Hank Childs, Sun Mar  7 16:05:03 PST 2004
//    Delete the networks before the databases, since the networks have
//    references to the databases (and the previous ordering yielded dangling
//    pointers).
//
//    Mark C. Miller, Wed Sep  8 17:06:25 PDT 2004
//    Added code to clear the vis window and the list of plots currently in
//    the window.
//
//    Mark C. Miller, Mon Sep 13 18:30:26 PDT 2004
//    Protected call to clear viswin and plotsCurrentlyInWindow with test
//    for emptiness
//
// ****************************************************************************
void
NetworkManager::ClearAllNetworks(void)
{
    debug3 << "NetworkManager::ClearAllNetworks(void)" << endl;
    int i;

    for (i = 0; i < networkCache.size(); i++)
    {
        if (networkCache[i] != NULL)
            delete networkCache[i];
        networkCache[i] = NULL;
    }

    for (i = 0; i < databaseCache.size(); i++)
    {
        if (databaseCache[i] != NULL)
            delete databaseCache[i];
        databaseCache[i] = NULL;
    }

    for (i = 0 ; i < globalCellCounts.size() ; i++)
    {
        globalCellCounts[i] = -1;
    }

    if (!plotsCurrentlyInWindow.empty())
    {
        viswin->ClearPlots();
        plotsCurrentlyInWindow.clear();
    }
}

// ****************************************************************************
//  Method: NetworkManager::ClearNetworksWithDatabase
//
//  Purpose:
//      Clears out the caches that reference a specific database.
//
//  Arguments:
//      db      The name of a database.
//
//  Programmer: Hank Childs
//  Creation:   January 5, 2004
//
//  Modifications:
//    Brad Whitlock, Thu Feb 26 11:58:58 PDT 2004
//    I replaced the commented out cerr with debug3.
//
//    Mark C. Miller, Wed Sep  8 17:06:25 PDT 2004
//    Added code to clear the vis window and the list of plots currently in
//    the window for the first plot id it encounters in this db
//
//    Mark C. Miller, Mon Sep 13 18:30:26 PDT 2004
//    Moved code to delete the networkCache entry to after the clearning
//    of the viswin.
// ****************************************************************************
void
NetworkManager::ClearNetworksWithDatabase(const std::string &db)
{
    debug3 << "NetworkManager::ClearNetworksWithDatabase()" << endl;
    int i,j;

    // 
    // Clear out the networks before the databases.  This is because if we
    // delete the databases first, the networks will have dangling pointers.
    //
    for (i = 0; i < networkCache.size(); i++)
    {
        if (networkCache[i] != NULL)
        {
            NetnodeDB *ndb = networkCache[i]->GetNetDB();
            if (ndb != NULL)
            {
                if (ndb->GetFilename() == db)
                {
                    for (j = 0 ; j < plotsCurrentlyInWindow.size() ; j++)
                    {
                        if (plotsCurrentlyInWindow[j] == i)
                        {
                            viswin->ClearPlots();
                            plotsCurrentlyInWindow.clear();
                            break;
                        }
                    }
                    delete networkCache[i];
                    networkCache[i] = NULL;
                    globalCellCounts[i] = -1;
                }
            }
        }
    }

    for (i = 0; i < databaseCache.size(); i++)
    {
        if (databaseCache[i] != NULL)
        {
            if (databaseCache[i]->GetFilename() == db)
            {
                delete databaseCache[i];
                databaseCache[i] = NULL;
            }
        }
    }
}

// ****************************************************************************
//  Method: NetworkManager::GetDBFromCache
//
//  Purpose:
//      Get a DB from the cache.  If it's not there already, create it and
//      return it.
//
//  Programmer: Sean Ahern
//  Creation:   Mon Dec 23 12:28:30 PST 2002
//
//  Modifications:
//
//    Brad Whitlock, Wed May 14 12:56:16 PDT 2003
//    I made it get the metadata and the SIL if the time is greater than 0.
//    This ensures that we pick up the metadata and SIL for the time state
//    that we're intersted in.
//
//    Brad Whitlock, Mon Aug 25 16:51:35 PST 2003
//    Changed the return statement to CATCH_RETURN2 so bad things don't happen
//    when we use fake exceptions.
//
//    Mark C. Miller Thu Oct  9 10:59:27 PDT 2003
//    I modified to force GetMetaData and GetSIL if they're invariant 
//
//    Hank Childs, Tue Nov  4 14:19:05 PST 2003
//    Checked in work-around (HACK) to avoid apparent compiler bug on AIX.
//
//    Hank Childs, Mon Jan  5 16:49:28 PST 2004
//    Do not assume that all of the cached databases are non-NULL.
//
//    Hank Childs, Mon Mar  1 08:48:26 PST 2004
//    Send the time to the database factory as well.
//
//    Hank Childs, Mon Mar 22 11:10:43 PST 2004
//    Allow for the DB type to be explicitly specified.
//
//   Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//   Added code to pass avtDatabaseMetaData to LoadBalancer->AddDatabase
//
// ****************************************************************************

NetnodeDB *
NetworkManager::GetDBFromCache(const string &filename, int time,
                               const char *format)
{
    // If we don't have a load balancer, we're dead.
    if (loadBalancer == NULL)
    {
        debug1 << "Internal error: A load balancer was never registered." 
               << endl;
        EXCEPTION0(ImproperUseException);
    }

    // Find a matching database 
    NetnodeDB* cachedDB = NULL;
    for (int i = 0; i < databaseCache.size(); i++)
    {
        if (databaseCache[i] == NULL)
            continue;
        if (databaseCache[i]->GetFilename() == filename)
        {
            cachedDB = databaseCache[i];
            break;
        }
    }

    // got a match
    if (cachedDB != NULL)
    {

        // even if we found the DB in the cache,
        // we need to update the metadata if its time-varying
        if (!cachedDB->GetDB()->MetaDataIsInvariant() ||
            !cachedDB->GetDB()->SILIsInvariant())
        {
            cachedDB->GetDB()->GetMetaData(time);
            cachedDB->GetDB()->GetSIL(time);
        }

        cachedDB->SetDBInfo(filename, "", time);
        return cachedDB;
    }

    // No match.  Load a new DB.
    debug3 << "Loading new database" << endl;
    TRY
    {
        avtDatabase *db = NULL;
        NetnodeDB *netDB = NULL;
        const char *filename_c = filename.c_str();
        if (filename.substr(filename.length() - 6) == ".visit")
            db = avtDatabaseFactory::VisitFile(filename_c, time, format);
        else
            db = avtDatabaseFactory::FileList(&filename_c, 1, time, format);

        // If we want to open the file at a later timestep, get the
        // SIL so that it contains the right data.
        if ((time > 0) ||
            (!db->MetaDataIsInvariant()) ||
            (!db->SILIsInvariant()))
        {
            debug2 << "NetworkManager::AddDB: We were instructed to open "
                   << filename.c_str() << " at timestate=" << time
                   << " so we're reading the SIL early."
                   << endl;
            db->GetMetaData(time);
            db->GetSIL(time);
        }

        netDB = new NetnodeDB(db);
        databaseCache.push_back(netDB);

        netDB->SetDBInfo(filename, "", time);
        const   avtIOInformation & ioinfo = db->GetIOInformation(time);
        const   avtDatabaseMetaData *md = db->GetMetaData(time);
        loadBalancer->AddDatabase(filename, ioinfo, md);
      
        // The code here should be:
        // CATCH_RETURN2(1, netDB);
        //
        // However: this is crashing on AIX when you do a re-open (see '3984).
        // After investigating this bug, I have determined the following:
        // (1) A valid pointer is being returned from this function.  However
        // the calling function receives NULL (leading to a seg fault).
        // (2) When compiling with fake exceptions and running under purify
        // on the Suns, there are no warnings.
        // (3) The catch return unwinds into 3 calls.  If you put a print
        // statement right before the last one (the return), everything is
        // returned correctly and there is no seg fault.
        //
        // I'm always very leery to call something a compiler bug.  However,
        // on different platforms there is no problem.  I think it is even
        // more fishy that things work if there is a print statement and don't
        // if there is not.  It certainly sounds like a compiler bug.
        //
        // Below is code that "makes this work".  It is essentially a
        // CATCH_RETURN2 with a print statement in the middle.  If CATCH_RETURN
        // were to ever change, this coding would have to change as well
        // (thus, I have introduced a maintenance problem).  As such, I put
        // a comment in VisItException.h referencing this comment.  If the
        // coding below can ever be replaced with an everyday CATCH_RETURN,
        // the comment in VisItException.h should be removed.
        //
#ifdef FAKE_EXCEPTIONS
           exception_delete(exception_caught); 
           jump_stack_top -= (1); 
           debug5 << "This debug statement (from the NetworkManager) is "
                  << "being issued to get around a compiler bug." << endl;
           return (netDB); 
#else
           return netDB;
#endif
    }
    CATCH(DatabaseException)
    {
        debug1 << "ERROR - could not create database " << filename.c_str()
            << endl;
        RETHROW;
    }
    ENDTRY
}

// ****************************************************************************
//  Method: NetworkManager::StartNetwork
//
//  Purpose:
//      Start building the pipeline.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 29, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Thu Nov  2 14:07:34 PST 2000
//    Changed the file initialization.  Apparently substr() may not portably 
//    be a const method of strings....
//
//    Hank Childs, Tue Jun 12 20:01:55 PDT 2001
//    Incorporate data specifications and the load balancer.
//
//    Jeremy Meredith, Thu Jul 26 12:08:42 PDT 2001
//    Fixed a bug passing the data file name to the load balancer.
//
//    Hank Childs, Thu Jul 26 20:17:56 PDT 2001
//    Re-enable caching by checking data specifications.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Jeremy Meredith, Fri Nov  9 10:24:23 PST 2001
//    Replaced netPool with netMRU, and simply made it release its
//    data, not delete the whole network. Made is only set the data
//    specification, not the pipeline specification yet.
//
//    Hank Childs, Fri Dec 14 17:39:36 PST 2001
//    Add support for using a more compact version of sil attributes.
//
//    Sean Ahern, Mon May 13 17:24:25 PDT 2002
//    Refactored to start support for a tree-based network scheme.
//
//    Sean Ahern, Tue May 28 09:47:21 PDT 2002
//    Added tree-based networks.
//
//    Brad Whitlock, Fri Jun 28 14:03:49 PST 2002
//    I renamed Network to DataNetwork to make the Windows compiler happy.
//
//    Hank Childs, Thu Sep 12 11:11:16 PDT 2002
//    There was an issue with multiple networks sharing NetnodeDBs, so I
//    made them all have their own reference.
//
//    Jeremy Meredith, Thu Oct 24 16:03:39 PDT 2002
//    Added material options.
//
//    Sean Ahern, Mon Dec 23 12:38:17 PST 2002
//    Changed this dramatically.  Broke out a good portion of the logic
//    into GetDBFromCache.
//
//    Sean Ahern, Sat Mar  8 00:59:16 America/Los_Angeles 2003
//    Cleaned up unnecessarily complicated code.
//
//    Jeremy Meredith, Wed Jul 30 10:45:45 PDT 2003
//    Added the check for requiring full connectivity.
//
//    Jeremy Meredith, Fri Sep  5 15:23:13 PDT 2003
//    Added a flag for the new MIR algorithm.
//
//    Jeremy Meredith, Thu Oct 30 16:09:32 PST 2003
//    Added code to make sure varLeaves was non-empty before accessing it.
//
//    Jeremy Meredith, Fri Oct 31 13:05:26 PST 2003
//    Made the error message for no-real-variables more informative.
//
//    Hank Childs, Sun Mar  7 16:05:03 PST 2004
//    Do not make the database be a node for the network.  Also add the EEF
//    as a node.
//
//    Jeremy Meredith, Tue Mar 23 14:40:20 PST 2004
//    Added the file format as an argument.  Made use of it in GetDBFromCache.
//
//    Mark C. Miller, Thu Aug 19 10:28:45 PDT 2004
//    Removed #ifdef 0 code block
//
// ****************************************************************************
void
NetworkManager::StartNetwork(const string &filename, const string &format,
                             const string &var,
                             int time,
                             const CompactSILRestrictionAttributes &atts,
                             const MaterialAttributes &matopts)
{
    // If the variable is an expression, we need to find a "real" variable
    // name to work with.
    std::string leaf = var;
    ExprNode *tree = ParsingExprList::GetExpressionTree(leaf);
    while (tree != NULL)
    {
        const set<string> &varLeaves = tree->GetVarLeaves();
        if (varLeaves.empty())
        {
            EXCEPTION1(ImproperUseException,
                       "After parsing, expression has no real variables.");
        }
        leaf = *varLeaves.begin();
        tree = ParsingExprList::GetExpressionTree(leaf);
    }

    // Start up the DataNetwork and add the database to it.
    workingNet = new DataNetwork;
    NetnodeDB *netDB = GetDBFromCache(filename, time, format.c_str());
    workingNet->SetNetDB(netDB);
    netDB->SetDBInfo(filename, leaf, time);

    // Put an ExpressionEvaluatorFilter right after the netDB to handle
    // expressions that come up the pipe.
    avtExpressionEvaluatorFilter *f = new avtExpressionEvaluatorFilter();
    NetnodeFilter *filt = new NetnodeFilter(f, "ExpressionEvaluator");
    filt->GetInputNodes().push_back(netDB);

    // Push the ExpressionEvaluator onto the working list.
    workingNetnodeList.push_back(filt);

    workingNet->AddNode(filt);
    // Push the variable name onto the name stack.
    nameStack.push_back(var);
    debug5 << "NetworkManager::AddDB: Adding " << var.c_str()
           << " to the name stack" << endl;

    // Set up the data spec.
    avtSILRestriction_p silr =
        new avtSILRestriction(workingNet->GetNetDB()->GetDB()->GetSIL(time), atts);
    avtDataSpecification *dspec = new avtDataSpecification(var.c_str(), time, silr);

    // Set up some options from the data specification
    dspec->SetNeedMixedVariableReconstruction(matopts.GetForceMIR());
    dspec->SetNeedSmoothMaterialInterfaces(matopts.GetSmoothing());
    dspec->SetNeedCleanZonesOnly(matopts.GetCleanZonesOnly());
    dspec->SetNeedValidFaceConnectivity(matopts.GetNeedValidConnectivity());
    dspec->SetUseNewMIRAlgorithm(matopts.GetUseNewMIRAlgorithm());
    workingNet->SetDataSpec(dspec);

    // The plot starts out as NULL.
    workingNet->SetPlot(NULL);
}

// ****************************************************************************
// Method: NetworkManager::DefineDB
//
// Purpose: 
//   Defines a new database with the specified name and list of files.
//
// Arguments:
//    dbName   : The database name.
//    files    : The list of files that make up the database.
//    time     : The timestep that we want to examine.
//    format   : The file format type of the DB.
//   
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 13:41:08 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed May 14 12:56:16 PDT 2003
//   I made it get the metadata and the SIL if the time is greater than 0.
//   This ensures that we pick up the metadata and SIL for the time state
//   that we're intersted in.
//
//   Hank Childs, Mon Jan  5 16:49:28 PST 2004
//   Account for databases in the cached lists that have been cleared.
//
//   Hank Childs, Mon Mar  1 08:48:26 PST 2004
//   Send the time to the database factory as well.
//
//   Hank Childs, Mon Mar 22 11:10:43 PST 2004
//   Send the file format type to the database factory.
//
//   Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//   Added code to pass avtDatabaseMetaData to LoadBalancer->AddDatabase
//
// ****************************************************************************
void
NetworkManager::DefineDB(const string &dbName, const string &dbPath,
    const stringVector &files, int time, const string &format)
{
    //
    // We gotta have a load balancer
    //
    if (loadBalancer == NULL)
    {
        debug1 << "Internal error: A load balancer was never registered."
               << endl;
        EXCEPTION0(ImproperUseException);
    }

    // Prepend the path to the files.
    stringVector filesWithPath;
    filesWithPath.reserve(files.size());
    int i;
    for(i = 0; i < files.size(); ++i)
        filesWithPath.push_back(dbPath + files[i]);

    //
    // If the database is in the virtualDatabases map, then check the
    // new list of files against the old list of files.
    //
    StringVectorMap::const_iterator it = virtualDatabases.find(dbName);
    if(it != virtualDatabases.end())
    {
        // If the list of files is the same, return because we don't
        // need to change the list of files. If the list of files differ
        // then replace the list of files.
        if(filesWithPath == it->second)
        {
            debug3 << "NetworkManager::DefineDB: No need to change "
                   << "virtual database definition for "
                   << dbName.c_str() << endl;

            // The definition is the same so we don't need to change it
            // but we have to make sure that it exists in the database
            // cache or else we'll have to define it. It's possible that
            // a clear cache rpc could have purged it from the database
            // cache.
            bool found = false;
            for(i = 0; i < databaseCache.size() && !found; ++i)
            {
                if (databaseCache[i] == NULL)
                    continue;
                found = (databaseCache[i]->GetFilename() == dbName);
            }

            // If the database was in the database cache, we can return.
            if(found)
                return;
        }
        else
        {
            debug3 << "NetworkManager::DefineDB: Changed "
                   << "virtual database definition for "
                   << dbName.c_str() << endl;
            virtualDatabases[dbName] = filesWithPath;
        }
    }

    //
    // Remove from the database cache databases that have the same name.
    //
    std::vector<NetnodeDB*> databaseBadMatches;
    for (i = 0; i < databaseCache.size(); i++)
    {
        if (databaseCache[i] == NULL)
            continue;
        if (databaseCache[i]->GetFilename() != dbName)
            databaseBadMatches.push_back(databaseCache[i]);
    }
    databaseCache = databaseBadMatches;

    const char **names = 0;
    TRY
    {
        // Open the database
        avtDatabase *db = NULL;
        const char *dbName_c = dbName.c_str();

        if (filesWithPath.size() > 0)
        {
            // Make an array of pointers that we can pass to the database
            // factory so we can create a database based on a virtual
            // file.
            names = new const char *[filesWithPath.size()];
            for(int i = 0; i < filesWithPath.size(); ++i)
                names[i] = filesWithPath[i].c_str();
            db = avtDatabaseFactory::FileList(names, filesWithPath.size(),
                                              time, format.c_str());
            delete [] names;
            names = 0;

            // Add the virtual database to the virtual database map.
            virtualDatabases[dbName] = filesWithPath;
            debug3 << "NetworkManager::DefineDB: Added new virtual database "
                   << "definition for " << dbName.c_str() << endl;
        }
        else if (dbName.substr(dbName.length() - 6) == ".visit")
            db = avtDatabaseFactory::VisitFile(dbName_c, time, format.c_str());
        else
            db = avtDatabaseFactory::FileList(&dbName_c, 1, time,
                                              format.c_str());

        // If we want to open the file at a later timestep, get the
        // metadata and the SIL so that it contains the right data.
        if ((time > 0) ||
            (!db->MetaDataIsInvariant()) || 
            (!db->SILIsInvariant()))
        {
            debug2 << "NetworkManager::DefineDB: We were instructed to define "
                       << dbName.c_str() << " at timestate=" << time
                       << " so we're reading the MetaData and SIL early."
                       << endl;
            db->GetMetaData(time);
            db->GetSIL(time);
        }

        // Create a netnode using the opened database and stick it in
        // the database cache.
        NetnodeDB *netDB = new NetnodeDB(db);
        netDB->SetDBInfo(dbName, "", time);
        databaseCache.push_back(netDB);

        // Add the database to the load balancer.
        const   avtIOInformation & ioinfo = db->GetIOInformation(time);
        const   avtDatabaseMetaData *md = db->GetMetaData(time);
        loadBalancer->AddDatabase(dbName, ioinfo, md);
    }
    CATCH(DatabaseException)
    {
        delete [] names;
        debug1 << "ERROR - could not define database " << dbName.c_str()
               << endl;
        RETHROW;
    }
    ENDTRY 
}

// ****************************************************************************
//  Method: NetworkManager::AddFilter
//
//  Purpose:
//      Create a new filter and add it to the network
//
//  Programmer: Jeremy Meredith
//  Creation:   February 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Sun Mar  4 16:58:13 PST 2001
//    Made it use (a new) PluginManager to create the operator.
//
//    Hank Childs, Thu Apr 12 15:34:22 PDT 2001
//    Added check to make sure that there were enough filters to compare to.
//
//    Jeremy Meredith, Thu Jul 26 03:39:21 PDT 2001
//    Added support for the new (real) operator plugin manager.
//
//    Jeremy Meredith, Fri Sep 28 13:32:56 PDT 2001
//    Made use of plugin manager keying off of ids instead of indexes.
//
//    Jeremy Meredith, Fri Nov  9 10:25:46 PST 2001
//    Added an error check, and renamed type to filtertype.
//
//    Sean Ahern, Tue May 28 08:52:48 PDT 2002
//    Added binary operators.
//
// ****************************************************************************
void
NetworkManager::AddFilter(const string &filtertype,
                          const AttributeGroup *atts,
                          const unsigned int nInputs)
{
    // Check that we have a network to work on.
    if (workingNet == NULL)
    {
        debug1 << "Adding a filter to a non-existent network." << endl;
        EXCEPTION0(ImproperUseException);
    }

    avtPluginFilter *f = OperatorPluginManager::Instance()->
        GetEnginePluginInfo(filtertype)->AllocAvtPluginFilter();
    f->SetAtts(atts);
    if (nInputs != 1)
    {
        debug1 << "Unable to handle filters with nInputs (" << nInputs << ") > 1"
            << endl;
        EXCEPTION0(ImproperUseException);
    }

    NetnodeFilter *filt = new NetnodeFilter(f, filtertype);
    std::vector<Netnode*> &filtInputs = filt->GetInputNodes();
    for (int i = 0; i < nInputs; i++)
    {
        // Pull a node off the working list and push it onto the filter
        // inputs.
        Netnode *n = workingNetnodeList.back();
        workingNetnodeList.pop_back();
        filtInputs.push_back(n);
    }
    // Push the filter onto the working list.
    workingNetnodeList.push_back(filt);
    workingNet->AddNode(filt);
}

// ****************************************************************************
//  Method:  NetworkManager::MakePlot
//
//  Purpose:
//    Create the plot for the network.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  1, 2001
//
//  Modifications:
//    Jeremy Meredith, Sun Mar  4 16:58:13 PST 2001
//    Made it use (a new) PluginManager to create the plot.
//
//    Jeremy Meredith, Tue Mar 20 12:05:50 PST 2001
//    Made it use the new plugin manager.  For now we do a reverse lookup
//    of the name to an index.
//
//    Jeremy Meredith, Fri Sep 28 13:32:56 PDT 2001
//    Made use of plugin manager keying off of ids instead of indexes.
//
//    Jeremy Meredith, Fri Nov  9 10:26:18 PST 2001
//    Allowed it to reuse plots so we can cache whole networks again.
//
//    Jeremy Meredith, Fri Feb  8 10:37:59 PST 2002
//    Made sure the total number of filters was the same when matching
//    networks (since when a plot is applied, no more filter can be).
//
//    Eric Brugger, Fri Mar 19 15:25:20 PST 2004
//    Modified the rpc to set the data extents in the engine.
//
// ****************************************************************************
void
NetworkManager::MakePlot(const string &id, const AttributeGroup *atts,
                         const vector<double> &dataExtents)
{
    if (workingNet == NULL)
    {
        debug1 << "Adding a plot to a non-existent network." << endl;
        EXCEPTION0(ImproperUseException);
    }

    if (*workingNet->GetPlot() != NULL)
    {
        debug1 << "Trying to stipulate plot type more than once." << endl;
        EXCEPTION0(ImproperUseException);
    }

    // Checking to see if the network has been built successfully.
    if (workingNetnodeList.size() != 1)
    {
        debug1 << "Network building still in progress.  Filter required to "
               << "absorb " << workingNetnodeList.size() << " nodes."  << endl;
        EXCEPTION0(ImproperUseException);
    }

    avtPlot *p = PlotPluginManager::Instance()->
                                GetEnginePluginInfo(id)->AllocAvtPlot();

    p->SetDataExtents(dataExtents);
    workingNet->SetPlot(p);
    workingNet->GetPlot()->SetAtts(atts);
    workingNet->SetPlottype(id);
}

// ****************************************************************************
//  Method:  NetworkManager::EndNetwork
//
//  Purpose:
//    Closes off the new network, assigns a pipeline specification, and a
//    network id.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  9, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Jan  5 16:04:57 PST 2004
//    Make sure that the network id is always unique for each network.
//
// ****************************************************************************
int
NetworkManager::EndNetwork(void)
{
    // Checking to see if the network has been built successfully.
    if (workingNetnodeList.size() != 1)
    {
        debug1 << "Network building still in progress.  Filter required to "
               << "absorb" << workingNetnodeList.size() << " nodes."  << endl;

        EXCEPTION0(ImproperUseException);
    }

    // set the pipeline specification
    int pipelineIndex =
        loadBalancer->AddPipeline(workingNet->GetNetDB()->GetFilename());
    avtPipelineSpecification_p pspec =
        new avtPipelineSpecification(workingNet->GetDataSpec(), pipelineIndex);
    workingNet->SetPipelineSpec(pspec);

    // Connect the workingNet to the workingNetnodeList.
    workingNet->SetTerminalNode(workingNetnodeList[0]);

    // Push the working net onto the network caches.
    workingNet->SetID(uniqueNetworkId++);
    networkCache.push_back(workingNet);
    globalCellCounts.push_back(-1);

    return workingNet->GetID();
}

// ****************************************************************************
//  Method:  NetworkManager::CancelNetwork
//
//  Purpose:
//    Terminates the current network in case of an error.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  6, 2003
//
// ****************************************************************************
void
NetworkManager::CancelNetwork(void)
{
    workingNet = NULL;
    workingNetnodeList.clear();
}

// ****************************************************************************
//  Method:  NetworkManager::UseNetwork
//
//  Purpose:
//    Allows a whole network to be reused automatically, without
//    needing to specify a database, filters, or plot.
//
//  Arguments:
//    id         the id of the network to reuse.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  9, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 21 11:41:59 PST 2001
//    Tell the load balancer to reset the pipeline if we re-use the network.
//
//    Hank Childs, Mon Jan  5 16:39:06 PST 2004
//    Make sure that the id requested hasn't already been cleared.
//
// ****************************************************************************
void
NetworkManager::UseNetwork(int id)
{
    if (workingNet)
    {
        debug1 << "Internal error: UseNetwork called with an open network" 
               << endl;
        EXCEPTION0(ImproperUseException);
    }

    if (id >= networkCache.size())
    {
        debug1 << "Internal error: asked to reuse network ID (" << id
            << " >= num saved networks (" << networkCache.size() << ")"
            << endl;
        EXCEPTION0(ImproperUseException);
    }

    // re-access the given network
    if (networkCache[id] == NULL)
    {
        debug1 << "Asked to use a network that was cleared out previously."
                << "  (presumably because a database was re-opened)." << endl;
        EXCEPTION0(ImproperUseException);
    }
 
    workingNet = networkCache[id];
    int pipelineIndex = workingNet->GetPipelineSpec()->GetPipelineIndex();
    loadBalancer->ResetPipeline(pipelineIndex);

    if (id != workingNet->GetID())
    {
        debug1 << "Internal error: network at position[" << id <<
            "] does not have same id (" << workingNet->GetID() << ")"
            << endl;
        EXCEPTION0(ImproperUseException);
    }
}

// ****************************************************************************
//  Method: NetworkManager::GetPlot
//
//  Purpose:
//      Gets the plot for the current working network 
//
//  Programmer: Mark C. Miller 
//  Creation:   November 19, 2002
//
// ****************************************************************************
avtPlot_p
NetworkManager::GetPlot(void)
{
    if (workingNet == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }

    return workingNet->GetPlot();
}

// ****************************************************************************
//  Method: NetworkManager::GetCurrentNetworkId
//
//  Purpose:
//      Gets the network id of the current network.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2002
//
// ****************************************************************************
int
NetworkManager::GetCurrentNetworkId(void)
{
    if (workingNet == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }

    return workingNet->GetID();
}

// ****************************************************************************
//  Method: NetworkManager::GetTotalGlobalCellCounts
//
//  Purpose:
//      Gets the network id of the current network.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2002
//
// ****************************************************************************
int
NetworkManager::GetTotalGlobalCellCounts(void) const
{
   int i, sum = 0;
   for (i = 0; i < globalCellCounts.size(); i++)
   {
      if (globalCellCounts[i] >= 0)
          sum += globalCellCounts[i];
   }
   return sum;
}

// ****************************************************************************
//  Method: NetworkManager::SetGlobalCellCount
//
//  Purpose:
//      Sets the global (over all processors) cell count for the given network 
//
//  Programmer: Mark C. Miller
//  Creation:   May 24, 2004 
//
// ****************************************************************************
void
NetworkManager::SetGlobalCellCount(int netId, int cellCount)
{
   globalCellCounts[netId] = cellCount;
   debug5 << "Setting cell count for network " << netId << " to " << cellCount << endl;
}

// ****************************************************************************
//  Method: NetworkManager::GetScalableThreshold
//
//  Purpose: Get the effective scalable threshold
//
//  Programmer: Mark C. Miller 
//  Creation:   May 11, 2004
//
// ****************************************************************************
int
NetworkManager::GetScalableThreshold(void) const
{
    int scalableAutoThreshold;
    RenderingAttributes::TriStateMode scalableActivationMode;

    scalableAutoThreshold =
        windowAttributes.GetRenderAtts().GetScalableAutoThreshold();
    scalableActivationMode = 
        windowAttributes.GetRenderAtts().GetScalableActivationMode();

    return RenderingAttributes::GetEffectiveScalableThreshold(
                                    scalableActivationMode,
                                    scalableAutoThreshold);
}

// ****************************************************************************
//  Method: NetworkManager::DoneWithNetwork
//
//  Purpose:
//      Indicates that we are done with a network -- we can clean up memory, 
//      etc.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Jan  5 16:39:06 PST 2004
//    Make sure that this isn't called on a previously cleared network.
//
//    Hank Childs, Thu Mar 18 16:06:56 PST 2004
//    If this plot is currently located in the vis window (for SR-mode), then
//    clear it out of the window before cleaning up the network.
//
//    Mark C. Miller, Wed Sep  8 17:06:25 PDT 2004
//    Moved the code to clear the vis window and plots currently in the 
//    vis window to inside the test for non-NULL networkCache
//
// ****************************************************************************
void
NetworkManager::DoneWithNetwork(int id)
{
    if (id >= networkCache.size())
    {
        debug1 << "Internal error: asked to reuse network ID (" << id
            << " >= num saved networks (" << networkCache.size() << ")"
            << endl;
        EXCEPTION0(ImproperUseException);
    }

    if (networkCache[id] != NULL)
    {

        for (int i = 0 ; i < plotsCurrentlyInWindow.size() ; i++)
        {
            if (plotsCurrentlyInWindow[i] == id)
            {
                viswin->ClearPlots();
                plotsCurrentlyInWindow.clear();
                break;
            }
        }

        networkCache[id]->ReleaseData();
        globalCellCounts[id] = -1;
    }
    else
    {
        debug1 << "Warning: DoneWithNetwork called on previously cleared "
               << "network." << endl;
    }
}

// ****************************************************************************
//  Method: NetworkManager::UpdatePlotAtts
//
//  Purpose:
//      Updates the plot attributes for a network.
//
//  Arguments:
//      id      The id of the network.
//      atts    The new attributes for the plot.
//
//  Programmer: Hank Childs
//  Creation:   November 30, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Jan  5 16:39:06 PST 2004
//    Make sure the network hasn't already been cleared.
//
// ****************************************************************************
void
NetworkManager::UpdatePlotAtts(int id, const AttributeGroup *atts)
{
    if (id >= networkCache.size())
    {
        debug1 << "Internal error: asked to reuse network ID (" << id
            << ") >= num saved networks (" << networkCache.size() << ")"
            << endl;
        EXCEPTION0(ImproperUseException);
    }

    if (networkCache[id] == NULL)
    {
        debug1 << "Asked to update the plot attributes of a network that has "
               << "already been cleared." << endl;
        EXCEPTION0(ImproperUseException);
    }
    if (networkCache[id]->GetID() != id)
    {
        debug1 << "Internal error: network at position[" << id
            << "] does not have " << "the same id ("
            << networkCache[id]->GetID() << ")" << endl;
        EXCEPTION0(ImproperUseException);
    }

    networkCache[id]->GetPlot()->SetAtts(atts);
}

// ****************************************************************************
//  Method: NetworkManager::GetOutput
//
//  Purpose:
//      Get the output of the network and close the network.
//      Also, flush LRU items from the cache
//
//  Programmer: Jeremy Meredith
//  Creation:   September 29, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Nov  2 14:10:51 PST 2000
//    Added another sanity check.
//
//    Hank Childs, Wed Jan 17 11:05:20 PST 2001
//    Added plots.
//
//    Jeremy Meredith, Thu Mar  1 13:41:13 PST 2001
//    Disable the section of code which allows re-using of entire networks.
//    This allows us to continue caching networks, but avoids the temporary
//    problem where different networks' plots might use different values
//    but we have not checked to see if the plots are equivalent.
//    Also....  I changed the way the networks are hooked up to allow for
//    zero-length pipelines.
//
//    Hank Childs, Wed Mar  7 11:33:51 PST 2001
//    Timestep now an argument to the database instead of set as a global.
//
//    Kathleen Bonnell, Tue May  1 16:57:02 PDT 2001 
//    Added try-catch block so that exceptions could be rethrown. 
//
//    Hank Childs, Tue Jun 12 14:55:38 PDT 2001
//    Incorporate data specifications.
//
//    Jeremy Meredith, Tue Jul  3 15:18:55 PDT 2001
//    Modified to better handle interruption of pipeline execution.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Jeremy Meredith, Fri Nov  9 10:29:17 PST 2001
//    Moved code to finish network matching into EndNetwork.
//    Added sending of window attributes into plot->Execute.
//
//    Kathleen Bonnell, Tue Nov 20 12:35:54 PST 2001
//    Set flag in dataSpec based on value of requireOriginalCells. 
//
//    Hank Childs, Tue Jun 18 16:49:01 PDT 2002
//    Set MayRequireZones before Executing.
//
//    Mark C. Miller, 10Nov02
//    Made changes to support scalable rendering
//
//    Brad Whitlock, Thu Jan 16 11:07:10 PDT 2003
//    I replaced the return in the TRY/CATCH block with CATCH_RETURN2 so that
//    fake exceptions work again.
//
//    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//    Modifed to use local GetScalableThreshold method
//
//    Mark C. Miller, Mon May 24 18:36:13 PDT 2004
//    Removed MPI_Allreduce. Moved code to check for scalable threshold being
//    exceeded to Engine::WriteData
//
//    Kathleen Bonnell, Wed Jun  2 09:48:29 PDT 2004 
//    Set MayRequireNodes. 
//
//    Mark C. Miller, Mon Aug 23 20:24:31 PDT 2004
//    Added cellCountMultiplier arg and call to get and set it
//
// ****************************************************************************
avtDataObjectWriter_p
NetworkManager::GetOutput(bool respondWithNullData, bool calledForRender,
    float *cellCountMultiplier)
{

    // Is the network complete?
    if (*(workingNet->GetPlot()) == NULL)
    {
        debug1 << "Never told what type of plot to use." << endl;
        EXCEPTION0(ImproperUseException);
    }

    TRY
    {
        // Hook up the network
        avtDataObject_p output = workingNet->GetOutput();

        workingNet->GetPipelineSpec()->GetDataSpecification()->
            SetMayRequireZones(requireOriginalCells); 
        workingNet->GetPipelineSpec()->GetDataSpecification()->
            SetMayRequireNodes(requireOriginalNodes); 

        avtDataObjectWriter_p writer = workingNet->GetWriter(output,
                                          workingNet->GetPipelineSpec(),
                                          &windowAttributes);

        // get the SR multiplier
        *cellCountMultiplier =
            workingNet->GetPlot()->GetCellCountMultiplierForSRThreshold();

        if (respondWithNullData)
        {
           // get the data object at the input to the writer
           avtDataObject_p writerInput = writer->GetInput();
           avtDataset_p origDataset;
           CopyTo(origDataset, writerInput);

           // make a copy of the dataset without the data
           avtDataset_p dummyDataset = new avtDataset(origDataset, true); 
           avtDataObject_p dummyDob;
           CopyTo(dummyDob, dummyDataset);
           writer = dummyDob->InstantiateWriter();
           writer->SetInput(dummyDob);
        }

        // Zero out the workingNet.  Remember that we've already pushed it
        // onto the cache in EndNetwork.
        workingNet = NULL;
        workingNetnodeList.clear();

        // return it
        CATCH_RETURN2(1, writer);
    }
    CATCHALL(...)
    {
        // Zero out the workingNet to ensure that the exception doesn't
        // cause a crash next time we try to build a network.  Remember
        // that we've already pushed it onto the cache in EndNetwork.
        workingNet = NULL;
        workingNetnodeList.clear();

        // rethrow
        RETHROW;
    }
    ENDTRY
}

// ****************************************************************************
//  Method:  NetworkManager::Render
//
//  Purpose: do a software scalable render
//
//  Programmer:  Mark C. Miller 
//  Creation:    08Apr03 
//
//  Modifications:
//
//    Hank Childs, Thu Mar 18 16:06:56 PST 2004
//    Do some bookkeeping for which plots are stored in the vis window already.
//    This allows us to not have to remove the plots and re-add them for each
//    render.
//
//    Mark C. Miller, Tue Mar 30 10:58:01 PST 2004
//    Added code to set image compositor's background color
//
//    Mark C. Miller, Thu Apr  1 11:06:09 PST 2004
//    Removed call to AdjustWindowAttributes
//    Added use of viewported screen capture
//    
//    Mark C. Miller, Fri Apr  2 11:06:09 PST 2004
//    Removed call to FullFrameOff
//
//    Mark C. Miller, Tue Apr 20 07:44:34 PDT 2004
//    Added code to issue a warning if a plot's actor has no data
//
//    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//    Added call to local GetScalableThreshold method
//
//    Mark C. Miller, Tue May 25 20:44:10 PDT 2004
//    Added code to pass annotationObjectList in SetAnnotationAttributes
//
//    Mark C. Miller, Thu May 27 11:05:15 PDT 2004
//    Removed window attributes arg from GetActor method
//    Added code to push colors into all plots
//    Made triangle count a debug5 statement (from debug1)
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added visualCueList arg to SetAnnotationAttributes
//
//    Mark C. Miller, Wed Jul  7 11:42:09 PDT 2004
//    Totally re-organized. Added calls to reduce cell counts for newly
//    added plots. Moved test for exceeding scalable threshold to after point
//    where each plot's network output is actually computed.
//
//    Mark C. Miller, Mon Jul 26 15:08:39 PDT 2004
//    Added code to post process the composited image when the engine
//    is doing more than just 3D annotations. Fixed bug in determination of
//    viewportedMode bool. Added code to pass frame and state info to
//    set Win/Annot atts.
//
//    Mark C. Miller, Wed Aug 11 23:42:18 PDT 2004
//    Added code to get cell count multiplier for SR mode and adjust
//    cell counts for SR threshold
//
//    Mark C. Miller, Mon Aug 23 20:24:31 PDT 2004
//    Added arg to GetOutput call
//
//    Chris Wojtan, Fri Jul 30 10:32:02 PDT 2004
//    Switched from single pass rendering to 2 pass randering, in order to
//    correctly draw plots with both opaque and translucent geometry
//
//    Chris Wojtan, Fri Jul 30 14:37:50 PDT 2004
//    Load the output of the first rendering pass into the input of
//    the second pass.
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Changed bool arg for 3D annots to an integer mode
//
//    Mark C. Miller, Tue Oct 19 20:18:22 PDT 2004
//    Added code to push color table name to plots
//    Added code to use correct whole image compositor based upon window mode
//
//    Jeremy Meredith, Thu Oct 21 17:33:09 PDT 2004
//    Finished the two-pass mode.  Cleaned things up a bit.
//    Refactored image writing to new functions.  Wrote the tiled
//    image compositor.
//
// ****************************************************************************
avtDataObjectWriter_p
NetworkManager::Render(intVector plotIds, bool getZBuffer, int annotMode)
{
    int i;
    DataNetwork *origWorkingNet = workingNet;


    TRY
    {
        int t1 = visitTimer->StartTimer();
        avtDataObjectWriter_p writer;
        bool needToSetUpWindowContents = false;
        int *cellCounts = new int[2 * plotIds.size()];

        // put all the plot objects into the VisWindow
        viswin->SetScalableRendering(false);

        //
        // Determine if the plots currently in the window are the same as
        // those we were asked to render.
        //
        if (plotIds.size() != plotsCurrentlyInWindow.size())
            needToSetUpWindowContents = true;
        else
        {
            for (int p = 0 ; p < plotIds.size() ; p++)
                if (plotIds[p] != plotsCurrentlyInWindow[p])
                    needToSetUpWindowContents = true;
        }

        if (needToSetUpWindowContents)
        {
            // array to record cell counts of added networks

            int t2 = visitTimer->StartTimer();
            int t3 = visitTimer->StartTimer();
            viswin->ClearPlots();
            visitTimer->StopTimer(t3, "Clearing plots out of vis window");

            for (i = 0; i < plotIds.size(); i++)
            {
                int t7 = visitTimer->StartTimer();
                // get the network output as we would normally
                workingNet = NULL;
                UseNetwork(plotIds[i]);
                float cellCountMultiplier;

                DataNetwork *workingNetSaved = workingNet;
                int t4 = visitTimer->StartTimer();
                avtDataObjectWriter_p tmpWriter = GetOutput(false, true,
                                                            &cellCountMultiplier);
                avtDataObject_p dob = tmpWriter->GetInput();

                // merge polygon info output across processors 
                dob->GetInfo().ParallelMerge(tmpWriter);
                visitTimer->StopTimer(t4, "Merging data info in parallel");

                int t5 = visitTimer->StartTimer();
                avtActor_p anActor = workingNetSaved->GetActor(dob);
                visitTimer->StopTimer(t5, "Calling GetActor for DOB");

                // record cell counts including and not including polys
                cellCounts[i] =
                    (int) (anActor->GetDataObject()->GetNumberOfCells(false) *
                                                     cellCountMultiplier);
                cellCounts[i+plotIds.size()] =
                    anActor->GetDataObject()->GetNumberOfCells(true);

                int t6 = visitTimer->StartTimer();
                viswin->AddPlot(anActor);
                visitTimer->StopTimer(t6, "Adding plot to the vis window");
                visitTimer->StopTimer(t7, "Setting up one plot");
            }

            //
            // Update any cell counts for the associated networks.
            // This involves global communication. Since we're going to
            // get sync'd up for the composite below, this
            // additional MPI_Allreduce is not so bad.
            // While we're at it, we'll issue any warning message for
            // plots with no data, too.
            //
#ifdef PARALLEL
            int *reducedCounts = new int[2 * plotIds.size()];
            MPI_Allreduce(cellCounts, reducedCounts, 2 * plotIds.size(),
                MPI_INT, MPI_SUM, MPI_COMM_WORLD);
            for (i = 0; i < 2 * plotIds.size(); i++)
                cellCounts[i] = reducedCounts[i];
            delete [] reducedCounts;
#endif

            // update the global cell counts for each network
            for (i = 0; i < plotIds.size(); i++)
                SetGlobalCellCount(plotIds[i], cellCounts[i+plotIds.size()]);

            plotsCurrentlyInWindow = plotIds;
            visitTimer->StopTimer(t2, "Setting up window contents");
        }

        int  scalableThreshold = GetScalableThreshold(); 
        // scalable threshold test (the 0.5 is to add some hysteresus to avoid 
        // the misfortune of oscillating switching of modes around the threshold)
        if (GetTotalGlobalCellCounts() < 0.5 * scalableThreshold)
        {
            debug5 << "Cell count has fallen below SR threshold. Sending the "
                      "AVT_NULL_IMAGE_MSG data object to viewer" << endl;

            avtNullData_p nullData = new avtNullData(NULL,AVT_NULL_IMAGE_MSG);
            avtDataObject_p dummyDob;
            CopyTo(dummyDob, nullData);
            writer = dummyDob->InstantiateWriter();
            writer->SetInput(dummyDob);
        }
        else
        {
            if (needToSetUpWindowContents)
            {
                // determine any networks with no data 
                vector<int> networksWithNoData;
                for (i = 0; i < plotIds.size(); i++)
                {
                    if (cellCounts[i] == 0)
                        networksWithNoData.push_back(i);
                }

                // issue warning messages for plots with no data
                if (networksWithNoData.size() > 0)
                {
                    string msg = "The plot(s) with id(s) = ";
                    for (i = 0; i < networksWithNoData.size(); i++)
                    {
                        char tmpStr[32];
                        SNPRINTF(tmpStr, sizeof(tmpStr), "%d", networksWithNoData[i]);
                        msg += tmpStr;
                        if (i < networksWithNoData.size() - 1)
                            msg += ", ";
                    }
                    msg += " yielded no data";
#ifdef PARALLEL
                    if (PAR_Rank() == 0)
#endif
                    {
                        avtCallback::IssueWarning(msg.c_str());
                    }
                }
            }

            //
            // Update plot's bg/fg colors. Ignored by avtPlot objects if colors
            // are unchanged
            //
            {
                double bg[4] = {1.,0.,0.,0.};
                double fg[4] = {0.,0.,1.,0.};

                annotationAttributes.GetForegroundColor().GetRgba(fg);
                annotationAttributes.GetDiscernibleBackgroundColor().GetRgba(bg);
                for (int i = 0; i < plotIds.size(); i++)
                {
                    workingNet = NULL;
                    UseNetwork(plotIds[i]);
                    workingNet->GetPlot()->SetBackgroundColor(bg);
                    workingNet->GetPlot()->SetForegroundColor(fg);
                    if (changedCtName != "")
                        workingNet->GetPlot()->SetColorTable(changedCtName.c_str());
                    workingNet = NULL;
                }
            }

            //
            // Add annotations if necessary 
            //
            SetAnnotationAttributes(annotationAttributes,
                                    annotationObjectList, visualCueList,
                                    frameAndState, annotMode);

            debug5 << "Rendering " << viswin->GetNumTriangles() << " triangles. " 
                   << "Balanced speedup = " << RenderBalance(viswin->GetNumTriangles())
                   << "x" << endl;

            //
            // Determine if we need to go for two passes
            //
            bool two_pass_mode = false;
#ifdef PARALLEL
            if (viswin->GetWindowMode() == WINMODE_3D)
            {
                two_pass_mode = viswin->TransparenciesExist();
                two_pass_mode = UnifyMaximumValue(two_pass_mode);
            }
#endif

            // render the image and capture it. Relies upon explicit render
            int t3 = visitTimer->StartTimer();
            bool viewportedMode = (annotMode != 1) || 
                                  (viswin->GetWindowMode() == WINMODE_2D) ||
                                  (viswin->GetWindowMode() == WINMODE_CURVE);


            // ************************************************************
            // FIRST PASS - Opaque only
            // ************************************************************

            avtImage_p theImage;
            if (two_pass_mode)
                theImage=viswin->ScreenCapture(viewportedMode,true,true,false);
            else
                theImage=viswin->ScreenCapture(viewportedMode,true);
            
            visitTimer->StopTimer(t3, "Screen capture for SR");

            if (dumpRenders)
                DumpImage(theImage, "before_ImageCompositer");


            avtWholeImageCompositer *imageCompositer;
            if (viswin->GetWindowMode() == WINMODE_3D)
            {
                imageCompositer = new avtWholeImageCompositerWithZ();
                imageCompositer->SetShouldOutputZBuffer(getZBuffer);
            }
            else
            {
                imageCompositer = new avtWholeImageCompositerNoZ();
            }

            //
            // Set the compositer's background color
            //
            const float *fbg = viswin->GetBackgroundColor();
            unsigned char bg_r = (unsigned char) (fbg[0] * 255.0);
            unsigned char bg_g = (unsigned char) (fbg[1] * 255.0);
            unsigned char bg_b = (unsigned char) (fbg[2] * 255.0);
            imageCompositer->SetBackground(bg_r, bg_g, bg_b);

            //
            // Set up the input image size and add it to compositer's input
            //
            int imageRows, imageCols;
            theImage->GetSize(&imageCols, &imageRows);
            imageCompositer->SetOutputImageSize(imageRows, imageCols);
            imageCompositer->AddImageInput(theImage, 0, 0);
            imageCompositer->SetShouldOutputZBuffer(getZBuffer || two_pass_mode);
            imageCompositer->SetAllProcessorsNeedResult(two_pass_mode);

            //
            // Do the parallel composite using a 1 stage pipeline
            //
            imageCompositer->Execute();
            avtDataObject_p compositedImageAsDataObject = imageCompositer->GetOutput();

            // Dump the composited image when debugging.  Note that we only
            // want all processors to dump it if we have done an Allreduce
            // in the compositor, and this only happens in two pass mode.
            if (dumpRenders)
                DumpImage(compositedImageAsDataObject,
                          "after_first_ImageCompositer", two_pass_mode);


            // ************************************************************
            // SECOND PASS - Translucent only
            // ************************************************************

            if (two_pass_mode)
            {
                int t1 = visitTimer->StartTimer();

                avtImage_p theImage2;
                theImage2=viswin->ScreenCapture(viewportedMode,true,false,true,
                                            imageCompositer->GetTypedOutput());
            
                visitTimer->StopTimer(t1, "Second-pass screen capture for SR");

                if (dumpRenders)
                    DumpImage(theImage2, "before_second_ImageCompositer");

                avtTiledImageCompositor imageCompositer2;

                //
                // Set up the input image size and add it to compositer's input
                //
                theImage2->GetSize(&imageCols, &imageRows);
                imageCompositer2.SetOutputImageSize(imageRows, imageCols);
                imageCompositer2.AddImageInput(theImage2, 0, 0);

                //
                // Do the parallel composite using a 1 stage pipeline
                //
                int t2 = visitTimer->StartTimer();
                imageCompositer2.Execute();
                compositedImageAsDataObject = imageCompositer2.GetOutput();
                visitTimer->StopTimer(t2, "tiled image compositor execute");
            }

            //
            // If the engine is doing more than just 3D attributes,
            // post-process the composited image.
            //
#ifdef PARALLEL
            if ((annotMode==2) && ((PAR_Rank() == 0) || getZBuffer))
#else
            if (annotMode==2)
#endif
            {
                avtImage_p compositedImage;
                CopyTo(compositedImage, compositedImageAsDataObject);
                avtImage_p postProcessedImage = 
                    viswin->PostProcessScreenCapture(compositedImage,
                                                     viewportedMode,
                                                     getZBuffer);
                CopyTo(compositedImageAsDataObject, postProcessedImage);
            }
            writer = compositedImageAsDataObject->InstantiateWriter();
            writer->SetInput(compositedImageAsDataObject);


            if (dumpRenders)
                DumpImage(compositedImageAsDataObject,
                          "after_ImageCompositer", false);

        }
        
        delete [] cellCounts;

        // return it
        visitTimer->StopTimer(t1, "Total time for NetworkManager::Render");
        CATCH_RETURN2(1, writer);

    }
    CATCHALL(...)
    {
        // rethrow
        RETHROW;
    }
    ENDTRY

    workingNet = origWorkingNet;
}

// ****************************************************************************
//  Method:  NetworkManager::SetWindowAttributes
//
//  Purpose:
//    Set the window attributes for the next plot execution.
//
//  Arguments:
//    atts       the new window attributes
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  9, 2001
//
//  Modifications:
//
//    Mark C. Miller, Mon Dec  9 17:19:02 PST 2002
//    added calls to set various parameters on the VisWindow object
//
//    Eric Brugger, Tue Jun 10 15:35:20 PDT 2003
//    I renamed camera to view normal in the view attributes.
//
//    Brad Whitlock, Tue Jul 1 14:19:20 PST 2003
//    I changed the code that converts ViewAttributes to avtView3D so it
//    uses convenience methods so we don't have to trapse all over the code
//    when the ViewAttributes change field names.
//
//    Eric Brugger, Wed Aug 20 13:53:02 PDT 2003
//    I modified the setting of the view information to set curve, 2d and
//    3d views.
//
//    Hank Childs, Sat Nov 15 14:59:47 PST 2003
//    Make sure vis window gets specular options.
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added argument for extents type string and code to set extents type
//    on the VisWindow
//
//    Hank Childs, Sun May  9 16:16:59 PDT 2004
//    Do not turn on display lists on the engine, since they will just eat up
//    extra memory.
//
//    Mark C. Miller, Tue May 25 16:42:09 PDT 2004
//    Added code to set the color tables
//
//    Mark C. Miller, Tue Jul 13 17:53:19 PDT 2004
//    Added call to UpdateView after resizing the window.
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Added code to deal with view extents
//
// ****************************************************************************
void
NetworkManager::SetWindowAttributes(const WindowAttributes &atts,
                                    const std::string& extstr,
                                    const double *vexts,
                                    const std::string& ctName)
{

    // do nothing if nothing changed
    if ((windowAttributes == atts) && (extentTypeString == extstr) &&
        (changedCtName == ctName))
    {
        bool extsAreDifferent = false;
        static float curexts[6];
        viswin->GetBounds(curexts);
        for (int i = 0; i < 6; i ++)
        {
            if (curexts[i] != (float) vexts[i])
            {
                extsAreDifferent = true;
                break;
            }
        }

        if (extsAreDifferent == false)
           return;
    }

    avtExtentType extType = AVT_UNKNOWN_EXTENT_TYPE;
    avtExtentType_FromString(extstr, extType);
    viswin->SetViewExtentsType(extType);
    float fexts[6];
    for (int i = 0; i < 6; i ++)
        fexts[i] = vexts[i];
    viswin->SetBounds(fexts);

    // only update size if its different
    int s0,s1;
    viswin->GetSize(s0,s1);
    if ((s0 != atts.GetSize()[0]) || (s1 != atts.GetSize()[1]))
    {
       viswin->SetSize(atts.GetSize()[0], atts.GetSize()[1]);
       viswin->UpdateView();
    }

    //
    // Set the view information.
    //
    const ViewCurveAttributes& viewCurveAtts = atts.GetViewCurve();
    avtViewCurve viewCurve;
    viewCurve.SetFromViewCurveAttributes(&viewCurveAtts);
    viswin->SetViewCurve(viewCurve);

    const View2DAttributes& view2DAtts = atts.GetView2D();
    avtView2D view2D;
    view2D.SetFromView2DAttributes(&view2DAtts);
    viswin->SetView2D(view2D);

    const View3DAttributes& view3DAtts = atts.GetView3D();
    avtView3D view3D;
    view3D.SetFromView3DAttributes(&view3DAtts);
    viswin->SetView3D(view3D);

    //
    // Set the color tables
    //
    avtColorTables::Instance()->SetColorTables(atts.GetColorTables());

    //
    // Set the lights.
    //
    const LightList& lights = atts.GetLights();
    viswin->SetLightList(&lights);

    viswin->SetSpecularProperties(atts.GetRenderAtts().GetSpecularFlag(),
                                  atts.GetRenderAtts().GetSpecularCoeff(),
                                  atts.GetRenderAtts().GetSpecularPower(),
                                  atts.GetRenderAtts().GetSpecularColor());

    //
    // Set the background/foreground colors
    //
    viswin->SetBackgroundColor(atts.GetBackground()[0]/255.0,
                               atts.GetBackground()[1]/255.0,
                               atts.GetBackground()[2]/255.0);
    viswin->SetForegroundColor(atts.GetForeground()[0]/255.0,
                               atts.GetForeground()[1]/255.0,
                               atts.GetForeground()[2]/255.0);

    //
    // Set the bacbround mode and gradient colors if necessary
    //
    int bgMode = atts.GetBackgroundMode();
    if (bgMode == 0)
       viswin->SetBackgroundMode(0);
    else
    {
       viswin->SetBackgroundMode(1);
       viswin->SetGradientBackgroundColors(bgMode-1, atts.GetGradBG1()[0],
                                                     atts.GetGradBG1()[1],
                                                     atts.GetGradBG1()[2],
                                                     atts.GetGradBG2()[0],
                                                     atts.GetGradBG2()[1],
                                                     atts.GetGradBG2()[2]);
    }

    if (viswin->GetAntialiasing() != atts.GetRenderAtts().GetAntialiasing())
       viswin->SetAntialiasing(atts.GetRenderAtts().GetAntialiasing());
    if (viswin->GetSurfaceRepresentation() != atts.GetRenderAtts().GetGeometryRepresentation())
       viswin->SetSurfaceRepresentation(atts.GetRenderAtts().GetGeometryRepresentation());
    viswin->SetDisplayListMode(0);  // never

    windowAttributes = atts;
    extentTypeString = extstr;
    changedCtName    = ctName;

}

// ****************************************************************************
//  Method:  NetworkManager::SetAnnotationAttributes
//
//  Purpose:
//    Set the annotation attributes for the engine's viswin.  Ordinarily, only
//    those annotations that live in a 3D world are rendered on the engine.
//    However, when the engine is used to render an image for a save window
//    in non-screen-capture mode, then the engine has to render all annotations.
//    Regardless, only processor 0 does annotation rendering work.
//
//  Programmer:  Mark C. Miller 
//  Creation:    15Jul03 
//
//  Modifications:
//    Mark C. Miller, Mon Mar 29 14:36:46 PST 2004
//    Added bool arg for 3D annotations only
//
//    Mark C. Miller, Tue May 25 20:44:10 PDT 2004
//    Added arg for annotation object list
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added code to deal with VisualCueList
//
//    Mark C. Miller, Tue Jul 27 15:11:11 PDT 2004
//    Added code to deal with frame and state in VisWindow for
//    AnnotationObjects
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Changed bool flag for 3D annotations to an integer mode
// ****************************************************************************
void
NetworkManager::SetAnnotationAttributes(const AnnotationAttributes &atts,
    const AnnotationObjectList &aolist, const VisualCueList &visCues,
    const int *fns, int annotMode)
{
   int i;

#ifdef PARALLEL
   if (PAR_Rank() == 0)
#endif
   {
      // copy the attributes and disable all non-3D attributes 
      AnnotationAttributes newAtts = atts;

      switch (annotMode)
      {
          case 0: // no annotations

              newAtts.SetUserInfoFlag(false);
              newAtts.SetDatabaseInfoFlag(false);
              newAtts.SetLegendInfoFlag(false);
              newAtts.SetTriadFlag(false);
              newAtts.SetBboxFlag(false);
              newAtts.SetAxesFlag(false);
              newAtts.SetAxesFlag2D(false);
              viswin->DeleteAllAnnotationObjects();
              break;

          case 1: // 3D annotations only

              newAtts.SetUserInfoFlag(false);
              newAtts.SetDatabaseInfoFlag(false);
              newAtts.SetLegendInfoFlag(false);
              newAtts.SetTriadFlag(false);
              newAtts.SetAxesFlag2D(false);
              viswin->DeleteAllAnnotationObjects();
              break;

          case 2: // all annotations

              viswin->DeleteAllAnnotationObjects();
              viswin->CreateAnnotationObjectsFromList(aolist);
              viswin->SetFrameAndState(fns[0],
                                       fns[1],fns[2],fns[3],
                                       fns[4],fns[5],fns[6]);
              break;

          default:

              EXCEPTION0(ImproperUseException);
              break;
      }

      viswin->SetAnnotationAtts(&newAtts);

      //
      // Set up all the visual cues (which are 3D annotations)
      //
      if ((visCues != visualCueList) && (annotMode != 0))
      {
          viswin->ClearPickPoints();
          viswin->ClearRefLines();
          for (i = 0; i < visCues.GetNumVisualCueInfos(); i++)
          {
              const VisualCueInfo& cue = visCues.GetVisualCueInfo(i);
              switch (cue.GetCueType())
              {
                  case VisualCueInfo::PickPoint:
                      viswin->QueryIsValid(&cue, NULL);
                      break;
                  case VisualCueInfo::RefLine:
                      viswin->QueryIsValid(NULL, &cue);
                      break;
                  default:
                      break;
              }
          }
      }
   }

   annotationAttributes = atts;
   annotationObjectList = aolist;
   visualCueList = visCues;
   for (i = 0; i < sizeof(frameAndState)/sizeof(frameAndState[0]); i++)
       frameAndState[i] = fns[i];

}

// ****************************************************************************
//  Method:  NetworkManager::StartPick
//
//  Purpose:
//    Set the zone numbers flag in data specification so that original cell
//    numbers are propagated through the pipeline.  
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 19, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun  2 09:48:29 PDT 2004
//    Added bool indicating whether to request originalCells or originalNodes.
//
// ****************************************************************************
void
NetworkManager::StartPickMode(const bool forZones)
{
    if (forZones)
        requireOriginalCells = true;
    else 
        requireOriginalNodes = true;
}

// ****************************************************************************
//  Method:  Network::StopPick
//
//  Purpose:
//    Set the zone numbers flag in data specification so that original cell
//    numbers will no longer be propagated through the pipeline.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 19, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun  2 09:48:29 PDT 2004
//    Turn off requireOriginalNodes.
// ****************************************************************************
void
NetworkManager::StopPickMode(void)
{
    requireOriginalCells = false;
    requireOriginalNodes = false;
}

// ****************************************************************************
//  Method:  Network::Pick
//
//  Purpose:
//    Performs a query on the database with the PickAttributes.
//
//  Arguments:
//    id         The network to use.
//    pa         The pick attributes.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 20, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Use LocateCellQuery and PickQuery, instead of Query call to database.
//    This allows correct distribution of work in parallel.
//
//    Kathleen Bonnell, Fri Jan 31 09:36:54 PST 2003   
//    Make call to LocateCellQuery unconditional.  Put all code in 
//    one try-catch block. 
//    
//    Kathleen Bonnell, Wed Jun 25 13:45:04 PDT 2003  
//    Reflect new pickAtts naming convention:  ZoneNumber is now ElementNumber. 
//
//    Kathleen Bonnell, Fri Oct 10 10:44:52 PDT 2003 
//    Set ElementType in QueryAtts, SetNodePoint in PickAtts from QueryAtts.
//
//    Kathleen Bonnell, Wed Nov  5 17:04:53 PST 2003 
//    avtLocateCellQuery now uses PickAtts internally instead of QueryAtts.
//    QueryAtts are created only to fulfill the api requirements of the
//    PerformQuery method.
//
//    Kathleen Bonnell, Tue Dec  2 18:08:34 PST 2003 
//    Don't use avtLocateCellQuery if domain and element are already provided. 
//
//    Kathleen Bonnell, Tue Dec  2 17:36:44 PST 2003 
//    Use a special query if the pick type is Curve.
//    
//    Hank Childs, Mon Jan  5 16:39:06 PST 2004
//    Make sure the network hasn't already been cleared.
//
//    Kathleen Bonnell, Mon Mar  8 08:01:48 PST 2004 
//    Send the SILRestriction's UseSet to PickQuery if it is available. 
//    Also send the Transform.
//
//    Kathleen Bonnell, Tue May  4 14:35:08 PDT 2004 
//    Send the SILRestriction to PickQuery (insted of UseSet).
//    If LocateCellQuery fails, set error condition in PickAtts.
//
//    Kathleen Bonnell, Wed May  5 13:07:12 PDT 2004 
//    Moved error-setting code to PickQuery, as it causes problems in parallel.
//
//    Kathleen Bonnell, Wed Jun  2 09:48:29 PDT 2004
//    Add support for new pick classes. 
//
//    Kathleen Bonnell, Thu Aug 26 11:18:47 PDT 2004 
//    Skip LocateQuery if picking on 2d boundary or contour plots. 
//
//    Kathleen Bonnell, Mon Aug 30 17:51:56 PDT 2004 
//    Send SkipLocate flag to pick query.
//
//    Kathleen Bonnell, Thu Oct  7 10:29:36 PDT 2004 
//    Added timing code for each PerformQuery. 
//
// ****************************************************************************

void
NetworkManager::Pick(const int id, PickAttributes *pa)
{
    if (id >= networkCache.size())
    {
        debug1 << "Internal error:  asked to use network ID (" << id << ") >= "
               << "num saved networks (" << networkCache.size() << ")" << endl;
        EXCEPTION0(ImproperUseException);
    }

    if (networkCache[id] == NULL)
    {
        debug1 << "Asked to pick on a network that has already been cleared."
               << endl;
        EXCEPTION0(ImproperUseException);
    }

    if (id != networkCache[id]->GetID())
    {
        debug1 << "Internal error: network at position[" << id << "] "
               << "does not have same id (" << networkCache[id]->GetID()
               << ")" << endl;
        EXCEPTION0(ImproperUseException);
    }

    avtDataObject_p queryInput = 
        networkCache[id]->GetPlot()->GetIntermediateDataObject();

    avtSILRestriction_p silr = networkCache[id]->GetDataSpec()->GetRestriction();

    if (*queryInput == NULL)
    {
        debug1 << "Could not retrieve query input." << endl;
        EXCEPTION0(NoInputException);
    }

    avtDataAttributes &queryInputAtts = queryInput->GetInfo().GetAttributes();
    avtDataValidity   &queryInputVal  = queryInput->GetInfo().GetValidity();
    bool skipLocate = queryInputAtts.GetTopologicalDimension() == 1 &&
                      queryInputAtts.GetSpatialDimension() == 2;
    avtLocateQuery *lQ = NULL;
    avtPickQuery *pQ = NULL;
    avtCurvePickQuery *cpQ = NULL;
    TRY
    {
        QueryAttributes qa;
        if (pa->GetPickType() != PickAttributes::CurveNode &&
            pa->GetPickType() != PickAttributes::CurveZone)
        {
            if (skipLocate) // picking lines, set up pick atts appropriately
            {
                float *pt = pa->GetRayPoint1();
                pt[2] = 0.;
                pa->SetRayPoint1(pt);
                pa->SetRayPoint2(pt);
                pa->SetPickPoint(pt);
                pa->SetCellPoint(pt);
            }
            if (pa->GetPickType() == PickAttributes::Zone)
            {
                pQ = new avtZonePickQuery;
                if (!skipLocate)
                {
                    lQ = new avtLocateCellQuery;
                    lQ->SetInput(queryInput);
                    lQ->SetPickAtts(pa);
                    int queryTimer = visitTimer->StartTimer();
                    lQ->PerformQuery(&qa); 
                    visitTimer->StopTimer(queryTimer, lQ->GetType());
                    *pa = *(lQ->GetPickAtts());
                    delete lQ;
                }
            }
            else if (pa->GetPickType() == PickAttributes::Node)
            {
                pQ = new avtNodePickQuery;
                if (!skipLocate)
                {
                    lQ = new avtLocateNodeQuery;
                    lQ->SetInput(queryInput);
                    lQ->SetPickAtts(pa);
                    int queryTimer = visitTimer->StartTimer();
                    lQ->PerformQuery(&qa); 
                    visitTimer->StopTimer(queryTimer, lQ->GetType());
                    *pa = *(lQ->GetPickAtts());
                    delete lQ;
                }
            }
            else if (pa->GetPickType() == PickAttributes::DomainNode)
            {
                pQ = new avtPickByNodeQuery;
            }
            else if (pa->GetPickType() == PickAttributes::DomainZone)
            {
                pQ = new avtPickByZoneQuery;
            }
            if (queryInputAtts.HasInvTransform() &&
                queryInputAtts.GetCanUseInvTransform())
            {
                pQ->SetInvTransform(queryInputAtts.GetInvTransform());
            }
            if (queryInputAtts.HasTransform() &&
                queryInputAtts.GetCanUseTransform())
            {
                pQ->SetTransform(queryInputAtts.GetTransform());
            }
            
            if (*silr != NULL)
            {
                pQ->SetSILRestriction(silr->MakeAttributes());
            }
            pQ->SetNeedTransform(queryInputVal.GetPointsWereTransformed());

            pQ->SetInput(networkCache[id]->GetNetDB()->GetOutput());
            pQ->SetPickAtts(pa);
            pQ->SetSkippedLocate(skipLocate);
            int queryTimer = visitTimer->StartTimer();
            pQ->PerformQuery(&qa); 
            visitTimer->StopTimer(queryTimer, pQ->GetType());
            *pa = *(pQ->GetPickAtts());

            delete pQ;

            if (pa->GetNeedActualCoords())
            {
                avtActualCoordsQuery *acq = NULL;
                if (pa->GetPickType() == PickAttributes::DomainNode)
                    acq = new avtActualNodeCoordsQuery;
                else if (pa->GetPickType() == PickAttributes::DomainZone)
                    acq = new avtActualZoneCoordsQuery;
                if (acq != NULL)
                {
                    acq->SetInput(queryInput);
                    acq->SetPickAtts(pa);
                    int queryTimer = visitTimer->StartTimer();
                    acq->PerformQuery(&qa);
                    visitTimer->StopTimer(queryTimer, acq->GetType());
                    *pa = *(acq->GetPickAtts());
                    delete acq;
                }
            }
        }
        else 
        {
            cpQ = new avtCurvePickQuery;
            cpQ->SetInput(queryInput);
            cpQ->SetPickAtts(pa);
            int queryTimer = visitTimer->StartTimer();
            cpQ->PerformQuery(&qa); 
            visitTimer->StopTimer(queryTimer, cpQ->GetType());
            *pa = *(cpQ->GetPickAtts());
            delete cpQ;
        }
        visitTimer->DumpTimings();
    }
    CATCHALL( ... )
    {
        if (lQ != NULL)
            delete lQ;
        if (pQ != NULL)
            delete pQ;
        if (cpQ != NULL)
            delete cpQ;
        RETHROW;
    }
    ENDTRY
}

// ****************************************************************************
//  Method:  Network::Query
//
//  Purpose:
//    Performs a query on the database. 
//
//  Arguments:
//    id         The network to use.
//    queryname  The name of the query to perform. 
//
//  Programmer:  Kathleen Bonnell
//  Creation:    September 16, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002   
//    Removed unnecessary debug, cerr statements.  Added Exception if
//    query input could not be retrieved.
//
//    Kathleen Bonnell, Mon Nov 18 09:42:12 PST 2002 
//    Add Eulerian query. 
//    
//    Hank Childs, Tue Mar 18 21:36:26 PST 2003
//    Added Total Revolved Surface Area query.
//
//    Jeremy Meredith, Sat Apr 12 11:31:04 PDT 2003
//    Added compactness queries.
//
//    Kathleen Bonnell, Wed Jul 23 15:34:11 PDT 2003 
//    Add Variable query. 
//
//    Hank Childs, Thu Oct  2 09:47:48 PDT 2003
//    Allow queries to involve multiple networks.  Add L2Norm query, more.
//
//    Kathleen Bonnell, Wed Oct 29 16:06:23 PST 2003 
//    Add PlotMinMax query.
//
//    Hank Childs, Mon Jan  5 16:39:06 PST 2004
//    Make sure the network hasn't already been cleared.
//
//    Hank Childs, Tue Feb  3 17:07:25 PST 2004
//    Added variable summation query.
//
//    Kathleen Bonnell, Tue Feb  3 17:43:12 PST 2004 
//    Renamed PlotMinMax query to simply MinMaxQuery. 
//
//    Kathleen Bonnell, Fri Feb 20 08:48:50 PST 2004 
//    Added NumNodes and NumZones. 
//
//    Kathleen Bonnell, Fri Feb 20 16:56:32 PST 2004 
//    Set QueryAtts' PipeIndex so that original data queries can be
//    load balanced. 
//
//    Kathleen Bonnell, Tue Mar 23 18:00:29 PST 2004 
//    Delay setting of PipeIndex until the networkIds have been verified. 
//
//    Kathleen Bonnell, Tue Mar 30 15:11:07 PST 2004 
//    Moved instantiation of individual queries to avtQueryFactory. 
//    Retrieve current SIL and send to query.
//
//    Kathleen Bonnell, Tue May  4 14:35:08 PDT 2004 
//    Send the SILRestriction to query (insted of UseSet).
//
//    Kathleen Bonnell, Thu Oct  7 10:29:36 PDT 2004 
//    Added timing code for each PerformQuery. 
//
// ****************************************************************************

void
NetworkManager::Query(const std::vector<int> &ids, QueryAttributes *qa)
{
    std::vector<avtDataObject_p> queryInputs;
    for (int i = 0 ; i < ids.size() ; i++)
    {
        int id = ids[i];
        if (id >= networkCache.size())
        {
            debug1 << "Internal error:  asked to use network ID (" << id 
                   << ") >= num saved networks ("
                   << networkCache.size() << ")" << endl;
            EXCEPTION0(ImproperUseException);
        }
     
        if (networkCache[id] == NULL)
        {
            debug1 << "Asked to query a network that has already been cleared."
                   << endl;
            EXCEPTION0(ImproperUseException);
        }

        if (id != networkCache[id]->GetID())
        {
            debug1 << "Internal error: network at position[" << id << "] "
                   << "does not have same id (" << networkCache[id]->GetID()
                   << ")" << endl;
            EXCEPTION0(ImproperUseException);
        }

        avtDataObject_p queryInput = 
            networkCache[id]->GetPlot()->GetIntermediateDataObject();

        if (*queryInput == NULL)
        {
            debug1 << "Could not retrieve query input." << endl;
            EXCEPTION0(NoInputException);
        }
        queryInputs.push_back(queryInput);
    }

    qa->SetPipeIndex(networkCache[ids[0]]->GetPipelineSpec()->GetPipelineIndex());
    std::string queryName = qa->GetName();
    avtDataObjectQuery *query = NULL;
    avtDataObject_p queryInput; 

    TRY
    {

        query = avtQueryFactory::Instance()->CreateQuery(qa);

        if (query == NULL)
        {
            //
            // For now, multiple input queries need to be instantiated here.
            //
            if (queryName == "L2Norm Between Curves") 

            {
                avtL2NormBetweenCurvesQuery *q = new avtL2NormBetweenCurvesQuery();
                q->SetNthInput(queryInputs[0], 0);
                q->SetNthInput(queryInputs[1], 1);
                query = q;
            }
            else if (queryName == "Area Between Curves") 
            {
                avtAreaBetweenCurvesQuery *q = new avtAreaBetweenCurvesQuery();
                q->SetNthInput(queryInputs[0], 0);
                q->SetNthInput(queryInputs[1], 1);
                query = q;
            }
        }

        if (query != NULL)
        {
            avtSILRestriction_p silr = 
               networkCache[ids[0]]->GetDataSpec()->GetRestriction();
            if (*silr != NULL)
            {
                query->SetSILRestriction(silr->MakeAttributes());
            }


            //
            // Use the plot's intermediate output as input to the query
            // unless the query requires original data, then use the
            // database output.
            //
            if (!query->OriginalData())
                queryInput = queryInputs[0];
            else 
                queryInput = networkCache[ids[0]]->GetNetDB()->GetOutput();

            query->SetInput(queryInput);
            int queryTimer = visitTimer->StartTimer();
            query->PerformQuery(qa);
            visitTimer->StopTimer(queryTimer, query->GetType());
            delete query;
        }
        visitTimer->DumpTimings();
    }
    CATCHALL( ... )
    {
        if (query != NULL)
            delete query;
        RETHROW;
    }
    ENDTRY
}

// ****************************************************************************
//  Function:  RenderBalance 
//
//  Purpose: compute rendering balance of worst case (e.g. max triangle count
//           over average triangle count. Valid result computed only at root.
//
//  Argument: number of triangles on calling processor
//
//  Programmer:  Mark C. Miller 
//  Creation:    24May03 
// ****************************************************************************
static double
RenderBalance(int numTrianglesIHave)
{
   double balance = 1.0;

#ifdef PARALLEL
   int rank, size, *triCounts;

   balance = -1.0;
   rank = PAR_Rank();
   size = PAR_Size();
   if (rank == 0)
      triCounts = new int [size]; 
   MPI_Gather(&numTrianglesIHave, 1, MPI_INT, triCounts, 1, MPI_INT, 0, MPI_COMM_WORLD);
   if (rank == 0)
   {  int i, maxTriangles, minTriangles, totTriangles, avgTriangles;
      minTriangles = triCounts[0];
      maxTriangles = minTriangles;
      totTriangles = 0;
      for (i = 0; i < size; i++)
      {
         if (triCounts[i] < minTriangles)
            minTriangles = triCounts[i];
         if (triCounts[i] > maxTriangles)
            maxTriangles = triCounts[i];
         totTriangles += triCounts[i];
      }
      avgTriangles = totTriangles / size;
      if (avgTriangles > 0)
         balance = (double) maxTriangles / (double) avgTriangles;
      delete [] triCounts;
   }
#endif

   return balance;
}


// ****************************************************************************
//  Method:  Network::CloneNetwork
//
//  Purpose:
//    Creates a clone of an existing network. 
//
//  Arguments:
//    id         The network to clone.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    March 19, 2004
//
//  Modifications:
//
// ****************************************************************************

void
NetworkManager::CloneNetwork(const int id)
{
    //
    // Ensure that it is possible to access the specified network.
    //
    if (workingNet != NULL)
    {
        string error = "Unable to clone an open network.";
        debug1 << error.c_str() << endl;
        EXCEPTION1(ImproperUseException,error);
    }

    if (id >= networkCache.size())
    {
        debug1 << "Internal error:  asked to clone network ID (" << id 
               << ") >= num saved networks (" << networkCache.size() << ")" 
               << endl;
        EXCEPTION0(ImproperUseException);
    }

    if (networkCache[id] == NULL)
    {
        string error = "Asked to clone a network that has already been cleared.";
        debug1 << error.c_str() << endl;
        EXCEPTION1(ImproperUseException, error);
    }

    if (id != networkCache[id]->GetID())
    {
        
        debug1 << "Internal error: network at position[" << id 
               << "] does not have same id (" 
               << networkCache[id]->GetID() << ")" << endl;
        EXCEPTION0(ImproperUseException);
    }

    workingNet = new ClonedDataNetwork(networkCache[id]);

    if (!workingNetnodeList.empty())
        workingNetnodeList.clear();
    workingNetnodeList.push_back(
        workingNet->GetNodeList()[workingNet->GetNodeList().size() -1]);
}


// ****************************************************************************
//  Method:  Network::AddQueryOverTimeFilter
//
//  Purpose:
//    Adds a QueryOverTimeFilter node to the network. 
//
//  Arguments:
//    qA         The atts that control the filter. 
//    clonedFromId  The Id of the nework from which this time-query network
//                  was cloned.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    March 19, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr 15 14:07:53 PDT 2004
//    Added arge 'clonedFromId' so that 'Actual Data' queries get the correct
//    input.
//
//    Kathleen Bonnell, Tue Apr 27 13:41:32 PDT 2004
//    Pass the cloned network's pipeline index to query atts. 
//
//    Kathleen Bonnell, Tue May  4 14:35:08 PDT 2004 
//    Send the SILRestriction to QueryOverTime filter (insted of UseSet).
//
// ****************************************************************************

void
NetworkManager::AddQueryOverTimeFilter(QueryOverTimeAttributes *qA,
                                       const int clonedFromId)
{
    if (workingNet == NULL)
    {
        string error =  "Adding a filter to a non-existent network." ;
        EXCEPTION1(ImproperUseException, error);
    }

    //
    // Determine which input the filter should use.
    //
    avtDataObject_p input;
    if (qA->GetQueryAtts().GetDataType() == QueryAttributes::OriginalData)
    {
        input = workingNet->GetNetDB()->GetOutput();
    }
    else 
    {
        input = networkCache[clonedFromId]->GetPlot()->GetIntermediateDataObject();
    }
    qA->GetQueryAtts().SetPipeIndex(networkCache[clonedFromId]->
        GetPipelineSpec()->GetPipelineIndex());

    //    
    // Pass down the current SILRestriction (via UseSet) in case the query 
    // needs to make use of this information.
    //    
    avtSILRestriction_p silr = workingNet->GetDataSpec()->GetRestriction();
   
    // 
    //  Create a transition node so that the new filter will receive
    //  the correct input.
    // 
    NetnodeTransition *trans = new NetnodeTransition(input);
    Netnode *n = workingNetnodeList.back();
    workingNetnodeList.pop_back();
    trans->GetInputNodes().push_back(n);
    
    workingNet->AddNode(trans);

    // 
    // Put a QueryOverTimeFilter right after the transition to handle 
    // the query. 
    // 
    avtQueryOverTimeFilter *qf = new avtQueryOverTimeFilter(qA);
    if (*silr != NULL)
        qf->SetSILAtts(silr->MakeAttributes());
    NetnodeFilter *qfilt = new NetnodeFilter(qf, "QueryOverTime");
    qfilt->GetInputNodes().push_back(trans);
    
    workingNetnodeList.push_back(qfilt);
    workingNet->AddNode(qfilt);
}

// ****************************************************************************
//  Function:  DumpImage
//
//  Purpose:
//    Write an image for debugging purposes.
//
//  Arguments:
//    img        the image
//    fmt        the file format for the avtFileWriter
//    allprocs   if false, only the first processor writes
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 21, 2004
//
// ****************************************************************************
static void
DumpImage(avtDataObject_p img, const char *fmt, bool allprocs)
{
    if (!allprocs && PAR_Rank() != 0)
        return;

    static int numDumps = 0;
    char tmpName[256];
    avtFileWriter *fileWriter = new avtFileWriter();

#ifdef PARALLEL
    if (allprocs)
        sprintf(tmpName, "%s_%03d_%03d.tif", fmt, PAR_Rank(), numDumps);
    else
        sprintf(tmpName, "%s_%03d.tif", fmt, numDumps);
#else
    sprintf(tmpName, "%s_%03d.tif", fmt, numDumps);
#endif

    fileWriter->SetFormat(TIFF);
    int useLZW = 6;
    fileWriter->Write(tmpName, img, 100, false, useLZW, false);

    numDumps++;
}

// ****************************************************************************
//  Function:  DumpImage
//
//  Purpose:
//    Write an image for debugging purposes.
//
//  Arguments:
//    img        the image
//    fmt        the file format for the avtFileWriter
//    allprocs   if false, only the first processor writes
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 21, 2004
//
// ****************************************************************************
static void
DumpImage(avtImage_p img, const char *fmt, bool allprocs)
{
    avtDataObject_p tmpImage;
    CopyTo(tmpImage, img);
    DumpImage(tmpImage, fmt, allprocs);
}
