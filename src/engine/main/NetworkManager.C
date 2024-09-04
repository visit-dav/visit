// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              NetworkManager.C                             //
// ************************************************************************* //

#include <AttributeSubject.h>
#include <NetworkManager.h>
#include <DataNetwork.h>
#include <ClonedDataNetwork.h>
#include <CumulativeQueryNamedSelectionExtension.h>
#include <DebugStream.h>
#include <avtDatabaseFactory.h>
#include <LoadBalancer.h>
#include <ConstructDataBinningAttributes.h>
#include <DBOptionsAttributes.h>
#include <ExportDBAttributes.h>
#include <MaterialAttributes.h>
#include <MeshManagementAttributes.h>
#include <avtExpressionEvaluatorFilter.h>
#include <avtNamedSelectionFilter.h>
#include <avtNamedSelectionManager.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <DatabaseException.h>
#include <NoInputException.h>
#include <avtPluginFilter.h>
#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>
#include <AnnotationObject.h>
#include <AnnotationObjectList.h>
#include <AnnotationAttributes.h>
#include <PickAttributes.h>
#include <SelectionProperties.h>
#include <VisualCueInfo.h>
#include <VisualCueList.h>
#include <avtApplyDataBinningExpression.h>
#include <avtCallback.h>
#include <avtColorTables.h>
#include <avtDebugDumpOptions.h>
#include <avtExtents.h>
#include <avtNullData.h>
#include <avtDatabaseMetaData.h>
#include <avtDataObjectQuery.h>
#include <avtMissingDataFilter.h>
#include <avtMultipleInputQuery.h>
#include <avtAreaBetweenCurvesQuery.h>
#include <avtFileWriter.h>
#include <avtDatabaseWriter.h>
#include <avtL2NormBetweenCurvesQuery.h>
#include <avtLocateQuery.h>
#include <avtLocateCellQuery.h>
#include <avtLocateNodeQuery.h>
#include <avtActualCoordsQuery.h>
#include <avtActualNodeCoordsQuery.h>
#include <avtActualZoneCoordsQuery.h>
#include <avtPickQuery.h>
#include <avtNamedSelection.h>
#include <avtNodePickQuery.h>
#include <avtParallel.h>
#include <avtPickByNodeQuery.h>
#include <avtPickByZoneQuery.h>
#include <avtTransparencyActor.h>
#include <avtZonePickQuery.h>
#include <avtCurvePickQuery.h>
#include <avtSoftwareShader.h>
#include <avtDataBinning.h>
#include <avtDataBinningConstructor.h>
#include <avtSourceFromAVTImage.h>
#include <avtSourceFromImage.h>
#include <avtSourceFromNullData.h>
#include <avtTiledImageCompositor.h>
#include <avtWholeImageCompositerWithZ.h>
#include <avtWholeImageCompositerNoZ.h>
#include <avtPlot.h>
#include <avtQueryOverTimeFilter.h>
#include <avtTimeLoopQOTFilter.h>
#include <avtDirectDatabaseQOTFilter.h>
#include <avtQueryFactory.h>
#include <avtMultiresFilter.h>
#include <avtValueImageCompositer.h>
#include <CompactSILRestrictionAttributes.h>
#include <VisWinRendering.h>
#include <VisWindow.h>
#include <VisWindowTypes.h>
#include <ParsingExprList.h>
#include <avtExprNode.h>
#include <DatabasePluginManager.h>
#include <DatabasePluginInfo.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#include <StackTimer.h>
#include <FileFunctions.h>
//#define ProgrammableCompositerDEBUG
//#define NetworkManagerDEBUG
//#define NetworkManagerTIME
#include <ProgrammableCompositer.h>

#include <vtkImageData.h>
#include <vtkRectilinearGrid.h>
#include <vtkFloatArray.h>
#include <vtkDataSetWriter.h>
#include <vtkCellData.h>

#ifdef PARALLEL
#include <mpi.h>
#endif
#include <visit-config.h>
#include <TimingsManager.h>
#include <SaveWindowAttributes.h>

#include <algorithm>
#include <climits>
#include <functional>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#ifdef _WIN32
#include <functional>
#include <direct.h>  // for _getcwd, _chdir
#else
#include <unistd.h>
#ifdef NetworkManagerTIME
#include <sys/time.h>
static
double elapsed(timeval &tv0, timeval &tv1)
{
    return tv1.tv_sec - tv0.tv_sec + (tv1.tv_usec - tv0.tv_usec)/1e6;
}
#endif
#endif
using std::vector;
using std::ostringstream;

// ****************************************************************************
// operator<<
//
// Programmer: Burlen Loring
// Creation: Wed Sep  9 08:38:26 PDT 2015
//
// Purpose:
//    dumps render state for debugging.
//
// Modifications:
//
// ****************************************************************************
#if defined NetworkManagerDEBUG || defined NetworkManagerTIME
ostream &operator<<(ostream &os, const NetworkManager::RenderState &rs)
{
    os  << "origWorkingNet = " << rs.origWorkingNet << endl
        << "windowID = " << rs.windowID << endl
        << "window = " << rs.window << endl
        << "windowInfo = " << &rs.windowInfo << endl
        << "needToSetUpWindowContents = " << rs.needToSetUpWindowContents << endl
        << "cellCounts = " << rs.cellCounts << endl
        << "cellCountTotal = " <<  rs.cellCountTotal << endl
        << "haveCells = " << rs.haveCells << endl
        << "renderOnViewer = " << rs.renderOnViewer << endl
        << "stereoType = " << rs.stereoType << endl
        << "timer = " << rs.timer << endl
        << "annotMode = " << rs.annotMode << endl
        << "threeD = " << rs.threeD << endl
        << "twoD = " << rs.twoD << endl
        << "gradientBg = " << rs.gradientBg << endl
        << "getZBuffer = " << rs.getZBuffer << endl
        << "getAlpha = " << rs.getAlpha << endl
        << "zBufferComposite = " << rs.zBufferComposite << endl
        << "allReducePass1 = " << rs.allReducePass1 << endl
        << "allReducePass2 = " << rs.allReducePass2 << endl
//        << "handledAnnotations = " << rs.handledAnnotations << endl
        << "handledCues = " << rs.handledCues << endl
        << "transparency = " << rs.transparency << endl
        << "transparencyInPass1 = " << rs.transparencyInPass1 << endl
        << "transparencyInPass2 = " << rs.transparencyInPass2 << endl
        << "orderComposite = " << rs.orderComposite << endl
        << "compositeOrder = " << rs.compositeOrder << endl
        << "viewportedMode = " << rs.viewportedMode << endl
        << "needZBufferToCompositeEvenIn2D = " << rs.needZBufferToCompositeEvenIn2D << endl
        << "shadowMap = " << rs.shadowMap << endl
        << "depthCues = " << rs.depthCues << endl
        << "imageBasedPlots = " << rs.imageBasedPlots << endl;

    os << "imageType = ";
    if(rs.imageType == ColorRGBImage)
        os << "ColorRGBImage";
    else if(rs.imageType == ColorRGBAImage)
        os << "ColorRGBAImage";
    else if(rs.imageType == LuminanceImage)
        os << "LuminanceImage";
    else if(rs.imageType == ValueImage)
        os << "ValueImage" << endl;
    os << endl;

    return os;
}
#endif

// Programmer: Brad Whitlock, Wed Jan 18 11:38:42 PST 2012
//
// Let's handle missing data as a 2 stage process for now. The first stage
// comes before EEF and identifies elements with missing data, adding an
// avtMissingData array to the pipeline, if called for. The second stage,
// which comes later, removes the elements that have missing data if needed.
// This 2 stage approach is needed to make sure that pick works since it
// ensures that the mesh connectivity for the data cached by EEF will not
// change based on the list of variables that are requested.
//
// If you do not define the TWO_STAGE_MISSING_DATA_FILTERING macro then
// missing data is handled in a single pass. It doesn't work with pick
// because it would seem that the cached dataset in EEF gets a different
// contract than the pick pipeline and thus a different mesh connectivity,
// which can result in walking off the end of arrays.
#define TWO_STAGE_MISSING_DATA_FILTERING

//
// Static functions.
//
static ref_ptr<avtDatabase> GetDatabase(void *, const std::string &,
                                        int, const char *);
static avtDataBinning *GetDataBinningCallbackBridge(void *arg, const char *name);

//
// Static data members of the NetworkManager class.
//
InitializeProgressCallback NetworkManager::initializeProgressCallback = NULL;
void                      *NetworkManager::initializeProgressCallbackArgs=NULL;
ProgressCallback           NetworkManager::progressCallback = NULL;
void                      *NetworkManager::progressCallbackArgs = NULL;


