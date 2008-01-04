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

// ************************************************************************* //
//                              NetworkManager.C                             //
// ************************************************************************* //

#include <snprintf.h>
#include <AttributeSubject.h>
#include <NetworkManager.h>
#include <DataNetwork.h>
#include <ClonedDataNetwork.h>
#include <DebugStream.h>
#include <avtDatabaseFactory.h>
#include <LoadBalancer.h>
#include <ConstructDDFAttributes.h>
#include <DBOptionsAttributes.h>
#include <ExportDBAttributes.h>
#include <MaterialAttributes.h>
#include <MeshManagementAttributes.h>
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
#include <avtApplyDDFExpression.h>
#include <avtCallback.h>
#include <avtColorTables.h>
#include <avtExtents.h>
#include <avtNullData.h>
#include <avtDataObjectQuery.h>
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
#include <avtNodePickQuery.h>
#include <avtParallel.h>
#include <avtPickByNodeQuery.h>
#include <avtPickByZoneQuery.h>
#include <avtZonePickQuery.h>
#include <avtCurvePickQuery.h>
#include <avtSoftwareShader.h>
#include <avtDDF.h>
#include <avtDDFConstructor.h>
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
#include <VisWinRendering.h> // for SetStereoEnabled
#include <ParsingExprList.h>
#include <avtExprNode.h>
#include <DatabasePluginManager.h>
#include <DatabasePluginInfo.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#ifdef PARALLEL
#include <mpi.h>
#include <parallel.h>
#endif
#include <visit-config.h>
#include <TimingsManager.h>
#include <SaveWindowAttributes.h>

#include <set>
#include <map>
using std::set;

//
// Static functions.
//
static double RenderBalance(int numTrianglesIHave);
static void   DumpImage(avtDataObject_p, const char *fmt, bool allprocs=true);
static void   DumpImage(avtImage_p, const char *fmt, bool allprocs=true);
static ref_ptr<avtDatabase> GetDatabase(void *, const std::string &,
                                        int, const char *);
static avtDDF *GetDDFCallbackBridge(void *arg, const char *name);

