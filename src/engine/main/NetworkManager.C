#include <NetworkManager.h>
#include <DataNetwork.h>
#include <DebugStream.h>
#include <avtDatabaseFactory.h>
#include <LoadBalancer.h>
#include <MaterialAttributes.h>
#include <ImproperUseException.h>
#include <DatabaseException.h>
#include <NoInputException.h>
#include <avtPluginFilter.h>
#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>
#include <PickAttributes.h>
#include <avtBinaryAddFilter.h>
#include <avtBinarySubtractFilter.h>
#include <avtBinaryMultiplyFilter.h>
#include <avtBinaryDivideFilter.h>
#include <avtBinaryPowerFilter.h>
#include <avtSinFilter.h>
#include <avtArctanFilter.h>
#include <avtArcsinFilter.h>
#include <avtArccosFilter.h>
#include <avtCosFilter.h>
#include <avtDegreeToRadianFilter.h>
#include <avtExtents.h>
#include <avtGradientFilter.h>
#include <avtMagnitudeFilter.h>
#include <avtNeighborFilter.h>
#include <avtNodeDegreeFilter.h>
#include <avtNullData.h>
#include <avtRadianToDegreeFilter.h>
#include <avtRevolvedSurfaceArea.h>
#include <avtRevolvedVolume.h>
#include <avtTanFilter.h>
#include <avtUnaryMinusFilter.h>
#include <avtAbsValFilter.h>
#include <avtNaturalLogFilter.h>
#include <avtBase10LogFilter.h>
#include <avtSquareRootFilter.h>
#include <avtSquareFilter.h>
#include <avtPolarCoordinatesFilter.h>
#include <avtVectorCrossProductFilter.h>
#include <avtVectorComponent1Filter.h>
#include <avtVectorComponent2Filter.h>
#include <avtVectorComponent3Filter.h>
#include <avtMeshXCoordinateFilter.h>
#include <avtMeshYCoordinateFilter.h>
#include <avtMeshZCoordinateFilter.h>
#include <avtProcessorIdFilter.h>
#include <avtRandomFilter.h>
#include <avtDataObjectQuery.h>
#include <avtCompactnessQuery.h>
#include <avtEulerianQuery.h>
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
#include <avtDegreeFilter.h>
#include <avtVMetrics.h>
#include <VisWindow.h>
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
    int i;

    for (i = 0; i < databaseCache.size(); i++)
        delete databaseCache[i];
    databaseCache.clear();

    for (i = 0; i < networkCache.size(); i++)
        delete networkCache[i];
    networkCache.clear();

}

// ****************************************************************************
//  Method: NetworkManager::AddDB
//
//  Purpose:
//      Add a DB to a network.  Use the existing network, if one exists.
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
//    Brad Whitlock, Wed May 14 12:56:16 PDT 2003
//    I made it get the metadata and the SIL if the time is greater than 0.
//    This ensures that we pick up the metadata and SIL for the time state
//    that we're intersted in.
//
//    Jeremy Meredith, Wed Jul 30 10:45:45 PDT 2003
//    Added the check for requiring full connectivity.
//
// ****************************************************************************