// Create vis window objects.
static void
NetworkManager_CreateVisWindow(int winID, VisWindow *&viswindow, bool &owns, void *)
{
    viswindow = new VisWindow;
    owns = true;
}

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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Made the viswin part of a map of EngineVisWinInfo objects
//    Removed some unnecessary initialization
//
//    Mark C. Miller, Wed Jan  5 10:14:21 PST 2005
//    Added call to NewVisWindow
//
//    Hank Childs, Mon Feb 28 16:56:20 PST 2005
//    Initialize inQueryMode.
//
//    Hank Childs, Fri Aug 26 15:44:48 PDT 2005
//    Register a callback to get databases.
//
//    Hank Childs, Sat Feb 18 11:31:23 PST 2006
//    Added a call back for the apply ddf expression.
//
//    Cyrus Harrison, Tue Feb 19 08:42:51 PST 2008
//    Removed dumpRenders (now controled by avtDebugDumpOptions)
//
//    Brad Whitlock, Tue Jun 24 15:40:29 PDT 2008
//    Added plugin managers.
//
//    Tom Fogal, Wed Dec  9 14:10:01 MST 2009
//    Remove creation of window 0; we'll do it dynamically when needed.
//
//    Hank Childs, Sat Aug 21 14:35:47 PDT 2010
//    Rename DDF to DataBinning.
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    cache the compositers to avoid repeated thread create/join overhead
//
// ****************************************************************************
NetworkManager::NetworkManager(void) : EngineBase(), virtualDatabases()
{
    workingNet = NULL;
    loadBalancer = NULL;
    requireOriginalCells = false;
    requireOriginalNodes = false;
    inQueryMode = false;
    uniqueNetworkId = 0;

    avtCallback::RegisterGetDatabaseCallback(GetDatabase, this);

    avtApplyDataBinningExpression::RegisterGetDataBinningCallback(
        GetDataBinningCallbackBridge, this);

    avtExpressionEvaluatorFilter::RegisterGetDataBinningCallback(
        GetDataBinningCallbackBridge, this);

    databasePlugins = NULL;
    operatorPlugins = NULL;
    plotPlugins = NULL;

    initialized = true;

    zcomp = new ProgrammableCompositer<unsigned char>;
    zcomp->SetThreadPoolSize(2);

    acomp = new ProgrammableCompositer<float>;
    acomp->SetThreadPoolSize(2);

    CreateVisWindowCB = NetworkManager_CreateVisWindow;
    CreateVisWindowCBData = (void*)this;
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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added code to delete VisWindow objects
//
//    Hank Childs, Mon Feb 13 23:14:23 PST 2006
//    Delete the DDFs.
//
//    Brad Whitlock, Tue Jun 24 15:41:08 PDT 2008
//    Added plugin managers.
//
//    Hank Childs, Sat Aug 21 14:35:47 PDT 2010
//    Rename DDF to DataBinning.
//
//    David Camp, Thu Jan 13 11:15:00 PST 2011
//    Added the call to DeleteInstance of the avtColorTables.
//    Help debug memory leaks.
//
//    Brad Whitlock, Thu Oct  4 11:45:14 PDT 2012
//    Change how networks are deleted.
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    delete cached compositers
//
// ****************************************************************************

NetworkManager::~NetworkManager(void)
{
    // Clear out networks, etc
    ClearAllNetworks();

    delete databasePlugins;
    delete operatorPlugins;
    delete plotPlugins;

    avtColorTables::Instance()->DeleteInstance();

    delete zcomp;
    delete acomp;
}

// ****************************************************************************
// Method: NetworkManager::SetDatabasePluginManager
//
// Purpose:
//   Set the database plugin manager we'll use.
//
// Arguments:
//   mgr : The new plugin manager.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 18:23:18 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
NetworkManager::SetDatabasePluginManager(DatabasePluginManager *mgr)
{
    if(databasePlugins != NULL)
        delete databasePlugins;
    databasePlugins = mgr;
}
// ****************************************************************************
// Method: NetworkManager::SetPlotPluginManager
//
// Purpose:
//   Set the plot plugin manager we'll use.
//
// Arguments:
//   mgr : The new plugin manager.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 18:23:18 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
NetworkManager::SetPlotPluginManager(PlotPluginManager *mgr)
{
    if(plotPlugins != NULL)
        delete plotPlugins;
    plotPlugins = mgr;
}

// ****************************************************************************
// Method: NetworkManager::SetOperatorPluginManager
//
// Purpose:
//   Set the operator plugin manager we'll use.
//
// Arguments:
//   mgr : The new plugin manager.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 18:23:18 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
NetworkManager::SetOperatorPluginManager(OperatorPluginManager *mgr)
{
    if(operatorPlugins != NULL)
        delete operatorPlugins;
    operatorPlugins = mgr;
}

// ****************************************************************************
// Method: NetworkManager::GetDatabasePluginManager
//
// Purpose:
//   Return the database plugin manager.
//
// Returns:    the database plugin manager.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 24 15:25:41 PDT 2008
//
// Modifications:
//
// ****************************************************************************

DatabasePluginManager *
NetworkManager::GetDatabasePluginManager() const
{
    return databasePlugins;
}

// ****************************************************************************
// Method: NetworkManager::GetOperatorPluginManager
//
// Purpose:
//   Return the operator plugin manager.
//
// Returns:    the operator plugin manager.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 24 15:26:07 PDT 2008
//
// Modifications:
//
// ****************************************************************************

OperatorPluginManager *
NetworkManager::GetOperatorPluginManager() const
{
    return operatorPlugins;
}

// ****************************************************************************
// Method: NetworkManager::GetPlotPluginManager
//
// Purpose:
//   Return the plot plugin manager.
//
// Returns:    the plot plugin manager.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 24 15:26:28 PDT 2008
//
// Modifications:
//
// ****************************************************************************

PlotPluginManager *
NetworkManager::GetPlotPluginManager() const
{
    return plotPlugins;
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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added code to delete VisWindow objects
//
//    Hank Childs, Thu Mar  2 10:06:33 PST 2006
//    Added support for image based plots.
//
//    Tom Fogal, Wed Sep 28 12:52:49 MDT 2011
//    Allow lazy init of visualization window.
//
//    Brad Whitlock, Wed Sep  7 13:31:27 PDT 2011
//    Tell the NSM to clear its cache.
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    eliminate temporaries and function call in loops condition
//
//    Brad Whitlock, Fri Feb  9 15:54:19 PST 2018
//    Check that we own the vis window before deleting it. We may have
//    borrowed it from the viewer when running in situ.
//
// ****************************************************************************

void
NetworkManager::ClearAllNetworks(void)
{
    size_t nNets = networkCache.size();
    for (size_t i = 0; i < nNets; ++i)
    {
        delete networkCache[i];
        networkCache[i] = NULL;
    }

    size_t nDbs = databaseCache.size();
    for (size_t i = 0; i < nDbs; ++i)
    {
        delete databaseCache[i];
        databaseCache[i] = NULL;
    }

    size_t nPlots = globalCellCounts.size();
    for (size_t i = 0; i < nPlots; ++i)
        globalCellCounts[i] = -1;

    std::map<int, EngineVisWinInfo>::iterator it = viswinMap.begin();
    std::map<int, EngineVisWinInfo>::iterator end = viswinMap.end();
    for (; it != end; ++it)
    {
        it->second.viswin->ClearPlots();
        it->second.plotsCurrentlyInWindow.clear();
        it->second.imageBasedPlots.clear();
        // Only delete the vis window if we own it.
        if(it->second.owns)
            delete it->second.viswin;
    }
    viswinMap.clear();

    avtNamedSelectionManager::GetInstance()->ClearCache();
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
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Modified for viswinMap object
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    move function call out of loop condition, and simplify logic
//
// ****************************************************************************
void
NetworkManager::ClearNetworksWithDatabase(const std::string &db)
{
    // Clear out the networks before the databases.  This is because if we
    // delete the databases first, the networks will have dangling pointers.
    int nNets = static_cast<int>(networkCache.size());
    for (int i = 0; i < nNets; ++i)
    {
        NetnodeDB *ndb;
        if (networkCache[i] && (ndb = networkCache[i]->GetNetDB()) &&
            (ndb->GetFilename() == db))
            DoneWithNetwork(i);
    }

    // Remove the database from the cache and delete it.
    size_t nDbs = databaseCache.size();
    for (size_t i = 0; i < nDbs; ++i)
    {
        if (databaseCache[i] && (databaseCache[i]->GetFilename() == db))
        {
            delete databaseCache[i];
            databaseCache[i] = NULL;
        }
    }
}

// ****************************************************************************
// Method: NetworkManager::GetOpenDatabases
//
// Purpose:
//   Returns a list of databases that have been opened.
//
// Returns:    A string vector of database names.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 15 10:33:01 PDT 2014
//
// Modifications:
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    move function call out of loop condition.
//
// ****************************************************************************

stringVector
NetworkManager::GetOpenDatabases() const
{
    stringVector dbs;
    size_t nDbs = databaseCache.size();
    for (size_t i = 0; i < nDbs; ++i)
    {
        if (databaseCache[i])
            dbs.push_back(databaseCache[i]->GetFilename());
    }
    return dbs;
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
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added code to pass avtDatabaseMetaData to LoadBalancer->AddDatabase
//
//    Hank Childs, Sun Feb 27 12:30:17 PST 2005
//    Added avtDatabase argument to AddDatabase.
//
//    Hank Childs, Tue Sep 20 13:03:00 PDT 2005
//    Don't assume that filename lengths are bigger than 6 characters ['6616].
//
//    Hank Childs, Fri Oct  7 17:22:01 PDT 2005
//    Tell database what it's full name is.  Also, remove some antiquated code
//    that the variable name in the DB info to the empty string.  I believe
//    the code was no longer doing anything useful, besides torpedoing SR
//    and CMFE when used in combination.  The test suite passes without it
//    and there is really no explanation for why we were doing it.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Changed interface to lb->AddDatabase
//
//    Hank Childs, Thu Jan 11 16:10:12 PST 2007
//    Added argument to DatabaseFactory calls.
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//    Added treatAllDBsAsTimeVarying to GetSIL call
//
//    Hank Childs, Fri Feb  1 15:45:28 PST 2008
//    Added argument for loading database plugins.
//
//    Mark C. Miller, Tue Jun 10 22:36:25 PDT 2008
//    Added support for ignoring bad extents from dbs.
//
//    Brad Whitlock, Tue Jun 24 15:45:43 PDT 2008
//    Pass the database plugin manager to the factory.
//
//    Kathleen Biagas, Thu June 27 10:38:54 MST 2013
//    If passed a virtual Database, expand it to the actual files.
//
//    Kathleen Biagas, Fri Jun 26 12:14:51 PDT 2015
//    Moved FileMatchesPatternCB to FileFunctions.
//
// ****************************************************************************

NetnodeDB *
NetworkManager::GetDBFromCache(const std::string &filename, int time,
    const char *format, bool treatAllDBsAsTimeVarying,
    bool fileMayHaveUnloadedPlugin, bool ignoreExtents)
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
    for (size_t i = 0; i < databaseCache.size(); i++)
    {
        if (databaseCache[i] == NULL)
            continue;
        if (databaseCache[i]->GetFilename() == filename)
        {
            cachedDB = databaseCache[i];
            break;
        }
    }

    // use of these makes calls to GetMetaData a little clearer
    const bool forceReadAllCyclesAndTimes = false;
    const bool forceReadThisCycleAndTime  = false;

    // got a match
    if (cachedDB != NULL)
    {
        // even if we found the DB in the cache,
        // we need to update the metadata if its time-varying
        if (treatAllDBsAsTimeVarying ||
            !cachedDB->GetDB()->MetaDataIsInvariant() ||
            !cachedDB->GetDB()->SILIsInvariant())
        {
            cachedDB->GetDB()->GetMetaData(time,
                                           forceReadAllCyclesAndTimes,
                                           forceReadThisCycleAndTime,
                                           treatAllDBsAsTimeVarying);
            cachedDB->GetDB()->GetSIL(time, treatAllDBsAsTimeVarying);
        }

        return cachedDB;
    }

    // No match.  Load a new DB.
    debug3 << "Loading new database" << endl;
    TRY
    {
        if (fileMayHaveUnloadedPlugin)
             GetDatabasePluginManager()->LoadPluginsNow();

        avtDatabase *db = NULL;
        NetnodeDB *netDB = NULL;
        const char *filename_c = filename.c_str();
        std::vector<std::string> plugins;  // unused
        if (filename.length() >= 6 &&
            filename.substr(filename.length() - 6) == ".visit")
        {
            db = avtDatabaseFactory::VisitFile(
                GetDatabasePluginManager(),
                filename_c,
                time,
                plugins,
                format);
        }
        else
        {
            if (filename.find("*") != std::string::npos)
            {
                debug3 << "  " << filename
                       << " is virtual need to expand it." << endl;
                char tmpcwd[1024];
#if defined(_WIN32)
                _getcwd(tmpcwd, 1023);
#else
                char* res = getcwd(tmpcwd, 1023);
                if(res == NULL)
                {
                    debug1 << "failed to get current working directory"
                           << " via getcwd()" << std::endl;
                }
#endif
                tmpcwd[1023] = '\0';

                std::string oldPath(tmpcwd);
                char slash = '/';
                size_t pathIndex = filename.rfind(slash);
                if (pathIndex == std::string::npos)
                {
                    char slash = '\\';
                    pathIndex = filename.rfind(slash);
                }
                std::string path(filename.substr(0, pathIndex));
                size_t dbIndex = filename.rfind(" database");
                std::string pattern(filename.substr(pathIndex+1, dbIndex - pathIndex - 1));
                if (pathIndex != std::string::npos)
                {
#if defined(_WIN32)
                    _chdir(path.c_str());
#else
                    int res = chdir(path.c_str());
                    if(res == -1)
                    {
                        debug1 << "failed to change the current working directory"
                               << " via chdir()" << std::endl;
                    }
#endif
                }
                // look for files that match pattern
                std::vector< std::string > fileNames;
                int returnFullPath = 1;
                void *cb_data[3];
                cb_data[0] = (void *)&fileNames;
                cb_data[1] =  (void *)&pattern;
                cb_data[2] =  (void *)&returnFullPath;
                FileFunctions::ReadAndProcessDirectory(path,
                    FileFunctions::FileMatchesPatternCB, (void*) cb_data,
                    false);
                char **names = new char *[fileNames.size()];
                for (size_t i = 0; i < fileNames.size(); ++i)
                {
                    char *charName = new char[fileNames[i].size() +1];
                    strcpy(charName, fileNames[i].c_str());
                    names[i] = charName;
                }
                if (pathIndex != std::string::npos)
                {
#if defined(_WIN32)
                    _chdir(oldPath.c_str());
#else
                    int res = chdir(oldPath.c_str());
                    if(res == -1)
                    {
                        debug1 << "failed to change the current working directory"
                               << " via chdir()" << std::endl;
                    }
#endif
                }

                db = avtDatabaseFactory::FileList(
                    GetDatabasePluginManager(),
                    names,
                    (int)fileNames.size(),
                    time,
                    plugins,
                    format);

                for (size_t i = 0; i < fileNames.size(); ++i)
                    delete [] names[i];
                delete [] names;
            }
            else
            {
                db = avtDatabaseFactory::FileList(
                    GetDatabasePluginManager(),
                    &filename_c,
                    1,
                    time,
                    plugins,
                    format);
            }
        }

        db->SetFullDBName(filename);
        db->SetIgnoreExtents(ignoreExtents);

        // If we want to open the file at a later timestep, get the
        // SIL so that it contains the right data.
        if ((time > 0) ||
            treatAllDBsAsTimeVarying ||
            (!db->MetaDataIsInvariant()) ||
            (!db->SILIsInvariant()))
        {
            debug2 << "NetworkManager::AddDB: We were instructed to open "
                   << filename.c_str() << " at timestate=" << time
                   << " so we're reading the SIL early."
                   << endl;
            db->GetMetaData(time,
                            forceReadAllCyclesAndTimes,
                            forceReadThisCycleAndTime,
                            treatAllDBsAsTimeVarying);
            db->GetSIL(time, treatAllDBsAsTimeVarying);
        }

        netDB = new NetnodeDB(db);
        databaseCache.push_back(netDB);

        netDB->SetDBInfo(filename, "", time);
        loadBalancer->AddDatabase(filename, db, time);

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
// Method: NetworkManager::StartNetwork
//
// Purpose:
//   Simple StartNetwork routine, passing the most common defaults to the
//   more complex version of the routine.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 10 16:34:18 PST 2011
//
// Modifications:
//
// ****************************************************************************

void
NetworkManager::StartNetwork(const std::string &format,
    const std::string &filename,
    const std::string &var,
    int time)
{
    // Arguments where we can use the defaults.
    std::string selName;
    MaterialAttributes matopts;
    MeshManagementAttributes meshopts;
    bool treatAllDBsAsTimeVarying = false;
    bool fileMayHaveUnloadedPlugin = false;
    bool ignoreExtents = false;
    int windowID = 0;

    // Make empty strings behave as though no format was specified.
    const char *defaultFormat = 0;
    if(format.size() > 0)
        defaultFormat = format.c_str();

    // This sucks.
    if (viswinMap.find(windowID) == viswinMap.end())
        NewVisWindow(windowID);

    // Construct the silr for the variable.
    NetnodeDB *netDB = GetDBFromCache(filename, time, defaultFormat,
                                      treatAllDBsAsTimeVarying,
                                      fileMayHaveUnloadedPlugin,
                                      ignoreExtents);
    if(netDB != NULL)
    {
        std::string leaf = ParsingExprList::GetRealVariable(var);
        avtSILRestriction_p silr = new avtSILRestriction(netDB->GetDB()->
            GetSIL(time, treatAllDBsAsTimeVarying));
        std::string mesh = netDB->GetDB()->GetMetaData(time)->MeshForVar(var);
        silr->SetTopSet(mesh.c_str());
        CompactSILRestrictionAttributes *silrAtts = silr->MakeCompactAttributes();

        StartNetwork(format, filename, var, time, *silrAtts, matopts, meshopts,
                     treatAllDBsAsTimeVarying, ignoreExtents, selName, windowID);

        delete silrAtts;
    }
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
//    Jeremy Meredith, Tue Dec 14 14:02:35 PST 2004
//    The code to get the real variable name was a duplicate of two
//    other spots in VisIt, but this one was out of date.  I
//    refactored the best one into ParsingExprList::GetRealVariable
//    and made this one point to it.
//
//    Brad Whitlock, Tue Feb 22 12:13:44 PDT 2005
//    Made format argument come first. I also made it possible for an empty
//    format string to make the code behave as through no format was specified.
//
//    Hank Childs, Wed Jul 13 10:38:02 PDT 2005
//    Added call to SetTime to prevent UMR when exporting databases.
//
//    Hank Childs, Wed Aug 17 21:55:55 PDT 2005
//    Pass along the material options to simplify the interface reconstruction.
//
//    Jeremy Meredith, Thu Aug 18 17:54:51 PDT 2005
//    Added a new isovolume algorithm, with adjustable VF cutoff.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added mesh management attributes
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added setting of flatness tolerance to data spec for CSG stuff
//
//    Hank Childs, Wed Jan 17 10:42:21 PST 2007
//    Tell the output of the expression evaluator filter that it is *not*
//    transient, since picks sometimes like to look at these.
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//    Added treatAllDBsAsTimeVarying to GetSIL call.
//
//    Hank Childs, Fri Feb  1 15:48:01 PST 2008
//    Add new Boolean argument to GetDBFromCache.
//
//    Mark C. Miller, Tue Jun 10 22:36:25 PDT 2008
//    Added support for ignoring bad extents from dbs.
//
//    Hank Childs, Fri Jan 16 15:30:15 PST 2009
//    Store the variable with the working network.
//
//    John C. Anderson, Thu Jan 15 10:20:20 2009
//    Added annealing time for Discrete MIR.
//
//    Jeremy Meredith, Fri Feb 13 11:22:39 EST 2009
//    Added MIR iteration capability.
//
//    Brad Whitlock, Mon Aug 22 13:34:36 PDT 2011
//    I added named selection creation here instead of at the end of the pipeline.
//
//    Brad Whitlock, Thu Sep  1 11:07:55 PDT 2011
//    Stash the selection name into the data request in case we need it
//    when creating other pipelines based on this one. For instance, the
//    avtExecuteThenTimeLoopFilter creates its own pipeline from the original
//    source so we need the selection name there to apply it.
//
//    Eric Brugger, Mon Oct 31 09:54:07 PDT 2011
//    Add a multi resolution display capability for AMR data.
//
//    Brad Whitlock, Wed Jan 18 11:30:34 PST 2012
//    I added missing data support.
//
//    Eric Brugger, Thu Jan  2 15:18:21 PST 2014
//    I added support for 3d multi resolution data selections.
//
//    Eric Brugger, Wed Jan  8 17:03:56 PST 2014
//    I added a ViewArea to the multi resolution data selection since the
//    view frustum was insufficient in 3d.
//
// ****************************************************************************

void
NetworkManager::StartNetwork(const std::string &format,
                             const std::string &filename,
                             const std::string &var,
                             int time,
                             const CompactSILRestrictionAttributes &atts,
                             const MaterialAttributes &matopts,
                             const MeshManagementAttributes &meshopts,
                             bool treatAllDBsAsTimeVarying,
                             bool ignoreExtents,
                             const std::string &selName,
                             int windowID)
{
    // If the variable is an expression, we need to find a "real" variable
    // name to work with.
    std::string leaf = ParsingExprList::GetRealVariable(var);

    // Make empty strings behave as though no format was specified.
    const char *defaultFormat = 0;
    if(format.size() > 0)
        defaultFormat = format.c_str();

    // Start up the DataNetwork and add the database to it.
    workingNet = new DataNetwork;
    bool fileMayHaveUnloadedPlugin = false;
    NetnodeDB *netDB = GetDBFromCache(filename, time, defaultFormat,
                                      treatAllDBsAsTimeVarying,
                                      fileMayHaveUnloadedPlugin,
                                      ignoreExtents);
    workingNet->SetNetDB(netDB);
    workingNet->SetVariable(leaf);
    netDB->SetDBInfo(filename, leaf, time);
    Netnode *input = netDB;
    NetnodeFilter *filt  = NULL;

    // Add missing data filter to handle missing data that come up the pipe.
    // It must precede the expressions filter so we have all of the variables
    // needed to produce the missing data mask.
    avtMissingDataFilter *mdf = new avtMissingDataFilter();
    mdf->SetMetaData(netDB->GetDB()->GetMetaData(time));
#ifdef TWO_STAGE_MISSING_DATA_FILTERING
    mdf->SetGenerateMode(true);
    mdf->SetRemoveMode(false);
    filt = new NetnodeFilter(mdf, "MissingDataIdentification");
#else
    filt = new NetnodeFilter(mdf, "MissingData");
#endif
    filt->GetInputNodes().push_back(input);
    workingNet->AddNode(filt);
    input = filt;

    // Add ExpressionEvaluatorFilter to handle expressions that come up the pipe.
    avtExpressionEvaluatorFilter *f = new avtExpressionEvaluatorFilter();
    filt = new NetnodeFilter(f, "ExpressionEvaluator");
    filt->GetInputNodes().push_back(input);
    f->GetOutput()->SetTransientStatus(false); // So the output can be queried.
    workingNet->SetExpressionNode(filt);
    workingNet->AddNode(filt);
    input = filt;

#ifdef TWO_STAGE_MISSING_DATA_FILTERING
    // Add missing data filter after expressions and selections to actually
    // remove any elements that were tagged as missing data. We have to split
    // the processing of missing data so the dataset cached by the EEF will
    // contain all cells so pick doesn't barf later.
    avtMissingDataFilter *mdf2 = new avtMissingDataFilter();
    mdf2->SetMetaData(netDB->GetDB()->GetMetaData(time));
    mdf2->SetGenerateMode(false);
    mdf2->SetRemoveMode(true);
    filt = new NetnodeFilter(mdf2, "MissingDataRemoval");
    filt->GetInputNodes().push_back(input);
    workingNet->AddNode(filt);
    input = filt;
#endif

    // Selections
    if(!selName.empty())
    {
        avtNamedSelectionFilter *f = new avtNamedSelectionFilter();
        f->SetSelectionName(selName);
        filt = new NetnodeFilter(f, "NamedSelection");
        filt->GetInputNodes().push_back(input);
        workingNet->SetSelectionName(selName);
        workingNet->AddNode(filt);
        input = filt;
    }

    // If we are in multiresolution mode then add a MultiresFilter
    // right after the expression filter.
    VisWindow *visWin = viswinMap[windowID].viswin;
    if (visWin->GetMultiresolutionMode())
    {
        int size[2];
        visWin->GetSize(size[0], size[1]);

        // Get the 2D transform matrix, viewport, view area and extents.
        double transform2D[16];
        double viewport2D[6];
        double area2D;
        double extents2D[6];
        avtView2D view2D = visWin->GetView2D();
        if (!view2D.windowValid)
        {
            for (int i = 0; i < 16; i++)
                transform2D[i] = DBL_MAX;
            for (int i = 0; i < 6; i++)
                viewport2D[i] = DBL_MAX;
            area2D = DBL_MAX;
            for (int i = 0; i < 6; i++)
                extents2D[i] = DBL_MAX;
        }
        else
        {
            view2D.GetActualViewport(viewport2D, size[0], size[1]);
            double ratioWindow = double(size[0]) / double(size[1]);
            double ratioViewport = (viewport2D[1] - viewport2D[0]) /
                                   (viewport2D[3] - viewport2D[2]);
            double ratio = ratioWindow * ratioViewport;
            view2D.GetCompositeProjectionTransformMatrix(transform2D, ratio);
            view2D.CalculateExtentsAndArea(extents2D, area2D, transform2D);
        }

        // Get the 3D transform matrix, viewport, view area and extents.
        double transform3D[16];
        double viewport3D[6];
        double area3D;
        double extents3D[6];
        avtView3D view3D = visWin->GetView3D();
        if (!view3D.windowValid)
        {
            for (int i = 0; i < 16; i++)
                transform3D[i] = DBL_MAX;
            for (int i = 0; i < 6; i++)
                viewport3D[i] = DBL_MAX;
            area3D = DBL_MAX;
            for (int i = 0; i < 6; i++)
                extents3D[i] = DBL_MAX;
        }
        else
        {
            double ratio = double(size[0]) / double(size[1]);
            view3D.GetCompositeProjectionTransformMatrix(transform3D, ratio);
            view3D.CalculateExtentsAndArea(extents3D, area3D, transform3D);
        }

        // Get the cell size.
        double cellSize = visWin->GetMultiresolutionCellSize();

        // Add the multires filter.
        avtMultiresFilter *f2 = new avtMultiresFilter(transform2D, transform3D,
            viewport2D, viewport3D, size, area2D, area3D, extents2D, extents3D,
            cellSize);
        filt = new NetnodeFilter(f2, "MultiresFilter");
        filt->GetInputNodes().push_back(input);
        workingNet->AddNode(filt);
        input = filt;
    }

    // Push the last filter onto the working list.
    workingNetnodeList.push_back(filt);

    // Push the variable name onto the name stack.
    nameStack.push_back(var);
    debug4 << "NetworkManager::AddDB: Adding " << var.c_str()
           << " to the name stack" << endl;

    // Set up the data spec.
    avtSILRestriction_p silr =
        new avtSILRestriction(workingNet->GetNetDB()->GetDB()->
            GetSIL(time, treatAllDBsAsTimeVarying), atts);
    avtDataRequest *dataRequest = new avtDataRequest(var.c_str(), time, silr);

    // Set up some options from the data specification
    dataRequest->SetNeedMixedVariableReconstruction(matopts.GetForceMIR());
    dataRequest->SetNeedSmoothMaterialInterfaces(matopts.GetSmoothing());
    dataRequest->SetNeedCleanZonesOnly(matopts.GetCleanZonesOnly());
    dataRequest->SetNeedValidFaceConnectivity(matopts.GetNeedValidConnectivity());
    dataRequest->SetMIRAlgorithm(matopts.GetAlgorithm());
    dataRequest->SetMIRNumIterations(matopts.GetIterationEnabled() ?
                                     matopts.GetNumIterations() : 0);
    dataRequest->SetMIRIterationDamping(matopts.GetIterationDamping());
    dataRequest->SetSimplifyHeavilyMixedZones(matopts.GetSimplifyHeavilyMixedZones());
    dataRequest->SetMaxMaterialsPerZone(matopts.GetMaxMaterialsPerZone());
    dataRequest->SetIsovolumeMIRVF(matopts.GetIsoVolumeFraction());
    dataRequest->SetAnnealingTime(matopts.GetAnnealingTime());
    dataRequest->SetDiscTol(meshopts.GetDiscretizationTolerance()[0]);
    dataRequest->SetFlatTol(meshopts.GetDiscretizationTolerance()[1]);
    dataRequest->SetDiscMode(meshopts.GetDiscretizationMode());
    dataRequest->SetDiscBoundaryOnly(meshopts.GetDiscretizeBoundaryOnly());
    dataRequest->SetPassNativeCSG(meshopts.GetPassNativeCSG());
    dataRequest->SetSelectionName(selName);
    workingNet->SetDataSpec(dataRequest);
    workingNet->SetTime(dataRequest->GetTimestep());

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
//   Brad Whitlock, Tue Feb 22 12:17:27 PDT 2005
//   I changed the code so it an empty format string behaves as though no
//   format was specified.
//
//   Hank Childs, Sun Feb 27 12:30:17 PST 2005
//   Added avtDatabase argument to AddDatabase.
//
//   Hank Childs, Fri Oct  7 09:30:57 PDT 2005
//   Tell the database what its full name is.
//
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Changed interface to lb->AddDatabase
//
//   Hank Childs, Thu Jan 11 16:10:12 PST 2007
//   Added argument to DatabaseFactory calls.
//
//   Brad Whitlock, Tue Jun 24 15:48:27 PDT 2008
//   Pass the database plugin manager to the database factory.
//
// ****************************************************************************

void
NetworkManager::DefineDB(const std::string &dbName, const std::string &dbPath,
    const stringVector &files, int time, const std::string &format)
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
    for(size_t i = 0; i < files.size(); ++i)
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
            for(size_t i = 0; i < databaseCache.size() && !found; ++i)
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
    for (size_t i = 0; i < databaseCache.size(); i++)
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

        // Make empty strings behave as though no format was specified.
        const char *defaultFormat = 0;
        if(format.size() > 0)
            defaultFormat = format.c_str();

        std::vector<std::string> plugins;  // unused
        if (filesWithPath.size() > 0)
        {
            // Make an array of pointers that we can pass to the database
            // factory so we can create a database based on a virtual
            // file.
            names = new const char *[filesWithPath.size()];
            for(size_t i = 0; i < filesWithPath.size(); ++i)
                names[i] = filesWithPath[i].c_str();
            db = avtDatabaseFactory::FileList(GetDatabasePluginManager(),
                  names, (int)filesWithPath.size(), time, plugins, defaultFormat);
            delete [] names;
            names = 0;

            // Add the virtual database to the virtual database map.
            virtualDatabases[dbName] = filesWithPath;
            debug3 << "NetworkManager::DefineDB: Added new virtual database "
                   << "definition for " << dbName.c_str() << endl;
        }
        else if (dbName.substr(dbName.length() - 6) == ".visit")
        {
            db = avtDatabaseFactory::VisitFile(GetDatabasePluginManager(),
                dbName_c, time, plugins, defaultFormat);
        }
        else
        {
            db = avtDatabaseFactory::FileList(GetDatabasePluginManager(),
                &dbName_c, 1, time, plugins, defaultFormat);
        }
        db->SetFullDBName(dbName);

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
        loadBalancer->AddDatabase(dbName, db, time);
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
//    Brad Whitlock, Tue Jun 24 16:08:45 PDT 2008
//    Changed how the operator plugin manager is accessed.
//
// ****************************************************************************
void
NetworkManager::AddFilter(const std::string &filtertype,
                          const AttributeGroup *atts,
                          const unsigned int nInputs)
{
    // Check that we have a network to work on.
    if (workingNet == NULL)
    {
        debug1 << "Adding a filter to a non-existent network." << endl;
        EXCEPTION0(ImproperUseException);
    }

    avtPluginFilter *f = GetOperatorPluginManager()->
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
    for (unsigned int i = 0; i < nInputs; i++)
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
//    Brad Whitlock, Wed Mar 21 23:00:00 PST 2007
//    Added plotName.
//
//    Gunther H. Weber, Thu Apr 12 10:52:36 PDT 2007
//    Add filter to beginning of pipeline if necessary
//
//    Brad Whitlock, Tue Jun 24 16:09:19 PDT 2008
//    Changed how the plot plugin manager is accessed.
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    Add support for depth peeling and ordered compositing
//
// ****************************************************************************

void
NetworkManager::MakePlot(const std::string &plotName, const std::string &pluginID,
    const AttributeGroup *atts, const std::vector<double> &dataExtents)
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

    avtPlot *p = GetPlotPluginManager()->
        GetEnginePluginInfo(pluginID)->AllocAvtPlot();

    // Check, whether plot wants to place a filter at the beginning of
    // the pipeline
    if (avtFilter *f = p->GetFilterForTopOfPipeline())
    {
        NetnodeFilter *filt = new NetnodeFilter(f, "InsertedPlotFilter");
        //f->GetOutput()->SetTransientStatus(true);

        if (workingNet->GetNodeList().size() > 1)
            workingNet->AddFilterNodeAfterExpressionEvaluator(filt);
        else
        {
            std::vector<Netnode*> &filtInputs = filt->GetInputNodes();
            Netnode *n = workingNetnodeList.back();
            workingNetnodeList.pop_back();
            filtInputs.push_back(n);
            // Push the filter onto the working list.
            workingNetnodeList.push_back(filt);
            workingNet->AddNode(filt);
        }
    }

    p->SetDataExtents(dataExtents);
    workingNet->SetPlot(p);
    workingNet->GetPlot()->SetAtts(atts);
    workingNet->SetPlottype(pluginID);
    workingNet->SetPlotName(plotName);
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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added code to set network's window ID and manage multiple VisWindow
//    objects
//
//    Gunther H. Weber, Thu Apr 12 11:00:57 PDT 2007
//    Added missing space to debug message
//
//    Hank Childs, Mon Apr  6 13:06:22 PDT 2009
//    Add support for sending named selection info to all filters.
//
//    Brad Whitlock, Tue Aug 10 16:11:25 PDT 2010
//    Use find() method.
//
//    Brad Whitlock, Mon Aug 22 10:39:12 PDT 2011
//    Changed how named selections are finally set up.
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    use std::map::count rather than std::map::find if you need
//    to check if something is in the map. it's faster because find
//    has to construct an iterator.
//
// ****************************************************************************

int
NetworkManager::EndNetwork(int windowID)
{
    // Checking to see if the network has been built successfully.
    if (workingNetnodeList.size() != 1)
    {
        debug1 << "Network building still in progress.  Filter required to "
               << "absorb " << workingNetnodeList.size() << " nodes."  << endl;

        EXCEPTION0(ImproperUseException);
    }

    // If a named selection has been applied, make sure all of the filters
    // know about it.
    if (!workingNet->GetSelectionName().empty())
    {
        const std::string &selName = workingNet->GetSelectionName();
        std::vector<Netnode *> netnodes = workingNet->GetNodeList();
        for (size_t i = 0 ; i < netnodes.size() ; i++)
        {
            avtFilter *filt = netnodes[i]->GetFilter();
            if (filt == NULL)
                continue;
            filt->RegisterNamedSelection(selName);
        }
        workingNet->GetPlot()->RegisterNamedSelection(selName);
    }

    // set the pipeline specification
    int pipelineIndex =
        loadBalancer->AddPipeline(workingNet->GetNetDB()->GetFilename());
    avtContract_p contract =
        new avtContract(workingNet->GetDataSpec(), pipelineIndex);
    workingNet->SetContract(contract);

    // Connect the workingNet to the workingNetnodeList.
    workingNet->SetTerminalNode(workingNetnodeList[0]);

    // Push the working net onto the network caches.
    workingNet->SetNetID(uniqueNetworkId++);
    workingNet->SetWinID(windowID);
    networkCache.push_back(workingNet);
    globalCellCounts.push_back(-1);

    //
    // If this plot this network is associated with is in a window we haven't
    // seen before, make a new VisWindow for it
    //
    if (!viswinMap.count(windowID))
        NewVisWindow(windowID);

    return workingNet->GetNetID();
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
// Method: NetworkManager::ValidNetworkId
//
// Purpose:
//   Check a network id for validity.
//
// Arguments:
//   id : The network id we're checking.
//
// Returns:    True if the network is valid; False otherwise.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 23 12:01:08 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
NetworkManager::ValidNetworkId(int id) const
{
    bool valid = (id >= 0 && id < (int)networkCache.size());
    if(!valid)
    {
        debug1 << "Internal error: asked to reuse network ID " << id
            << ", which is not in num saved networks [0," << networkCache.size() << ")"
            << endl;
    }
    return valid;
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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Changed workingNet->GetID to workingNet->GetNetID
//
//    Hank Childs, Fri Jan 16 14:39:33 PST 2009
//    Set the proper database info when calling UseNetwork.
//
//    Brad Whitlock, Tue Sep 23 11:47:39 PDT 2014
//    Fix an unsigned vs signed comparison the right way.
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

    if (!ValidNetworkId(id))
    {
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
    NetnodeDB *db = workingNet->GetNetDB();
    int time = workingNet->GetTime();
    std::string filename = db->GetFilename();
    std::string var = workingNet->GetVariable();
    db->SetDBInfo(filename, var, time);
    int pipelineIndex = workingNet->GetContract()->GetPipelineIndex();
    loadBalancer->ResetPipeline(pipelineIndex);

    if (id != workingNet->GetNetID())
    {
        debug1 << "Internal error: network at position[" << id <<
            "] does not have same id (" << workingNet->GetNetID() << ")"
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
//  Modifications:
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Changed workingNet->GetID to workingNet->GetNetID
//
// ****************************************************************************
int
NetworkManager::GetCurrentNetworkId(void) const
{
    if (workingNet == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }

    return workingNet->GetNetID();
}

// ****************************************************************************
//  Method: NetworkManager::GetCurrentWindowId
//
//  Purpose:
//      Gets the window id of the current network.
//
//  Programmer: Mark C. Miller
//  Creation:   January 4, 2005
//
// ****************************************************************************
int
NetworkManager::GetCurrentWindowId(void) const
{
    if (workingNet == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }

    return workingNet->GetWinID();
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
//  Modifications:
//
//    Hank Childs, Thu Dec  2 09:48:29 PST 2004
//    Look out for overflow.
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Modified to compute counts for networks in only the specified window
//
//    Burlen Loring, Mon Sep  7 05:41:30 PDT 2015
//    use long long for cell count
//
// ****************************************************************************

long long
NetworkManager::GetTotalGlobalCellCounts(int winID) const
{
    long long sum = 0;
    size_t nNets= networkCache.size();
    for (size_t i = 0; i < nNets; ++i)
    {
        if (networkCache[i] == NULL ||
            networkCache[i]->GetWinID() != winID)
            continue;

        // Make sure we don't have an overflow issue.
        if (globalCellCounts[i] == INT_MAX)
            return INT_MAX;

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
NetworkManager::SetGlobalCellCount(int netId, long long cellCount)
{
   globalCellCounts[netId] = cellCount;
}


// ****************************************************************************
//  Method: NetworkManager::GetScalableThreshold
//
//  Purpose: Get the effective scalable threshold
//
//  Programmer: Mark C. Miller
//  Creation:   May 11, 2004
//
//  Modifications:
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Modified to get information for specific window
//
//    Brad Whitlock, Fri Feb 15 15:30:31 PST 2008
//    Test value of iterator.
//
//    Burlen Loring, Wed Sep  2 09:19:51 PDT 2015
//    Factor implementation into a private method that
//    doesn't need to search for the current window/do error
//    checking etc when used correctly (ie from within
//    the class)
//
// ****************************************************************************
int
NetworkManager::GetScalableThreshold(int windowID) const
{
    std::map<int, EngineVisWinInfo>::const_iterator it;
    it = viswinMap.find(windowID);
    if(it == viswinMap.end())
    {
        // requested window doesn't currently exist
        int scalableAutoThreshold
            = RenderingAttributes::DEFAULT_SCALABLE_AUTO_THRESHOLD;

        RenderingAttributes::TriStateMode
        scalableActivationMode
            = static_cast<RenderingAttributes::TriStateMode>(
               RenderingAttributes::DEFAULT_SCALABLE_ACTIVATION_MODE);

        return  RenderingAttributes::GetEffectiveScalableThreshold(
                                    scalableActivationMode,
                                    scalableAutoThreshold);
    }
    return
        GetScalableThreshold(it->second.windowAttributes.GetRenderAtts());
}

// ****************************************************************************
//  Method: NetworkManager::GetScalableThreshold
//
//  Purpose: Get the effective scalable threshold
//
//  Programmer: Burlen Loring
//  Creation:   Wed Sep  2 09:13:12 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

int
NetworkManager::GetScalableThreshold(const RenderingAttributes &renderAtts) const
{
    int scalableAutoThreshold = renderAtts.GetScalableAutoThreshold();

    RenderingAttributes::TriStateMode
    scalableActivationMode = renderAtts.GetScalableActivationMode();

    return RenderingAttributes::GetEffectiveScalableThreshold(
                                    scalableActivationMode,
                                    scalableAutoThreshold);
}


// ****************************************************************************
// Method:  NetworkManager::GetCompactDomainsThreshold
//
// Purpose: Get/Set compact domains options.
//
//
// Programmer:  Dave Pugmire
// Creation:    August 24, 2010
//
// ****************************************************************************

int
NetworkManager::GetCompactDomainsThreshold(int windowId) const
{
    int compactDomainsAutoThreshold = RenderingAttributes::DEFAULT_COMPACT_DOMAINS_AUTO_THRESHOLD;
    RenderingAttributes::TriStateMode compactDomainsActivationMode =
       (RenderingAttributes::TriStateMode)RenderingAttributes::DEFAULT_COMPACT_DOMAINS_ACTIVATION_MODE;

    // since we're in a const method, we can't use the [] operator to index
    // into the map directly becuase that operator will modify the map if the
    // key is new
    std::map<int, EngineVisWinInfo>::const_iterator it;
    it = viswinMap.find(windowId);
    if(it != viswinMap.end())
    {
        const EngineVisWinInfo &viswinInfo = it->second;
        const WindowAttributes &windowAttributes = viswinInfo.windowAttributes;

        compactDomainsAutoThreshold =
            windowAttributes.GetRenderAtts().GetCompactDomainsAutoThreshold();
        compactDomainsActivationMode =
            windowAttributes.GetRenderAtts().GetCompactDomainsActivationMode();
    }


    int t = RenderingAttributes::GetEffectiveCompactDomainsThreshold(compactDomainsActivationMode,
                                                                     compactDomainsAutoThreshold);
    return t;
}

// ****************************************************************************
//  Method: NetworkManager::GetShouldUseCompression
//
//  Purpose: Determine if we should use compression
//
//  Programmer: Mark C. Miller
//  Creation:   November 3, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Jun  9 15:05:25 PDT 2006
//    Remove unused variable.
//
// ****************************************************************************
bool
NetworkManager::GetShouldUseCompression(int windowID) const
{
    RenderingAttributes::TriStateMode compressionActivationMode;

    // since we're in a const method, we can't use the [] operator to index
    // into the map directly becuase that operator will modify the map if the
    // key is new
    std::map<int, EngineVisWinInfo>::const_iterator it;
    it = viswinMap.find(windowID);
    const EngineVisWinInfo &viswinInfo = it->second;
    const WindowAttributes &windowAttributes = viswinInfo.windowAttributes;

    compressionActivationMode =
        windowAttributes.GetRenderAtts().GetCompressionActivationMode();

    return compressionActivationMode != RenderingAttributes::Never;
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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Modified to manage multiple VisWindow objects
//
//    Hank Childs, Thu Mar  2 10:06:33 PST 2006
//    Clear out image based plots.
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    move function call out of loop condition
//
// ****************************************************************************

void
NetworkManager::DoneWithNetwork(int id)
{
    if (!ValidNetworkId(id))
    {
        EXCEPTION0(ImproperUseException);
    }

    if (networkCache[id] != NULL)
    {
        int thisNetworksWinID = networkCache[id]->GetWinID();

        viswinMap[thisNetworksWinID].viswin->ClearPlots();
        viswinMap[thisNetworksWinID].plotsCurrentlyInWindow.clear();
        viswinMap[thisNetworksWinID].imageBasedPlots.clear();

        // Delete the associated VisWindow if this is the last plot that
        // references it
        bool otherNetsUseThisWindow = false;
        int nNets = static_cast<int>(networkCache.size());
        for (int i = 0; i < nNets; ++i)
        {
            if (i == id)
                continue;

            if (networkCache[i] && (thisNetworksWinID ==
                                    networkCache[i]->GetWinID()))
            {
                otherNetsUseThisWindow = true;
                break;
            }
        }

        delete networkCache[id];
        networkCache[id] = NULL;
        globalCellCounts[id] = -1;

        // mark this VisWindow for deletion, but never delete window 0
        if (!otherNetsUseThisWindow && thisNetworksWinID)
        {
            debug2 << "Marking VisWindow for Deletion id=" << thisNetworksWinID << endl;
            // TODO -- why not just delete it here??
            viswinMap[thisNetworksWinID].markedForDeletion = true;
        }

    }
    else
    {
        debug1 << "Warning: DoneWithNetwork called on previously "
            "cleared network." << endl;
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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Changed networkCache[id]->GetID() to networkCache[id]->GetNetID()
//
// ****************************************************************************
void
NetworkManager::UpdatePlotAtts(int id, const AttributeGroup *atts)
{
    if (!ValidNetworkId(id))
    {
        EXCEPTION0(ImproperUseException);
    }

    if (networkCache[id] == NULL)
    {
        debug1 << "Asked to update the plot attributes of a network that has "
               << "already been cleared." << endl;
        EXCEPTION0(ImproperUseException);
    }
    if (networkCache[id]->GetNetID() != id)
    {
        debug1 << "Internal error: network at position[" << id
            << "] does not have " << "the same id ("
            << networkCache[id]->GetNetID() << ")" << endl;
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
//    Set flag in dataRequest based on value of requireOriginalCells.
//
//    Hank Childs, Tue Jun 18 16:49:01 PDT 2002
//    Set MayRequireZones before Executing.
//
//    Mark C. Miller, 10Nov02
//    Made changes to support scalable rendering
//
//    Brad Whitlock, Thu Jan 16 11:07:10 PDT 2003
//    I replaced the return in the TRY/CATCH/ENDTRY block with CATCH_RETURN2 so
//    that fake exceptions work again.
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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Modified to use viswinMap
//
//    Hank Childs, Sun Mar  6 07:29:34 PST 2005
//    Turn off dynamic load balancing if we are in query mode.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL.
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    clean up a compiler warning. documemt use of INT_MAX
//
// ****************************************************************************

avtDataObjectWriter_p
NetworkManager::GetOutput(bool respondWithNullData, bool calledForRender,
    float *cellCountMultiplier)
{
    (void)calledForRender;

    // Is the network complete?
    if (*(workingNet->GetPlot()) == NULL)
    {
        debug1 << "Never told what type of plot to use." << endl;
        EXCEPTION0(ImproperUseException);
    }

    TRY
    {
        int winID = workingNet->GetWinID();
        WindowAttributes &windowAttributes = viswinMap[winID].windowAttributes;

        // Hook up the network
        avtDataObject_p output = workingNet->GetOutput();

        workingNet->GetContract()->GetDataRequest()->
            SetMayRequireZones(requireOriginalCells);
        workingNet->GetContract()->GetDataRequest()->
            SetMayRequireNodes(requireOriginalNodes);
        if (inQueryMode)
            workingNet->GetContract()->NoStreaming();

        avtDataObjectWriter_p writer = workingNet->GetWriter(output,
                                          workingNet->GetContract(),
                                          &windowAttributes);

        // get the SR multiplier. note: the default for image based plots
        // (ie volume rendering) is INT_MAX. for other plots the default
        // is 1.0.
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
    CATCHALL
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
//  Method: NetworkManager::HasNonMeshPlots
//
//  Purpose: Scan the plot list to see if any plots in it are NOT mesh plots
//
//  Programmer:  Mark C. Miller
//  Creation:    May 12, 2005
//
//  Modifications:
//
//    Hank Childs, Sun Feb 25 12:47:26 PST 2007
//    Make logic in this method match that of the ViewerPlotList, where
//    Label plots are counted as mesh plots.
//
// ****************************************************************************
bool
NetworkManager::HasNonMeshPlots(const intVector plotIds)
{
    bool hasNonMeshPlots = false;
    for (size_t i = 0; i < plotIds.size(); i++)
    {
        workingNet = NULL;
        UseNetwork(plotIds[i]);
        if (std::string(workingNet->GetPlot()->GetName()) != "MeshPlot" &&
            std::string(workingNet->GetPlot()->GetName()) != "LabelPlot")
        {
            hasNonMeshPlots = true;
            break;
        }
    }
    return hasNonMeshPlots;
}


// ****************************************************************************
//  Method: NetworkManager::NeedZBufferToCompositeEvenIn2D
//
//  Purpose:
//      Determines if we need ZBuffer information, even in 2D mode.  This is
//      necessary because some plots (read: streamline) use their own custom
//      data decompositions.  And if there is a second plot (read: pseudocolor)
//      then they will overlap and get blended.  This issue normally doesn't
//      occur because it is determined before the composite.  But when the
//      data decomposition is different between plots, then the burden is
//      shifted to the compositor.  Hence the need for the ZBuffer, so it
//      can properly respect the shift factor.
//
//  Programmer:  Hank Childs
//  Creation:    August 13, 2008
//
//  Modifications:
//
//    Hank Childs, Thu Aug 14 14:31:49 PDT 2008
//    Make sure the workingNet is set to NULL.  This caused problems with
//    SR transitions (tests/rendering/offscreensave.py) and with picking
//    in SR mode (tests/queries/queriesOverTime.py).
//
// ****************************************************************************

bool
NetworkManager::NeedZBufferToCompositeEvenIn2D(const intVector plotIds)
{
    bool needsZBufferToCompositeEvenIn2D = false;

    for (size_t i = 0; i < plotIds.size(); i++)
    {
        workingNet = NULL;
        UseNetwork(plotIds[i]);
        if (workingNet->GetPlot()->NeedZBufferToCompositeEvenIn2D())
        {
            needsZBufferToCompositeEvenIn2D = true;
            break;
        }
    }

    return needsZBufferToCompositeEvenIn2D;
}


// ****************************************************************************
//  Method: NetworkManager::Render
//
//  Purpose: do a software scalable render
//
//  Programmer:  Mark C. Miller
//  Creation:    08Apr03
//
//  Modifications:
//
//    Mark C. Miller, Thu Apr  1 11:06:09 PST 2004
//    Removed call to AdjustWindowAttributes
//    Added use of viewported screen capture
//
//    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//    Added call to local GetScalableThreshold method
//
//    Mark C. Miller, Thu May 27 11:05:15 PDT 2004
//    Removed window attributes arg from GetActor method
//    Added code to push colors into all plots
//    Made triangle count a debug5 statement (from debug1)
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added visualCueList arg to SetAnnotationAttributes
//
//    Mark C. Miller, Wed Aug 11 23:42:18 PDT 2004
//    Added code to get cell count multiplier for SR mode and adjust
//    cell counts for SR threshold
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
//    Hank Childs, Wed Nov  3 14:12:29 PST 2004
//    Fix typo and fix problem with shadows in parallel.
//
//    Hank Childs, Wed Nov 24 17:28:07 PST 2004
//    Added imageBasedPlots.
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Modified to use viswinMap
//
//    Mark C. Miller, Mon Mar  7 12:06:08 PST 2005
//    Changed calls from GetNumTriangles to GetNumPrimitives.
//
//    Mark C. Miller, Wed Jun  8 11:03:31 PDT 2005
//    Added code to deal with opaque mesh plots correctly.
//
//    Hank Childs, Sun Dec  4 16:58:32 PST 2005
//    Added progress to scalable renderings.
//
//    Hank Childs, Fri Mar  3 08:32:02 PST 2006
//    Do not do shadowing in 2D.
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye arg as well as logic to force rendering for one or the
//    other eye.
//
//    Mark C. Miller, Mon Aug 14 12:26:18 PDT 2006
//    Added code to return viswin to its true stereo type upon completion
//
//    Brad Whitlock, Wed Mar 21 23:06:04 PST 2007
//    Set the actor's name to that of the plot so legend attributes can be
//    set via the annotation object list.
//
//    Jeremy Meredith, Wed Aug 29 15:24:13 EDT 2007
//    Added depth cueing.
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
//    Cyrus Harrison, Tue Feb 19 09:33:47 PST 2008
//    Changed to use avtDebugDumpOptions::DumpEnabled to determine if
//    debug sr images should be created.
//
//    Tom Fogal, Tue Jun 10 14:51:51 EDT 2008
//    Use RenderSetup to greatly shorten this method.
//
//    Tom Fogal, Thu Jun 12 14:57:29 EDT 2008
//    RenderSetup should be in the TRY block.
//    When splitting the function, we turned a conditional block into an
//    unconditional block.  This removes the block structure, unindenting a
//    large block.
//
//    Tom Fogal, Fri Jun 13 13:23:00 EDT 2008
//    Utilize RenderGeometry for initial rendering.
//    Utilize RenderTranslucent for second pass rendering.
//    Utilize RenderShadows.
//
//    Tom Fogal, Mon Jun 16 09:55:02 EDT 2008
//    Add/Use SetCompositerBackground
//    Add/Use MakeCompositer
//    Utilize RenderDepthCues.
//    Utilize RenderPostProcess.
//
//    Tom Fogal, Thu Jun 19 10:38:32 EDT 2008
//    Moved this->r_mgmt.viewportedMode calculation into here, because we
//    shouldn't try to query properties of the viswindow until we've set it up
//    (in RenderSetup)!  This fixes a lot of SR mode bugs.
//
//    Tom Fogal, Mon Jun 23 11:01:15 EDT 2008
//    Use the new CreateNullDataWriter when switching to SR mode.
//
//    Tom Fogal, Fri Jun 27 11:14:25 EDT 2008
//    Reverse the boolean for detailing gradient background to MakeCompositer;
//    had interpreted it wrong in the original implementation (a background
//    mode of 0 is /not/ a gradient BG).
//
//    Tom Fogal, Mon Jul 14 09:03:18 PDT 2008
//    Use StartTimer instead of a StackTimer.
//
//    Tom Fogal, Fri Jul 18 15:15:37 EDT 2008
//    Use Shadowing and DepthCueing methods instead of querying those modes
//    inline.
//
//    Tom Fogal, Mon Sep  1 14:22:57 EDT 2008
//    Removed asserts.
//
//    Tom Fogal, Thu Oct 23 11:41:24 EDT 2008
//    Check to see if non-root processes have null images, and skip a
//    compositing step if they do.  This prevents the volume plot from
//    having swathes of faded image data.
//
//    Hank Childs, Fri Nov 14 09:32:03 PST 2008
//    Add a bunch of timings statements.
//
//    Brad Whitlock, Tue Mar  3 09:37:58 PST 2009
//    I added the checkThreshold argument and made the routine return
//    avtDataObject_p.
//
//    Tom Fogal, Thu May 28 15:42:19 MDT 2009
//    Invalidate the cache earlier, to make sure we don't use cached values
//    from the last rendering.
//    Move an image dump up.  It was giving misleading results before.
//
//    Tom Fogal, Fri May 29 20:50:23 MDT 2009
//    Remove transparency cache invalidation from here ...
//
//    Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL.
//
//    Tom Fogal, Tue Jul 21 19:23:53 MDT 2009
//    Account for SR threshold check.
//    Make sure to call *our* render methods.  Allows derived classes to fall
//    back on this class, even if it overrides rendering methods.
//
//    Tom Fogal, Wed May 18 12:31:48 MDT 2011
//    Adjust for new debug image dumping interface.
//
//    Brad Whitlock, Wed Oct 29 09:57:16 PDT 2014
//    Don't call RenderBalance. It was doing collective communication and
//    causing a hang when not all ranks are writing debugging logs. It did not
//    provide much information anyway.
//
//    Burlen Loring, Tue Aug 18 13:30:14 PDT 2015
//    Added option to do depth peeling for transparent geometry
//
//    Burlen Loring, Thu Sep  3 10:24:42 PDT 2015
//    Factored into internal method that can be called from IceTNetworkManager
//    to eliminate duplicated setup work when this class is called as a fallback
//    ie currently for translucent rendering.
//
//    Brad Whitlock, Thu Sep 21 16:46:50 PDT 2017
//    Added getAlpha.
//
// ****************************************************************************

avtDataObject_p
NetworkManager::Render(avtImageType imgT, bool getZBuffer,
    intVector plotIds, bool checkThreshold,
    int annotMode, int windowID, bool leftEye,
    int &outImgWidth, int &outImgHeight)
{
    StackTimer t0("NetworkManager::Render");
    DataNetwork *origWorkingNet = workingNet;
    avtDataObject_p output;

    TRY
    {
        RenderSetup(imgT, windowID, plotIds, getZBuffer,
            annotMode, leftEye, checkThreshold);

        // Return the window size in case the image we make is empty.
        renderState.window->GetSize(outImgHeight, outImgWidth);

        if (renderState.renderOnViewer)
        {
            debug2 << "below scalable rendering threshold" << endl;
            RenderCleanup();
            CATCH_RETURN2(1, output);
        }

        output = RenderInternal();
        RenderCleanup();
    }
    CATCHALL
    {
        RenderCleanup();
        RETHROW;
    }
    ENDTRY

    workingNet = origWorkingNet;

    return output;
}

// ****************************************************************************
// Method: NetworkManager::RenderValues
//
// Purpose:
//   Render the current plot(s) as a value image.
//
// Arguments:
//   plotIds    : The network ids of the plots to render.
//   getZbuffer : Whether we want the Z buffer in the returned image.
//   windowID   : The vis window Id for the plots.
//   leftEye    : Whether we're rendering the left eye image.
//
// Returns:    an avtImage containing a floating point image of the data.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 13:48:09 PDT 2017
//
// Modifications:
//
// ****************************************************************************

avtDataObject_p
NetworkManager::RenderValues(intVector plotIds, bool getZBuffer, int windowID, bool leftEye)
{
    StackTimer t0("NetworkManager::Render");
    DataNetwork *origWorkingNet = workingNet;
    avtDataObject_p output;
    avtImage_p image;

    TRY
    {
        int annotMode = 0;
        bool checkThreshold = false;
        RenderSetup(ValueImage, windowID, plotIds, getZBuffer,
            annotMode, leftEye, checkThreshold);

        VisWindow *viswin = renderState.window;
        if (PAR_Size() < 2)
        {
            avtImage_p image = viswin->ScreenCaptureValues(getZBuffer);
            CopyTo(output, image);
        }
        else// if (renderState.zBufferComposite)
        {
            // We always request Z when compositing.
            avtImage_p image = viswin->ScreenCaptureValues(true);

            // need to do compositing,
            avtValueImageCompositer *compositer;
            compositer = new avtValueImageCompositer();
            compositer->SetShouldOutputZBuffer(getZBuffer);
            compositer->SetBackground(256.);
            int imageRows, imageCols;
            image->GetSize(&imageCols, &imageRows);
            compositer->SetOutputImageSize(imageRows, imageCols);
            compositer->AddImageInput(image, 0, 0);
            compositer->Execute();
            avtImage_p compImage = compositer->GetTypedOutput();
            delete compositer;

            CopyTo(output, compImage);
        }
        RenderCleanup();
    }
    CATCHALL
    {
        RenderCleanup();
        RETHROW;
    }
    ENDTRY

    workingNet = origWorkingNet;

    return output;
}

// ****************************************************************************
//  Method: NetworkManager::RenderInternal
//
//  Purpose: do the actual rendering and compositing work. this was
//          originally lumped together with setup/tear down. I factored
//          it out so that the setup/tear down was not done twice when
//          IceTNetworkManager called it.
//
//  Programmer:  Burlen Loring
//  Creation:    Thu Sep  3 10:26:48 PDT 2015
//
//  Modifications:
//
// ****************************************************************************
//
avtDataObject_p
NetworkManager::RenderInternal()
{
    CallInitializeProgressCallback(RenderingStages());

    // ************************************************************
    // pass 1a : opaque (and translucent geometry if serial)
    // ************************************************************
    avtImage_p pass = RenderGeometry();
#if 0
    int w,h;
    if(*pass != NULL)
    {
        TRY
        {
            pass->GetImage().GetSize(&w, &h);
            debug5 << "NetworkManager::RenderInternal: 0: w=" << w
                   << ", h=" << h
                   << ", colorChannels=" << pass->GetImage().GetNumberOfColorChannels()
                   << endl;
            vtkFloatArray *arr = pass->GetImage().GetZBufferVTK();
            if(arr != NULL)
                debug5 << "We have a zbuffer with " << arr->GetNumberOfTuples() << " tuples." << endl;
            else
                debug5 << "We have NO zbuffer." << endl;

            if(avtDebugDumpOptions::DumpEnabled())
                this->DumpImage(pass, "render-geom");
        }
        CATCH(VisItException)
        {
            // Catches a no input exception.
        }
        ENDTRY
    }
#endif
    // ************************************************************
    // pass 1b : shadow mapping
    // ************************************************************
    if (renderState.shadowMap)
        NetworkManager::RenderShadows(pass);

    // ************************************************************
    // pass 1c : depth cues
    // ************************************************************
    if (renderState.depthCues)
        NetworkManager::RenderDepthCues(pass);


    // ************************************************************
    // pass 2 : translucent geometry if parallel
    // ************************************************************
    if (renderState.transparencyInPass2)
        pass = NetworkManager::RenderTranslucent(pass);

    // ************************************************************
    // pass 3 : 2d overlays
    // ************************************************************
    RenderPostProcess(pass);

#if 0
    if(*pass != NULL)
    {
        TRY
        {
            pass->GetImage().GetSize(&w, &h);
            debug5 << "NetworkManager::RenderInternal: 1: w=" << w
                   << ", h=" << h
                   << ", colorChannels=" << pass->GetImage().GetNumberOfColorChannels()
                   << endl;
            vtkFloatArray *arr = pass->GetImage().GetZBufferVTK();
            if(arr != NULL)
                debug5 << "We have a zbuffer with " << arr->GetNumberOfTuples() << " tuples." << endl;
            else
                debug5 << "We have NO zbuffer." << endl;
            if(avtDebugDumpOptions::DumpEnabled())
                this->DumpImage(pass, "render-geom-final");
        }
        CATCH(VisItException)
        {
            // Catches a no input exception.
        }
        ENDTRY
    }
#endif

    avtDataObject_p output;
    CopyTo(output, pass);

    return output;
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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 200
//    Modified to use viswinMap
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added code to support stereo rendering. Some stereo modes (e.g. crystal
//    eyes) require images in separate buffers for left/right eyes. Such
//    modes are handled by multiple external render requests to the engine.
//    Other modes (e.g. Red-Blue) can be rendered in a single ext. render.
//    Consequently, the engine's viswin object is put into stereo mode only
//    for certain cases.
//
//    Brad Whitlock, Mon Sep 18 11:54:28 PDT 2006
//    Added color texturing flag.
//
//    Brad Whitlock, Mon Nov 19 14:41:47 PST 2007
//    Added support for image backgrounds.
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
//    Tom Fogal, Fri Jul 18 19:10:06 EDT 2008
//    Use enum definitions instead of int.
//
//    Eric Brugger, Fri Oct 28 10:45:26 PDT 2011
//    Add a multi resolution display capability for AMR data.
//
//    Burlen Loring, Thu Aug 13 08:38:52 PDT 2015
//    Added options for depth peeling
//
//    Burlen Loring, Sun Sep  6 08:44:26 PDT 2015
//    Added option for ordered composting
//
// ****************************************************************************

void
NetworkManager::SetWindowAttributes(const WindowAttributes &atts,
                                    const std::string& extstr,
                                    const double *vexts,
                                    const std::string& ctName,
                                    int windowID)
{
    if (viswinMap.find(windowID) == viswinMap.end())
        NewVisWindow(windowID);

    EngineVisWinInfo &viswinInfo = viswinMap[windowID];
    viswinInfo.markedForDeletion = false;

    SetWindowAttributes(viswinInfo, atts, extstr, vexts, ctName);
}




void
NetworkManager::SetWindowAttributes(EngineVisWinInfo &viswinInfo,
    const WindowAttributes &atts, const std::string& extstr,
    const double *vexts, const std::string& ctName)
{
    StackTimer t0("NetworkManager::SetWindowAttributes");
    VisWindow *viswin = viswinInfo.viswin;
    WindowAttributes &windowAttributes = viswinInfo.windowAttributes;
    std::string &extentTypeString = viswinInfo.extentTypeString;
    std::string &changedCtName = viswinInfo.changedCtName;

    { // New scope
        StackTimer t("SetBounds");
        // do nothing if nothing changed
        if ((windowAttributes == atts) && (extentTypeString == extstr) &&
            (changedCtName == ctName))
        {
            bool extsAreDifferent = false;
            static double curexts[6];
            viswin->GetBounds(curexts);
            for (int i = 0; i < 6; ++i)
            {
                if (curexts[i] != (double) vexts[i])
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
        double fexts[6];
        for (int i = 0; i < 6; i ++)
            fexts[i] = vexts[i];
        viswin->SetBounds(fexts);
    }

    { // New scope
        StackTimer t("SetSize");
        // only update size if its different
        int s0,s1;
        viswin->GetSize(s0,s1);
        if ((s0 != atts.GetSize()[0]) || (s1 != atts.GetSize()[1]))
        {
            {
                StackTimer t1("viswin->SetSize");
                viswin->SetSize(atts.GetSize()[0], atts.GetSize()[1]);
            }
            {
                StackTimer t1("viswin->UpdateView");
                viswin->UpdateView();
            }
        }
    }

    { // New scope
        StackTimer t("SetView");

        const ViewCurveAttributes& viewCurveAtts = atts.GetViewCurve();
        avtViewCurve viewCurve;
        viewCurve.SetFromViewCurveAttributes(&viewCurveAtts);
        viswin->SetViewCurve(viewCurve);

        const View2DAttributes& view2DAtts = atts.GetView2D();
        avtView2D view2D;
        view2D.SetFromView2DAttributes(&view2DAtts);
        if (view2DAtts.GetFullFrameActivationMode() == View2DAttributes::Auto)
        {
            double extents[6];
            viswin->GetBounds(extents);
            bool newFullFrameMode = view2DAtts.GetUseFullFrame(extents);
            if (!viswin->DoAllPlotsAxesHaveSameUnits())
                newFullFrameMode = true;
            view2D.fullFrame = newFullFrameMode;
        }
        viswin->SetView2D(view2D);

        const View3DAttributes& view3DAtts = atts.GetView3D();
        avtView3D view3D;
        view3D.SetFromView3DAttributes(&view3DAtts);
        viswin->SetView3D(view3D);

        const ViewAxisArrayAttributes& viewAxisArrayAtts = atts.GetViewAxisArray();
        avtViewAxisArray viewAxisArray;
        viewAxisArray.SetFromViewAxisArrayAttributes(&viewAxisArrayAtts);
        viswin->SetViewAxisArray(viewAxisArray);
    }

    { // New scope
        StackTimer t("Set lights and colors");
        // Set the color tables
        avtColorTables::Instance()->SetColorTables(atts.GetColorTables());

        // Set the lights.
        const LightList& lights = atts.GetLights();
        viswin->SetLightList(&lights);
    }

    { // New scope
        StackTimer t("Set fore/background");

        // Set the background/foreground colors
        const unsigned char *bg = atts.GetBackground();
        viswin->SetBackgroundColor(bg[0]/255.0, bg[1]/255.0, bg[2]/255.0);

        const unsigned char *fg = atts.GetForeground();
        viswin->SetForegroundColor(fg[0]/255.0, fg[1]/255.0, fg[2]/255.0);

        // Set the background mode and gradient colors if necessary
        // Yes, these are WindowAttributes, but this particular attribute should
        // match the annotation attribute values; it is hard to make the types
        // agree as the classes are autogenerated.
        int bgm = atts.GetBackgroundMode();
        AnnotationAttributes::BackgroundMode bgMode =
            static_cast<AnnotationAttributes::BackgroundMode>(bgm);
        viswin->SetBackgroundMode(bgMode);
        if (bgMode == AnnotationAttributes::Gradient)
        {
            const double *gbg1 = atts.GetGradBG1();
            const double *gbg2 = atts.GetGradBG2();
            viswin->SetGradientBackgroundColors(atts.GetGradientBackgroundStyle(),
               gbg1[0], gbg1[1], gbg1[2], gbg2[0], gbg2[1], gbg2[2]);
        }
        else if(bgm >= AnnotationAttributes::Image)
        {
           viswin->SetBackgroundImage(atts.GetBackgroundImage(),
               atts.GetImageRepeatX(), atts.GetImageRepeatY());
        }
    }

    { // New scope
       StackTimer t("Set rendering properties");

        const RenderingAttributes &renderAtts = atts.GetRenderAtts();

        // Set the specular properties.
        viswin->SetSpecularProperties(renderAtts.GetSpecularFlag(),
                                      renderAtts.GetSpecularCoeff(),
                                      renderAtts.GetSpecularPower(),
                                      renderAtts.GetSpecularColor());

        // Set the color texturing flag.
        viswin->SetColorTexturingFlag(renderAtts.GetColorTexturingFlag());

        if (viswin->GetAntialiasing() != renderAtts.GetAntialiasing())
            viswin->SetAntialiasing(renderAtts.GetAntialiasing());

        if (viswin->GetOrderComposite() != renderAtts.GetOrderComposite())
            viswin->SetOrderComposite(renderAtts.GetOrderComposite());

        if (viswin->GetDepthCompositeThreads() !=
            static_cast<size_t>(renderAtts.GetDepthCompositeThreads()))
            viswin->SetDepthCompositeThreads(renderAtts.GetDepthCompositeThreads());

        if (viswin->GetAlphaCompositeThreads() !=
            static_cast<size_t>(renderAtts.GetAlphaCompositeThreads()))
            viswin->SetAlphaCompositeThreads(renderAtts.GetAlphaCompositeThreads());

        if (viswin->GetDepthCompositeBlocking() !=
            static_cast<size_t>(renderAtts.GetDepthCompositeBlocking()))
            viswin->SetDepthCompositeBlocking(renderAtts.GetDepthCompositeBlocking());

        if (viswin->GetAlphaCompositeBlocking() !=
            static_cast<size_t>(renderAtts.GetAlphaCompositeBlocking()))
            viswin->SetAlphaCompositeBlocking(renderAtts.GetAlphaCompositeBlocking());

        if (viswin->GetDepthPeeling() != renderAtts.GetDepthPeeling())
            viswin->SetDepthPeeling(renderAtts.GetDepthPeeling());

        if (viswin->GetOcclusionRatio() != renderAtts.GetOcclusionRatio())
            viswin->SetOcclusionRatio(renderAtts.GetOcclusionRatio());

        if (viswin->GetNumberOfPeels() != renderAtts.GetNumberOfPeels())
            viswin->SetNumberOfPeels(renderAtts.GetNumberOfPeels());

        if (viswin->GetMultiresolutionMode() != renderAtts.GetMultiresolutionMode())
            viswin->SetMultiresolutionMode(renderAtts.GetMultiresolutionMode());

        if (viswin->GetMultiresolutionCellSize() != renderAtts.GetMultiresolutionCellSize())
            viswin->SetMultiresolutionCellSize(renderAtts.GetMultiresolutionCellSize());

        if (viswin->GetSurfaceRepresentation() != renderAtts.GetGeometryRepresentation())
            viswin->SetSurfaceRepresentation(renderAtts.GetGeometryRepresentation());

        // handle stereo rendering settings
        bool stereo = renderAtts.GetStereoRendering();
        int stereoType = renderAtts.GetStereoType();
        if ((viswin->GetStereo() != stereo) || (viswin->GetStereoType() != stereoType))
            viswin->SetStereoRendering(stereo, stereoType);

        // update compositer thread pool size and blocking parameters
        zcomp->SetThreadPoolSize(renderAtts.GetDepthCompositeThreads());
        zcomp->SetBlocking(renderAtts.GetDepthCompositeBlocking());

        acomp->SetThreadPoolSize(renderAtts.GetAlphaCompositeThreads());
        acomp->SetBlocking(renderAtts.GetAlphaCompositeBlocking());
    }

    windowAttributes = atts;
    extentTypeString = extstr;
    changedCtName = ctName;
}

// ****************************************************************************
//  Method:  NetworkManager::UpdateVisualCues
//
//  Purpose:
//    Update the visual cues in the VisWindow. This has to be deferred until
//    after plots have been added to the window as the behavior of visual
//    cues is influenced by the presence of plots. Again, only processor 0
//    does any of this work.
//
//  Programmer:  Mark C. Miller
//  Creation:    Tuesday, Janurary 18, 2005
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 24 19:25:44 PST 2005
//    Made all procs render 3D visual cues not just proc 0
//
//    Brad Whitlock, Tue Mar 13 11:36:55 PDT 2007
//    Updated due to code generation changes.
//
//    Burlen Loring, Wed Sep  2 14:00:58 PDT 2015
//    Make use of render state.
//
// ****************************************************************************

void
NetworkManager::UpdateVisualCues()
{
    bool &needUpdate = renderState.windowInfo->visualCuesNeedUpdate;
    if (needUpdate == false)
        return;

    renderState.window->ClearPickPoints();
    renderState.window->ClearRefLines();

    VisualCueList &visualCueList = renderState.windowInfo->visualCueList;
    int nCues =  visualCueList.GetNumCues();
    for (int i = 0; i < nCues; ++i)
    {
        const VisualCueInfo& cue = visualCueList.GetCues(i);
        switch (cue.GetCueType())
        {
            case VisualCueInfo::PickPoint:
                renderState.window->QueryIsValid(&cue, NULL);
                break;
            case VisualCueInfo::RefLine:
                renderState.window->QueryIsValid(NULL, &cue);
                break;
            default:
                break;
        }
    }

    needUpdate = false;
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
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 200
//    Modified to use viswinMap
//
//    Mark C. Miller, Wed Jan  5 10:14:21 PST 2005
//    Fixed loop termination variable for setting frameAndState at end
//    of routine.
//
//    Mark C. Miller, Mon Jan 24 19:25:44 PST 2005
//    Made all procs render 3D visual cues not just proc 0
//
//    Brad Whitlock, Mon Jan 28 10:46:25 PDT 2008
//    Changed for new AnnotationAttributes.
//
//    Brad Whitlock, Fri Jul 17 10:10:44 PDT 2009
//    Added code to make sure that 3D text annotations are drawn when we
//    want 3D annotations to be drawn.
//
//    Kathleen Biagas, Wed Sep  2 09:06:41 PDT 2015
//    Added 3D Line annotations.
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    factor work into private method used during rendering which can
//    use some previously cached values rather than recompute them.
//    I also fixed the indentation.
//
//    Brad Whitlock, Thu Mar 16 16:03:29 PDT 2017
//    Just remember the original annotations rather than apply them here.
//
// ****************************************************************************

void
NetworkManager::SetAnnotationAttributes(const AnnotationAttributes &atts,
    const AnnotationObjectList &aolist, const VisualCueList &visCues,
    const int *fns, int windowID, int annotMode)
{
    if (viswinMap.find(windowID) == viswinMap.end())
        NewVisWindow(windowID);

    EngineVisWinInfo &viswinInfo = viswinMap[windowID];
    viswinInfo.markedForDeletion = false;

    // Save the desired annotations.
    viswinInfo.annotationAttributes = atts;
    viswinInfo.annotationObjectList = aolist;

    // defer processing of visual cues until rendering time
    if (visCues != viswinInfo.visualCueList)
    {
        viswinInfo.visualCuesNeedUpdate = true;
        viswinInfo.visualCueList = visCues;
    }

    for (int i = 0; i < 7; ++i)
        viswinInfo.frameAndState[i] = fns[i];
    viswinInfo.viswin->SetFrameAndState(fns[0],
                                        fns[1],fns[2],fns[3],
                                        fns[4],fns[5],fns[6]);
}

// ****************************************************************************
// Method: NetworkManager::AnnotationAttributesForRender
//
// Purpose:
//   Return an annotation object list suitable for the annotation mode.
//
// Arguments:
//   atts     : The input annotations.
//   annotMode: The annotation mode.
//
// Returns:    An annotation object suitable for the annotation mode.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 16 16:01:50 PDT 2017
//
// Modifications:
//
// ****************************************************************************

AnnotationAttributes
NetworkManager::AnnotationAttributesForRender(const AnnotationAttributes &atts,
    int annotMode) const
{
    AnnotationAttributes newAtts(atts);

    switch (annotMode)
    {
    case 0: // no annotations
        newAtts.SetUserInfoFlag(false);
        newAtts.SetDatabaseInfoFlag(false);
        newAtts.SetLegendInfoFlag(false);
        newAtts.GetAxes3D().SetTriadFlag(false);
        newAtts.GetAxes3D().SetBboxFlag(false);
        newAtts.GetAxes3D().SetVisible(false);
        newAtts.GetAxes2D().SetVisible(false);
        break;

    case 1: // 3D annotations only
        newAtts.SetUserInfoFlag(false);
        newAtts.SetDatabaseInfoFlag(false);
        newAtts.SetLegendInfoFlag(false);
        newAtts.GetAxes3D().SetTriadFlag(false);
        newAtts.GetAxes2D().SetVisible(false);
        break;

    case 2: // all annotations
        // no changes needed.
        break;
    }

    return newAtts;
}

// ****************************************************************************
// Method: NetworkManager::AnnotationObjectListForRender
//
// Purpose:
//   Return an annotation object list suitable for the annotation mode.
//
// Arguments:
//   aolist : The input annotation object list.
//   annotMode: The annotation mode.
//
// Returns:    An annotation object list suitable for the annotation mode.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 16 16:01:50 PDT 2017
//
// Modifications:
//
// ****************************************************************************

AnnotationObjectList
NetworkManager::AnnotationObjectListForRender(const AnnotationObjectList &aolist,
    int annotMode) const
{
    AnnotationObjectList newList;
    switch (annotMode)
    {
    case 0: // no annotations
        // leave the list empty.
        break;

    case 1: // 3D annotations only
        // Add back in the 3D annotation objects.
        for(int aIndex = 0; aIndex < aolist.GetNumAnnotations(); ++aIndex)
        {
            if(aolist[aIndex].GetObjectType() == AnnotationObject::Text3D)
                newList.AddAnnotation(aolist[aIndex]);
            else if(aolist[aIndex].GetObjectType() == AnnotationObject::Line3D)
                newList.AddAnnotation(aolist[aIndex]);
        }
        break;

    case 2: // all annotations
        newList = aolist;
        break;
    }

    return newList;
}

// ****************************************************************************
// Method: NetworkManager::ApplyAnnotations
//
// Purpose:
//   Apply annotation and annotation object list to the vis window.
//
// Arguments:
//   viswindow : The vis window that we're using.
//   atts      : The new annotation attributes.
//   aolist    : The new annotation object list.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 16 16:00:53 PDT 2017
//
// Modifications:
//
// ****************************************************************************

void
NetworkManager::ApplyAnnotations(VisWindow *viswin,
    const AnnotationAttributes &atts, const AnnotationObjectList &aolist)
{
    // Set the annotation attributes.
    viswin->SetAnnotationAtts(&atts, true);

    // Set the annotation objects.
    viswin->DeleteAllAnnotationObjects();
    viswin->CreateAnnotationObjectsFromList(aolist);
}

// ****************************************************************************
//  Method:  NetworkManager::StartPickMode
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
//    Hank Childs, Thu Mar  3 09:15:00 PST 2005
//    Modify inQueryMode.
//
// ****************************************************************************
void
NetworkManager::StartPickMode(const bool forZones)
{
    if (forZones)
        requireOriginalCells = true;
    else
        requireOriginalNodes = true;
    inQueryMode = true;
}

// ****************************************************************************
//  Method:  Network::StopPickMode
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
//
//    Hank Childs, Thu Mar  3 09:15:00 PST 2005
//    Modify inQueryMode.
//
// ****************************************************************************
void
NetworkManager::StopPickMode(void)
{
    requireOriginalCells = false;
    requireOriginalNodes = false;
    inQueryMode = false;
}

// ****************************************************************************
//  Method:  NetworkManager::StartQueryMode
//
//  Purpose:
//      Notifies the network manager that we are in "query mode".  This means
//      that certain optimizations cannot be performed, namely DLB.
//
//  Programmer:  Hank Childs
//  Creation:    February 29, 2005
//
// ****************************************************************************
void
NetworkManager::StartQueryMode(void)
{
    inQueryMode = true;
}

// ****************************************************************************
//  Method:  Network::StopQueryMode
//
//  Purpose:
//      Notifies the network manager that we are out of "query mode".  This
//      means that certain optimizations can now be performed, namely DLB.
//
//  Programmer:  Hank Childs
//  Creation:    February 29, 2005
//
// ****************************************************************************
void
NetworkManager::StopQueryMode(void)
{
    inQueryMode = false;
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
//    Kathleen Bonnell, Thu Oct 21 15:55:46 PDT 2004
//    Added support for picking on glyphed data.
//
//    Kathleen Bonnell, Tue Nov  2 10:18:16 PST 2004
//    Enusure that GlyphPick is sending the correct domain to Pick query.
//
//    Kathleen Bonnell, Thu Nov  4 15:18:15 PST 2004
//    Allow the 'PickQuery' portion to be skipped completely if no
//    intersection was found.
//
//    Kathleen Bonnell, Tue Nov  9 10:42:51 PST 2004
//    Rework parallel code for GlyphPicking.
//
//    Kathleen Bonnell, Thu Dec  2 12:50:41 PST 2004
//    Make skipLocate dependent upon NeedBoundarySurfaces instead of
//    TopologicalDimension, because during SR mode the DataAtts may get
//    overwritten with incorrect values.
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Modified to use specific window id
//
//    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005
//    Set pickatts matSelected flag from info in avtDataAttributes.
//
//    Hank Childs, Sun Mar  6 11:02:21 PST 2005
//    Added even more obscure way of determining if plot used boundary
//    surfaces, since original test for this no longer works due to changes in
//    the way data specifications are managed inside pipeline specifications
//    (they are now copied rather than shared, which was incorrect).
//
//    Kathleen Bonnell, Wed May 11 17:14:03 PDT 2005
//    Use EEF output instead of DB output for input to Pick Query. Added
//    UnifyMaximumValue call to ensure all processors know if they should
//    perform an ActualCoords query.
//
//    Kathleen Bonnell, Tue Jun 28 10:57:39 PDT 2005
//    Re-add test for topological dimension of 1 to skipLocate test. Retrieve
//    ghost type from queryInputAtts and set in PickAtts.
//
//    Kathleen Bonnell, Wed Nov 16 11:15:06 PST 2005
//    Ensure that Pick's input has the same DataAttributes as the plot
//    being picked.
//
//    Kathleen Bonnell, Thu Nov 17 13:39:42 PST 2005
//    Back-out yesterdays changes, did not work well.
//    Remove skipLocate test of pipeline-DataSpec-NeedBoundarySurfaces, and
//    dataatts-topodim and dataatts-spat dim.  Those values unreliable
//    depending on which plots follow the picked-plot in the pipeline. Instead
//    retrieve the 'linesData' flag from PickAtts to determine skipLocate.
//
//    Kathleen Bonnell, Tue Jan  3 15:06:23 PST 2006
//    Set an error message in PickAttributes if GlyphPick fails.
//
//    Kathleen Bonnell, Wed Jun 14 16:41:03 PDT 2006
//    Send silr and pipeline Index (via QueryAtts) to LocateQuery.
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added bool for leftEye to Render calls.
//
//    Kathleen Bonnell, Tue Nov 27 15:44:08 PST 2007
//    Fix memory leak associated with silr->MakeAttributes().
//
//    Hank Childs, Fri Feb 15 13:19:51 PST 2008
//    Add else statement to make Klocwork happy.  Also make sure we don't
//    delete already freed memory during error condition.
//
//    Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL.
//
//    Brad Whitlock, Tue Jan 10 14:41:54 PST 2012
//    Use the expression node without assuming where it exists in the working
//    net node list.
//
// ****************************************************************************

void
NetworkManager::Pick(const int id, const int winId, PickAttributes *pa)
{
    if (!ValidNetworkId(id))
    {
        EXCEPTION0(ImproperUseException);
    }

    if (networkCache[id] == NULL)
    {
        debug1 << "Asked to pick on a network that has already been cleared."
               << endl;
        EXCEPTION0(ImproperUseException);
    }

    if (id != networkCache[id]->GetNetID())
    {
        debug1 << "Internal error: network at position[" << id << "] "
               << "does not have same id (" << networkCache[id]->GetNetID()
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

    avtSILRestriction_p silr =networkCache[id]->GetDataSpec()->GetRestriction();
    avtDataAttributes &queryInputAtts = queryInput->GetInfo().GetAttributes();

    pa->SetMatSelected(queryInputAtts.MIROccurred() || pa->GetMatSelected());
    if (pa->GetRequiresGlyphPick())
    {
        if (networkCache[id]->ActorIsNull())
        {
            //
            // GlyphPick requires the vtkActors to be populated, so render now,
            // but only the plot we are interested in, the only important arg
            // is the plot id (network id).
            //
            intVector pids;
            pids.push_back(id);
            int outW = 0, outH = 0;
            avtImageType imgT = ColorRGBImage;
            bool checkThreshold = true;
            bool getZBuffer = false;
            int annotMode = 0;
            bool leftEye = true;
            Render(imgT, getZBuffer, pids, checkThreshold, annotMode, winId, leftEye, outW, outH);
        }
        int d = -1, e = -1;
        double t = +FLT_MAX;
        bool fc = false;
        networkCache[id]->GetActor(NULL)->MakePickable();
        //
        // Retrieve the necessary information from the renderer on the
        // VisWindow.
        //
        VisWindow *viswin = viswinMap[winId].viswin;
        viswin->GlyphPick(pa->GetRayPoint1(), pa->GetRayPoint2(), d, e, fc, t, false);
        //
        // Make sure all processors are on the same page.
        //
        intVector domElFC;
        intVector odomElFC;
        if (ThisProcessorHasMinimumValue(t))
        {
            odomElFC.push_back(d + queryInputAtts.GetBlockOrigin());
            odomElFC.push_back(e);
            odomElFC.push_back(int(fc));
        }
        else
        {
            odomElFC.push_back(-1);
            odomElFC.push_back(-1);
            odomElFC.push_back(0);
        }
        UnifyMaximumValue(odomElFC, domElFC);
        if (domElFC[0] != -1 && domElFC[1] != -1)
        {
            pa->SetDomain(domElFC[0]);
            pa->SetElementNumber(domElFC[1]);
            double dummyPt[3] = { FLT_MAX, 0., 0.};
            pa->SetPickPoint(dummyPt);
            pa->SetCellPoint(dummyPt);
            if (domElFC[2])
                pa->SetPickType(PickAttributes::DomainZone);
            else
                pa->SetPickType(PickAttributes::DomainNode);
        }
        else
        {
            debug5 << "VisWin GlyphPick failed" << endl;
            networkCache[id]->GetActor(NULL)->MakeUnPickable();
            pa->SetError(true);
            pa->SetErrorMessage("Pick could not find a valid intersection.");
            return;
        }
    }

    avtDataValidity   &queryInputVal  = queryInput->GetInfo().GetValidity();
    bool skipLocate = pa->GetLinesData();

    avtLocateQuery *lQ = NULL;
    avtPickQuery *pQ = NULL;
    avtCurvePickQuery *cpQ = NULL;
    //
    // This appears to be the only way to get the correct ghost type,
    // no matter which pipelines are in existence.
    //
    pa->SetGhostType(queryInputAtts.GetContainsGhostZones());
    TRY
    {
        QueryAttributes qa;
        qa.SetPipeIndex(networkCache[id]->GetContract()->GetPipelineIndex());
        if (pa->GetPickType() != PickAttributes::CurveNode &&
            pa->GetPickType() != PickAttributes::CurveZone)
        {
            if (skipLocate) // picking lines, set up pick atts appropriately
            {
                double *pt = pa->GetRayPoint1();
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
                    if (*silr != NULL)
                    {
                        const SILRestrictionAttributes *silAtts =
                             silr->MakeAttributes();
                        lQ->SetSILRestriction(silAtts);
                        delete silAtts;
                    }
                    int queryTimer = visitTimer->StartTimer();
                    lQ->PerformQuery(&qa);
                    visitTimer->StopTimer(queryTimer, lQ->GetType());
                    *pa = *(lQ->GetPickAtts());
                    delete lQ;
                    lQ = NULL;
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
                    if (*silr != NULL)
                    {
                        const SILRestrictionAttributes *silAtts =
                             silr->MakeAttributes();
                        lQ->SetSILRestriction(silAtts);
                        delete silAtts;
                    }
                    int queryTimer = visitTimer->StartTimer();
                    lQ->PerformQuery(&qa);
                    visitTimer->StopTimer(queryTimer, lQ->GetType());
                    *pa = *(lQ->GetPickAtts());
                    delete lQ;
                    lQ = NULL;
                }
            }
            else if (pa->GetPickType() == PickAttributes::DomainNode)
            {
                skipLocate = true;
                pQ = new avtPickByNodeQuery;
            }
            else if (pa->GetPickType() == PickAttributes::DomainZone)
            {
                skipLocate = true;
                pQ = new avtPickByZoneQuery;
            }
            else
            {
                // This should never happen, as per confirmation with Kathleen.
                EXCEPTION0(ImproperUseException);
            }
            if (skipLocate || pa->GetLocationSuccessful())
            {
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
                    const SILRestrictionAttributes *silAtts =
                             silr->MakeAttributes();
                    pQ->SetSILRestriction(silAtts);
                    delete silAtts;
                }
                pQ->SetNeedTransform(queryInputVal.GetPointsWereTransformed());
                pQ->SetInput(networkCache[id]->GetExpressionNode()->GetOutput());

                pQ->SetPickAtts(pa);
                pQ->SetSkippedLocate(skipLocate);
                int queryTimer = visitTimer->StartTimer();
                pQ->PerformQuery(&qa);
                visitTimer->StopTimer(queryTimer, pQ->GetType());
                *pa = *(pQ->GetPickAtts());
            }
            else
            {
                pa->SetError(true);
                pa->SetErrorMessage("Chosen pick did not intersect surface.");
            }

            delete pQ;
            pQ = NULL;
            bool doACQuery = (bool)UnifyMaximumValue(
                                   (int)pa->GetNeedActualCoords());
            if (doACQuery)
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
                    acq = NULL;
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
            cpQ = NULL;
        }
        visitTimer->DumpTimings();
    }
    CATCHALL
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
    if (pa->GetRequiresGlyphPick())
    {
        networkCache[id]->GetActor(NULL)->MakeUnPickable();
    }
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
//    Kathleen Bonnell, Tue Nov 27 15:44:08 PST 2007
//    Fix memory leak associated with silr->MakeAttributes().
//
//    Hank Childs, Fri Feb 15 13:13:21 PST 2008
//    Prevent possible problem of freeing freed memory during error condition.
//
//    Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL.
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    report the details of exceptions
//
// ****************************************************************************

void
NetworkManager::Query(const std::vector<int> &ids, QueryAttributes *qa)
{
    std::vector<avtDataObject_p> queryInputs;
    for (size_t i = 0 ; i < ids.size() ; i++)
    {
        int id = ids[i];
        if (!ValidNetworkId(id))
        {
            EXCEPTION1(ImproperUseException, "Invalid network id");
        }

        if (networkCache[id] == NULL)
        {
            EXCEPTION1(ImproperUseException,
                "Asked to query a network that has already been cleared.");
        }

        if (id != networkCache[id]->GetNetID())
        {
            ostringstream oss;
            oss << "Internal error: network at position[" << id << "] "
                   << "does not have same id (" << networkCache[id]->GetNetID()
                   << ")";
            EXCEPTION1(ImproperUseException, oss.str().c_str());
        }

        avtDataObject_p queryInput =
            networkCache[id]->GetPlot()->GetIntermediateDataObject();

        if (*queryInput == NULL)
        {
            EXCEPTION0(NoInputException);
        }
        queryInputs.push_back(queryInput);
    }

    qa->SetPipeIndex(networkCache[ids[0]]->GetContract()->GetPipelineIndex());
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
                const SILRestrictionAttributes *silAtts =
                             silr->MakeAttributes();
                query->SetSILRestriction(silAtts);
                delete silAtts;
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
            query = NULL;
        }
        visitTimer->DumpTimings();
    }
    CATCHALL
    {
        delete query;
        RETHROW;
    }
    ENDTRY
}

// ****************************************************************************
//  Method:  NetworkManager::CreateNamedSelection
//
//  Purpose:
//      Creates a named selection from a plot.
//
//  Arguments:
//    id         The network to use.
//    props      The new selection properties.
//
//  Programmer:  Hank Childs
//  Creation:    January 30, 2009
//
//  Modifications:
//    Brad Whitlock, Mon Dec 13 15:22:00 PST 2010
//    I added support for cumulative query selections.
//
//    Brad Whitlock, Tue Sep  6 15:56:53 PDT 2011
//    Pass a default named selection extension to the NSM.
//
//    Brad Whitlock, Thu Oct 27 14:34:45 PDT 2011
//    I improved the exception handling.
//
// ****************************************************************************

SelectionSummary
NetworkManager::CreateNamedSelection(int id, const SelectionProperties &props)
{
    const char *mName = "NetworkManager::CreateNamedSelection: ";
    avtExpressionEvaluatorFilter *f = NULL;
    avtDataObject_p dob;

    std::string source(props.GetSource());
    debug1 << mName << "selection source " << source << endl;

    if(id >= 0)
    {
        // The selection source is a plot that has been executed.

        if (!ValidNetworkId(id))
        {
            EXCEPTION0(ImproperUseException);
        }

        if (networkCache[id] == NULL)
        {
            debug1 << mName << "Asked to construct a named selection from a network "
                   << "that has already been cleared." << endl;
            EXCEPTION0(ImproperUseException);
        }

        if (id != networkCache[id]->GetNetID())
        {
            debug1 << mName << "Internal error: network at position[" << id << "] "
                   << "does not have same id (" << networkCache[id]->GetNetID()
                   << ")" << endl;
            EXCEPTION0(ImproperUseException);
        }

        if(props.GetSelectionType() == SelectionProperties::CumulativeQuerySelection &&
           !networkCache[id]->GetPlot()->CompatibleWithCumulativeQuery())
        {
            // Work off of the source file instead of the plot.
            source = networkCache[id]->GetNetDB()->GetFilename();
            debug1 << mName << "Do not use the plot's intermediate data object "
                   "for selection. Use its database source: " << source << endl;
            // Do not allow use of the plot's output.
            id = -1;
        }
        else
        {
            dob = networkCache[id]->GetPlot()->GetIntermediateDataObject();

            debug1 << mName << "Cached network's plot id: "
                   << networkCache[id]->GetPlotName()
                   << ", selection plot: " << source << endl;
        }
    }

    if(id < 0)
    {
        // We're going to assume that the source is a database name.
        int ts = 0;
        NetnodeDB *netDB = GetDBFromCache(source, ts);
        if(netDB != NULL)
        {
            // Try and determine a suitable variable to use to start our pipeline.
            std::string var;
            if(props.GetSelectionType() == SelectionProperties::CumulativeQuerySelection)
            {
                if(!props.GetVariables().empty())
                    var = props.GetVariables()[0];
            }
            if(var.empty())
            {
                // We had no variables. Try and pick one using this heuristic. Most
                // of the time the user would have been doing CQ and we won't get here.
                TRY
                {
                    avtDatabaseMetaData *md = netDB->GetDB()->GetMetaData(0);
                    if(md->GetNumScalars() > 0)
                        var = md->GetScalars(0).name;
                    if(var.empty() && md->GetNumVectors() > 0)
                        var = md->GetVectors(0).name;
                    if(var.empty() && md->GetNumTensors() > 0)
                        var = md->GetTensors(0).name;
                    if(var.empty() && md->GetNumArrays() > 0)
                        var = md->GetArrays(0).name;
                }
                CATCH(VisItException)
                {
                }
                ENDTRY
            }

            if(!var.empty())
            {
                // We're going to try and compute a selection without a plot so
                // we need to create a pipeline to do the work. This section is a
                // stripped down hybrid of StartNetwork and EndNetwork.
                TRY
                {
                    debug1 << mName << "Try creating new db source for "
                           << source << " named selection." << endl;
                    std::string leaf = ParsingExprList::GetRealVariable(var);

                    // Add an expression filter since we may need to do expressions.
                    f = new avtExpressionEvaluatorFilter();
                    f->SetInput(netDB->GetDB()->GetOutput(leaf.c_str(), ts));
                    dob = f->GetOutput();

                    // Create the data request and the contract.
                    avtSILRestriction_p silr = new avtSILRestriction(netDB->GetDB()->
                        GetSIL(ts, false));
                    std::string mesh = netDB->GetDB()->GetMetaData(0)->MeshForVar(var);
                    silr->SetTopSet(mesh.c_str());
                    avtDataRequest_p dataRequest = new avtDataRequest(var.c_str(), ts, silr);
                    int pipelineIndex = loadBalancer->AddPipeline(source);
                    avtContract_p contract = new avtContract(dataRequest, pipelineIndex);

                    // Execute with an empty source so the contract gets put in the data
                    // object's contract from last execute without really executing. This
                    // will make the contract available for when we really create the
                    // selection.
                    avtDataObjectSource *oldSrc = dob->GetSource();
                    dob->SetSource(NULL);
                    dob->Update(contract);

                    dob->SetSource(oldSrc);
                }
                CATCH(VisItException)
                {
                    if(f != NULL)
                        delete f;
                    RETHROW;
                }
                ENDTRY
            }
        }
        else
        {
           debug1 << mName << "Could not get database " << source << " from cache." << endl;
        }
    }

    if (*dob == NULL)
    {
        debug1 << mName << "Could not find a valid data set from which to create"
                           " a named selection." << endl;
        if(f != NULL)
            delete f;
        EXCEPTION0(NoInputException);
    }

    avtNamedSelectionManager *nsm = avtNamedSelectionManager::GetInstance();
    SelectionSummary summary;
    if(props.GetSelectionType() == SelectionProperties::CumulativeQuerySelection &&
       !props.GetVariables().empty())
    {
        CumulativeQueryNamedSelectionExtension CQ;

        if(props.GetVariables().size() != props.GetVariableMins().size() ||
           props.GetVariableMins().size() != props.GetVariableMaxs().size())
        {
            debug1 << mName << "The cumulative query is malformed. It must have the same "
                      "number of elements for each of the variables, mins, maxs lists."
                   << endl;
            if(f != NULL)
                delete f;
            EXCEPTION0(ImproperUseException);
        }

        // For cumulative queries, we need to set up additional processing that
        // we can't do within the pipeline library so we do it here.
        debug1 << mName << "Creating CQ named selection." << endl;
        nsm->CreateNamedSelection(dob, props, &CQ);

        // Get the filled out summary from the CQ object.
        summary = CQ.GetSelectionSummary();
    }
    else
    {
        debug1 << mName << "Creating named selection." << endl;
        avtNamedSelectionExtension ext;
        nsm->CreateNamedSelection(dob, props, &ext);
    }

    if(f != NULL)
        delete f;

    summary.SetName(props.GetName());

    // Get the size of the selection.
    avtNamedSelection *sel = nsm->GetNamedSelection(props.GetName());

    if(sel != 0)
    {
        summary.SetCellCount(sel->GetSize());

        // Some way to get the total number of cells for the summary...
    }

    return summary;
}


// ****************************************************************************
//  Method:  NetworkManager::DeleteNamedSelection
//
//  Purpose:
//      Deletes a named selection.
//
//  Arguments:
//    selName    The name of the selection.
//    clearCache Whether we should clear the NSM's cache of intermediate results
//               related to this selection.
//
//  Programmer:  Hank Childs
//  Creation:    January 30, 2009
//
//  Modifications:
//    Brad Whitlock, Thu Oct 27 14:23:12 PDT 2011
//    Make it not an error when we can't remove a selection that does not exist.
//
// ****************************************************************************

void
NetworkManager::DeleteNamedSelection(const std::string &selName)
{
    avtNamedSelectionManager *nsm = avtNamedSelectionManager::GetInstance();
    nsm->DeleteNamedSelection(selName, false);
}


// ****************************************************************************
//  Method:  NetworkManager::LoadNamedSelection
//
//  Purpose:
//      Loads a named selection.
//
//  Arguments:
//    selName    The name of the selection.
//
//  Programmer:  Hank Childs
//  Creation:    January 30, 2009
//
// ****************************************************************************

void
NetworkManager::LoadNamedSelection(const std::string &selName)
{
    avtNamedSelectionManager *nsm = avtNamedSelectionManager::GetInstance();
    nsm->LoadNamedSelection(selName);
}


// ****************************************************************************
//  Method:  NetworkManager::SaveNamedSelection
//
//  Purpose:
//      Saves a named selection.
//
//  Arguments:
//    selName    The name of the selection.
//
//  Programmer:  Hank Childs
//  Creation:    January 30, 2009
//
// ****************************************************************************

void
NetworkManager::SaveNamedSelection(const std::string &selName)
{
    avtNamedSelectionManager *nsm = avtNamedSelectionManager::GetInstance();
    nsm->SaveNamedSelection(selName);
}


// ****************************************************************************
//  Method:  NetworkManager::ConstructDataBinning
//
//  Purpose:
//      Constructs a derived data function.
//
//  Arguments:
//    id         The network to use.
//    atts       The ConstructDataBinning attributes.
//
//  Programmer:  Hank Childs
//  Creation:    February 13, 2006
//
//  Modifications:
//
//    Hank Childs, Thu Mar 30 12:40:13 PST 2006
//    Have DDF class write out the data set.  Also make reference to DDF
//    result in the first DDF, not the last.
//
//    Hank Childs, Sat Aug 21 14:35:47 PDT 2010
//    Rename DDF to DataBinning.
//
// ****************************************************************************

void
NetworkManager::ConstructDataBinning(int id, ConstructDataBinningAttributes *atts)
{
    if (!ValidNetworkId(id))
    {
        EXCEPTION0(ImproperUseException);
    }

    if (networkCache[id] == NULL)
    {
        debug1 << "Asked to construct a DataBinning from a network that has already "
               << "been cleared." << endl;
        EXCEPTION0(ImproperUseException);
    }

    if (id != networkCache[id]->GetNetID())
    {
        debug1 << "Internal error: network at position[" << id << "] "
               << "does not have same id (" << networkCache[id]->GetNetID()
               << ")" << endl;
        EXCEPTION0(ImproperUseException);
    }

    avtDataObject_p dob =
        networkCache[id]->GetPlot()->GetIntermediateDataObject();

    if (*dob == NULL)
    {
        debug1 << "Could not find a valid data set to construct a DataBinning from"
               << endl;
        EXCEPTION0(NoInputException);
    }

    avtDataBinningConstructor ddfc;
    ddfc.SetInput(dob);
    avtContract_p spec = networkCache[id]->GetContract();
    loadBalancer->ResetPipeline(spec->GetPipelineIndex());
    avtDataBinning *d = ddfc.ConstructDataBinning(atts, spec);
    // This should be cleaned up at some point.
    if (d != NULL)
    {
        d->OutputDataBinning(atts->GetName());
        bool foundMatch = false;
        for (size_t i = 0 ; i < dataBinningNames.size() ; i++)
            if (dataBinningNames[i] == atts->GetName())
            {
                foundMatch = true;
                dataBinnings[i] = d;
            }
        if (!foundMatch)
        {
            dataBinnings.push_back(d);
            dataBinningNames.push_back(atts->GetName());
        }
    }
}


// ****************************************************************************
//  Method: NetworkManager::GetDataBinning
//
//  Purpose:
//      Gets a DataBinning.
//
//  Programmer: Hank Childs
//  Creation:   February 18, 2006
//
// ****************************************************************************

avtDataBinning *
NetworkManager::GetDataBinning(const char *name)
{
    for (size_t i = 0 ; i < dataBinningNames.size() ; i++)
    {
        if (dataBinningNames[i] == name)
            return dataBinnings[i];
    }

    return NULL;
}

// ****************************************************************************
//  Method:  NetworkManager::ExportDatabases
//
//  Purpose:
//      Exports a database.
//
//  Arguments:
//    ids        The networks to use.
//    atts       The export database attributes.
//    timeSuffix A string that represents a time to be appended to the filename.
//
//  Note:        Work partially supported by DOE Grant SC0007548.
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Jan 24 16:54:20 PST 2014
//
//  Modifications:
//    Brad Whitlock, Thu Jul 24 22:27:34 EDT 2014
//    Pass timeSuffix.
//
//    Kathleen Biagas, Fri Apr 23 2021
//    Change atts arg from 'const &' to * so that actual dir name can be
//    returned in them.  If Dirname is '.', GetCWD to store instead.
//
//    Kathleen Biagas, Mon May 10 2021
//    Move re-setting of Dirname when '.' to end of method, so it doesn't
//    affect dirs stored internally in exported db's (like Silo).
//
// ****************************************************************************

void
NetworkManager::ExportDatabases(const intVector &ids,
    ExportDBAttributes *atts,
    const std::string &timeSuffix)
{
    // Determine the filename and extension.
    std::string filename, ext;
    const std::string &f = atts->GetFilename();
    std::string::size_type idx = f.rfind(".");
    if(idx != std::string::npos)
    {
        filename = f.substr(0, idx);
        ext = f.substr(idx, f.size() - idx);
    }
    else
        filename = f;

    if(ids.size() > 1)
    {
        // FUTURE: Get the plugin info here, create the writer.

        // FUTURE: Call writer->OpenFile here.

        bool err = false;
        std::string errMsg;
        for(size_t i = 0; i < ids.size(); ++i)
        {
            // Rig up a temporary ExportDBAttributes where we change the filename a little.
            ExportDBAttributes eAtts(*atts);
            char plotid[4];
            snprintf(plotid, 4, "_%02d", int(i));

            if(atts->GetAllTimes())
                eAtts.SetFilename(filename + plotid + std::string("_") + timeSuffix + ext);
            else
                eAtts.SetFilename(filename + plotid + ext);

            TRY
            {
                ExportSingleDatabase(ids[i], eAtts);
            }
            CATCH2(VisItException, e)
            {
                err = true;
                errMsg = e.Message();
            }
            ENDTRY
        }

        // FUTURE: Call writer->CloseFile here.

        if(err)
        {
            EXCEPTION1(VisItException, errMsg); // use RECONSTITUTE_EXCEPTION?
        }
    }
    else if(!ids.empty())
    {
        ExportDBAttributes eAtts(*atts);
        if(atts->GetAllTimes())
            eAtts.SetFilename(filename + timeSuffix + ext);
        else
            eAtts.SetFilename(filename + ext);
        ExportSingleDatabase(ids[0], eAtts);
    }

    if (atts->GetDirname() == ".")
    {
        // get the real path used for displaying back to user
        atts->SetDirname(FileFunctions::GetCurrentWorkingDirectory());
    }

}

// ****************************************************************************
//  Method:  NetworkManager::ExportSingleDatabase
//
//  Purpose:
//      Exports a database.
//
//  Arguments:
//    id         The network to use.
//    atts       The export database attributes.
//
//  Programmer:  Hank Childs
//  Creation:    May 26, 2005
//
//  Modifications:
//
//    Jeremy Meredith, Tue Mar 27 17:10:38 EDT 2007
//    We need to give the pipeline specification to the writer, otherwise
//    it doesn't load balance (and all processors write the whole dataset).
//
//    Hank Childs, Wed Mar 28 10:01:24 PDT 2007
//    Beef up error messages.
//
//    Mark C. Miller, Mon Aug  6 13:36:16 PDT 2007
//    Accounted for possible null return from GetEnginePluginInfo
//
//    Brad Whitlock, Tue Jun 24 16:10:41 PDT 2008
//    Changed how the database plugin manager is accessed.
//
//    Brad Whitlock, Tue Jan 21 15:29:31 PST 2014
//    I added code to allow a plot to modify the contract and I passed the
//    plot name to the writer so we have some more information.
//
//    Brad Whitlock, Fri Feb 28 16:27:47 PST 2014
//    Reorder some exception handling, add some more in case bad stuff happens
//    down in the writer.
//    Work partially supported by DOE Grant SC0007548.
//
//    Brad Whitlock, Thu Aug  6 17:00:03 PDT 2015
//    Add support for writing using groups of ranks.
//
// ****************************************************************************

void
NetworkManager::ExportSingleDatabase(int id, const ExportDBAttributes &atts)
{
    if (!ValidNetworkId(id))
    {
        EXCEPTION1(ImproperUseException, " this condition often arises when"
           " you have tried to export a database after the engine has just been"
           " closed or crashed.  Try \"ReOpen\"ing the file and exporting "
           "again.");
    }

    if (networkCache[id] == NULL)
    {
        debug1 << "Asked to export a DB from a network that has already "
               << "been cleared." << endl;
        EXCEPTION0(ImproperUseException);
    }

    if (id != networkCache[id]->GetNetID())
    {
        debug1 << "Internal error: network at position[" << id << "] "
               << "does not have same id (" << networkCache[id]->GetNetID()
               << ")" << endl;
        EXCEPTION0(ImproperUseException);
    }

    avtDataObject_p dob =
        networkCache[id]->GetPlot()->GetIntermediateDataObject();

    if (*dob == NULL)
    {
        debug1 << "Could not find a valid input to export." << endl;
        EXCEPTION0(NoInputException);
    }
    if (strcmp(dob->GetType(), "avtDataset") != 0)
    {
        EXCEPTION0(ImproperUseException);
    }

    const std::string &db_type = atts.GetDb_type_fullname();
    if (!GetDatabasePluginManager()->PluginAvailable(db_type))
    {
        char msg[1024];
        snprintf(msg, 1024, "Unable to load plugin \"%s\" for exporting.",
                 db_type.c_str());
        EXCEPTION1(ImproperUseException, msg);
    }
    EngineDatabasePluginInfo *info = GetDatabasePluginManager()->
        GetEnginePluginInfo(db_type);
    if (info == NULL)
    {
        char msg[1024];
        snprintf(msg, 1024, "Unable to get plugin info for \"%s\".",
                 db_type.c_str());
        EXCEPTION1(ImproperUseException, msg);
    }

    DBOptionsAttributes opts = atts.GetOpts();
    info->SetWriteOptions(&opts);
    avtDatabaseWriter *wrtr = info->GetWriter();
    if (wrtr == NULL)
    {
        char msg[1024];
        snprintf(msg, 1024, "Unable to locate writer for \"%s\".",
                 db_type.c_str());
        EXCEPTION1(ImproperUseException, msg);
    }

    TRY
    {
        std::string plotName(networkCache[id]->GetPlot()->GetName());
        int time = networkCache[id]->GetTime();
        ref_ptr<avtDatabase> db = networkCache[id]->GetNetDB()->GetDB();

        // Set the contract to use for the export. Give the plot a chance to
        // enhance the contract as would be the case in a normal execute.
        avtContract_p c = networkCache[id]->GetContract();
        c = networkCache[id]->GetPlot()->ModifyContract(c);
        wrtr->SetContractToUse(c);
        wrtr->SetInput(dob);

        std::string qualFilename;
        if (atts.GetDirname() == "")
            qualFilename = atts.GetFilename();
        else
            qualFilename = atts.GetDirname() + std::string(VISIT_SLASH_STRING)
                         + atts.GetFilename();
        bool doAll = false;
        std::vector<std::string> vars = atts.GetVariables();
        if (vars.size() == 1 && vars[0] == "<all>")
        {
            doAll = true;
            vars.clear();
        }

        wrtr->Write(plotName, qualFilename,
            db->GetMetaData(time), vars, doAll,
            atts.GetWriteUsingGroups(), atts.GetGroupSize());

        delete wrtr;
    }
    CATCH2(VisItException, e)
    {
        (void)e;
        delete wrtr;
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
//
//  Modifications:
//
//    Hank Childs, Wed Jan  3 14:15:47 PST 2007
//    Initialize argument to prevent UMR.  (The array being initialized is
//    not used for rank != 0.  Regardless, this fixes a purify error.)
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Tom Fogal, Mon Jun 23 10:50:18 EDT 2008
//    Changed from an internal method to a class method, so children can use
//    it.
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    I fixed the code indentation and cleaned up a warning.
//
// ****************************************************************************
double
NetworkManager::RenderBalance(int numTrianglesIHave)
{
   double balance = 1.0;
#ifndef PARALLEL
    (void)numTrianglesIHave;
#else
    int rank, size, *triCounts = NULL;

    balance = -1.0;
    rank = PAR_Rank();
    size = PAR_Size();
    if (rank == 0)
       triCounts = new int [size];
    MPI_Gather(&numTrianglesIHave, 1, MPI_INT, triCounts, 1, MPI_INT, 0,
               VISIT_MPI_COMM);
    if (rank == 0)
    {
        int i, maxTriangles, minTriangles, totTriangles, avgTriangles;
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
        std::string error = "Unable to clone an open network.";
        debug1 << error.c_str() << endl;
        EXCEPTION1(ImproperUseException,error);
    }

    if (!ValidNetworkId(id))
    {
        EXCEPTION0(ImproperUseException);
    }

    if (networkCache[id] == NULL)
    {
        std::string error = "Asked to clone a network that has already been cleared.";
        debug1 << error.c_str() << endl;
        EXCEPTION1(ImproperUseException, error);
    }

    if (id != networkCache[id]->GetNetID())
    {

        debug1 << "Internal error: network at position[" << id
               << "] does not have same id ("
               << networkCache[id]->GetNetID() << ")" << endl;
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
//    Kathleen Bonnell, Wed May 11 17:14:03 PDT 2005
//    Use EEF output instead of DB output for input to filters.
//
//    Kathleen Bonnell, Tue Nov 27 15:44:08 PST 2007
//    Fix memory leak associated with silr->MakeAttributes(), add support for
//    "Locate and Pick Zone/Node" queries.
//
//    Kathleen Bonnell, Tue Jul  8 14:25:25 PDT 2008
//    Change the DataRequest variable if the var being queried is different
//    than what is in the cloned pipeline.
//
//    Brad Whitlock, Tue Jan 10 14:40:49 PST 2012
//    Use the expression node without assuming where it exists in the working
//    net node list.
//
//    Kathleen Biagas, Fri May 26 12:35:43 PDT 2017
//    Default useActualData to true, retrieve the value from QueryInputParams
//    if available.  Use it as a value to help determine where input is
//    gathered from.
//
//    Alister Maguire, Mon Sep 23 12:35:44 MST 2019
//    Refactored to handle two QOT types: DirectDatabaset and TimeLoop.
//
//    Alister Maguire, Tue Oct 29 14:46:38 MST 2019
//    Updated to perform the network cloning within this method. If we're
//    using the TimeLoop route, use a true clone. If we're using the
//    DirectDatabase route, create a psuedo clone by starting an entirely
//    new network based off of the cloning target.
//
//    Alister Maguire, Mon Mar  9 13:31:50 PDT 2020
//    Assume we can use the direct route if query atts say yes and there are
//    no expressions. Also, we can use this route for screen picks.
//
// ****************************************************************************

void
NetworkManager::AddQueryOverTimeFilter(QueryOverTimeAttributes *qA,
                                       const int clonedFromId)
{
    if (networkCache[clonedFromId] == NULL)
    {
        std::string error =  "Adding a filter to a non-existent network." ;
        EXCEPTION1(ImproperUseException, error);
    }

    //
    // We need to determine if we can use the direct database QOT
    // filter.
    //
    bool useDirectDatabaseQOT = false;

    if (qA->GetCanUseDirectDatabaseRoute())
    {
        //
        // The query atts think that we can use this route, but we
        // we need to make sure that the current expressions are
        // compatible.
        //
        avtExpressionEvaluatorFilter *eef =
            dynamic_cast<avtExpressionEvaluatorFilter *>
            (networkCache[clonedFromId]->GetExpressionNode()->GetFilter());

        if (eef != NULL)
        {
            useDirectDatabaseQOT = eef->CanApplyToDirectDatabaseQOT();
        }
        else
        {
            //
            // Assume we're safe.
            //
            useDirectDatabaseQOT = true;
        }
    }

    bool useActualData = false;
    if (qA->GetQueryAtts().GetQueryInputParams().
        HasNumericEntry("use_actual_data"))
    {
        useActualData = qA->GetQueryAtts().GetQueryInputParams().
            GetEntry("use_actual_data")->ToBool();

        if (useActualData && useDirectDatabaseQOT)
        {
            useDirectDatabaseQOT = false;
        }
    }

    qA->SetCanUseDirectDatabaseRoute(useDirectDatabaseQOT);

    qA->GetQueryAtts().SetPipeIndex(networkCache[clonedFromId]->
        GetContract()->GetPipelineIndex());

    avtDataObject_p input = NULL;

    if (useDirectDatabaseQOT)
    {
        //
        // We're using the direct route. Let's start an entirely new
        // network based off of the cloning target.
        //
        NetnodeDB *nndb = networkCache[clonedFromId]->GetNetDB();
        StartNetwork(nndb->GetDB()->GetFileFormat(),
                     nndb->GetFilename(),
                     nndb->GetVarName(),
                     nndb->GetTime());

        input = workingNet->GetExpressionNode()->GetOutput();

        //
        // We need to let the database readers know that we're asking for
        // a specialized QOT dataset.
        //
        avtDataRequest_p dr = new avtDataRequest(workingNet->GetDataSpec(),
            qA->GetQueryAtts().GetVariables()[0].c_str());
        dr->SetRetrieveQOTDataset(true);
        dr->SetQOTAtts(qA);

        //
        // Add the remaining variables as secondaries.
        //
        stringVector vars = qA->GetQueryAtts().GetVariables();
        for (int i = 1; i < vars.size(); ++i)
        {
            dr->AddSecondaryVariable(vars[i].c_str());
        }

        workingNet->SetDataSpec(dr);
    }
    else
    {
        //
        // A screen pick requires that we use actual data.
        //
        if (qA->GetQueryAtts().GetName() == "Locate and Pick Zone" ||
            qA->GetQueryAtts().GetName() == "Locate and Pick Node")
        {
            useActualData = true;
        }

        //
        // We're using the TimeLoop route. This means we need a true clone.
        //
        CloneNetwork(clonedFromId);

        if (useActualData)
        {
            input = networkCache[clonedFromId]->GetPlot()->
                    GetIntermediateDataObject();
        }
        else
        {
            input = workingNet->GetExpressionNode()->GetOutput();
        }

        if (strcmp(workingNet->GetDataSpec()->GetVariable(),
                        qA->GetQueryAtts().GetVariables()[0].c_str()) != 0)
        {
            avtDataRequest_p dr = new avtDataRequest(workingNet->GetDataSpec(),
                qA->GetQueryAtts().GetVariables()[0].c_str());

            workingNet->SetDataSpec(dr);
        }
    }

    //
    //  Create a transition node so that the new filter will receive
    //  the correct input.
    //
    NetnodeTransition *trans = new NetnodeTransition(input);
    Netnode *n = workingNetnodeList.back();
    workingNetnodeList.pop_back();
    trans->GetInputNodes().push_back(n);

    workingNet->AddNode(trans);

    avtQueryOverTimeFilter *qotFilter = NULL;

    if (useDirectDatabaseQOT)
    {
        qotFilter = new avtDirectDatabaseQOTFilter(qA);
    }
    else
    {
        qotFilter = new avtTimeLoopQOTFilter(qA);

        //
        // Pass down the current SILRestriction (via UseSet) in case the query
        // needs to make use of this information.
        //
        avtSILRestriction_p silr = workingNet->GetDataSpec()->GetRestriction();
        if (*silr != NULL)
        {
            const SILRestrictionAttributes *silAtts = silr->MakeAttributes();
            qotFilter->SetSILAtts(silAtts);
            delete silAtts;
        }
    }

    //
    // Put a QueryOverTimeFilter right after the transition to handle
    // the query.
    //
    NetnodeFilter *nodeFilter = new NetnodeFilter(qotFilter, "QueryOverTime");
    nodeFilter->GetInputNodes().push_back(trans);

    workingNetnodeList.push_back(nodeFilter);
    workingNet->AddNode(nodeFilter);
}


// ****************************************************************************
//  Method:  NetworkManager::NewVisWindow
//
//  Purpose: Adds a new VisWindow object and initializes its annotations
//
//  Programmer:  Mark C. Miller
//  Creation:    January 5, 2005
//
//  Modifications:
//
//    Mark C. Miller, Tue Jan 18 12:44:34 PST 2005
//    Added initialization of visualCuesNeedUpdate
//
//    Mark C. Miller, Mon Mar  7 13:41:45 PST 2005
//    Made it also delete any VisWindows marked for deletion
//
//    Brad Whitlock, Mon Jan 28 10:47:35 PDT 2008
//    Changed for updated AnnotationAttributes.
//
//    Tom Fogal, Tue Dec  8 16:57:21 MST 2009
//    Detect failed window initialization and erase the window from the map.
//
// ****************************************************************************

void
NetworkManager::NewVisWindow(int winID)
{
    StackTimer t0("NetworkManager::NewVisWindow");

    //
    // Delete any VisWindow objects that are marked for deletion
    //
    std::vector<int> idsToDelete;
    std::map<int, EngineVisWinInfo>::iterator it;
    for (it = viswinMap.begin(); it != viswinMap.end(); it++)
    {
        if (it->second.markedForDeletion)
            idsToDelete.push_back(it->first);
    }
    for (size_t i = 0; i < idsToDelete.size(); i++)
    {
        debug2 << "Deleting VisWindow for id=" << idsToDelete[i] << endl;
        if(viswinMap[idsToDelete[i]].owns)
            delete viswinMap[idsToDelete[i]].viswin;
        viswinMap.erase(idsToDelete[i]);
    }

    debug2 << "Creating new VisWindow for id=" << winID << endl;

    TRY
    {
        StackTimer t1("Create new vis window");
        // Call a creation callback to make the vis window for winID.
        VisWindow *viswin = NULL;
        bool owns = true;
        (*CreateVisWindowCB)(winID, viswin, owns, CreateVisWindowCBData);

        viswinMap[winID].viswin = viswin;
        viswinMap[winID].owns = owns;
        viswinMap[winID].visualCuesNeedUpdate = false;
        viswinMap[winID].markedForDeletion = false;
        viswinMap[winID].viswin->Realize();
    }
    CATCHALL
    {
        debug1 << "VisWindow initialization failed." << std::endl;
        viswinMap.erase(winID);
        RETHROW;
    }
    ENDTRY

    viswinMap[winID].viswin->DisableUpdates();
}

// ****************************************************************************
//  Method: NetworkManager::RegisterInitializeProgressCallback
//
//  Purpose:
//      Registers a callback that allows the network manager to initialize
//      its progress.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2005
//
// ****************************************************************************

void
NetworkManager::RegisterInitializeProgressCallback(
                                     InitializeProgressCallback pc, void *args)
{
    initializeProgressCallback     = pc;
    initializeProgressCallbackArgs = args;
}


// ****************************************************************************
//  Method: NetworkManager::RegisterProgressCallback
//
//  Purpose:
//      Registers a callback that allows the network manager to indicate
//      its progress.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2005
//
// ****************************************************************************

void
NetworkManager::RegisterProgressCallback(ProgressCallback pc, void *args)
{
    progressCallback     = pc;
    progressCallbackArgs = args;
}


// ****************************************************************************
//  Method: NetworkManager:CallInitializeProgressCallback
//
//  Purpose:
//      Makes the callback to initialize progress, provided its non-NULL.
//
//  Programmer: Hank Childs
//  Creation:   January 3, 2005
//
// ****************************************************************************

void
NetworkManager::CallInitializeProgressCallback(int nstages)
{
    if (initializeProgressCallback != NULL)
        initializeProgressCallback(initializeProgressCallbackArgs, nstages);
}


// ****************************************************************************
//  Method: NetworkManager:CallProgressCallback
//
//  Purpose:
//      Makes the callback to state progress, provided its non-NULL.
//
//  Programmer: Hank Childs
//  Creation:   January 3, 2005
//
// ****************************************************************************

void
NetworkManager::CallProgressCallback(const char *module, const char *msg,
                                     int amt, int total)
{
    if (progressCallback != NULL)
        progressCallback(progressCallbackArgs, module, msg, amt, total);
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
//  Modifications:
//
//    Mark C. Miller, Thu Dec 16 10:25:09 PST 2004
//    Added numDumpsAll to correct numbering in output images in parallel
//
//    Tom Fogal / Hank Childs, Wed Jul 25 17:13:18 PDT 2007
//    Fully qualified the `TIFF' format type, as it was being pulled from an
//    improper enum.  Changed `useLZW' from 6 to 0 so the writer wouldn't
//    complain about patent issues and bail out.
//
//    Tom Fogal, Thu Jul 26 10:23:16 PDT 2007
//    Changed default format to PNG, to avoid patent issues with old TIFF
//    libraries.
//
//    Brad Whitlock, Fri Feb 15 15:27:21 PST 2008
//    Delete fileWriter.
//
//    Cyrus Harrison, Tue Feb 19 08:38:12 PST 2008
//    Added support for optional debug dump directory.
//
//    Tom Fogal, Wed May 18 12:07:36 MDT 2011
//    Promote to a class method, simplify a bit.
//
// ****************************************************************************

void
NetworkManager::DumpImage(avtDataObject_p img, const char *fmt)
const
{
    // Burlen Loring, Sat Aug 29 09:20:30 PDT 2015
    // when I tried to use this in parallel there was an
    // MPI error in the engine event loop, a bcast is unmatched.
    char tmpName[256];
    avtFileWriter *fileWriter = new avtFileWriter();

    FormatDebugImage(tmpName, 256, fmt);

    std::string dump_image = avtDebugDumpOptions::GetDumpDirectory() + tmpName +
                        ".png";

    fileWriter->SetFormat(SaveWindowAttributes::PNG);
    int compress = 1;
    fileWriter->Write(dump_image.c_str(), img, 100, false, compress, false);
    delete fileWriter;

#if 0
    // Write the zbuffer as a VTK dataset.
    avtImage_p image;
    CopyTo(image, img);
    if(image->GetImage().GetZBufferVTK() != NULL)
    {
        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
        int dims[3] = {1,1,1};
        image->GetImage().GetSize(&dims[0], &dims[1]);
        dims[0]++; dims[1]++;

        vtkFloatArray *x = vtkFloatArray::New();
        x->SetNumberOfTuples(dims[0]);
        for(int i = 0; i < dims[0]; ++i)
            x->SetTuple1(i,i);
        vtkFloatArray *y = vtkFloatArray::New();
        y->SetNumberOfTuples(dims[1]);
        for(int i = 0; i < dims[1]; ++i)
            y->SetTuple1(i,i);
        rgrid->SetDimensions(dims);
        rgrid->SetXCoordinates(x);
        rgrid->SetYCoordinates(y);
        x->Delete();
        y->Delete();

        rgrid->GetCellData()->AddArray(image->GetImage().GetZBufferVTK());

        vtkDataSetWriter *writer = vtkDataSetWriter::New();
        writer->SetFileName((std::string(dump_image) + ".vtk").c_str());
        writer->SetInputData(rgrid);
        writer->Update();

        writer->Delete();
        rgrid->Delete();
    }
#endif
}

// ****************************************************************************
//  Function:  FormatDebugImage
//
//  Purpose:
//    Formats a string to use for writing debug images.
//
//  Arguments:
//    out        where to write the string to
//    outlen     length of the 'out' buffer.
//    prefix     string to tack on to the beginning of saved images
//
//  Programmer:  Tom Fogal
//  Creation:    May 18, 2011
//
//  Modifications:
// ****************************************************************************
void
NetworkManager::FormatDebugImage(char *out, size_t outlen, const char *prefix)
const
{
  static int numDumps = 0;
  snprintf(out, outlen, "%s_%03d_%03d", prefix, PAR_Rank(), numDumps);
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
//    prefix     prefix for the filename
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 21, 2004
//
//  Tom Fogal, Wed May 18 12:13:16 MDT 2011
//  Remove allprocs argument.  Make it a class method.
//
// ****************************************************************************
void
NetworkManager::DumpImage(const avtImage_p img, const char *prefix) const
{
    avtDataObject_p tmpImage;
    CopyTo(tmpImage, img);
    DumpImage(tmpImage, prefix);
}


// ****************************************************************************
//  Function:  GetDatabase
//
//  Purpose:
//      A callback for our expression language that can get a database.
//
//  Arguments:
//    nm         A void * pointer to the network manager.
//    filename   The database filename.
//    time       The time index.
//    format     The preferred format for the file.
//
//  Programmer:  Hank Childs
//  Creation:    August 26, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Feb  1 15:49:59 PST 2008
//    Added argument to GetDBFromCache to indicate we might not have loaded
//    the right plugin.
//
// ****************************************************************************

static ref_ptr<avtDatabase>
GetDatabase(void *nm, const std::string &filename, int time,const char *format)
{
    NetworkManager *nm2 = (NetworkManager *) nm;
    bool treatAllDBsAsTimeVarying = false;
    // This database is being requested by an AVT filter (likely a CMFE
    // expression), so we have no idea if the right plugin has been loaded.
    bool fileMayHaveUnloadedPlugin = true;
    bool ignoreExtents = false;
    NetnodeDB *db = nm2->GetDBFromCache(filename, time, format,
                          treatAllDBsAsTimeVarying, fileMayHaveUnloadedPlugin,
                          ignoreExtents);
    return db->GetDB();
}


// ****************************************************************************
//  Function: GetDataBinningCallbackBridge
//
//  Purpose:
//      The bridge that can go to the network manager and ask for a DataBinning.
//
//  Programmer: Hank Childs
//  Creation:   February 18, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Aug 21 14:35:47 PDT 2010
//    Rename DDF to DataBinning.
//
// ****************************************************************************

static avtDataBinning *
GetDataBinningCallbackBridge(void *arg, const char *name)
{
    NetworkManager *nm = (NetworkManager *) arg;
    return nm->GetDataBinning(name);
}

#ifdef PARALLEL
// ****************************************************************************
//  Function: HaveData
//
//  Purpose:
//    Scans the image to see if it is a uniform color. if it is
//    not then we have data. This is a simplified and more robust
//    (handles alpha  channel and non-white background) version of
//    a function IsBlank by Tom Fogal on October 22, 2008
//
//  Programmer: Burlen Loring
//  Creation:   Fri Sep 11 02:22:01 PDT 2015
//
//  Modification:
//
// ****************************************************************************

static
int HaveData(vtkUnsignedCharArray *rgba)
{
    // rather than assume a background color, or
    // pass one in, test that all the pixels have the
    // same color/alpha value.
    size_t nt = rgba->GetNumberOfTuples();
    int nc = rgba->GetNumberOfComponents();
    unsigned char *prgba = rgba->GetPointer(0);
    for (int j = 0; j < nc; ++j)
    {
        unsigned char *p = prgba + j;
        unsigned char bg = p[0];
        for (size_t i = 1; i < nt; ++i)
            if (p[i*nc] != bg)
                return 1;
    }
    return 0;
}

// ****************************************************************************
//  Function: HaveData
//
//  Purpose:
//    Templated overload for images that have been split into
//    color channel arrays. See the VTK array overload for
//    details.
//
//  Programmer: Burlen Loring
//  Creation:   Fri Sep 11 02:22:01 PDT 2015
//
//  Modification:
//
// ****************************************************************************

bool nequal(float a, float b)
{ return fabs(a - b) > 1e-5f; } // ok range is 0 to 1

bool nequal(unsigned char a, unsigned char b)
{ return a != b; }

template <typename T>
int HaveData(T *r, T *g, T *b, T *a, size_t n)
{
    // rather than assume a background color, or
    // pass one in, test that all the pixels have the
    // same color/alpha value.
    T *rgba[4] = {r, g, b, a};
    for (int j = 0; j < 4; ++j)
    {
        T *c = rgba[j];
        if (c)
        {
            T bg = c[0];
            for (size_t i = 1; i < n; ++i)
                if (nequal(c[i], bg))
                    return 1;
        }
    }
    return 0;
}

// ****************************************************************************
//  Function: HaveData
//
//  Purpose:
//    Scans the image to see if it has uniform depth value
//
//  Programmer: Burlen Loring
//  Creation:   Fri Sep 11 02:22:01 PDT 2015
//
//  Modification:
//
// ****************************************************************************

template <typename T>
int HaveData(T *z, size_t n)
{
    T bg = z[0];
    for (size_t i = 1; i < n; ++i)
        if (nequal(z[i], bg))
            return 1;
    return 0;
}

// ****************************************************************************
//  Function: HaveData
//
//  Purpose:
//    Scans the image to see if it has uniform depth value
//
//  Programmer: Burlen Loring
//  Creation:   Fri Sep 11 02:22:01 PDT 2015
//
//  Modification:
//
// ****************************************************************************

static
int HaveData(vtkFloatArray *z)
{
    return z ? HaveData(z->GetPointer(0), z->GetNumberOfTuples()) : 0;
}

#endif


// ****************************************************************************
//  Function: OnlyRootNodeHasData
//
//  Purpose:
//    Report whether or not the root node has all the data for this rendering.
//    The image should be the image rendered by this process; global
//    communication will figure out which nodes have image data.  Thus,
//    this method MUST be called synchronously!
//
//    Some plots / DBs simply don't support decomposition.  In that case,
//    we don't want to do image composition because only the root node
//    will have any data.  If the plot utilizes transparency, we'll fade
//    process 0's (correct) image by compositing, because process 0 will
//    have the right image and everybody else will have a plain white BG.
//
//  Programmer: Tom Fogal
//  Creation:   October 17, 2008
//
//  Modifications:
//
//    Hank Childs, Wed Nov 19 15:53:15 PST 2008
//    Test was implemented to return true if *any* data had no data, which is
//    not what we wanted.
//
//    Burlen Loring, Fri Sep 11 03:14:46 PDT 2015
//    handle non-white backgrounds, handle alpha channel, simplify
//    the logic. do less communication, use reduction rather then
//    all gathering a comm size vector.
//
//    Burlen Loring, Wed Sep 23 13:53:45 PDT 2015
//    If there are no rgb colors in the image and a depth buffer is present
//    use that
//
// ****************************************************************************

static int
OnlyRootNodeHasData(avtImage_p &img)
{
#ifndef PARALLEL
    (void)img;
    return 1;
#else
    // look for the color buffer first, if not then look
    // for the depth buffer
    vtkImageData *im = img->GetImage().GetImageVTKDirect();
    vtkUnsignedCharArray *color = im ? dynamic_cast<vtkUnsignedCharArray*>(
        im->GetPointData()->GetArray("ImageScalars")) : NULL;

    vtkFloatArray *depth = !color ? img->GetImage().GetZBufferVTKDirect() : NULL;

    // if the root has data but no one else does
    // then on the root after the reduce b1 == b2 == 1.
    int b1 = HaveData(color) || HaveData(depth);
    int b2 = 0;
    MPI_Reduce(&b1, &b2, 1, MPI_INT, MPI_SUM, 0, VISIT_MPI_COMM);
    int rank;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    if (rank == 0)
        b2 = ((b1 == 1) && (b2 == 1)) ? 1 : 0;
    MPI_Bcast(&b2, 1, MPI_INT, 0, VISIT_MPI_COMM);
    return b2;
#endif
}

// ****************************************************************************
//  Function: OnlyRootNodeHasData
//
//  Purpose:
//    Templated version for images that have been split into channels
//    see avtImage version for explanation of purpose.
//
//  Programmer: Burlen LOring
//  Creation:   , Fri Sep 11 10:46:13 PDT 2015
//
//  Modifications:
//
//    Burlen Loring, Wed Sep 23 13:53:45 PDT 2015
//    If there are no rgb colors in the image and a depth buffer is present
//    use that
//
// ****************************************************************************

template <typename T>
int
OnlyRootNodeHasData(T *r, T *g, T *b, T *a, float *z, size_t n)
{
#ifndef PARALLEL
    (void)r;
    (void)g;
    (void)b;
    (void)a;
    (void)n;
    (void)z;
    return 1;
#else
    // if the root has data but no one else does
    // then on the root after the reduce b1 == b2 == 1.
    int b1 = r ? HaveData(r, g, b, a, n) : HaveData(z, n);
    int b2 = 0;
    MPI_Reduce(&b1, &b2, 1, MPI_INT, MPI_SUM, 0, VISIT_MPI_COMM);
    int rank;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    if (rank == 0)
        b2 = ((b1 == 1) && (b2 == 1)) ? 1 : 0;
    MPI_Bcast(&b2, 1, MPI_INT, 0, VISIT_MPI_COMM);
    return b2;
#endif
}

// ****************************************************************************
//  Function: BroadcastImage
//
//  Purpose:
//    Broadcasts an image from a processor to all other processors.
//    This method MUST be called synchronously!
//
//  Arguments:
//    img       The image to broadcast, or fill with a received image.
//    send_zbuf Set if we should send over the Z buffer too
//    root      Which process will source the image
//
//  Programmer: Tom Fogal
//  Creation:   October 24, 2008
//
//  Modifications:
//
//    Hank Childs, Thu Jan  8 15:23:44 CST 2009
//    Fix memory leak.
//
//    Burlen Loring, Fri Sep  4 12:28:14 PDT 2015
//    handle 4 channel images. eliminate an unecessary memcpy
//    by recv'ing directly into arrays used by output image.
//
//    Burlen Loring, Fri Sep 11 11:19:35 PDT 2015
//    Pass in width and height so that receiving ranks can
//    construct an image
//
//    Burlen Loring, Wed Sep 23 13:22:30 PDT 2015
//    Skip sending RGBA values for ordered compositing, only
//    depth buffer is needed.
//
// ****************************************************************************

static void
BroadcastImage(avtImage_p &img, int w, int h, bool hasAlpha,
    bool sendColor, bool sendDepth)
{
#ifndef PARALLEL
    (void)img;
    (void)w;
    (void)h;
    (void)hasAlpha;
    (void)sendColor;
    (void)sendDepth;
#else
    const int npix = w*h;
    const int nchan = hasAlpha ? 4 : 3;

    vtkImageData *im = NULL;
    vtkFloatArray *depth = NULL;
    vtkUnsignedCharArray *color = NULL;

    if (PAR_Rank() == 0)
    {
        im = img->GetImage().GetImageVTKDirect();
        color = sendColor ?  (im ? dynamic_cast<vtkUnsignedCharArray*>(
                im->GetPointData()->GetArray("ImageScalars")) : NULL) : NULL;
        depth = sendDepth ? img->GetImage().GetZBufferVTKDirect() : NULL;
    }
    else
    {
        // create the recv buffers and pass them into
        // the output image.
        im = vtkImageData::New();
        im->SetDimensions(w, h, 1);
        img->GetImage().SetImageVTK(im);
        im->Delete();

        if (sendColor)
        {
            color = vtkUnsignedCharArray::New();
            color->SetName("ImageScalars");
            color->SetNumberOfComponents(nchan);
            color->SetNumberOfTuples(npix);

            im->GetPointData()->SetScalars(color);
            color->Delete();
        }

        if (sendDepth)
        {
            depth = vtkFloatArray::New();
            depth->SetName("depth");
            depth->SetNumberOfTuples(npix);

            img->GetImage().SetZBufferVTK(depth);
            depth->Delete();
        }
    }

    size_t img_size = npix * nchan;
    if (sendColor)
        MPI_Bcast(color->GetPointer(0), img_size, MPI_BYTE, 0, VISIT_MPI_COMM);

    if (sendDepth)
        MPI_Bcast(depth->GetPointer(0), npix, MPI_FLOAT, 0, VISIT_MPI_COMM);
#endif
}

// ****************************************************************************
//  Method:  Network::PickForIntersection
//
//  Purpose:  Finds the intersection point in the data, that was
//            'picked'.
//
//  Arguments:
//    winId      The window Id to use.
//    pa         PickAttributes to set/get info about the pick.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    March 2, 2006
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 14:21:18 PDT 2007
//    Added parallel logic.
//
// ****************************************************************************

void
NetworkManager::PickForIntersection(const int winId, PickAttributes *pa)
{
    // the network ids are stored in Incident Elements
    intVector ids = pa->GetIncidentElements();
    intVector validIds;
    bool needRender = false;
    for (size_t i = 0; i < ids.size(); i++)
    {
        if (!ValidNetworkId(ids[i]))
        {
            continue;
        }

        if (networkCache[ids[i]] == NULL)
        {
            debug1 << "Asked to pick on a network that has "
                   << "already been cleared." << endl;
            continue;
        }

        if (ids[i] != networkCache[ids[i]]->GetNetID())
        {
             debug1 << "Internal error: network at position[" << ids[i] << "] "
                    << "does not have same id ("
                    << networkCache[ids[i]]->GetNetID() << ")" << endl;
             continue;
        }
        validIds.push_back(ids[i]);
        needRender = networkCache[ids[i]]->ActorIsNull();
    }
    if (validIds.size() == 0)
    {
        return;
    }

    if (needRender)
    {
        avtImageType imgT = ColorRGBImage;
        bool checkThreshold = true;
        bool getZBuffer = false;
        int annotMode = 0;
        bool leftEye = true;
        int outW = 0, outH = 0;
        Render(imgT, getZBuffer, validIds, checkThreshold, annotMode, winId, leftEye, outW, outH);
    }
    int x, y;
    double isect[3];
    x = (int)pa->GetRayPoint1()[0];
    y = (int)pa->GetRayPoint1()[1];
    VisWindow *viswin = viswinMap[winId].viswin;
    bool  gotOne = viswin->FindIntersection(x, y, isect);

    //
    // Communicate the data to processor 0.
    //
    int   rank   = PAR_Rank();
    int   tmp    = (gotOne ? rank : -1);
    tmp = UnifyMaximumValue(tmp);
    bool dataAlreadyOnProc0 = (tmp == 0);
    bool dataNotOnProc0 = (tmp > 0);
    bool validPick = dataAlreadyOnProc0 || dataNotOnProc0;

    if (dataNotOnProc0)
    {
        bool iHaveTheData = (tmp == rank ? true : false);
        GetDoubleArrayToRootProc(isect, 3, iHaveTheData);
    }

    //
    // Have processor 0 put the information into the pick attributes.
    //
    if (rank == 0 && validPick)
    {
        pa->SetPickPoint(isect);
        pa->SetFulfilled(true);
    }
}

// ****************************************************************************
//  Method: SetStereoEnabled
//
//  Purpose: Setup stereo rendering mode
//
//  Programmer: Mark C. Miller
//  Creation:   August 9, 2006
//
// ****************************************************************************

void
NetworkManager::SetStereoEnabled()
{
    VisWinRendering::SetStereoEnabled();
}

// ****************************************************************************
//  Method: PlotsNeedUpdating
//
//  Purpose: Determine if any of the plots we currently have differ from those
//           in the window, which necessitates a plot update.
//
//  Arguments:
//    plots          plots to render
//    plotsInWindow  plots already in the window
//
//  Programmer: Tom Fogal
//  Creation:   June 9, 2008
//
//  Modifications:
//
//    Tom Fogal, Mon Sep  1 14:55:18 EDT 2008
//    Changed an assert to an if-and-a-throw.
//
// ****************************************************************************
bool
NetworkManager::PlotsNeedUpdating(const intVector &plots,
                                  const intVector &plotsInWindow) const
{
    // They should really be the same size, but at least it won't be a UMR if
    // there are more plotsInWindow -- though I think that would be another bug
    // altogether.
    if(plotsInWindow.size() < plots.size())
    {
        EXCEPTION1(ImproperUseException, "Differing number of current and "
                   "window plots. This probably means there is a bug in "
                   "SetUpWindowContents.");
    }

    for(size_t p = 0; p < plots.size(); ++p)
    {
        if(plots[p] != plotsInWindow[p])
        {
            return true;
        }
    }
    return false;
}

// ****************************************************************************
//  Method: ViewerExecute
//
//  Purpose: Determine if we should force the viewer to re-execute.
//
//  Arguments:
//    viswin            which window we are dealing with
//    plots             the plots to test for re-rendering
//    windowAttributes  used to look up scale modes
//
//  Programmer: Tom Fogal
//  Creation:   June 9, 2008
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Sep 25 10:38:27 PDT 2007
//    Retrieve scaling modes from 2d and curve view atts and set them in the
//    plot before it executes so the plot will be created with correct scaling.
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    Use const references rather than make a copy of attribute objects.
//    Use a switch statement rather than an if if else. pull function calls
//    out of loop conditions.
//
// ****************************************************************************

bool
NetworkManager::ViewerExecute(const VisWindow * const viswin,
                              const intVector &plots,
                              const WindowAttributes &windowAttributes)
{
    DataNetwork *net = workingNet;
    bool retval = false;

    const ViewCurveAttributes &vca = windowAttributes.GetViewCurve();
    const View2DAttributes &v2a = windowAttributes.GetView2D();

    ScaleMode ds = (ScaleMode)vca.GetDomainScale();
    ScaleMode rs = (ScaleMode)vca.GetRangeScale();
    ScaleMode xs = (ScaleMode)v2a.GetXScale();
    ScaleMode ys = (ScaleMode)v2a.GetYScale();

    // Scan through all the plots looking for a scaling mode which forces an
    // update.
    int mode = viswin->GetWindowMode();
    size_t nPlots = plots.size();
    for(size_t p = 0; (p < nPlots) && !retval; ++p)
    {
        switch (mode)
        {
        case WINMODE_2D:
            workingNet = NULL;
            UseNetwork(plots[p]);
            if (workingNet->GetPlot()->
               ScaleModeRequiresUpdate(WINMODE_2D, xs, ys))
                retval = true;
            break;

        case WINMODE_CURVE:
            workingNet = NULL;
            UseNetwork(plots[p]);
            if (workingNet->GetPlot()->
               ScaleModeRequiresUpdate(WINMODE_CURVE, ds, rs))
                retval = true;
            break;
        }
    }
    workingNet = net;
    return retval;
}

// ****************************************************************************
//  Method: SetUpWindowContents
//
//  Purpose: Pre-rendering content setup.
//
//  Arguments:
//    windowID            window to render in
//    plotIds             the plots to test for re-rendering
//    forceViewerExecute  make the viewer regenerate actors
//
//  Programmer: Tom Fogal
//  Creation:   June 9, 2008
//
//  Modifications:
//
//    Hank Childs, Thu Mar 18 16:06:56 PST 2004
//    Do some bookkeeping for which plots are stored in the vis window already.
//    This allows us to not have to remove the plots and re-add them for each
//    render.
//
//    Mark C. Miller, Fri Apr  2 11:06:09 PST 2004
//    Removed call to FullFrameOff
//
//    Mark C. Miller, Tue May 25 20:44:10 PDT 2004
//    Added code to pass annotationObjectList in SetAnnotationAttributes
//
//    Mark C. Miller, Wed Jul  7 11:42:09 PDT 2004
//    Totally re-organized. Added calls to reduce cell counts for newly
//    added plots. Moved test for exceeding scalable threshold to after point
//    where each plot's network output is actually computed.
//
//    Mark C. Miller, Wed Dec  8 19:42:02 PST 2004
//    Fixed problem where wrong cell-count (un multiplied one) was being used
//    to update global cell counts on the networks
//
//    Mark C. Miller, Tue Jan 18 12:44:34 PST 2005
//    Moved code to modify visual cues to UpdateVisualCues
//
//    Mark C. Miller, Mon Jan 24 19:25:44 PST 2005
//    Made all procs render 3D visual cues not just proc 0
//
//    Mark C. Miller, Tue Mar 14 17:49:26 PST 2006
//    Fixed bug in selecting which cellCounts entries to store as global
//    cell counts for the whole network
//
//    Brad Whitlock, Wed Jul 26 13:16:06 PST 2006
//    Added code to set the fullframe scale into the plot's mappers.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Hank Childs, Wed Sep 19 16:41:53 PDT 2007
//    Have visual cues be added after adding the plots.  Otherwise, they won't
//    know if the window is 2D or 3D and whether they should offset themselves.
//
//    Tom Fogal, Thu Jun 12 14:48:41 EDT 2008
//    Added in logic to UpdateVisualCues and set cell counts, which I
//    apparently forgot to copy when splitting the code up!
//
//    Tom Fogal, Fri Jun 13 09:40:03 EDT 2008
//    Added member qualification to cellCounts, fixing a parallel-only compile
//    error.
//
//    Hank Childs, Fri Aug 15 14:32:16 PDT 2008
//    Set values of needZBufferToCompositeEvenIn2D.
//
//    Tom Fogal, Mon Sep  1 14:33:09 EDT 2008
//    Change an assert to an exception.
//
//    Brad Whitlock, Thu Dec  6 10:02:35 PST 2012
//    Use long instead of int in the SR cell count calculation.
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    use const ref to instead of making a copy of attributes.
//    make use of cached render state where we can. store accurate
//    per process cell counts returned by plots. this can be used to skip
//    compositing when only rank 0 has cells. document use of INT_MAX.
//
//    Burlen Loring, Mon Oct 26 10:43:34 PDT 2015
//    make sure ffscale gets applied to all plots in 2d mode.
//
//    Eric Brugger, Fri Mar 10 13:17:11 PST 2017
//    I created a single implementation for deciding whether to go into
//    scalable rendering mode or not to eliminate a bug where VisIt would
//    go into scalable rendering mode and immediately go back out and
//    a blank image would get displayed.
//
//    Brad Whitlock, Tue Feb 13 15:23:13 PST 2018
//    Make parallel axis window modes be considered 2D so we use viewported
//    mode to composite images to avoid an offset.
//
// ****************************************************************************

void
NetworkManager::SetUpWindowContents(const intVector &plotIds,
                                    bool forceViewerExecute)
{
    EngineVisWinInfo *viswinInfo = renderState.windowInfo;
    VisWindow *viswin = renderState.window;

    // Doesn't make sense to use this unless we need to set the contents up
    if(!renderState.needToSetUpWindowContents)
    {
        EXCEPTION1(ImproperUseException, "Window contents already setup.");
    }

    viswin->ClearPlots();
    viswinInfo->imageBasedPlots.clear();

    // see if there are any non-mesh plots in the list
    // If there's both mesh and non-mesh plots, we don't make the mesh opaque
    // (since it'd block other plots).
    bool hasNonMeshPlots = HasNonMeshPlots(plotIds);

    // see if we need the z-buffer to composite correctly in 2D.
    renderState.needZBufferToCompositeEvenIn2D =
        NeedZBufferToCompositeEvenIn2D(plotIds);

    // Fullframe scale.
    double FFScale[3] = {1.0, 1.0, 1.0};
    bool determinedWindowMode = false;
    size_t nPlotIds = plotIds.size();
    vector<float> cellCountMultiplier(nPlotIds, 1.0f);
    for (size_t i = 0; i < nPlotIds; i++)
    {
        // get the network output as we would normally
        workingNet = NULL;
        UseNetwork(plotIds[i]);

        DataNetwork *workingNetSaved = workingNet;

        avtDataObjectWriter_p tmpWriter =
            GetOutput(false, true, &cellCountMultiplier[i]);

        avtDataObject_p dob = tmpWriter->GetInput();

        // merge polygon info output across processors
        dob->GetInfo().ParallelMerge(tmpWriter);

        avtPlot_p plot = workingNetSaved->GetPlot();

        if (hasNonMeshPlots && (strcmp(plot->GetName(), "MeshPlot") == 0))
        {
           const AttributeSubject *meshAtts =
                plot->SetOpaqueMeshIsAppropriate(false);
           if (meshAtts)
               plot->SetAtts(meshAtts);
        }

        const ViewCurveAttributes &vca = viswinInfo->windowAttributes.GetViewCurve();
        const View2DAttributes &v2a = viswinInfo->windowAttributes.GetView2D();

        ScaleMode ds = (ScaleMode)vca.GetDomainScale();
        ScaleMode rs = (ScaleMode)vca.GetRangeScale();
        ScaleMode xs = (ScaleMode)v2a.GetXScale();
        ScaleMode ys = (ScaleMode)v2a.GetYScale();

        plot->SetScaleMode(ds, rs, WINMODE_CURVE);
        plot->SetScaleMode(xs, ys, WINMODE_2D);

        avtActor_p anActor = workingNetSaved->GetActor(dob, forceViewerExecute);

        // Make sure that the actor's name is set to the plot's name so
        // the legend annotation objects in the annotation object list
        // will be able to set the plot's legend attributes.
        anActor->SetActorName(workingNetSaved->GetPlotName().c_str());

        // record cell counts including and not including polys
        // notes: 1) INT_MAX is the default value returned by "image based"
        // plots such as volume rendering other plots default to 1.0.
        // 2) when the bool on the get number of cells call is false it
        // means get the true number of cells, otherwise it gets the
        // number of cells where dataset topodim is < 3.
        // 3) second cell count (topodim < 3) is not currently used for
        // anything. so I'm removing it.
        //
        // num cells as reported by VTK
        renderState.cellCounts[i] =
            anActor->GetDataObject()->GetNumberOfCells(false);

        viswin->AddPlot(anActor);

        if (plot->PlotIsImageBased())
            viswinInfo->imageBasedPlots.push_back(plot);

        // Now that a plot has been added to the viswindow, we know
        // if the window is 3D or 2D or curve.
        if (!determinedWindowMode)
        {
            // window mode
            renderState.threeD = viswin->GetWindowMode() == WINMODE_3D;

            renderState.twoD =
                (viswin->GetWindowMode() == WINMODE_2D) ||
                (viswin->GetWindowMode() == WINMODE_CURVE) ||
                (viswin->GetWindowMode() == WINMODE_AXISARRAY) ||
                (viswin->GetWindowMode() == WINMODE_PARALLELAXES) ||
                (viswin->GetWindowMode() == WINMODE_VERTPARALLELAXES);

            determinedWindowMode = true;

            // full frame scaling
            if (renderState.twoD)
            {
                int axis = 0;
                double scale = 1.0;
                viswin->GetScaleFactorAndType(scale, axis);
                FFScale[axis] = scale;
            }
        }

        // If we need to set the fullframe scale, set it now.
        if (renderState.twoD)
            plot->GetMapper()->SetFullFrameScaling(
                viswin->GetFullFrameMode(), FFScale);
    }

    if (renderState.annotMode == 2)
    {
        UpdateVisualCues();
        renderState.handledCues = true;
    }

    // see which ranks have data and thus will contribute to
    // compositing. ranks without data can be excluded to reduce
    // overhead.
    int rank = PAR_Rank();
    int nranks = PAR_Size();
    renderState.haveCells.resize(nranks, 0);
    for (size_t i = 0; i < nPlotIds; ++i)
    {
        // do any plots on this rank have cells??
        if (renderState.cellCounts[i] != 0)
            renderState.haveCells[rank] = 1;
    }

#ifdef PARALLEL
    // share the result
    MPI_Allgather(MPI_IN_PLACE, 0, MPI_INT,
        &renderState.haveCells[0], 1, MPI_INT,
        VISIT_MPI_COMM);
#endif

    // check for cases when data is gathered to rank 0. when
    // only rank 0 has data we can skip compositing
    renderState.onlyRootHasCells = true;
    for (int i = 1; (i < nranks) && renderState.onlyRootHasCells; ++i)
        if (renderState.haveCells[i])
            renderState.onlyRootHasCells = false;

    // Determine the cell counts.
    vector<long long> globalCellCountsTmp(nPlotIds, 0);
    CalculateCellCountTotal(renderState.cellCounts, cellCountMultiplier,
        globalCellCountsTmp, renderState.cellCountTotal);
    for (size_t i = 0; i < nPlotIds; ++i)
        globalCellCounts[plotIds[i]] = globalCellCountsTmp[i];
}


// ****************************************************************************
//  Method: CalculateCellCountTotal
//
//  Purpose: Calculate the total cell count over all plots and processors.
//
//  Arguments:
//    cellCounts          Contains the cell count for the current rank for
//                        each plot on input and the cell count over all the
//                        ranks for each plot on output.
//    cellCountMultiplier Contains the cell count multiplier for each plot.
//    globalCellCounts    On output it contains the cell counts for each
//                        plot, factoring in the cellCountMultiplier.
//    cellCountTotal      On output it contains the total cell count
//                        over all plots.
//
//  Programmer: Eric Brugger
//  Creation:   March 10, 2017
//
//  Modifications:
//
// ****************************************************************************

void
NetworkManager::CalculateCellCountTotal(vector<long long> &cellCounts,
    const vector<float> &cellCountMultipliers,
    vector<long long> &globalCellCounts, long long &cellCountTotal)
{
    size_t nPlots = cellCounts.size();

#ifdef PARALLEL
    // Tally up the local cell counts for each plot into a global
    // per-plot count.
    MPI_Allreduce(MPI_IN_PLACE, &cellCounts[0], nPlots, MPI_LONG_LONG,
        MPI_SUM, VISIT_MPI_COMM);
#endif

    cellCountTotal = 0;
    for (size_t i = 0; i < nPlots; ++i)
    {
        // TODO -- don't use INT_MAX
        // notes: 1) INT_MAX appears in engine-viewer communication logic
        // to work around the below. 2) global cell count as computed in
        // other places in the engine includes the cell count multiplier.
        // 3) the default cellCountMultiplier for image based plots
        // (ray cast volume rendering) is INT_MAX. for other plots it is
        // 1.0f.
        globalCellCounts[i] =
            (cellCountMultipliers[i] > static_cast<float>(INT_MAX/2)) ?
                INT_MAX : cellCounts[i]*cellCountMultipliers[i];

        cellCountTotal = ((globalCellCounts[i] == INT_MAX) ||
             (cellCountTotal == INT_MAX)) ?
                INT_MAX : cellCountTotal + globalCellCounts[i];
    }
}


// ****************************************************************************
//  Method: RenderSetup
//
//  Purpose: Everything we need to do for a render before actually rendering
//           anything; making sure networks are up to date, etc.
//
//  Arguments:
//    plotIds    plots to render
//    getZBuffer whether or not to render the Z buffer
//    annotMode  annotation mode
//    windowID   window identifier for window we'll render in
//    leftEye    in stereo mode, if this is the left eye
//
//  Programmer: Tom Fogal
//  Creation:   June 9, 2008
//
//  Modifications:
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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Modified to use viswinMap
//
//    Brad Whitlock, Tue May 30 14:01:56 PST 2006
//    Added code to set up annotations before adding plots in some cases so
//    annotations that depend on plots being added in order to update
//    themselves get the opportunity to do so.
//
//    Tom Fogal, Thu Jun 12 15:10:03 EDT 2008
//    Removed the TRY block from this code; our caller should have one for us.
//
//    Tom Fogal, Wed Jun 18 15:31:59 EDT 2008
//    Made `plotIds' a reference; this fixes an SR mode bug.
//
//    Tom Fogal, Fri Jul 11 19:46:18 PDT 2008
//    Removed duplicate timer.
//
//    Tom Fogal, Fri Jul 18 17:29:53 EDT 2008
//    Move viewportedMode calculation into here, where it belongs anyway...
//
//    Hank Childs, Fri Jul 25 09:40:16 PDT 2008 (copied from Render)
//    Add some checks for combinations of transparent rendering and volume
//    rendering and also for multiple volume renderings.
//
//    Tom Fogal, Mon Jul 28 16:37:36 EDT 2008
//    Moved tests for invalid rendering combinations to here.
//
//    Tom Fogal, Sun Aug  3 22:57:44 EDT 2008
//    When initializing state, make sure to erase whatever memoization we had
//    from the last frame.
//
//    Tom Fogal, Mon May 25 18:36:19 MDT 2009
//    Force transparency calculation here, so the value we get cached for later
//    rendering.  This prevents us from doing global comm while doing the
//    rendering proper.
//
//    Tom Fogal, Tue May 26 15:45:43 MDT 2009
//    Minor touchups to debug statements.
//
//    Tom Fogal, Fri May 29 20:50:23 MDT 2009
//    ... and move it (transparency cache invalidation) to here.
//    Secondly, don't invalidate it twice!
//
//    Tom Fogal, Tue Jul 21 19:27:49 MDT 2009
//    Account for skipping the SR check.
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    refactor to so that all of the internal configuration is done
//    from/in this method. added ordered compositing support.
//
//    Brad Whitlock, Thu Mar 16 16:04:03 PDT 2017
//    Apply annotations here and remember whether we'll need to change them
//    back in order to not hose the vis window state.
//
//    Brad Whitlock, Thu Sep 21 16:49:49 PDT 2017
//    Added getAlpha.
//
//    Alister Maguire, Mon May 18 16:06:51 PDT 2020
//    If OSPRay is enabled, pass the ospray settings to the render window.
//
//    Eric Brugger, Mon Sep  2 16:59:47 PDT 2024
//    Disable ordered compositing since it doesn't work with VTK9 because
//    the alpha values left in the frame buffer are incorrect.
//
// ****************************************************************************

void
NetworkManager::RenderSetup(avtImageType imgT, int windowID, intVector& plotIds,
                            bool getZBuffer, int annotMode, bool leftEye,
                            bool checkSRThreshold)
{
    renderState.imageType = imgT;
    renderState.origWorkingNet = workingNet;
    renderState.windowID = windowID;

    if(!viswinMap.count(windowID))
    {
        char invalid[256];
        snprintf(invalid, sizeof(invalid),
                 "Attempt to render on invalid window id=%d", windowID);
        EXCEPTION1(ImproperUseException, invalid);
    }

    // Start render timer.
    this->StartTimer();

    EngineVisWinInfo &viswinInfo = viswinMap.find(windowID)->second;
    viswinInfo.markedForDeletion = false;
    std::string &changedCtName = viswinInfo.changedCtName;
    std::vector<int>& plotsCurrentlyInWindow = viswinInfo.plotsCurrentlyInWindow;
    RenderingAttributes &renderAtts = viswinInfo.windowAttributes.GetRenderAtts();

    VisWindow *viswin = viswinInfo.viswin;

    renderState.window = viswin;
    renderState.windowInfo = &viswinInfo;
    renderState.renderOnViewer = false;
    renderState.needToSetUpWindowContents = false;

    size_t nPlots = plotIds.size();
    renderState.cellCounts.resize(nPlots, 0);
    renderState.handledCues = false;
    renderState.stereoType = -1;

#if defined(VISIT_OSPRAY) || defined(HAVE_OSPRAY)
    //
    // Pass the OSPRay settings through so that the correct backend is
    // used when saving windows and such.
    //
    renderState.window->SetOsprayRendering(renderAtts.GetOsprayRendering());
    renderState.window->SetOspraySPP(renderAtts.GetOspraySPP());
    renderState.window->SetOsprayAO(renderAtts.GetOsprayAO());
    renderState.window->SetOsprayShadows(renderAtts.GetOsprayShadows());
#endif

    // Apply any rendering-related changes to the annotation attributes.
    // This may mean turning some of them off, etc. Keep track of whether
    // we need to restore them later.
    renderState.annotMode = annotMode;
    debug5 << "NetworkManager::RenderSetup: annotMode=" << annotMode << endl;
    AnnotationAttributes annot(AnnotationAttributesForRender(
        viswinInfo.annotationAttributes, renderState.annotMode));
    AnnotationObjectList aolist(AnnotationObjectListForRender(
        viswinInfo.annotationObjectList, renderState.annotMode));
    renderState.restoreAnnotations = (annot  != viswinInfo.annotationAttributes) ||
                                     (aolist != viswinInfo.annotationObjectList);
    ApplyAnnotations(viswin, annot, aolist);

    // Explicitly specify left / right eye, for stereo rendering.
    if(viswin->GetStereo())
    {
        renderState.stereoType = viswin->GetStereoType();
        viswin->SetStereoRendering(true, leftEye ? 4 :5);
    }

    // put all plots objects into the VisWindow
    viswin->SetScalableRendering(false);

    // Determine if the plots currently in the window are the same as those
    // we were asked to render.
    bool forceViewerExecute = false;
    if (plotIds.size() != plotsCurrentlyInWindow.size())
    {
        renderState.needToSetUpWindowContents = true;
    }
    else
    {
        forceViewerExecute =
            ViewerExecute(viswin, plotIds, viswinInfo.windowAttributes);

        renderState.needToSetUpWindowContents =
            PlotsNeedUpdating(plotIds, plotsCurrentlyInWindow) ||
            forceViewerExecute;
    }

    // set up the window contents
    if(renderState.needToSetUpWindowContents)
    {
        SetUpWindowContents(plotIds, forceViewerExecute);
        plotsCurrentlyInWindow = plotIds;
    }

    if (checkSRThreshold)
    {
        // scalable threshold test (the 0.5 is to add some hysteresus to avoid
        // the misfortune of oscillating switching of modes around the threshold)
        long scalableThreshold = GetScalableThreshold(renderAtts);
        if (renderState.cellCountTotal < 0.5*scalableThreshold)
        {
            renderState.renderOnViewer = true;
            return;
        }
    }

    // issue warning messages for plots with no data
    if ((renderState.needToSetUpWindowContents) && (PAR_Rank() == 0))
    {
        // determine any networks with no data
        std::vector<int> networksWithNoData;
        for (size_t i = 0; i < nPlots; ++i)
        {
            // INT_MAX is a code used by ray cast volume plot to force
            // scalable rendering. this plot reports 0 cells and handles
            // rendering/compositing internally. would like not to use INT_MAX
            // for this. see note's in TODO below.
            if ((renderState.cellCounts[i] == 0) && (globalCellCounts[plotIds[i]] < INT_MAX))
                networksWithNoData.push_back(static_cast<int>(i));
        }
        size_t nWithout = networksWithNoData.size();
        if (nWithout)
        {
            ostringstream oss;
            oss << "The plot(s) with id(s) = " << networksWithNoData[0];
            for (size_t i = 1; i < nWithout; ++i)
                oss << ", " << networksWithNoData[i];
            oss << " yielded no data";
            avtCallback::IssueWarning(oss.str().c_str());
        }
    }

    // force transparency actor recalulate it's transparency
    avtTransparencyActor* tact = viswin->GetTransparencyActor();
    tact->InvalidateTransparencyCache();

    // check if the render is over the screen size or the view port
    renderState.viewportedMode =
        (renderState.annotMode != 1) || renderState.twoD;

    // do special affects
    renderState.shadowMap = renderState.threeD && renderAtts.GetDoShadowing();
    renderState.depthCues = renderState.threeD && renderAtts.GetDoDepthCueing();

    // Update plot's bg/fg colors. Ignored by avtPlot objects if colors
    // are unchanged
    double bg[4] = {1.,0.,0.,0.};
    double fg[4] = {0.,0.,1.,0.};
    const AnnotationAttributes &annotationAttributes = viswinInfo.annotationAttributes;

    annotationAttributes.GetForegroundColor().GetRgba(fg);
    annotationAttributes.GetDiscernibleBackgroundColor().GetRgba(bg);
    for (size_t i = 0; i < plotIds.size(); i++)
    {
        workingNet = NULL;
        UseNetwork(plotIds[i]);
        workingNet->GetPlot()->SetBackgroundColor(bg);
        workingNet->GetPlot()->SetForegroundColor(fg);
        if (changedCtName != "")
            workingNet->GetPlot()->SetColorTable(changedCtName.c_str());
        workingNet = NULL;
    }

    if (!renderState.handledCues)
    {
        UpdateVisualCues();
        renderState.handledCues = true;
    }

    // update transparency related settings
    renderState.transparency = viswin->TransparenciesExist();

    bool parallel = PAR_Size() > 1;
    renderState.transparencyInPass1 = !parallel && renderState.transparency;

    renderState.transparencyInPass2 = parallel &&
        renderState.threeD && renderState.transparency;

    // any special volume rendering
    renderState.imageBasedPlots = !viswinInfo.imageBasedPlots.empty();

    // will need to read back z-buffer
    renderState.getZBuffer = renderState.transparencyInPass2 ||
        (renderState.threeD && (getZBuffer || renderState.shadowMap ||
         renderState.depthCues || renderState.imageBasedPlots));

    // will need to read back alpha
    renderState.getAlpha = imgT == ColorRGBAImage;

    // note : z even in 2d is determined in SetUpWindowContents
    renderState.gradientBg =
        viswin->GetBackgroundMode() == AnnotationAttributes::Gradient;

    renderState.zBufferComposite = renderState.threeD ||
        renderState.gradientBg || renderState.needZBufferToCompositeEvenIn2D;

    // see if we can do ordered compositing
    renderState.orderComposite = false;
    if (renderState.transparencyInPass2)
    {
        // update the transparency actor's appender so that bounds
        // which we need to get from it's output are valid
        tact->SetUpActor();

	// We just leave ordered compositing false since the alpha
	// values left in the frame buffer are not correct with VTK9.
#if 0
        renderState.orderComposite = viswin->GetOrderComposite() &&
                tact->ComputeCompositingOrder(viswin->GetCamera(),
                    renderState.compositeOrder);
#endif
    }

    // do all procs need the composited image
    renderState.allReducePass1 =
        (renderState.transparencyInPass2 && !renderState.orderComposite)
        || renderState.shadowMap || renderState.depthCues
        || renderState.imageBasedPlots;

     renderState.allReducePass2 = false;

    // Two imageBasedPlots don't do the right thing currently, so put up a
    // warning about it.
    std::vector<avtPlot_p>& imageBasedPlots = viswinInfo.imageBasedPlots;
    if (renderState.transparencyInPass2 && renderState.imageBasedPlots)
    {
        static bool warnTransparentAndIBPs = false;
        if (!warnTransparentAndIBPs)
        {
            // This message is based on how it can occur in VisIt right now.
            // It may need to be generalized in the future.
            avtCallback::IssueWarning(
                "VisIt does not support the rendering of transparent "
                "geometry with ray-traced volume plots.  The volume plots are not being "
                "rendered.  (This message will only be issued once per session.)");
            warnTransparentAndIBPs = false;
        }
        imageBasedPlots.clear();
    }

    // Test to make sure we don't have two RayCasted VRs.
    if (imageBasedPlots.size() > 1)
    {
        static bool issuedWarning = false;
        if (!issuedWarning)
        {
            // This message is based on how it can occur in VisIt right now.
            // It may need to be generalized in the future.
            avtCallback::IssueWarning(
                "VisIt does not support multiple ray-traced volume renderings. "
                "Only the first volume plot will be rendered.  (This message will only "
                "be issued once per session.)");
            issuedWarning = false;
        }
        std::vector<avtPlot_p> imageBasedPlots_tmp;
        imageBasedPlots_tmp.push_back(imageBasedPlots[0]);
        imageBasedPlots = imageBasedPlots_tmp;
    }

#ifdef NetworkManagerDEBUG
    debug5 << "NetworkManager::RenderSetup" << endl
        << renderState << endl << endl;
#endif
}

// ****************************************************************************
//  Method: RenderCleanup
//
//  Purpose: Events which occur after a rendering.
//
//  Arguments:
//
//  Programmer: Tom Fogal
//  Creation:   June 10, 2008
//
//  Modifications:
//
//    Tom Fogal, Fri Jul 11 19:46:18 PDT 2008
//    Removed duplicate timer.
//
//    Tom Fogal, Mon Jul 14 12:33:27 PDT 2008
//    New timing scheme; call StopTimer.
//
//    Tom Fogal, Mon Sep  1 14:47:31 EDT 2008
//    Remove an assert.
//
//    Tom Fogal, Tue Jul 21 19:25:39 MDT 2009
//    Account for skipping the SR check.
//
//    Burlen Loring, Tue Sep  1 15:04:14 PDT 2015
//    no longer need to initialize render state in here
//
// ****************************************************************************

void
NetworkManager::RenderCleanup()
{
    // return viswindow to its true stereo mode
    if(renderState.stereoType != -1)
        renderState.window->SetStereoRendering(true, renderState.stereoType);

    this->StopTimer();
}

// ****************************************************************************
//  Method: CreateNullDataWriter
//
//  Purpose: Creates the appropriate null object to give back when indicating
//           that a render should be re-requested in SR mode.
//
//  Arguments:
//    windowID  which window rendering will occur in
//
//  Programmer: Tom Fogal
//  Creation:   June 23, 2008
//
// ****************************************************************************
avtDataObjectWriter_p
NetworkManager::CreateNullDataWriter() const
{
    // Sending back a null result from the render process is used as a
    // special case to indicate that the viewer should set the rendering
    // mode to scalable rendering and then send back another render request
    // RPC.
    debug5 << "Cell count has fallen below SR threshold. Sending the "
              "AVT_NULL_IMAGE_MSG data object to viewer" << endl;

    avtDataObjectWriter_p writer;
    avtNullData_p nullData = new avtNullData(NULL,AVT_NULL_IMAGE_MSG);
    avtDataObject_p dummyDob;
    CopyTo(dummyDob, nullData);
    writer = dummyDob->InstantiateWriter();
    writer->SetInput(dummyDob);
    return writer;
}

// ****************************************************************************
//  Method: RenderingStages
//
//  Purpose: Computes the number of stages a Render will require.
//
//  Arguments:
//    windowID  which window rendering will occur in
//
//  Programmer: Tom Fogal
//  Creation:   June 13, 2008
//
//  Modifications:
//
//    Tom Fogal, Sat Jun 14 14:57:45 EDT 2008
//    Removed const qualification, which isn't fixable because way down the
//    call chain we hit VTK, which isn't const-correct.
//
//    Tom Fogal, Fri Jul 18 15:19:42 EDT 2008
//    Use Shadowing/DepthCueing methods.
//
//    Burlen Loring, Tue Sep  1 14:58:34 PDT 2015
//    Make use of renderState for viswin etc. fix misuse of post increment
//    operator
//
// ****************************************************************************

int
NetworkManager::RenderingStages()
{
    const std::vector<avtPlot_p>& imageBasedPlots
        = renderState.windowInfo->imageBasedPlots;

    // There is always one stage for rendering and two for composition.
    int stages = 3;

    stages += (renderState.shadowMap ? 2 : 0);
    stages += (renderState.depthCues ? 1 : 0);
    stages += (renderState.transparencyInPass2 ? 1 : 0);

    std::vector<avtPlot_p>::const_iterator iter;
    for(iter = imageBasedPlots.begin();
        iter != imageBasedPlots.end();
        ++iter)
    {
        stages += (*(*iter))->GetNumberOfStagesForImageBasedPlot(
            renderState.windowInfo->windowAttributes);
    }

    return stages;
}

// ****************************************************************************
//  Method: RenderGeometry
//
//  Purpose: Renders the geometry for a scene; this is always the opaque
//           objects, and may or may not include translucent objects (depending
//           on the current multipass rendering settings).
//
//  Arguments:
//    windowID  which window rendering will occur in
//
//  Programmer: Tom Fogal
//  Creation:   June 13, 2008
//
//  Modifications:
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    I refactored this method to add support for depth peeling and
//    ordered compositing.
//
//    Burlen Loring, Sat Oct 17 08:23:57 PDT 2015
//    fix a compiler warning
//
//    Kathleen Biagas, Mon May 16 14:46:39 PDT 2016
//    Use start/stop timer instead of TimedClodeBlock to prevent compiler
//    error on BGQ due to #ifdef inside the code block.
//
//    Brad Whitlock, Thu Sep 21 16:52:41 PDT 2017
//    Added support for returning alpha.
//
// ****************************************************************************

avtImage_p
NetworkManager::RenderGeometry()
{
    StackTimer t0("NetworkManager::RenderGeometry");

    // render the image and capture it. Relies upon explicit render
    CallProgressCallback("NetworkManager", "render pass 1", 0, 1);

    VisWindow *viswin = renderState.window;

    if (renderState.transparencyInPass1)
    {
        // if we are using depth peeling we can skip the
        // local camera order geometry sort
        avtTransparencyActor* tact = viswin->GetTransparencyActor();
        int sortOp = avtTransparencyActor::SORT_NONE;
        if (viswin->GetDepthPeeling())
            viswin->EnableDepthPeeling();
        else
            sortOp |= avtTransparencyActor::SORT_DEPTH;
        tact->SetSortOp(sortOp);
    }

    avtImage_p output;
    if (PAR_Size() < 2)
    {
        // If we're returning alpha then don't put the background in.
        bool disableBackground = renderState.getAlpha;

        // don't bother with the compositing code if not in parallel
        viswin->ScreenRender(renderState.imageType,
            renderState.viewportedMode,
            /*doZBbuffer=*/true, /*opaque on=*/true,
            /*translucent on=*/renderState.transparencyInPass1,
            disableBackground, /*input image=*/NULL);

        output = viswin->ScreenReadBack(renderState.viewportedMode,
            /*read z=*/renderState.getZBuffer, /*read a=*/renderState.getAlpha);

        CallProgressCallback("NetworkManager", "render pass 1", 1, 1);
        CallProgressCallback("NetworkManager", "composite pass 1", 0, 1);
    }
    else
    if (renderState.zBufferComposite)
    {
        // do z-buffer composite

        // TODO -- in theory we could skip render and readback when
        // this rank doesn't have geometry, but in practice it's not
        // working. a very odd segv with a bad stack occurs if any ranks
        // skip render/readback
        unsigned char *ri = NULL, *gi = NULL, *bi = NULL, *ai = NULL;
        float *zi = NULL;
        int w = 0, h = 0;
        int rank = PAR_Rank();
        //if (!renderState.haveCells[rank])
        int readbackTimer = visitTimer->StartTimer();
        {
            // configure for ordered composite. 1) enable alpha channel
            // 2) use solid bg 3) set clear color to 0 0 0 0
            double bgColor[3] = {0.0};
            AnnotationAttributes::BackgroundMode bgMode = AnnotationAttributes::Solid;
            if (renderState.orderComposite)
            {
                viswin->EnableAlphaChannel();

                bgMode = viswin->GetBackgroundMode();
                viswin->SetBackgroundMode(AnnotationAttributes::Solid);

                memcpy(bgColor, viswin->GetBackgroundColor(), 3*sizeof(double));
                viswin->SetBackgroundColor(0.0, 0.0, 0.0);
            }

            viswin->ScreenRender(renderState.imageType,
                renderState.viewportedMode,
                /*canvas z=*/true, /*opaque on=*/true,
                /*translucent on=*/false, /*no bg=*/false,
                /*input image=*/NULL);

            viswin->ScreenReadBack(ri,gi,bi,ai,zi, w,h,
                renderState.viewportedMode, /*read z=*/true,
                /*read a=*/renderState.orderComposite);

#ifdef ProgrammableCompositerDEBUG
            writeVTK("geom_in.vtk", ri,gi,bi,ai,zi,w,h);
#endif
            if (renderState.orderComposite)
            {
                viswin->DisableAlphaChannel();
                viswin->SetBackgroundMode(bgMode);
                viswin->SetBackgroundColor(bgColor[0], bgColor[1], bgColor[2]);
            }
        }
        visitTimer->StopTimer(readbackTimer, "Render & Readback");

        CallProgressCallback("NetworkManager", "render pass 1", 1, 1);
        CallProgressCallback("NetworkManager", "composite pass 1", 0, 1);

        // some plots/databases do not do a domain decomposition or
        // gather the data to a single node. in those cases we could
        // skip compositing.
        if (renderState.onlyRootHasCells || OnlyRootNodeHasData(ri, gi, bi, ai, zi, w*h))
        {
#ifdef NetworkManagerDEBUG
            debug2 << "skipped compositing because only root has data" << endl;
#endif
            output = new avtImage(NULL);

            if (rank == 0)
                Merge(output, ri,gi,bi,ai, zi, w,h, true);
            else
                Free(ri,gi,bi,ai, zi);

            if (renderState.allReducePass1 || renderState.orderComposite)
                BroadcastImage(output, w, h, renderState.orderComposite,
                    renderState.allReducePass1, renderState.getZBuffer);
        }
        else
        {
            int nranks = PAR_Size();
            StackTimer t1("Z Compositing");

            // make the list of ranks that need to composite
            // because they have local geometry. if a rank is not
            // in this list then it will do no compositing work.
            // rank 0 always receives the result, whether or not
            // it is in the list.
            vector<int> ranksToComp;
            for (int i = 0; i < nranks; ++i)
                if (renderState.haveCells[i])
                    ranksToComp.push_back(i);

            // composite and redistribute the result (if needed)
            zcomp->Initialize(w,h);
            zcomp->SetInput(ri,gi,bi,ai, zi, true);
            zcomp->SetOrder(ranksToComp);
            zcomp->SetBroadcastColor(renderState.allReducePass1);
            zcomp->SetBroadcastDepth(renderState.allReducePass1
                || renderState.orderComposite || renderState.getZBuffer);
            zcomp->Execute();

            unsigned char *ro = NULL, *go = NULL, *bo = NULL, *ao = NULL;
            float *zo = NULL;

            zcomp->GetOutput(ro,go,bo,ao, zo, true);

            if ((rank == 0) || renderState.allReducePass1 || renderState.orderComposite)
            {
                output = new avtImage(NULL);
                Merge(output, ro,go,bo,ao, zo, w,h, true);
#ifdef ProgrammableCompositerDEBUG
                writeVTK("geom_out.vtk", ro,go,bo,ao,zo,w,h);
#endif
            }

            zcomp->Clear();
        }
    }
    else
    {
        StackTimer t2("NonZ Compositing");

        // do visit's non-z-buffer composite
        viswin->ScreenRender(renderState.imageType,
            renderState.viewportedMode,
            /*doZBuffer=*/true, /*opaque on=*/true,
            /*translucent on=*/renderState.transparencyInPass1,
            /*disable bg=*/false, /*input image=*/NULL);

        output = viswin->ScreenReadBack(renderState.viewportedMode,
            /*read z=*/false, /*read a=*/false);

        CallProgressCallback("NetworkManager", "render pass 1", 1, 1);
        CallProgressCallback("NetworkManager", "composite pass 1", 0, 1);

        // some plots/databases do not do a domain decomposition or
        // gather the data to a single node. in those cases we could
        // skip compositing
        if (renderState.onlyRootHasCells || OnlyRootNodeHasData(output))
        {
#ifdef NetworkManagerDEBUG
            debug2 << "skipped compositing because only root has data" << endl;
#endif
            if (renderState.allReducePass1)
            {
                int w, h;
                output->GetSize(&h, &w);
                BroadcastImage(output, w, h, false, true, false);
            }
        }
        else
        {
            // need to do compositing,
            avtWholeImageCompositer *compositer;
            compositer = new avtWholeImageCompositerNoZ();
            compositer->SetShouldOutputZBuffer(renderState.getZBuffer);
            compositer->SetAllProcessorsNeedResult(renderState.allReducePass1);
            compositer->SetBackground(viswin->GetBackgroundColor());
            int imageRows, imageCols;
            output->GetSize(&imageCols, &imageRows);
            compositer->SetOutputImageSize(imageRows, imageCols);
            compositer->AddImageInput(output, 0, 0);
            compositer->Execute();
            output = compositer->GetTypedOutput();
            delete compositer;
        }
    }

    CallProgressCallback("NetworkManager", "composite pass 1", 1, 1);

    return output;
}

// ****************************************************************************
//  Method: RenderTranslucent
//
//  Purpose: Renders the translucent parts of a scene, and composites it within
//           an existing rendering. in serial (nranks < 2) transparency is
//           handled in pass 1(ie RenderGeometry). This method is only invoked
//           in parallel.
//
//  Arguments:
//    input     An existing image which translucent geometry should be
//              composited with.
//
//  Programmer: Tom Fogal
//  Creation:   June 13, 2008
//
//  Modifications:
//
//    Jeremy Meredith, Fri Oct 22 13:55:47 PDT 2004
//    Forced the second pass to turn off gradient backgrounds before rendering.
//    It was causing erasing of the first-pass results.
//
//    Tom Fogal, Wed Jun 18 16:00:05 EDT 2008
//    Made the input image a constant reference.
//
//    Burlen Loring, Wed Sep  2 11:06:19 PDT 2015
//    refactored for depth peeling and ordered compositing
//
//    Brad Whitlock, Wed Feb 28 11:14:59 PST 2018
//    Enable passing input image's alpha through the compositing. This lets us
//    create images where the translucent geometry is overlayed over a
//    transparent background so that transparency is preserved (if we are
//    requesting an image that has alpha).
//
// ****************************************************************************

avtImage_p
NetworkManager::RenderTranslucent(avtImage_p& input)
{
    StackTimer t0("NetworkManager::RenderTranslucent");
    CallProgressCallback("NetworkManager", "render pass 2", 0, 1);

    VisWindow *viswin = renderState.window;

    // if we have a compositing order we can skip global
    // parallel geometry sort
    int sortOp = avtTransparencyActor::SORT_NONE;
    if (!renderState.orderComposite)
        sortOp |= avtTransparencyActor::SORT_DISTRIBUTE;

    // if we are using depth peeling we can skip the
    // local camera order geometry sort
    if (viswin->GetDepthPeeling())
        viswin->EnableDepthPeeling();
    else
        sortOp |= avtTransparencyActor::SORT_DEPTH;

    avtTransparencyActor* tact = viswin->GetTransparencyActor();
    tact->SetSortOp(sortOp);

    // disbale background
    AnnotationAttributes::BackgroundMode bgMode
        = viswin->GetBackgroundMode();
    viswin->SetBackgroundMode(AnnotationAttributes::Solid);

    int rank = PAR_Rank();

    avtImage_p output;
    // in parallel do either ordered compositing or gather
    // the set of disjoint image tiles after global geometry
    // sort that occured in the transparecny actor.
    if (renderState.orderComposite)
    {
        viswin->EnableAlphaChannel();

        double bgColor[4] = {0., 0., 0., 0.};
        memcpy(bgColor, viswin->GetBackgroundColor(), 3*sizeof(double));
        bgColor[3] = renderState.getAlpha ? 0. : 1.;
        viswin->SetBackgroundColor(0.0, 0.0, 0.0);

        viswin->ScreenRender(renderState.imageType,
            /*mode=*/renderState.viewportedMode,
            /*canvas z=*/true, /*opaque on=*/false,
            /*translucent on=*/true, /*no bg=*/true,
            /*pass 1=*/input);

        float *ri = NULL, *gi = NULL, *bi = NULL, *ai = NULL, *zi = NULL;
        int w = 0, h = 0;
        viswin->ScreenReadBack(
            ri,gi,bi,ai,zi, w,h,
            /*mode=*/renderState.viewportedMode,
            /*read z=*/false, /*read a=*/true);

#ifdef ProgrammableCompositerDEBUG
        writeVTK("trans_in.vtk", ri,gi,bi,ai,zi,w,h);
#endif
        viswin->DisableAlphaChannel();

        viswin->SetBackgroundMode(bgMode);
        viswin->SetBackgroundColor(bgColor[0], bgColor[1], bgColor[2]);

        CallProgressCallback("NetworkManager", "render pass 2", 1, 1);
        CallProgressCallback("NetworkManager", "composite pass 2", 0, 1);

        acomp->Initialize(w,h);
        acomp->SetOrder(renderState.compositeOrder);
        acomp->SetInput(ri,gi,bi,ai, zi, true);
        acomp->SetBroadcastColor(renderState.allReducePass2);
        acomp->SetBroadcastDepth(renderState.allReducePass2);

        // only rank 0 needs the background
        if (rank == 0)
        {
            float *rb = NULL, *gb = NULL, *bb = NULL, *ab = NULL, *zb = NULL;
            Split(rb,gb,bb,ab, zb, w,h, renderState.getAlpha?4:3, input);
            acomp->SetBackgroundColor(bgColor);
            acomp->SetBackground(rb,gb,bb,ab,zb,true);

            if(!renderState.getAlpha)
            {
                // If we're not getting alpha then we do not want a
                // transparent background.
                if (bgMode == AnnotationAttributes::Solid)
                {
                    acomp->ApplyBackgroundColor(bgColor);
                }
                else
                {
                    viswin->ScreenRender(renderState.imageType,
                        /*mode=*/renderState.viewportedMode,
                        /*canvas z=*/true, /*opaque on=*/false,
                        /*translucent on=*/false, /*no bg=*/false,
                        /*pass 1=*/NULL);

                    float *rbi = NULL, *gbi = NULL, *bbi = NULL, *abi = NULL, *zbi = NULL;
                    int w = 0, h = 0;
                    viswin->ScreenReadBack(
                        rbi,gbi,bbi,abi,zbi, w,h,
                        /*mode=*/renderState.viewportedMode,
                        /*read z=*/false, /*read a=*/false);

#ifdef ProgrammableCompositerDEBUG
                    writeVTK("bg_in.vtk", rbi,gbi,bbi,abi,zbi,w,h);
#endif
                    acomp->ApplyBackgroundImage(rbi, gbi, bbi);
                }
            }
        }

        acomp->Execute();

        // only rank 0 has output
        if ((rank == 0) || renderState.allReducePass2)
        {
            float *ro = NULL, *go = NULL, *bo = NULL, *ao = NULL, *zo = NULL;
            acomp->GetOutput(ro,go,bo,ao, zo, true);

#ifdef ProgrammableCompositerDEBUG
            writeVTK("acomp_out.vtk", ro,go,bo,ao,zo, w,h);
#endif
            output = new avtImage(NULL);
            Merge(output, ro,go,bo,ao, zo, w,h, true);

            output->GetImage().SetZBufferVTK(input->GetImage().GetZBufferVTK());
        }

        acomp->Clear();
    }
    else
    {
        // render the translucent geometry
        viswin->ScreenRender(renderState.imageType,
            /*mode=*/renderState.viewportedMode,
            /*canvas z=*/true, /*opaque on=*/false,
            /*translucent on=*/true, /*no bg=*/false,
            /*pass 1=*/input);

        // gather disjoint tiles to rank 0
        avtImage_p rgb = viswin->ScreenReadBack(
            renderState.viewportedMode,
            /*z=*/false, /*a=*/false);

#ifdef ProgrammableCompositerDEBUG
        writeVTK("transtic_in.vtk", rgb->GetImage());
#endif
        CallProgressCallback("NetworkManager", "render pass 2", 1, 1);
        CallProgressCallback("NetworkManager", "composite pass 2", 0, 1);

        int w = 0, h = 0;
        rgb->GetSize(&h, &w); // h,w is intentional.

        avtTiledImageCompositor *comp = new avtTiledImageCompositor;
        comp->SetAllProcessorsNeedResult(renderState.allReducePass2);
        comp->SetOutputImageSize(w, h);
        comp->AddImageInput(rgb, 0, 0);
        comp->Execute();
        output = comp->GetTypedOutput();

        if ((rank == 0) || renderState.allReducePass2)
            output->GetImage().SetZBufferVTK(input->GetImage().GetZBufferVTK());

        delete comp;
    }

    // restore stuff
    if (viswin->GetDepthPeeling())
        viswin->DisableDepthPeeling();
    viswin->SetBackgroundMode(bgMode);

    CallProgressCallback("NetworkManager", "composite pass 2", 0, 1);

    return output;
}


// ****************************************************************************
//  Method: StartTimer
//
//  Purpose: Called at the beginning of the render process.  Use in conjunction
//           with StopTimer to time the length of rendering.
//
//  Arguments:
//
//  Programmer: Tom Fogal
//  Creation:   July 14, 2008
//
// ****************************************************************************
void
NetworkManager::StartTimer()
{
    renderState.timer = visitTimer->StartTimer();
}

// ****************************************************************************
//  Method: StopTimer
//
//  Purpose: Called at the end of the render process.  Use in conjunction
//           with StartTimer to time the length of rendering.
//
//  Arguments:
//    windowID   the window the rendering was performed in.
//
//  Programmer: Tom Fogal
//  Creation:   July 14, 2008
//
// ****************************************************************************
void
NetworkManager::StopTimer()
{
    char msg[1024];
    const VisWindow *viswin = renderState.window;
    int rows,cols;
    viswin->GetSize(rows, cols);

    snprintf(msg, 1023, "NM::Render %lld cells %d pixels",
             renderState.cellCountTotal, rows*cols);
    visitTimer->StopTimer(renderState.timer, msg);
    renderState.timer = -1;
}

// ****************************************************************************
//  Method: RenderShadows
//
//  Purpose: Add shadows to the scene.
//
//  Arguments:
//    windowID      the window we're rendering now, to add shadows to.
//    input_as_dob  input image to put shadows on, cast as a DataObject.
//
//  Programmer: Tom Fogal
//  Creation:   June 13, 2008
//
//  Modifications:
//
//    Hank Childs, Sat Oct 23 14:06:21 PDT 2004
//    Added support for shadows.  Also cleaned up memory leak.
//
//    Jeremy Meredith, Thu Oct 21 17:33:09 PDT 2004
//    Finished the two-pass mode.  Cleaned things up a bit.
//    Refactored image writing to new functions.  Wrote the tiled
//    image compositor.
//
//    Jeremy Meredith, Fri Oct 29 16:41:59 PDT 2004
//    Refactored the code to find the shadow light's view into a method
//    of avtSoftwareShader, since it needs to do a lot of work related to
//    shadowing to even figure out what the view should be.  Separated out
//    the light-view image size from the normal camera one.
//
//    Tom Fogal, Wed Jun 18 16:01:06 EDT 2008
//    Made the input image a reference.
//
//    Burlen Loring, Tue Sep  1 14:40:31 PDT 2015
//    Make use of renderState for active viswin.
//
// ****************************************************************************

void
NetworkManager::RenderShadows(avtImage_p& input) const
{
    StackTimer t0("NetworkManager::RenderShadows");
    CallProgressCallback("NetworkManager", "Creating shadows",0,1);

    VisWindow *viswin = renderState.window;

    //
    // Figure out which direction the light is pointing.
    //
    avtView3D cur_view = viswin->GetView3D();
    const LightList *light_list = viswin->GetLightList();
    const LightAttributes &la = light_list->GetLight0();
    double light_dir[3];
    bool canShade = avtSoftwareShader::GetLightDirection(la, cur_view,
                                                         light_dir);

    if(canShade)
    {
        //
        // Get the image attributes
        //

        int width, height;
        viswin->GetSize(width, height);

        //
        // Create a light source view
        //
        int light_width = (width > 2048) ? 4096 : width*2;
        int light_height = (height > 2048) ? 4096 : height*2;
        avtView3D light_view;
        light_view = avtSoftwareShader::FindLightView(
                            input, cur_view, light_dir,
                            double(light_width)/double(light_height)
                     );
        //
        // Now create a new image from the light source.
        //
        viswin->SetSize(light_width,light_height);
        viswin->SetView3D(light_view);
        avtImage_p myLightImage = viswin->ScreenCapture(
                                  /*viewport=*/renderState.viewportedMode, /*z=*/true
                                  );

        viswin->SetSize(width,height);
        avtWholeImageCompositer *wic = new avtWholeImageCompositerWithZ();
        wic->SetShouldOutputZBuffer(true);
        int imageRows, imageCols;
        myLightImage->GetSize(&imageCols, &imageRows);
        wic->SetOutputImageSize(imageRows, imageCols);
        wic->AddImageInput(myLightImage, 0, 0);
        wic->SetShouldOutputZBuffer(1);
        wic->SetAllProcessorsNeedResult(false);

        //
        // Do the parallel composite using a 1 stage pipeline
        //
        wic->Execute();
        avtImage_p lightImage = wic->GetTypedOutput();
        viswin->SetView3D(cur_view);

        if (PAR_Rank() == 0)
        {
            CallProgressCallback("NetworkManager", "Synch'ing up shadows", 0, 1);

            double shadow_strength = renderState.windowInfo->
                windowAttributes.GetRenderAtts().GetShadowStrength();

            avtSoftwareShader::AddShadows(lightImage, input, light_view,
                                          cur_view, shadow_strength);

            CallProgressCallback("NetworkManager", "Synch'ing up shadows", 1, 1);
        }
        delete wic;
    }
    CallProgressCallback("NetworkManager", "Creating shadows",1,1);
}

// ****************************************************************************
//  Method: RenderDepthCues
//
//  Purpose: Adds depth cueing (fog) to a scene.
//
//  Arguments:
//    windowID      the window we're rendering now, to add shadows to.
//    input_as_dob  input image to add depth cueing to, cast as DataObject.
//
//  Programmer: Tom Fogal
//  Creation:   June 16, 2008
//
//  Modifications:
//
//    Tom Fogal, Wed Jun 18 16:00:29 EDT 2008
//    Made the input image a reference.
//
//    Jeremy Meredith, Fri Apr 30 14:23:19 EDT 2010
//    Added automatic mode.
//
//    Burlen Loring, Tue Sep  1 14:40:31 PDT 2015
//    Make use of renderState for active viswin. use const ref to rather
//    than copy of attributes.
//
// ****************************************************************************

void
NetworkManager::RenderDepthCues(avtImage_p& input) const
{
    StackTimer t0("NetworkManager::RenderDepthCues");
    CallProgressCallback("NetworkManager", "Applying depth cueing", 0,1);
    if (PAR_Rank() == 0)
    {
        const RenderingAttributes &renderAtts =
             renderState.windowInfo->windowAttributes.GetRenderAtts();

        const AnnotationAttributes &annoAtts =
             renderState.windowInfo->annotationAttributes;

        avtSoftwareShader::AddDepthCueing(
             input,
             renderState.window->GetView3D(),
             renderAtts.GetDepthCueingAutomatic(),
             renderAtts.GetStartCuePoint(),
             renderAtts.GetEndCuePoint(),
             annoAtts.GetBackgroundColor().GetColor());
    }
    CallProgressCallback("NetworkManager", "Applying depth cueing", 1,1);
}

// ****************************************************************************
//  Method: RenderPostProcess
//
//  Purpose: Post-Rendering plots to apply.  These are typically 2D overlays.
//
//  Arguments:
//    image_plots   the set of plots to apply
//    input_as_dob  input image to apply these plots to, cast as DataObject.
//    windowID      the window we're rendering now
//
//  Programmer: Tom Fogal
//  Creation:   June 16, 2008
//
//  Modifications:
//
//    Mark C. Miller, Mon Jul 26 15:08:39 PDT 2004
//    Added code to post process the composited image when the engine
//    is doing more than just 3D annotations. Fixed bug in determination of
//    viewportedMode bool. Added code to pass frame and state info to
//    set Win/Annot atts.
//
//    Hank Childs, Wed Nov 24 17:28:07 PST 2004
//    Added imageBasedPlots.
//
//    Hank Childs, Thu Mar  2 10:06:33 PST 2006
//    Add support for image based plots.
//
//    Tom Fogal, Wed Jun 18 16:04:03 EDT 2008
//    Made the input image a reference.
//
//    Tom Fogal, Wed Jul  2 15:09:47 EDT 2008
//    Avoid VisWindow lookup if not in `2'nd annotation mode.
//
//    Hank Childs, Fri Nov 14 09:32:54 PST 2008
//    Add a bunch of timings statements.
//
//    Burlne Loring, Tue Sep 15 10:18:12 PDT 2015
//    fix segv on ranks without result.
//
// ****************************************************************************

void
NetworkManager::RenderPostProcess(avtImage_p &input)
{
    StackTimer t0("NetworkManager::RenderPostProcess");

    // If we had modified annotations prior to rendering, restore them. This is
    // needed for in situ where the viewer and engine code share a vis window.
    if(renderState.restoreAnnotations)
    {
        debug5 << "NetworkManager::RenderPostProcess: Restoring annotations." << endl;
        ApplyAnnotations(renderState.window,
                         renderState.windowInfo->annotationAttributes,
                         renderState.windowInfo->annotationObjectList);
    }

    if (renderState.imageBasedPlots)
    {
        std::vector<avtPlot_p>::iterator it = renderState.windowInfo->imageBasedPlots.begin();
        std::vector<avtPlot_p>::iterator end = renderState.windowInfo->imageBasedPlots.end();
        for(; it != end; ++it)
            input = (*it)->ImageExecute(input, renderState.windowInfo->windowAttributes);
    }

    if ((renderState.annotMode == 2) && (PAR_Rank() == 0) &&
        (renderState.imageType == ColorRGBImage || renderState.imageType == ColorRGBAImage)
       )
    {
        // Make sure that the vis window has the right attributes that include the pieces of legend we want.
        renderState.window->SetAnnotationAtts(&renderState.windowInfo->annotationAttributes, true);

        // Render the vis window by inserting the composited image plus any annotations we draw in the foreground.
        input = renderState.window->PostProcessScreenCapture(
            input, renderState.viewportedMode, renderState.getZBuffer);
    }
}

// ****************************************************************************
//  Method: GetQueryParameters
//
//  Purpose: Retrieves default parameters for named query.
//
//  Arguments:
//    qName     The name of the query.
//
//  Returns:    The default parameters in string format.
//
//  Programmer: Kathleen Biagas
//  Creation:   July 15, 2011
//
//  Modifications:
//
// ****************************************************************************

std::string
NetworkManager::GetQueryParameters(const std::string &qName)
{
    return avtQueryFactory::Instance()->GetDefaultInputParams(qName);
}

void
NetworkManager::SetCreateVisWindow(void (*cb)(int, VisWindow *&, bool &, void *), void *cbdata)
{
    CreateVisWindowCB = cb;
    CreateVisWindowCBData = cbdata;
}
