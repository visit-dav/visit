#include <NetworkManager.h>
#include <DataNetwork.h>
#include <DebugStream.h>
#include <avtDatabaseFactory.h>
#include <LoadBalancer.h>
#include <MaterialAttributes.h>
#include <avtExpressionEvaluatorFilter.h>
#include <ImproperUseException.h>
#include <DatabaseException.h>
#include <NoInputException.h>
#include <avtPluginFilter.h>
#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>
#include <PickAttributes.h>
#include <avtExtents.h>
#include <avtNullData.h>
#include <avtDatabaseMetaData.h>
#include <avtDataObjectQuery.h>
#include <avtCompactnessQuery.h>
#include <avtEulerianQuery.h>
#include <avtAreaBetweenCurvesQuery.h>
#include <avtCycleQuery.h>
#include <avtIntegrateQuery.h>
#include <avtL2NormQuery.h>
#include <avtL2NormBetweenCurvesQuery.h>
#include <avtTimeQuery.h>
#include <avtLocateCellQuery.h>
#include <avtPickQuery.h>
#include <avtSourceFromAVTImage.h>
#include <avtSourceFromImage.h>
#include <avtSourceFromNullData.h>
#include <avtTotalSurfaceAreaQuery.h>
#include <avtTotalVolumeQuery.h>
#include <avtTotalRevolvedVolumeQuery.h>
#include <avtTotalRevolvedSurfaceAreaQuery.h>
#include <avtVariableQuery.h>
#include <avtWholeImageCompositer.h>
#include <avtPlot.h>
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

static double RenderBalance(int numTrianglesIHave);

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
// ****************************************************************************
NetworkManager::NetworkManager(void) : virtualDatabases()
{
    //cerr << "NetworkManager::NetworkManager(void)" << endl;
    workingNet = NULL;
    loadBalancer = NULL;
    requireOriginalCells = false;

    // stuff to support scalable rendering
    {
       viswin = new VisWindow(true);
       AnnotationAttributes newAtts = *(viswin->GetAnnotationAtts());
       newAtts.SetUserInfoFlag(false);
       newAtts.SetDatabaseInfoFlag(false);
       newAtts.SetLegendInfoFlag(false);
       newAtts.SetTriadFlag(false);
       newAtts.SetBboxFlag(false);
       newAtts.SetAxesFlag(false);
       viswin->SetAnnotationAtts(&newAtts);
       viswin->DisableUpdates();
    }
}