//
// Static data members of the NetworkManager class.
//
InitializeProgressCallback NetworkManager::initializeProgressCallback = NULL;
void                      *NetworkManager::initializeProgressCallbackArgs=NULL;
ProgressCallback           NetworkManager::progressCallback = NULL;
void                      *NetworkManager::progressCallbackArgs = NULL;



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
// ****************************************************************************
NetworkManager::NetworkManager(void) : virtualDatabases()
{
    workingNet = NULL;
    loadBalancer = NULL;
    requireOriginalCells = false;
    requireOriginalNodes = false;
    inQueryMode = false;
    uniqueNetworkId = 0;
    dumpRenders = false;

    // stuff to support scalable rendering. We always have window 0
    NewVisWindow(0);

    avtCallback::RegisterGetDatabaseCallback(GetDatabase, this);
    avtApplyDDFExpression::RegisterGetDDFCallback(GetDDFCallbackBridge, this);
    avtExpressionEvaluatorFilter::RegisterGetDDFCallback(
                                                  GetDDFCallbackBridge, this);
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
// ****************************************************************************
NetworkManager::~NetworkManager(void)
{
    for (int i = 0; i < networkCache.size(); i++)
        if (networkCache[i] != NULL)
            delete networkCache[i];

    std::map<int, EngineVisWinInfo>::iterator it;
    for (it = viswinMap.begin(); it != viswinMap.end(); it++)
        delete it->second.viswin;

    for (int d = 0 ; d < ddf.size() ; d++)
        delete ddf[d];
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

    std::map<int, EngineVisWinInfo>::iterator it;
    for (it = viswinMap.begin(); it != viswinMap.end(); it++)
    {
        it->second.viswin->ClearPlots();
        it->second.plotsCurrentlyInWindow.clear();
        it->second.imageBasedPlots.clear();
        delete it->second.viswin;
    }
    viswinMap.clear();
    NewVisWindow(0);
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
// ****************************************************************************
void
NetworkManager::ClearNetworksWithDatabase(const std::string &db)
{
    debug3 << "NetworkManager::ClearNetworksWithDatabase()" << endl;
    int i;

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
                    DoneWithNetwork(i);
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
// ****************************************************************************

NetnodeDB *
NetworkManager::GetDBFromCache(const string &filename, int time,
    const char *format, bool treatAllDBsAsTimeVarying)
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
        avtDatabase *db = NULL;
        NetnodeDB *netDB = NULL;
        const char *filename_c = filename.c_str();
        std::vector<std::string> plugins;  // unused
        if (filename.length() >= 6 &&
            filename.substr(filename.length() - 6) == ".visit")
            db = avtDatabaseFactory::VisitFile(filename_c, time, plugins, format);
        else
            db = avtDatabaseFactory::FileList(&filename_c, 1, time, plugins, 
                                              format);
        db->SetFullDBName(filename);

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
// ****************************************************************************

void
NetworkManager::StartNetwork(const string &format,
                             const string &filename,
                             const string &var,
                             int time,
                             const CompactSILRestrictionAttributes &atts,
                             const MaterialAttributes &matopts,
                             const MeshManagementAttributes &meshopts,
			     bool treatAllDBsAsTimeVarying)
{
    // If the variable is an expression, we need to find a "real" variable
    // name to work with.
    string leaf = ParsingExprList::GetRealVariable(var);

    // Make empty strings behave as though no format was specified.
    const char *defaultFormat = 0;
    if(format.size() > 0)
        defaultFormat = format.c_str();

    // Start up the DataNetwork and add the database to it.
    workingNet = new DataNetwork;
    NetnodeDB *netDB = GetDBFromCache(filename, time, defaultFormat,
                                      treatAllDBsAsTimeVarying);
    workingNet->SetNetDB(netDB);
    netDB->SetDBInfo(filename, leaf, time);

    // Put an ExpressionEvaluatorFilter right after the netDB to handle
    // expressions that come up the pipe.
    avtExpressionEvaluatorFilter *f = new avtExpressionEvaluatorFilter();
    NetnodeFilter *filt = new NetnodeFilter(f, "ExpressionEvaluator");
    filt->GetInputNodes().push_back(netDB);
    f->GetOutput()->SetTransientStatus(false);

    // Push the ExpressionEvaluator onto the working list.
    workingNetnodeList.push_back(filt);

    workingNet->AddNode(filt);
    // Push the variable name onto the name stack.
    nameStack.push_back(var);
    debug5 << "NetworkManager::AddDB: Adding " << var.c_str()
           << " to the name stack" << endl;

    // Set up the data spec.
    avtSILRestriction_p silr =
        new avtSILRestriction(workingNet->GetNetDB()->GetDB()->
	    GetSIL(time, treatAllDBsAsTimeVarying), atts);
    avtDataSpecification *dspec = new avtDataSpecification(var.c_str(), time, silr);

    // Set up some options from the data specification
    dspec->SetNeedMixedVariableReconstruction(matopts.GetForceMIR());
    dspec->SetNeedSmoothMaterialInterfaces(matopts.GetSmoothing());
    dspec->SetNeedCleanZonesOnly(matopts.GetCleanZonesOnly());
    dspec->SetNeedValidFaceConnectivity(matopts.GetNeedValidConnectivity());
    dspec->SetMIRAlgorithm(matopts.GetAlgorithm());
    dspec->SetSimplifyHeavilyMixedZones(matopts.GetSimplifyHeavilyMixedZones());
    dspec->SetMaxMaterialsPerZone(matopts.GetMaxMaterialsPerZone());
    dspec->SetIsovolumeMIRVF(matopts.GetIsoVolumeFraction());
    dspec->SetDiscTol(meshopts.GetDiscretizationTolerance()[0]);
    dspec->SetFlatTol(meshopts.GetDiscretizationTolerance()[1]);
    dspec->SetDiscMode(meshopts.GetDiscretizationMode());
    dspec->SetDiscBoundaryOnly(meshopts.GetDiscretizeBoundaryOnly());
    dspec->SetPassNativeCSG(meshopts.GetPassNativeCSG());
    workingNet->SetDataSpec(dspec);
    workingNet->SetTime(dspec->GetTimestep());

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
            for(int i = 0; i < filesWithPath.size(); ++i)
                names[i] = filesWithPath[i].c_str();
            db = avtDatabaseFactory::FileList(names, filesWithPath.size(),
                                              time, plugins, defaultFormat);
            delete [] names;
            names = 0;

            // Add the virtual database to the virtual database map.
            virtualDatabases[dbName] = filesWithPath;
            debug3 << "NetworkManager::DefineDB: Added new virtual database "
                   << "definition for " << dbName.c_str() << endl;
        }
        else if (dbName.substr(dbName.length() - 6) == ".visit")
            db = avtDatabaseFactory::VisitFile(dbName_c, time, plugins,
                                               defaultFormat);
        else
            db = avtDatabaseFactory::FileList(&dbName_c, 1, time, plugins,
                                              defaultFormat);
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
//    Brad Whitlock, Wed Mar 21 23:00:00 PST 2007
//    Added plotName.
//
//    Gunther H. Weber, Thu Apr 12 10:52:36 PDT 2007
//    Add filter to beginning of pipeline if necessary
//
// ****************************************************************************
void
NetworkManager::MakePlot(const string &plotName, const string &pluginID, 
    const AttributeGroup *atts, const vector<double> &dataExtents)
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
                                GetEnginePluginInfo(pluginID)->AllocAvtPlot();

    // Check, whether plot wants to place a filter at the beginning of
    // the pipeline
    if (avtFilter *f = p->GetFilterForTopOfPipeline()) {
	debug5 << "NetworkManager::MakePlot(): Inserting filter on top of pipeline." << std::endl;
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
	debug5 << "NetworkManager::MakePlot(): Added filter after expression evaluator." << std::endl;
    }

    p->SetDataExtents(dataExtents);
    workingNet->SetPlot(p);
    workingNet->GetPlot()->SetAtts(atts);
    workingNet->SetPlottype(pluginID);
    workingNet->SetPlotName(plotName);

    debug5 << "NetworkManager::MakePlot(): Leaving NetworkManager::MakePlot()." << std::endl;
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

    // set the pipeline specification
    int pipelineIndex =
        loadBalancer->AddPipeline(workingNet->GetNetDB()->GetFilename());
    avtPipelineSpecification_p pspec =
        new avtPipelineSpecification(workingNet->GetDataSpec(), pipelineIndex);
    workingNet->SetPipelineSpec(pspec);

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
    if (viswinMap.find(windowID) == viswinMap.end())
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
// ****************************************************************************
int
NetworkManager::GetTotalGlobalCellCounts(int winID) const
{
   int i, sum = 0;

    for (i = 0; i < networkCache.size(); i++)
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
//  Modifications:
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Modified to get information for specific window
//
// ****************************************************************************
int
NetworkManager::GetScalableThreshold(int windowID) const
{
    int scalableAutoThreshold;
    RenderingAttributes::TriStateMode scalableActivationMode;

    // since we're in a const method, we can't use the [] operator to index
    // into the map directly becuase that operator will modify the map if the
    // key is new
    std::map<int, EngineVisWinInfo>::const_iterator it;
    it = viswinMap.find(windowID);
    const EngineVisWinInfo &viswinInfo = it->second;
    const WindowAttributes &windowAttributes = viswinInfo.windowAttributes; 

    scalableAutoThreshold =
        windowAttributes.GetRenderAtts().GetScalableAutoThreshold();
    scalableActivationMode = 
        windowAttributes.GetRenderAtts().GetScalableActivationMode();

    int t = RenderingAttributes::GetEffectiveScalableThreshold(
                                    scalableActivationMode,
                                    scalableAutoThreshold);
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
// ****************************************************************************

void
NetworkManager::DoneWithNetwork(int id)
{
    int i;

    if (id >= networkCache.size())
    {
        debug1 << "Internal error: Done with network ID (" << id
            << " >= num saved networks (" << networkCache.size() << ")"
            << endl;
        EXCEPTION0(ImproperUseException);
    }

    if (networkCache[id] != NULL)
    {
        int thisNetworksWinID = networkCache[id]->GetWinID();

        viswinMap[thisNetworksWinID].viswin->ClearPlots();
        viswinMap[thisNetworksWinID].plotsCurrentlyInWindow.clear();
        viswinMap[thisNetworksWinID].imageBasedPlots.clear();

        //
        // Delete the associated VisWindow if this is the last plot that
        // references it
        //
        bool otherNetsUseThisWindow = false;
        for (i = 0; i < networkCache.size(); i++)
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

        // mark this VisWindow for deletion 
        if (!otherNetsUseThisWindow && thisNetworksWinID) // never delete window 0
        {
            debug1 << "Marking VisWindow for Deletion id=" << thisNetworksWinID << endl;
            viswinMap[thisNetworksWinID].markedForDeletion = true;
        }

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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Changed networkCache[id]->GetID() to networkCache[id]->GetNetID()
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
//    Set flag in dataSpec based on value of requireOriginalCells. 
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
        int winID = workingNet->GetWinID();
        WindowAttributes &windowAttributes = viswinMap[winID].windowAttributes;

        // Hook up the network
        avtDataObject_p output = workingNet->GetOutput();

        workingNet->GetPipelineSpec()->GetDataSpecification()->
            SetMayRequireZones(requireOriginalCells); 
        workingNet->GetPipelineSpec()->GetDataSpecification()->
            SetMayRequireNodes(requireOriginalNodes); 
        if (inQueryMode)
            workingNet->GetPipelineSpec()->NoDynamicLoadBalancing();

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
    for (int i = 0; i < plotIds.size(); i++)
    {
        workingNet = NULL;
        UseNetwork(plotIds[i]);
        if (string(workingNet->GetPlot()->GetName()) != "MeshPlot" &&
            string(workingNet->GetPlot()->GetName()) != "LabelPlot")
        {
            hasNonMeshPlots = true;
            break;
        }
    }
    return hasNonMeshPlots;
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
//    Jeremy Meredith, Fri Oct 22 13:55:47 PDT 2004
//    Forced the second pass to turn off gradient backgrounds before rendering.
//    It was causing erasing of the first-pass results.
//
//    Hank Childs, Sat Oct 23 14:06:21 PDT 2004
//    Added support for shadows.  Also cleaned up memory leak.
//
//    Jeremy Meredith, Fri Oct 29 16:41:59 PDT 2004
//    Refactored the code to find the shadow light's view into a method
//    of avtSoftwareShader, since it needs to do a lot of work related to
//    shadowing to even figure out what the view should be.  Separated out
//    the light-view image size from the normal camera one.
//
//    Hank Childs, Wed Nov  3 14:12:29 PST 2004
//    Fix typo and fix problem with shadows in parallel.
//
//    Hank Childs, Wed Nov 24 17:28:07 PST 2004
//    Added imageBasedPlots.
//
//    Mark C. Miller, Wed Dec  8 19:42:02 PST 2004
//    Fixed problem where wrong cell-count (un multiplied one) was being used
//    to update global cell counts on the networks
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 200
//    Modified to use viswinMap
//
//    Mark C. Miller, Tue Jan 18 12:44:34 PST 2005
//    Added call to UpdateVisualCues
//
//    Mark C. Miller, Mon Jan 24 19:25:44 PST 2005
//    Made all procs render 3D visual cues not just proc 0
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
//    Hank Childs, Thu Mar  2 10:06:33 PST 2006
//    Add support for image based plots.
//
//    Hank Childs, Fri Mar  3 08:32:02 PST 2006
//    Do not do shadowing in 2D.
//
//    Mark C. Miller, Tue Mar 14 17:49:26 PST 2006
//    Fixed bug in selecting which cellCounts entries to store as global
//    cell counts for the whole network
//
//    Brad Whitlock, Tue May 30 14:01:56 PST 2006
//    Added code to set up annotations before adding plots in some cases so
//    annotations that depend on plots being added in order to update 
//    themselves get the opportunity to do so.
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye arg as well as logic to force rendering for one or the
//    other eye.
//
//    Brad Whitlock, Wed Jul 26 13:16:06 PST 2006
//    Added code to set the fullframe scale into the plot's mappers.
//
//    Mark C. Miller, Mon Aug 14 12:26:18 PDT 2006
//    Added code to return viswin to its true stereo type upon completion
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Brad Whitlock, Wed Mar 21 23:06:04 PST 2007
//    Set the actor's name to that of the plot so legend attributes can be
//    set via the annotation object list.
//
//    Jeremy Meredith, Wed Aug 29 15:24:13 EDT 2007
//    Added depth cueing.
//
//    Hank Childs, Wed Sep 19 16:41:53 PDT 2007
//    Have visual cues be added after adding the plots.  Otherwise, they won't
//    know if the window is 2D or 3D and whether they should offset themselves.
//
//    Kathleen Bonnell, Tue Sep 25 10:38:27 PDT 2007 
//    Retrieve scaling modes from 2d and curve view atts and set them in the
//    plot before it executes so the plot will be created with correct scaling. 
//
// ****************************************************************************

avtDataObjectWriter_p
NetworkManager::Render(intVector plotIds, bool getZBuffer, int annotMode,
    int windowID, bool leftEye)
{
    int i;
    DataNetwork *origWorkingNet = workingNet;

    if (viswinMap.find(windowID) == viswinMap.end())
    {
        char tmpStr[256];
        SNPRINTF(tmpStr, sizeof(tmpStr), "Attempt to render on invalid window id=%d", windowID);
        EXCEPTION1(ImproperUseException, tmpStr);
    }

    EngineVisWinInfo &viswinInfo = viswinMap[windowID];
    viswinInfo.markedForDeletion = false;
    VisWindow *viswin = viswinInfo.viswin;
    WindowAttributes &windowAttributes = viswinInfo.windowAttributes;
    std::string &changedCtName = viswinInfo.changedCtName;
    AnnotationAttributes &annotationAttributes = viswinInfo.annotationAttributes;
    AnnotationObjectList &annotationObjectList = viswinInfo.annotationObjectList;
    VisualCueList &visualCueList = viswinInfo.visualCueList;
    int *const &frameAndState = viswinInfo.frameAndState;
    std::vector<int>& plotsCurrentlyInWindow = viswinMap[windowID].plotsCurrentlyInWindow;
    std::vector<avtPlot_p>& imageBasedPlots = viswinMap[windowID].imageBasedPlots;

    TRY
    {
        int t1 = visitTimer->StartTimer();
        avtDataObjectWriter_p writer;
        bool needToSetUpWindowContents = false;
        bool forceViewerExecute = false;
        int *cellCounts = new int[2 * plotIds.size()];
        bool handledAnnotations = false;
        bool handledCues = false;
        int stereoType = -1;

        ViewCurveAttributes vca = windowAttributes.GetViewCurve();
        View2DAttributes v2a = windowAttributes.GetView2D();
        ScaleMode ds = (ScaleMode)vca.GetDomainScale();
        ScaleMode rs = (ScaleMode)vca.GetRangeScale();
        ScaleMode xs = (ScaleMode)v2a.GetXScale();
        ScaleMode ys = (ScaleMode)v2a.GetYScale();

        //
        // Explicitly specify left or right eye for stereo 
        //
        if (viswin->GetStereo())
        {
            stereoType = viswin->GetStereoType();
            viswin->SetStereoRendering(true, leftEye ? 4 : 5);
        }

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
            DataNetwork *wm = workingNet;
            for (int p = 0 ; p < plotIds.size() ; p++)
            {
                if (plotIds[p] != plotsCurrentlyInWindow[p])
                    needToSetUpWindowContents = true;

                if (viswin->GetWindowMode() == WINMODE_2D)
                {
                    workingNet = NULL;
                    UseNetwork(plotIds[p]);
                    if (workingNet->GetPlot()->ScaleModeRequiresUpdate(
                        WINMODE_2D, xs, ys))
                    {
                        needToSetUpWindowContents = true;
                        forceViewerExecute = true;
                    }
                }
                else if (viswin->GetWindowMode() == WINMODE_CURVE)
                {
                    workingNet = NULL;
                    UseNetwork(plotIds[p]);
                    if (workingNet->GetPlot()->ScaleModeRequiresUpdate(
                        WINMODE_CURVE, ds, rs))
                    {
                        needToSetUpWindowContents = true;
                        forceViewerExecute = true;
                    }
                }
            }
            workingNet = wm;
        }

        if (needToSetUpWindowContents)
        {
            int t2 = visitTimer->StartTimer();
            int t3 = visitTimer->StartTimer();
            viswin->ClearPlots();
            imageBasedPlots.clear();
            visitTimer->StopTimer(t3, "Clearing plots out of vis window");

            //
            // If we're doing all annotations on the engine then we need to add
            // the annotations to the window before we add plots so the 
            // annotations that depend on the plot list being updated in order 
            // to change their text with respect to time can update.
            //
            // However: visual cues (i.e. reflines) need to be added after the 
            // plots are added.
            //
            if(annotMode == 2)
            {
                SetAnnotationAttributes(annotationAttributes,
                                        annotationObjectList, visualCueList,
                                        frameAndState, windowID, annotMode);
                handledAnnotations = true;
            }

            // see if there are any non-mesh plots in the list
            bool hasNonMeshPlots = HasNonMeshPlots(plotIds);

            // Fullframe scale.
            double FFScale[] = {1., 1., 1.};
            bool setFFScale = false;
            bool useFFScale = false;
            bool determinedFFScale = false;

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

                if (hasNonMeshPlots &&
                    string(workingNetSaved->GetPlot()->GetName()) == "MeshPlot")
                {
                   const AttributeSubject *meshAtts = workingNetSaved->
                         GetPlot()->SetOpaqueMeshIsAppropriate(false);
                   if (meshAtts != 0)
                       workingNetSaved->GetPlot()->SetAtts(meshAtts);
                }

                workingNetSaved->GetPlot()->SetScaleMode(ds,rs,WINMODE_CURVE);
                workingNetSaved->GetPlot()->SetScaleMode(xs,ys,WINMODE_2D);

                int t5 = visitTimer->StartTimer();
                avtActor_p anActor = workingNetSaved->GetActor(dob, 
                                     forceViewerExecute);
                visitTimer->StopTimer(t5, "Calling GetActor for DOB");

                // Make sure that the actor's name is set to the plot's name so
                // the legend annotation objects in the annotation object list
                // will be able to set the plot's legend attributes.
                anActor->SetActorName(workingNetSaved->GetPlotName().c_str());

                // record cell counts including and not including polys
                if (cellCountMultiplier > INT_MAX/2.)
                {
                    cellCounts[i] = INT_MAX;
                    cellCounts[i+plotIds.size()] = INT_MAX;
                }
                else
                {
                    cellCounts[i] =
                    (int) (anActor->GetDataObject()->GetNumberOfCells(false) *
                                                     cellCountMultiplier);
                    cellCounts[i+plotIds.size()] =
                             anActor->GetDataObject()->GetNumberOfCells(true);
                }

                int t6 = visitTimer->StartTimer();
                viswin->AddPlot(anActor);
                avtPlot_p plot = workingNetSaved->GetPlot();
                if (plot->PlotIsImageBased())
                    imageBasedPlots.push_back(plot);
                visitTimer->StopTimer(t6, "Adding plot to the vis window");

                // Now that a plot has been added to the viswindow, we know
                // if the window is 2D or curve.
                if(!determinedFFScale)
                {
                    determinedFFScale = true;
                    setFFScale = viswin->GetWindowMode() == WINMODE_2D ||
                                 viswin->GetWindowMode() == WINMODE_CURVE;
                    useFFScale = viswin->GetFullFrameMode();
                    if(setFFScale)
                    {
                        int fft = 0;
                        double ffs = 1.;
                        viswin->GetScaleFactorAndType(ffs, fft);
                        if(fft == 0)
                            FFScale[0] = ffs;
                        else if(fft == 1)
                            FFScale[1] = ffs;
                    }
                }

                // If we need to set the fullframe scale, set it now.
                if(setFFScale)
                {
                    workingNetSaved->GetPlot()->GetMapper()->
                        SetFullFrameScaling(useFFScale, FFScale);
                }

                visitTimer->StopTimer(t7, "Setting up one plot");
            }

            if (annotMode == 2)
            {
                UpdateVisualCues(windowID);
                handledCues = true;
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
                MPI_INT, MPI_SUM, VISIT_MPI_COMM);
            for (i = 0; i < 2 * plotIds.size(); i++)
                if (cellCounts[i] != INT_MAX) // if accounts for overflow
                    cellCounts[i] = reducedCounts[i];
            delete [] reducedCounts;
#endif

            // update the global cell counts for each network
            for (i = 0; i < plotIds.size(); i++)
                SetGlobalCellCount(plotIds[i], cellCounts[i]);

            plotsCurrentlyInWindow = plotIds;
            visitTimer->StopTimer(t2, "Setting up window contents");
        }

        int  scalableThreshold = GetScalableThreshold(windowID); 
        // scalable threshold test (the 0.5 is to add some hysteresus to avoid 
        // the misfortune of oscillating switching of modes around the threshold)
        if (GetTotalGlobalCellCounts(windowID) < 0.5 * scalableThreshold)
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
            if (!handledAnnotations)
            {
                SetAnnotationAttributes(annotationAttributes,
                                        annotationObjectList, visualCueList,
                                        frameAndState, windowID, annotMode);
                handledAnnotations = true;
            }
            if (!handledCues)
            {
                UpdateVisualCues(windowID);
                handledCues = true;
            }

            debug5 << "Rendering " << viswin->GetNumPrimitives() 
                   << " primitives.  Balanced speedup = " 
                   << RenderBalance(viswin->GetNumPrimitives()) << "x" << endl;

            //
            // Determine if we need to go for two passes
            //
            bool doShadows = windowAttributes.GetRenderAtts().GetDoShadowing();
            bool doDepthCueing =
                windowAttributes.GetRenderAtts().GetDoDepthCueing();
            bool two_pass_mode = false;
            if (viswin->GetWindowMode() == WINMODE_3D)
            {
#ifdef PARALLEL
                two_pass_mode = viswin->TransparenciesExist();
                two_pass_mode = UnifyMaximumValue(two_pass_mode);
#endif
            }
            else
            {
                doShadows = false;
                doDepthCueing = false;
            }

            int nstages = 3;  // Rendering + Two for Compositing
            nstages += (doShadows ? 2 : 0);
            nstages += (doDepthCueing ? 1 : 0);
            nstages += (two_pass_mode ? 1 : 0);
            for (int ss = 0 ; ss < imageBasedPlots.size() ; ss++)
            {
                nstages += imageBasedPlots[ss]
                       ->GetNumberOfStagesForImageBasedPlot(windowAttributes);
            }
            
            CallInitializeProgressCallback(nstages);

            // render the image and capture it. Relies upon explicit render
            int t3 = visitTimer->StartTimer();
            bool viewportedMode = (annotMode != 1) || 
                                  (viswin->GetWindowMode() == WINMODE_2D) ||
                                  (viswin->GetWindowMode() == WINMODE_CURVE);


            // ************************************************************
            // FIRST PASS - Opaque only
            // ************************************************************

            CallProgressCallback("NetworkManager", "Render geometry", 0, 1);
            avtImage_p theImage;
            if (two_pass_mode)
                theImage=viswin->ScreenCapture(viewportedMode,true,true,false);
            else
                theImage=viswin->ScreenCapture(viewportedMode,true);
            CallProgressCallback("NetworkManager", "Render geometry", 1, 1);
            CallProgressCallback("NetworkManager", "Compositing", 0, 1);
            
            visitTimer->StopTimer(t3, "Screen capture for SR");

            if (dumpRenders)
                DumpImage(theImage, "before_OpaqueComposite");

            avtWholeImageCompositer *imageCompositer;
            bool haveImagePlots = imageBasedPlots.size();
            if (viswin->GetWindowMode() == WINMODE_3D)
            {
                imageCompositer = new avtWholeImageCompositerWithZ();
                imageCompositer->SetShouldOutputZBuffer(getZBuffer ||
                                                        two_pass_mode ||
                                                        doShadows || 
                                                        doDepthCueing ||
                                                        haveImagePlots);
            }
            else
            {
                // we have to use z-buffer in 2D windows with gradient background
                if (viswin->GetBackgroundMode() == 0)
                    imageCompositer = new avtWholeImageCompositerNoZ();
                else
                    imageCompositer = new avtWholeImageCompositerWithZ();
                imageCompositer->SetShouldOutputZBuffer(two_pass_mode);
            }

            //
            // Set the compositer's background color
            //
            const double *fbg = viswin->GetBackgroundColor();
            unsigned char bg_r = (unsigned char) ((float)fbg[0] * 255.f);
            unsigned char bg_g = (unsigned char) ((float)fbg[1] * 255.f);
            unsigned char bg_b = (unsigned char) ((float)fbg[2] * 255.f);

            imageCompositer->SetBackground(bg_r, bg_g, bg_b);

            //
            // Set up the input image size and add it to compositer's input
            //
            int imageRows, imageCols;
            theImage->GetSize(&imageCols, &imageRows);
            imageCompositer->SetOutputImageSize(imageRows, imageCols);
            imageCompositer->AddImageInput(theImage, 0, 0);
            imageCompositer->SetAllProcessorsNeedResult(two_pass_mode || 
                                                        doShadows ||
                                                        doDepthCueing ||
                                                        haveImagePlots);

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
                          "after_OpaqueComposite", two_pass_mode);
            CallProgressCallback("NetworkManager", "Compositing", 1, 1);


            // ************************************************************
            // SECOND PASS - Translucent only
            // ************************************************************

            if (two_pass_mode)
            {
                CallProgressCallback("NetworkManager", "Transparent rendering",
                                     0, 1);
                int t1 = visitTimer->StartTimer();

                //
                // We have to disable any gradient background before
                // rendering, as those will overwrite the first pass
                //
                int bm = viswin->GetBackgroundMode();
                viswin->SetBackgroundMode(0);

                //
                // Do the screen capture
                //
                avtImage_p theImage2;
                theImage2=viswin->ScreenCapture(viewportedMode,true,false,true,
                                            imageCompositer->GetTypedOutput());

                // Restore the background mode for next time
                viswin->SetBackgroundMode(bm);
            
                visitTimer->StopTimer(t1, "Second-pass screen capture for SR");

                if (dumpRenders)
                    DumpImage(theImage2, "before_TranslucentComposite");

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
                CallProgressCallback("NetworkManager", "Transparent rendering",
                                     1, 1);
            }

            //
            // Do shadows if appropriate.
            //
            if (doShadows)
            {
                CallProgressCallback("NetworkManager", "Creating shadows",0,1);
                avtView3D cur_view = viswin->GetView3D();

                //
                // Figure out which direction the light is pointing.
                //
                const LightList *light_list = viswin->GetLightList();
                const LightAttributes &la = light_list->GetLight0();
                double light_dir[3];
                bool canShade = avtSoftwareShader::GetLightDirection(la,
                                                          cur_view, light_dir);

                double strength = 
                    windowAttributes.GetRenderAtts().GetShadowStrength();

                if (canShade)
                {
                    //
                    // Get the image attributes
                    //
                    avtImage_p compositedImage;
                    CopyTo(compositedImage, compositedImageAsDataObject);

                    int width, height;
                    viswin->GetSize(width, height);

                    //
                    // Create a light source view
                    //
                    int light_width = (width > 2048) ? 4096 : width*2;
                    int light_height = (height > 2048) ? 4096 : height*2;
                    avtView3D light_view;
                    light_view = avtSoftwareShader::FindLightView(
                                 compositedImage, cur_view, light_dir, 
                                 double(light_width)/double(light_height));

                    //
                    // Now create a new image from the light source.
                    //
                    viswin->SetSize(light_width,light_height);
                    viswin->SetView3D(light_view);
                    avtImage_p myLightImage =
                                    viswin->ScreenCapture(viewportedMode,true);
                    viswin->SetSize(width,height);
                    avtWholeImageCompositer *wic =
                                            new avtWholeImageCompositerWithZ();
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
    

#ifdef PARALLEL
                    if (PAR_Rank() == 0)
#endif
                    {
                        CallProgressCallback("NetworkManager",
                                             "Synch'ing up shadows", 0, 1);
                        avtSoftwareShader::AddShadows(lightImage, 
                                                      compositedImage,
                                                      light_view,
                                                      cur_view,
                                                      strength);
                        CallProgressCallback("NetworkManager",
                                             "Synch'ing up shadows", 1, 1);
                    }
                    delete wic;
                }
                CallProgressCallback("NetworkManager", "Creating shadows",1,1);
            }

            //
            // Do depth cueing if appropriate.
            //
            if (doDepthCueing)
            {
                CallProgressCallback("NetworkManager", "Applying depth cueing",0,1);
                avtView3D cur_view = viswin->GetView3D();

                //
                // Get the image attributes
                //
                avtImage_p compositedImage;
                CopyTo(compositedImage, compositedImageAsDataObject);

                int width, height;
                viswin->GetSize(width, height);

#ifdef PARALLEL
                if (PAR_Rank() == 0)
#endif
                {
                    const double *start =
                        windowAttributes.GetRenderAtts().GetStartCuePoint();
                    const double *end   =
                        windowAttributes.GetRenderAtts().GetEndCuePoint();
                    unsigned char color[] =
                        {annotationAttributes.GetBackgroundColor().Red(),
                         annotationAttributes.GetBackgroundColor().Green(),
                         annotationAttributes.GetBackgroundColor().Blue()};
                    avtSoftwareShader::AddDepthCueing(compositedImage,cur_view,
                                                      start, end, color);
                }
                CallProgressCallback("NetworkManager", "Applying depth cueing",1,1);
            }

            //
            // If the engine is doing more than just 3D annotations,
            // post-process the composited image.
            //
            if (imageBasedPlots.size() > 0)
            {
                avtImage_p compositedImage;
                CopyTo(compositedImage, compositedImageAsDataObject);
                for (int kk = 0 ; kk < imageBasedPlots.size() ; kk++)
                {
                    avtImage_p newImage = 
                           imageBasedPlots[kk]->ImageExecute(compositedImage, 
                                                             windowAttributes);
                    compositedImage = newImage;
                }
                CopyTo(compositedImageAsDataObject, compositedImage);
            }
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
                          "after_AllComposites", false);
            delete imageCompositer;
        }
        
        delete [] cellCounts;

        // return viswindow to its true stereo mode
        if (stereoType != -1)
            viswin->SetStereoRendering(true, stereoType);

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
    VisWindow *viswin = viswinInfo.viswin;
    WindowAttributes &windowAttributes = viswinInfo.windowAttributes;
    std::string &extentTypeString = viswinInfo.extentTypeString;
    std::string &changedCtName = viswinInfo.changedCtName;

    // do nothing if nothing changed
    if ((windowAttributes == atts) && (extentTypeString == extstr) &&
        (changedCtName == ctName))
    {
        bool extsAreDifferent = false;
        static double curexts[6];
        viswin->GetBounds(curexts);
        for (int i = 0; i < 6; i ++)
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

    //
    // Set the color tables
    //
    avtColorTables::Instance()->SetColorTables(atts.GetColorTables());

    //
    // Set the lights.
    //
    const LightList& lights = atts.GetLights();
    viswin->SetLightList(&lights);

    // Set the specular properties.
    viswin->SetSpecularProperties(atts.GetRenderAtts().GetSpecularFlag(),
                                  atts.GetRenderAtts().GetSpecularCoeff(),
                                  atts.GetRenderAtts().GetSpecularPower(),
                                  atts.GetRenderAtts().GetSpecularColor());

    // Set the color texturing flag.
    viswin->SetColorTexturingFlag(atts.GetRenderAtts().GetColorTexturingFlag());

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
    viswin->SetBackgroundMode(bgMode);
    if (bgMode == 1)
    {
       viswin->SetGradientBackgroundColors(atts.GetGradientBackgroundStyle(),
           atts.GetGradBG1()[0],
           atts.GetGradBG1()[1],
           atts.GetGradBG1()[2],
           atts.GetGradBG2()[0],
           atts.GetGradBG2()[1],
           atts.GetGradBG2()[2]);
    }
    else if(bgMode >= 2)
    {
       viswin->SetBackgroundImage(atts.GetBackgroundImage(), 
           atts.GetImageRepeatX(), atts.GetImageRepeatY());
    }

    if (viswin->GetAntialiasing() != atts.GetRenderAtts().GetAntialiasing())
       viswin->SetAntialiasing(atts.GetRenderAtts().GetAntialiasing());
    if (viswin->GetSurfaceRepresentation() != atts.GetRenderAtts().GetGeometryRepresentation())
       viswin->SetSurfaceRepresentation(atts.GetRenderAtts().GetGeometryRepresentation());
    viswin->SetDisplayListMode(0);  // never

    // handle stereo rendering settings
    if (viswin->GetStereo() != atts.GetRenderAtts().GetStereoRendering() ||
        viswin->GetStereoType() != atts.GetRenderAtts().GetStereoType())
        viswin->SetStereoRendering(atts.GetRenderAtts().GetStereoRendering(),
                                   atts.GetRenderAtts().GetStereoType());

    windowAttributes = atts;
    extentTypeString = extstr;
    changedCtName    = ctName;

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
// ****************************************************************************

void
NetworkManager::UpdateVisualCues(int windowID)
{
    if (viswinMap.find(windowID) == viswinMap.end())
    {
        char tmpStr[256];
        SNPRINTF(tmpStr, sizeof(tmpStr), "Attempt to render on invalid window id=%d", windowID);
        EXCEPTION1(ImproperUseException, tmpStr);
    }

    EngineVisWinInfo &viswinInfo = viswinMap[windowID];
    viswinInfo.markedForDeletion = false;
    VisWindow *viswin = viswinInfo.viswin;
    bool &visualCuesNeedUpdate = viswinInfo.visualCuesNeedUpdate;
    VisualCueList &visualCueList = viswinInfo.visualCueList;

    if (visualCuesNeedUpdate == false)
        return;

    viswin->ClearPickPoints();
    viswin->ClearRefLines();
    for (int i = 0; i < visualCueList.GetNumCues(); i++)
    {
        const VisualCueInfo& cue = visualCueList.GetCues(i);
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

    visualCuesNeedUpdate = false;
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
//    Mark C. Miller, Tue Jan 18 12:44:34 PST 2005
//    Moved code to modify visual cues to UpdateVisualCues
//
//    Mark C. Miller, Mon Jan 24 19:25:44 PST 2005
//    Made all procs render 3D visual cues not just proc 0
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
    VisWindow *viswin = viswinInfo.viswin;
    AnnotationAttributes &annotationAttributes = viswinInfo.annotationAttributes;
    AnnotationObjectList &annotationObjectList = viswinInfo.annotationObjectList;
    bool &visualCuesNeedUpdate = viswinInfo.visualCuesNeedUpdate;
    VisualCueList &visualCueList = viswinInfo.visualCueList;
    int *const &frameAndState = viswinInfo.frameAndState;

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

   }

   // defer processing of visual cues until rendering time 
   if (visCues != visualCueList)
   {
       visualCuesNeedUpdate = true;
       visualCueList = visCues;
   }

   annotationAttributes = atts;
   annotationObjectList = aolist;
   for (i = 0; i < 7; i++)
       frameAndState[i] = fns[i];

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
// ****************************************************************************
 
void
NetworkManager::Pick(const int id, const int winId, PickAttributes *pa)
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
            Render(pids, false, 0, winId, true);
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
        qa.SetPipeIndex(networkCache[id]->GetPipelineSpec()->GetPipelineIndex());
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
                pQ->SetInput(networkCache[id]->GetNodeList()[0]->GetOutput());

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
//  Method:  NetworkManager::ConstructDDF
//
//  Purpose:
//      Constructs a derived data function.
//
//  Arguments:
//    id         The network to use.
//    atts       The ConstructDDF attributes.
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
// ****************************************************************************

void
NetworkManager::ConstructDDF(int id, ConstructDDFAttributes *atts)
{
    if (id >= networkCache.size())
    {
        debug1 << "Internal error:  asked to use network ID (" << id 
               << ") >= num saved networks ("
               << networkCache.size() << ")" << endl;
        EXCEPTION0(ImproperUseException);
    }
 
    if (networkCache[id] == NULL)
    {
        debug1 << "Asked to construct a DDF from a network that has already "
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
        debug1 << "Could not find a valid data set to construct a DDF from"
               << endl;
        EXCEPTION0(NoInputException);
    }

    avtDDFConstructor ddfc;
    ddfc.SetInput(dob);
    avtPipelineSpecification_p spec = networkCache[id]->GetPipelineSpec();
    loadBalancer->ResetPipeline(spec->GetPipelineIndex());
    avtDDF *d = ddfc.ConstructDDF(atts, spec);
    // This should be cleaned up at some point.
    if (d != NULL)
    {
        d->OutputDDF(atts->GetDdfName());
        bool foundMatch = false;
        for (int i = 0 ; i < ddf_names.size() ; i++)
            if (ddf_names[i] == atts->GetDdfName())
            {
                foundMatch = true;
                ddf[i] = d;
            }
        if (!foundMatch)
        {
            ddf.push_back(d);
            ddf_names.push_back(atts->GetDdfName());
        }
    }
}


// ****************************************************************************
//  Method: NetworkManager::GetDDF
//
//  Purpose:
//      Gets a DDF.
//
//  Programmer: Hank Childs
//  Creation:   February 18, 2006
//
// ****************************************************************************

avtDDF *
NetworkManager::GetDDF(const char *name)
{
    for (int i = 0 ; i < ddf_names.size() ; i++)
    {
        if (ddf_names[i] == name)
            return ddf[i];
    }

    return NULL;
}


// ****************************************************************************
//  Method:  NetworkManager::ExportDatabase
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
// ****************************************************************************

void
NetworkManager::ExportDatabase(int id, ExportDBAttributes *atts)
{
    if (id >= networkCache.size())
    {
        debug1 << "Internal error:  asked to use network ID (" << id 
               << ") >= num saved networks ("
               << networkCache.size() << ")" << endl;
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

    const std::string &db_type = atts->GetDb_type_fullname();
    DatabasePluginManager *manager = DatabasePluginManager::Instance();
    if (!manager->PluginAvailable(db_type))
    {
        char msg[1024];
        SNPRINTF(msg, 1024, "Unable to load plugin \"%s\" for exporting.", 
                 db_type.c_str());
        EXCEPTION1(ImproperUseException, msg);
    }
    EngineDatabasePluginInfo *info = manager->GetEnginePluginInfo(db_type);
    if (info == NULL)
    {
        char msg[1024];
        SNPRINTF(msg, 1024, "Unable to get plugin info for \"%s\".", 
                 db_type.c_str());
        EXCEPTION1(ImproperUseException, msg);
    }
    DBOptionsAttributes opts = atts->GetOpts();
    info->SetWriteOptions(&opts);
    avtDatabaseWriter *wrtr = info->GetWriter();

    if (wrtr == NULL)
    {
        char msg[1024];
        SNPRINTF(msg, 1024, "Unable to locate writer for \"%s\".", 
                 db_type.c_str());
        EXCEPTION1(ImproperUseException, msg);
    }

    if (strcmp(dob->GetType(), "avtDataset") != 0)
    {
        EXCEPTION0(ImproperUseException);
    }

    int time = networkCache[id]->GetTime();
    ref_ptr<avtDatabase> db = networkCache[id]->GetNetDB()->GetDatabase();
    wrtr->SetInput(dob);

    wrtr->SetPipelineSpecToUse(networkCache[id]->GetPipelineSpec());
    
    string qualFilename;
    if (atts->GetDirname() == "")
        qualFilename = atts->GetFilename();
    else
        qualFilename = atts->GetDirname() + std::string(SLASH_STRING)
                     + atts->GetFilename();
    bool doAll = false;
    std::vector<std::string> vars = atts->GetVariables();
    if (vars.size() == 1 && vars[0] == "<all>")
    {
        doAll = true;
        vars.clear();
    }
    wrtr->Write(qualFilename, db->GetMetaData(time), vars, doAll);
    delete wrtr;
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
// ****************************************************************************
static double
RenderBalance(int numTrianglesIHave)
{
   double balance = 1.0;

#ifdef PARALLEL
   int rank, size, *triCounts = NULL;

   balance = -1.0;
   rank = PAR_Rank();
   size = PAR_Size();
   if (rank == 0)
      triCounts = new int [size]; 
   MPI_Gather(&numTrianglesIHave, 1, MPI_INT, triCounts, 1, MPI_INT, 0, VISIT_MPI_COMM);
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
    if (qA->GetQueryAtts().GetName() == "Locate and Pick Zone" ||
        qA->GetQueryAtts().GetName() == "Locate and Pick Node" )
    {
        input = networkCache[clonedFromId]->GetPlot()->
                GetIntermediateDataObject();
    }
    else if (qA->GetQueryAtts().GetDataType() == QueryAttributes::OriginalData)
    {
        input = workingNet->GetNodeList()[0]->GetOutput();
    }
    else 
    {
        input = networkCache[clonedFromId]->GetPlot()->
                GetIntermediateDataObject();
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
    {
        const SILRestrictionAttributes *silAtts = silr->MakeAttributes();
        qf->SetSILAtts(silAtts);
        delete silAtts;
    }
    NetnodeFilter *qfilt = new NetnodeFilter(qf, "QueryOverTime");
    qfilt->GetInputNodes().push_back(trans);
    
    workingNetnodeList.push_back(qfilt);
    workingNet->AddNode(qfilt);
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
// ****************************************************************************

void
NetworkManager::NewVisWindow(int winID)
{
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
    for (int i = 0; i < idsToDelete.size(); i++)
    {
        debug1 << "Deleting VisWindow for id=" << idsToDelete[i] << endl;
        delete viswinMap[idsToDelete[i]].viswin;
        viswinMap.erase(idsToDelete[i]);
    }

    debug1 << "Creating new VisWindow for id=" << winID << endl;

    viswinMap[winID].viswin = new VisWindow();
    viswinMap[winID].visualCuesNeedUpdate = false;
    viswinMap[winID].markedForDeletion = false;

    AnnotationAttributes &annotAtts = viswinMap[winID].annotationAttributes;

    annotAtts = *(viswinMap[winID].viswin->GetAnnotationAtts());
    annotAtts.SetUserInfoFlag(false);
    annotAtts.SetDatabaseInfoFlag(false);
    annotAtts.SetLegendInfoFlag(false);
    annotAtts.SetTriadFlag(false);
    annotAtts.SetBboxFlag(false);
    annotAtts.SetAxesFlag(false);
    annotAtts.SetAxesFlag2D(false);
    viswinMap[winID].viswin->SetAnnotationAtts(&annotAtts);

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
// ****************************************************************************
static void
DumpImage(avtDataObject_p img, const char *fmt, bool allprocs)
{
    if (!allprocs && PAR_Rank() != 0)
        return;

    static int numDumps = 0;
    static int numDumpsAll = 0;
    char tmpName[256];
    avtFileWriter *fileWriter = new avtFileWriter();

#ifdef PARALLEL
    if (allprocs)
        sprintf(tmpName, "%s_%03d_%03d.png", fmt, PAR_Rank(), numDumpsAll);
    else
        sprintf(tmpName, "%s_%03d.png", fmt, numDumps);
#else
    sprintf(tmpName, "%s_%03d.png", fmt, numDumps);
#endif

    fileWriter->SetFormat(SaveWindowAttributes::PNG);
    int compress = 1;
    fileWriter->Write(tmpName, img, 100, false, compress, false);

    if (allprocs)
        numDumpsAll++;
    else
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
// ****************************************************************************

static ref_ptr<avtDatabase>
GetDatabase(void *nm, const std::string &filename, int time,const char *format)
{
    NetworkManager *nm2 = (NetworkManager *) nm;
    NetnodeDB *db = nm2->GetDBFromCache(filename, time, format);
    return db->GetDB();
}


// ****************************************************************************
//  Function: GetDDFCallbackBridge
//
//  Purpose:
//      The bridge that can go to the network manager and ask for a DDF.
//
//  Programmer: Hank Childs
//  Creation:   February 18, 2006
//
// ****************************************************************************

static avtDDF *
GetDDFCallbackBridge(void *arg, const char *name)
{
    NetworkManager *nm = (NetworkManager *) arg;
    return nm->GetDDF(name);
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
    for (int i = 0; i < ids.size(); i++)
    {
        if (ids[i] >= networkCache.size())
        {
            debug1 << "Internal error:  asked to use network ID (" 
                   << ids[i] << ") >= " << "num saved networks (" 
                   << networkCache.size() << ")" << endl;
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
        Render(validIds, false, 0, winId, true);
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