void
NetworkManager::AddDB(const string &filename, const string &var, int time,
                      const CompactSILRestrictionAttributes &atts,
                      const MaterialAttributes &matopts)
{
    //
    // We gotta have a load balancer
    //
    if (loadBalancer == NULL)
    {
        debug1 << "Internal error: A load balancer was never registered." <<
            endl;
        EXCEPTION0(ImproperUseException);
    }

    //
    // If this is the first thing on the network, then we're starting a new
    // network.
    //
    bool newNetwork = false;
    if (workingNet == NULL)
    {
        // Create a new network.
        workingNet = new DataNetwork;
        newNetwork = true;
    }

    //
    // Figure out how to get a database
    //

    // Generate the list of matching databases.
    std::vector<NetnodeDB*> databaseMatches;
    for (int i = 0; i < databaseCache.size(); i++)
    {
        if (databaseCache[i]->GetFilename() == filename)
            databaseMatches.push_back(databaseCache[i]);
    }

    // If this is not a new network, we better have matched a DB in the
    // cache.  If we didn't, the user is trying to use different databases,
    // something that we don't yet support.
    if (!newNetwork && databaseMatches.empty())
    {
        char error[] = "Using more than one database in an expression is not supported.";
        debug1 << error << endl;
        EXCEPTION1(ImproperUseException,error);
    }

    // Only if we have a new network do we need to load a new database.  If it's
    // an existing network, we already have the database set.
    if (newNetwork)
    {
        // databaseMatches now contains the matching databases.  If it's empty,
        // we have to create a new database.
        if (!databaseMatches.empty())
        {
            debug3 << "Using database in cache:"
                   << " filename:" << databaseMatches[0]->GetFilename().c_str()
                   << endl;

            NetnodeDB *netDB =new NetnodeDB(databaseMatches[0]->GetDatabase());
            workingNet->GetDBs().push_back(netDB);
            workingNet->AddNode(netDB);

            netDB->SetDBSpec(filename, var, time);
            databaseCache.push_back(netDB);
        }
        else
        {
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
                // metadata and the SIL so that it contains the right data.
                if(time > 0)
                {
                    debug2 << "NetworkManager::AddDB: We were instructed to open "
                           << filename.c_str() << " at timestate=" << time
                           << " so we're reading the MetaData and SIL early."
                           << endl;
                    db->GetMetaData(time);
                    db->GetSIL(time);
                }

                netDB = new NetnodeDB(db);
                workingNet->GetDBs().push_back(netDB);
                workingNet->AddNode(netDB);
                databaseCache.push_back(netDB);

                netDB->SetDBSpec(filename, var, time);
                const   avtIOInformation & ioinfo = db->GetIOInformation();
                loadBalancer->AddDatabase(filename, ioinfo);
            }
            CATCH(DatabaseException)
            {
                debug1 << "ERROR - could not create database " << filename.c_str()
                    << endl;
                RETHROW;
            }
            ENDTRY
        }

        // Push the db onto the working list.
        workingNetnodeList.push_back(workingNet->GetDBs().back());
    }

    // Push the name onto the name stack.
    nameStack.push_back(var);
    debug5 << "NetworkManager::AddDB: Adding " << var.c_str()
           << " to the name stack" << endl;

    // Set up the data spec.
    if (newNetwork)
    {
        avtSILRestriction_p silr =
            new avtSILRestriction(workingNet->GetDBs()[0]->GetDB()->GetSIL(), atts);
        avtDataSpecification *dspec = new avtDataSpecification(var.c_str(), time, silr);
        // Set up some options from the data specification
        dspec->SetNeedMixedVariableReconstruction(matopts.GetForceMIR());
        dspec->SetNeedSmoothMaterialInterfaces(matopts.GetSmoothing());
        dspec->SetNeedCleanZonesOnly(matopts.GetCleanZonesOnly());
        dspec->SetNeedValidFaceConnectivity(matopts.GetNeedValidConnectivity());
        workingNet->SetDataSpec(dspec);
    }
    else
    {
        // Check if the dspec already knows about our variable.
        avtDataSpecification_p dspec = workingNet->GetDataSpec();
        bool found = false;

        // First, check the normal variable.
        if (strcmp(dspec->GetVariable(), var.c_str()) == 0)
            found = true;
        // Now check the secondary variables.
        if (dspec->HasSecondaryVariable(var.c_str()))
            found = true;
        // If we didn't find it in the dspec, add it.
        if (!found)
            dspec->AddSecondaryVariable(var.c_str());
    }

    // The plot starts out as NULL.
    workingNet->SetPlot(NULL);
}