// ****************************************************************************
//  Method: NetworkManager destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   September 29, 2000
//
//  Modifications:
//      Sean Ahern, Fri May 24 11:12:25 PDT 2002
//      Got rid of deletion of the database cache, since all DBs in there
//      will be deleted by deleting the network cache.
//
// ****************************************************************************
NetworkManager::~NetworkManager(void)
{
    //cerr << "NetworkManager::~NetworkManager(void)" << endl;
    for (int i = 0; i < networkCache.size(); i++)
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
//      Sean Ahern, Tue May 14 14:35:42 PDT 2002
//      Retrofitted for my new network management techniques.
//
//      Sean Ahern, Tue May 21 16:09:56 PDT 2002
//      Told the underlying objects to free their data.
//
// ****************************************************************************
void
NetworkManager::ClearAllNetworks(void)
{
    //cerr << "NetworkManager::ClearAllNetworks(void)" << endl;
    int i;

    for (i = 0; i < databaseCache.size(); i++)
        delete databaseCache[i];
    databaseCache.clear();

    for (i = 0; i < networkCache.size(); i++)
        delete networkCache[i];
    networkCache.clear();
    globalCellCounts.clear();

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
// ****************************************************************************

NetnodeDB *
NetworkManager::GetDBFromCache(const string &filename, int time)
{
    //cerr << "NetworkManager::GetDBFromCache()" << endl;

    // If we don't have a load balancer, we're dead.
    if (loadBalancer == NULL)
    {
        debug1 << "Internal error: A load balancer was never registered." << endl;
        EXCEPTION0(ImproperUseException);
    }

    // Find a matching database 
    NetnodeDB* cachedDB = NULL;
    for (int i = 0; i < databaseCache.size(); i++)
    {
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
            db = avtDatabaseFactory::VisitFile(filename_c);
        else
            db = avtDatabaseFactory::FileList(&filename_c, 1);

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
        const   avtIOInformation & ioinfo = db->GetIOInformation();
        loadBalancer->AddDatabase(filename, ioinfo);
        CATCH_RETURN2(1, netDB);
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
// ****************************************************************************
void
NetworkManager::StartNetwork(const string &filename, const string &var,
                             int time,
                             const CompactSILRestrictionAttributes &atts,
                             const MaterialAttributes &matopts)
{
    //cerr << "NetworkManager::StartNetwork()" << endl;

    // Check to make sure that there is no existing network.
    // MCM_FIX: in automatic SR mode, this logic won't work.
#if 0
    if (workingNet != NULL)
    {
        char error[] = "Trying to create pipeline twice.";
        debug1 << error << endl;
        EXCEPTION1(ImproperUseException,error);
    }
#endif

    // If the variable is an expression, we need to find a "real" variable
    // name to work with.
    std::string leaf = var;
    ExprNode *tree = ParsingExprList::GetExpressionTree(leaf);
    while (tree != NULL)
    {
        leaf = *tree->GetVarLeaves().begin();
        tree = ParsingExprList::GetExpressionTree(leaf);
    }

    // Start up the DataNetwork and add the database to it.
    workingNet = new DataNetwork;
    NetnodeDB *netDB = GetDBFromCache(filename, time);
    workingNet->SetNetDB(netDB);
    workingNet->AddNode(netDB);
    netDB->SetDBInfo(filename, leaf, time);

    // Put an ExpressionEvaluatorFilter right after the netDB to handle
    // expressions that come up the pipe.
    avtExpressionEvaluatorFilter *f = new avtExpressionEvaluatorFilter();
    NetnodeFilter *filt = new NetnodeFilter(f, "ExpressionEvaluator");
    filt->GetInputNodes().push_back(netDB);

    // Push the ExpressionEvaluator onto the working list.
    workingNetnodeList.push_back(filt);

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
// ****************************************************************************
void
NetworkManager::DefineDB(const string &dbName, const string &dbPath,
    const stringVector &files, int time)
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
                found = (databaseCache[i]->GetFilename() == dbName);

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
            db = avtDatabaseFactory::FileList(names, filesWithPath.size());
            delete [] names;
            names = 0;

            // Add the virtual database to the virtual database map.
            virtualDatabases[dbName] = filesWithPath;
            debug3 << "NetworkManager::DefineDB: Added new virtual database "
                   << "definition for " << dbName.c_str() << endl;
        }
        else if (dbName.substr(dbName.length() - 6) == ".visit")
            db = avtDatabaseFactory::VisitFile(dbName_c);
        else
            db = avtDatabaseFactory::FileList(&dbName_c, 1);

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
        const   avtIOInformation & ioinfo = db->GetIOInformation();
        loadBalancer->AddDatabase(dbName, ioinfo);
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
    //cerr << "NetworkManager::AddFilter()" << endl;
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
// ****************************************************************************
void
NetworkManager::MakePlot(const string &id, const AttributeGroup *atts)
{
    //cerr << "NetworkManager::MakePlot()" << endl;
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
               << "absorb" << workingNetnodeList.size() << " nodes."  << endl;
        EXCEPTION0(ImproperUseException);
    }

    avtPlot *p = PlotPluginManager::Instance()->
                                GetEnginePluginInfo(id)->AllocAvtPlot();
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
// ****************************************************************************
int
NetworkManager::EndNetwork(void)
{
    //cerr << "NetworkManager::EndNetwork(void)" << endl;
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
    workingNet->SetID(networkCache.size());
    networkCache.push_back(workingNet);
    globalCellCounts.push_back(-1);

    return workingNet->GetID();
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
// ****************************************************************************
void
NetworkManager::UseNetwork(int id)
{
    //cerr << "NetworkManager::UseNetwork()" << endl;
    if (workingNet)
    {
        debug1 << "Internal error: UseNetwork called with an open network" << endl;
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
    //cerr << "NetworkManager::GetPlot(void)" << endl;
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
    //cerr << "NetworkManager::GetCurrentNetworkId(void)" << endl;
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
      sum += globalCellCounts[i];
   return sum;
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
// ****************************************************************************
void
NetworkManager::DoneWithNetwork(int id)
{
    //cerr << "NetworkManager::DoneWithNetwork()" << endl;
    if (id >= networkCache.size())
    {
        debug1 << "Internal error: asked to reuse network ID (" << id
            << " >= num saved networks (" << networkCache.size() << ")"
            << endl;
        EXCEPTION0(ImproperUseException);
    }

    networkCache[id]->ReleaseData();
    globalCellCounts[id] = -1;

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
// ****************************************************************************
void
NetworkManager::UpdatePlotAtts(int id, const AttributeGroup *atts)
{
    //cerr << "NetworkManager::UpdatePlotAtts()" << endl;
    if (id >= networkCache.size())
    {
        debug1 << "Internal error: asked to reuse network ID (" << id
            << ") >= num saved networks (" << networkCache.size() << ")"
            << endl;
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
// ****************************************************************************
avtDataObjectWriter_p
NetworkManager::GetOutput(bool respondWithNullData, bool calledForRender)
{

    // Is the network complete?
    if (*(workingNet->GetPlot()) == NULL)
    {
        debug1 << "Never told what type of plot to use." << endl;
        EXCEPTION0(ImproperUseException);
    }

    TRY
    {
        bool clearNetwork = true;

        // Hook up the network
        avtDataObject_p output = workingNet->GetOutput();
        int netId = GetCurrentNetworkId();


        workingNet->GetPipelineSpec()->GetDataSpecification()->
            SetMayRequireZones(requireOriginalCells); 

        avtDataObjectWriter_p writer = workingNet->GetWriter(output,
                                          workingNet->GetPipelineSpec(),
                                          &windowAttributes);

        // compute this network's cell count if we haven't already 
        if (globalCellCounts[netId] == -1)
        {
           int localCellCount = writer->GetInput()->GetNumberOfCells();
           int totalCellCount;
#ifdef PARALLEL
           MPI_Allreduce(&localCellCount, &totalCellCount, 1, MPI_INT, MPI_SUM,
              MPI_COMM_WORLD);
#else
           totalCellCount = localCellCount; 
#endif
           globalCellCounts[netId] = totalCellCount;
        }

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
        else if (!calledForRender)
        {
           int  scalableThreshold = windowAttributes.GetRenderAtts().GetScalableThreshold();

           if (GetTotalGlobalCellCounts() > scalableThreshold)
           {
              avtNullData_p nullData = new avtNullData(NULL,AVT_NULL_DATASET_MSG);
              avtDataObject_p dummyDob;
              CopyTo(dummyDob, nullData);
              writer = dummyDob->InstantiateWriter();
              writer->SetInput(dummyDob);
              clearNetwork = false;
           }

        }

        // Zero out the workingNet.  Remember that we've already pushed it
        // onto the cache in EndNetwork.
        if (clearNetwork)
        {
           workingNet = NULL;
           workingNetnodeList.clear();
        }

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
// ****************************************************************************
avtDataObjectWriter_p
NetworkManager::Render(intVector plotIds, bool getZBuffer)
{
    DataNetwork *origWorkingNet = workingNet;

    TRY
    {
       avtDataObjectWriter_p writer;
       int  scalableThreshold = windowAttributes.GetRenderAtts().GetScalableThreshold();

       // scalable threshold test (the 0.5 is to add some hysteresus to avoid 
       // the misfortune of oscillating switching of modes around the threshold)
       if (GetTotalGlobalCellCounts() < 0.5 * scalableThreshold)
       {

          avtNullData_p nullData = new avtNullData(NULL,AVT_NULL_IMAGE_MSG);
          avtDataObject_p dummyDob;
          CopyTo(dummyDob, nullData);
          writer = dummyDob->InstantiateWriter();
          writer->SetInput(dummyDob);

       }
       else
       {

          // put all the plot objects into the VisWindow
          viswin->SetScalableRendering(false);
          for (int i = 0; i < plotIds.size(); i++)
          {
             // get the network output as we would normally
             workingNet = NULL;
             UseNetwork(plotIds[i]);
             DataNetwork *workingNetSaved = workingNet;
             avtDataObjectWriter_p tmpWriter = GetOutput(false,true);
             avtDataObject_p dob = tmpWriter->GetInput();

             // merge polygon info output across processors 
             dob->GetInfo().ParallelMerge(tmpWriter);

             // get this plots object handle
             avtPlot_p aPlot = workingNetSaved->GetPlot();

             // set foreground and background colors of the plot
             double color[3];
             color[0] = windowAttributes.GetBackground()[0]/255.0,
             color[1] = windowAttributes.GetBackground()[1]/255.0,
             color[2] = windowAttributes.GetBackground()[2]/255.0,
             aPlot->SetBackgroundColor(color);
             color[0] = windowAttributes.GetForeground()[0]/255.0,
             color[1] = windowAttributes.GetForeground()[1]/255.0,
             color[2] = windowAttributes.GetForeground()[2]/255.0,
             aPlot->SetForegroundColor(color);

             // do the part of the execute we'd do in the viewer
             avtActor_p anActor = aPlot->Execute(NULL, dob);

             viswin->AddPlot(anActor);
          }

          int numTriangles = viswin->GetNumTriangles();
          debug1 << "Rendering " << numTriangles << " triangles. " 
                 << "Balanced speedup = " << RenderBalance(numTriangles)
                 << "x" << endl;

          // render the image and capture it. Relies upon explicit render
          avtImage_p theImage = viswin->ScreenCapture(true);
          viswin->ClearPlots();

          // do the parallel composite using a 1 stage pipeline
          int imageRows, imageCols;
          viswin->GetSize(imageCols, imageRows);
          avtWholeImageCompositer imageCompositer;
          imageCompositer.SetOutputImageSize(imageRows, imageCols);
          imageCompositer.AddImageInput(theImage, 0, 0);
          imageCompositer.SetShouldOutputZBuffer(getZBuffer);
          imageCompositer.Execute();
          avtDataObject_p compositedImageAsDataObject = imageCompositer.GetOutput();
          writer = compositedImageAsDataObject->InstantiateWriter();
          writer->SetInput(compositedImageAsDataObject);
       }

       // return it
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
// ****************************************************************************
void
NetworkManager::SetWindowAttributes(const WindowAttributes &atts)
{
    viswin->SetSize(atts.GetSize()[0], atts.GetSize()[1]);

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
    // Set the lights.
    //
    const LightList& lights = atts.GetLights();
    viswin->SetLightList(&lights);

    //
    // Set the background.
    //
    viswin->SetBackgroundColor(atts.GetBackground()[0]/255.0,
                               atts.GetBackground()[1]/255.0,
                               atts.GetBackground()[2]/255.0);
    viswin->SetForegroundColor(atts.GetForeground()[0]/255.0,
                               atts.GetForeground()[1]/255.0,
                               atts.GetForeground()[2]/255.0);

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

    // rendering options
    viswin->SetAntialiasing(atts.GetRenderAtts().GetAntialiasing());
    viswin->SetSurfaceRepresentation(atts.GetRenderAtts().GetGeometryRepresentation());
    viswin->SetImmediateModeRendering(!atts.GetRenderAtts().GetDisplayLists());

    windowAttributes = atts;
}

// ****************************************************************************
//  Method:  NetworkManager::SetAnnotationAttributes
//
//  Purpose:
//    Set the annotation attributes for the engine's viswin.  Note, only
//    those annotations that live in a 3D world are rendered on the engine.
//    Furthermore, only processor 0 does annotation rendering work.
//
//  Programmer:  Mark C. Miller 
//  Creation:    15Jul03 
//
// ****************************************************************************
void
NetworkManager::SetAnnotationAttributes(const AnnotationAttributes &atts)
{
#ifdef PARALLEL
   if (PAR_Rank())
#endif
   {
      // copy the attributes and disable all non-3D attributes 
      AnnotationAttributes newAtts = atts;
      newAtts.SetUserInfoFlag(false);
      newAtts.SetDatabaseInfoFlag(false);
      newAtts.SetLegendInfoFlag(false);
      newAtts.SetTriadFlag(false);
      viswin->SetAnnotationAtts(&newAtts);
   }
   annotationAttributes = atts;
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
// ****************************************************************************
void
NetworkManager::StartPickMode(void)
{
    //cerr << "NetworkManager::StartPickMode(void)" << endl;
    requireOriginalCells = true;
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
// ****************************************************************************
void
NetworkManager::StopPickMode(void)
{
    //cerr << "NetworkManager::StopPickMode(void)" << endl;
    requireOriginalCells = false;
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

    avtLocateCellQuery *lcQ = new avtLocateCellQuery;
    avtPickQuery *pQ = new avtPickQuery;
    TRY
    {
        QueryAttributes qa;
        qa.SetRayPoint1(pa->GetRayPoint1());
        qa.SetRayPoint2(pa->GetRayPoint2());
        if (pa->GetPickType() == PickAttributes::Zone)
            qa.SetElementType(QueryAttributes::Zone);
        else
            qa.SetElementType(QueryAttributes::Node);
        lcQ->SetInput(queryInput);
        lcQ->PerformQuery(&qa); 
        pa->SetDomain(qa.GetDomain());
        pa->SetElementNumber(qa.GetElement());
        pa->SetCellPoint(qa.GetCellPoint());
        pa->SetPickPoint(qa.GetWorldPoint());
        pa->SetNodePoint(qa.GetNodePoint());
        delete lcQ;

        pQ->SetInput(queryInput);
        pQ->SetPickAtts(pa);
        pQ->PerformQuery(&qa); 
        *pa = *(pQ->GetPickAtts());
        delete pQ;
    }
    CATCHALL( ... )
    {
        if (lcQ != NULL)
            delete lcQ;
        if (pQ != NULL)
            delete pQ;
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

    std::string queryName = qa->GetName();
    avtDataObjectQuery *query = NULL;
    avtDataObject_p queryInput = queryInputs[0];

    TRY
    {
        if (strcmp(queryName.c_str(), "Surface area") == 0)
        {
            query = new avtTotalSurfaceAreaQuery();
        }
        else if (strcmp(queryName.c_str(), "Volume") == 0)
        {
            query = new avtTotalVolumeQuery();
        }
        else if (strcmp(queryName.c_str(), "Revolved volume") == 0)
        {
            query = new avtTotalRevolvedVolumeQuery();
        }
        else if (strcmp(queryName.c_str(), "Revolved surface area") == 0)
        {
            query = new avtTotalRevolvedSurfaceAreaQuery();
        }
        else if (strcmp(queryName.c_str(), "Eulerian") == 0)
        {
            query = new avtEulerianQuery();
        }
        else if (strcmp(queryName.c_str(), "Compactness") == 0)
        {
            query = new avtCompactnessQuery();
        }
        else if (strcmp(queryName.c_str(), "Cycle") == 0)
        {
            query = new avtCycleQuery();
        }
        else if (strcmp(queryName.c_str(), "Integrate") == 0)
        {
            query = new avtIntegrateQuery();
        }
        else if (strcmp(queryName.c_str(), "Time") == 0)
        {
            query = new avtTimeQuery();
        }
        else if (strcmp(queryName.c_str(), "L2Norm") == 0)
        {
            query = new avtL2NormQuery();
        }
        else if (strcmp(queryName.c_str(), "L2Norm Between Curves") == 0)
        {
            avtL2NormBetweenCurvesQuery *q = new avtL2NormBetweenCurvesQuery();
            q->SetNthInput(queryInputs[0], 0);
            q->SetNthInput(queryInputs[1], 1);
            query = q;
        }
        else if (strcmp(queryName.c_str(), "Area Between Curves") == 0)
        {
            avtAreaBetweenCurvesQuery *q = new avtAreaBetweenCurvesQuery();
            q->SetNthInput(queryInputs[0], 0);
            q->SetNthInput(queryInputs[1], 1);
            query = q;
        }
        else if (strcmp(queryName.c_str(), "Variable by Zone") == 0)
        {
            query = new avtVariableQuery();
        }
        else if (strcmp(queryName.c_str(), "Variable by Node") == 0) 
        {
            query = new avtVariableQuery();
        }
        if (query != NULL)
        {
            query->SetInput(queryInput);
            query->PerformQuery(qa);
            delete query;
        }
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