// ****************************************************************************
// Method: NetworkManager::AddDB
//
// Purpose: 
//    Open a DB and add it to the database cache.
//
// Arguments:
//    filename : The database filename.
//    time     : The timestep that we want to examine.
//
// Note:       This code was adapted from the other AddDB method.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 10 15:02:22 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed May 14 12:56:16 PDT 2003
//   I made it get the metadata and the SIL if the time is greater than 0.
//   This ensures that we pick up the metadata and SIL for the time state
//   that we're intersted in.
//   
// ****************************************************************************
void
NetworkManager::AddDB(const string &filename, int time)
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

    // Generate the list of matching databases.
    std::vector<NetnodeDB*> databaseMatches;
    for (int i = 0; i < databaseCache.size(); i++)
    {
        if (databaseCache[i]->GetFilename() == filename)
            databaseMatches.push_back(databaseCache[i]);
    }

    // If no cached databases matched the filename, open a new database.
    if (databaseMatches.empty())
    {
        debug3 << "Loading new database" << endl;
        TRY
        {
            // Open the database
            avtDatabase *db = NULL;
            const char *filename_c = filename.c_str();
            if (filename.substr(filename.length() - 6) == ".visit")
                db = avtDatabaseFactory::VisitFile(filename_c);
            else
                db = avtDatabaseFactory::FileList(&filename_c, 1);

            // If we want to open the file at a later timestep, get the
            // metadata and the SIL so that it contains the right data.
            if(time > 0)
            {
                debug2 << "NetworkManager::AddDB: We were instructed to open "
                       << filename.c_str() << " at timestate=" << time
                       << " so we're reading the MetaData and SIL early."
                       << endl;
                db->GetMetaData(time);
                db->GetSIL(time);
            }

            // Create a netnode using the opened database and stick it in
            // the database cache.
            NetnodeDB *netDB = new NetnodeDB(db);
            netDB->SetDBSpec(filename, "", time);
            databaseCache.push_back(netDB);

            // Add the database to the load balancer.
            const   avtIOInformation & ioinfo = db->GetIOInformation();
            loadBalancer->AddDatabase(filename, ioinfo);
        }
        CATCH(DatabaseException)
        {
            debug1 << "ERROR - could not create database " << filename.c_str()
                   << endl;
            RETHROW;
        }
        ENDTRY
    }
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
        if(time > 0)
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
        netDB->SetDBSpec(dbName, "", time);
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
        // Pull a node off the working list, and push it onto the filter
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
//  Method: NetworkManager::AddNamedFunction
//
//  Purpose:
//     Determines what filter to apply based on a function name and a
//     number of arguments.
//
//  Programmer: Sean Ahern
//  Creation:   Fri Apr 19 13:46:15 PDT 2002
//
//  Modifications:
//    Hank Childs, Fri Jun  7 14:20:41 PDT 2002
//    Added expression for degrees.
//
//    Brad Whitlock, Fri Jun 28 14:11:51 PST 2002
//    Made it work on Windows.
//
//    Akira Haddox, Thu Aug 15 16:29:30 PDT 2002
//    Added expressions for gradient, magnitude.
//
//    Akira Haddox Mon Aug 19 15:40:17 PDT 2002
//    Added verdict mesh quality expressions.
//
//    Akira Haddox, Thu Aug 22 08:37:28 PDT 2002
//    Added expressions for eulerian, neighbor, nodeDegree.
//
//    Hank Childs, Sun Sep  8 21:42:23 PDT 2002
//    Added revolved volume.
//
//    Kathleen Bonnell, Fri Nov 15 17:05:43 PST 2002 
//    Removed eulerian, it is now a query, not an expression
//
//    Hank Childs, Mon Nov 18 11:57:52 PST 2002
//    Added degree to radian, radian to degree.
//
//    Hank Childs, Mon Nov 18 13:51:24 PST 2002
//    Added polar coordinates, and vector component expressions.
//
//    Hank Childs, Fri Mar  7 09:43:21 PST 2003
//    Added 'random' expression.
//
//    Hank Childs, Tue Mar 18 21:36:26 PST 2003
//    Added revolved surface area.
//
// ****************************************************************************
void
NetworkManager::AddNamedFunction(const std::string &functionName, const int nargs)
{
    // Can we even add filters?
    if (workingNet == NULL)
    {
        debug1 << "Trying to add a filter to a non-existent network." << endl;
        EXCEPTION0(ImproperUseException);
    }

    // We know we can add a filter.  Figure out which one.
    avtExpressionFilter *f = NULL;
    if (nargs == 1)
    {
        avtSingleInputExpressionFilter *single = NULL;
        if (functionName == "-")
            single = new avtUnaryMinusFilter();
        else if (functionName == "sin")
            single = new avtSinFilter();
        else if (functionName == "cos")
            single = new avtCosFilter();
        else if (functionName == "tan")
            single = new avtTanFilter();
        else if (functionName == "atan")
            single = new avtArctanFilter();
        else if (functionName == "asin")
            single = new avtArcsinFilter();
        else if (functionName == "acos")
            single = new avtArccosFilter();
        else if (functionName == "deg2rad")
            single = new avtDegreeToRadianFilter();
        else if (functionName == "rad2deg")
            single = new avtRadianToDegreeFilter();
        else if (functionName == "abs")
            single = new avtAbsValFilter();
        else if (functionName == "ln")
            single = new avtNaturalLogFilter();
        else if ((functionName == "log") || (functionName == "log10"))
            single = new avtBase10LogFilter();
        else if (functionName == "sqrt")
            single = new avtSquareRootFilter();
        else if ((functionName == "sq") || (functionName == "sqr"))
            single = new avtSquareFilter();
        else if (functionName == "degree")
            single = new avtDegreeFilter();
        else if (functionName == "polar")
            single = new avtPolarCoordinatesFilter();
        else if (functionName == "coord0" || functionName == "X")
            single = new avtMeshXCoordinateFilter();
        else if (functionName == "coord1" || functionName == "Y")
            single = new avtMeshYCoordinateFilter();
        else if (functionName == "coord2" || functionName == "Z")
            single = new avtMeshZCoordinateFilter();
        else if (functionName == "comp0")
            single = new avtVectorComponent1Filter();
        else if (functionName == "comp1")
            single = new avtVectorComponent2Filter();
        else if (functionName == "comp2")
            single = new avtVectorComponent3Filter();
        else if (functionName == "procid")
            single = new avtProcessorIdFilter();
        else if (functionName == "random")
            single = new avtRandomFilter();
        else if (functionName == "gradient")
            single = new avtGradientFilter();
        else if (functionName == "magnitude")
            single = new avtMagnitudeFilter();
        else if (functionName == "neighbor")
            single = new avtNeighborFilter();
        else if (functionName == "node_degree")
            single = new avtNodeDegreeFilter();
        // Begin Verdict Metrics
        else if (functionName == "area")
            single = new avtVMetricArea();
        else if (functionName == "aspect")
            single = new avtVMetricAspectRatio();
        else if (functionName == "skew")
            single = new avtVMetricSkew();
        else if (functionName == "taper")
            single = new avtVMetricTaper();
        else if (functionName == "volume")
            single = new avtVMetricVolume();
        else if (functionName == "stretch")
            single = new avtVMetricStretch();
        else if (functionName == "diagonal")
            single = new avtVMetricDiagonal();
        else if (functionName == "dimension")
            single = new avtVMetricDimension();
        else if (functionName == "oddy")
            single = new avtVMetricOddy();
        else if (functionName == "condition")
            single = new avtVMetricCondition();
        else if (functionName == "jacobian")
            single = new avtVMetricJacobian();
        else if (functionName == "scaled_jacobian")
            single = new avtVMetricScaledJacobian();
        else if (functionName == "shear")
            single = new avtVMetricShear();
        else if (functionName == "shape")
            single = new avtVMetricShape();
        else if (functionName == "relative_size")
            single = new avtVMetricRelativeSize();
        else if (functionName == "shape_and_size")
            single = new avtVMetricShapeAndSize();
        else if (functionName == "aspect_gamma")
            single = new avtVMetricAspectGamma();
        else if (functionName == "warpage")
            single = new avtVMetricWarpage();
        else if (functionName == "largest_angle")
            single = new avtVMetricLargestAngle();
        else if (functionName == "smallest_angle")
            single = new avtVMetricSmallestAngle();
        else if (functionName == "revolved_volume")
            single = new avtRevolvedVolume;
        else if (functionName == "revolved_surface_area")
            single = new avtRevolvedSurfaceArea;
        else
        {
            std::string error;
            error = std::string("NetworkManager::AddNamedFunction: "
                                "Unknown function: \"") + 
                    functionName + std::string("\".");
            EXCEPTION1(VisItException, error);
        }

        // Set the variable the expression should process
        string inputName = nameStack.back();
        nameStack.pop_back();
        debug5 << "NetworkManager::AddNamedFunction: Pulling "
               << inputName.c_str() << " from the name stack" << endl;
        single->AddInputVariableName(inputName.c_str());

        // Set the variable the expression should output
        string outputName = functionName + "(" + inputName + ")";
        single->SetOutputVariableName(outputName.c_str());
        nameStack.push_back(outputName);
        debug5 << "NetworkManager::AddNamedFunction: Adding "
               << outputName.c_str() << " to the name stack" << endl;

        f = single;
    }
    else if (nargs == 2)
    {
        avtMultipleInputExpressionFilter *multiple = NULL;
        if (functionName == "+")
            multiple = new avtBinaryAddFilter();
        else if (functionName == "-")
            multiple = new avtBinarySubtractFilter();
        else if (functionName == "*")
            multiple = new avtBinaryMultiplyFilter();
        else if (functionName == "/")
            multiple = new avtBinaryDivideFilter();
        else if (functionName == "^")
            multiple = new avtBinaryPowerFilter();
        else if (functionName == "cross")
            multiple = new avtVectorCrossProductFilter();
        else
        {
            std::string error;
            error = std::string("NetworkManager::AddNamedFunction: "
                                "Unknown function: \"")
                    + functionName + std::string("\".");
            EXCEPTION1(VisItException, error);
        }

        // Set the variables the expression should process
        string inputName2 = nameStack.back();
        nameStack.pop_back();
        debug5 << "NetworkManager::AddNamedFunction: Pulling "
               << inputName2.c_str() << " from the name stack" << endl;
        string inputName1 = nameStack.back();
        nameStack.pop_back();
        debug5 << "NetworkManager::AddNamedFunction: Pulling "
               << inputName1.c_str() << " from the name stack" << endl;
        multiple->AddInputVariableName(inputName1.c_str());
        multiple->AddInputVariableName(inputName2.c_str());

        // Set the variable the expression should output
        string outputName = inputName1 + functionName + inputName2;
        multiple->SetOutputVariableName(outputName.c_str());
        nameStack.push_back(outputName);
        debug5 << "NetworkManager::AddNamedFunction: Adding "
               << outputName.c_str() << " to the name stack" << endl;

        f = multiple;
    }
    else
    {
        char num[20];
        sprintf(num, "%d", nargs);
        std::string error;
        error = std::string("NetworkManager::AddNamedFunction: "
                            "No filters named \"") + 
                functionName + std::string("\" that support ") +
                std::string(num) + std::string(" arguments.");
        EXCEPTION1(VisItException, error);
    }

    NetnodeFilter *filt = new NetnodeFilter(f, functionName);
    std::vector<Netnode*> &filtInputs = filt->GetInputNodes();

#if 0
    for(int i=0;i<nargs;i++)
    {
#endif
        Netnode *n = workingNetnodeList.back();
        workingNetnodeList.pop_back();
        filtInputs.push_back(n);
#if 0
    }
#endif

    workingNetnodeList.push_back(filt);
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
        debug1 << "Network still-in progress.  Filter required." << endl;
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
    // Checking to see if the network has been built successfully.
    if (workingNetnodeList.size() != 1)
    {
        debug1 << "Network building still in progress.  Filter required to "
               << "absorb" << workingNetnodeList.size() << " nodes."  << endl;

        EXCEPTION0(ImproperUseException);
    }

    // set the pipeline specification
    int pipelineIndex =
        loadBalancer->AddPipeline(workingNet->GetDBs()[0]->GetFilename());
    avtPipelineSpecification_p pspec =
        new avtPipelineSpecification(workingNet->GetDataSpec(), pipelineIndex);
    workingNet->SetPipelineSpec(pspec);

    // Connect the workingNet to the workingNetnodeList.
    workingNet->SetTerminalNode(workingNetnodeList[0]);

    // Push the working net onto the network caches.
    workingNet->SetID(networkCache.size());
    networkCache.push_back(workingNet);

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
    if (id >= networkCache.size())
    {
        debug1 << "Internal error: asked to reuse network ID (" << id
            << " >= num saved networks (" << networkCache.size() << ")"
            << endl;
        EXCEPTION0(ImproperUseException);
    }

    networkCache[id]->ReleaseData();

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
NetworkManager::GetOutput(bool respondWithNullData)
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

        avtDataObjectWriter_p writer = workingNet->GetWriter(output,
                                          workingNet->GetPipelineSpec(),
                                          &windowAttributes);

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
// ****************************************************************************
avtDataObjectWriter_p
NetworkManager::Render(intVector plotIds, bool getZBuffer)
{
    DataNetwork *origWorkingNet = workingNet;

    TRY
    {
       avtDataObjectWriter_p writer;

       // put all the plot objects into the VisWindow
       viswin->SetScalableRendering(false);
       for (int i = 0; i < plotIds.size(); i++)
       {
          // get the network output as we would normally
          workingNet = NULL;
          UseNetwork(plotIds[i]);
          DataNetwork *workingNetSaved = workingNet;
          avtDataObjectWriter_p tmpWriter = GetOutput(false);
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
// ****************************************************************************
void
NetworkManager::SetWindowAttributes(const WindowAttributes &atts)
{
    viswin->SetSize(atts.GetSize()[0], atts.GetSize()[1]);

#if 0
    // What are bounds? 
    viswin->SetBounds();
    // What about 2D views?
    viswin->SetView2D();
#endif

    {  const LightList& lights = atts.GetLights();
       viswin->SetLightList(&lights);
    }

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

    // Set the 3D view
    {
        const ViewAttributes& viewAtts = atts.GetView();
        avtView3D view3d;
        view3d.SetFromViewAttributes(&viewAtts);
        view3d.imagePan[0] = viewAtts.GetImagePan()[0];
        view3d.imagePan[1] = viewAtts.GetImagePan()[1];
        view3d.imageZoom = viewAtts.GetImageZoom();
        viswin->SetView3D(view3d);
    }

    viswin->SetViewport(atts.GetView().GetViewportCoords()[0],
                        atts.GetView().GetViewportCoords()[1],
                        atts.GetView().GetViewportCoords()[2],
                        atts.GetView().GetViewportCoords()[3]);

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
        lcQ->SetInput(queryInput);
        lcQ->PerformQuery(&qa); 
        pa->SetDomain(qa.GetDomain());
        pa->SetElementNumber(qa.GetElement());
        pa->SetCellPoint(qa.GetCellPoint());
        pa->SetPickPoint(qa.GetWorldPoint());
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
// ****************************************************************************
void
NetworkManager::Query(const int id, QueryAttributes *qa)
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

    std::string queryName = qa->GetName();
    avtDataObjectQuery *query = NULL;

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
