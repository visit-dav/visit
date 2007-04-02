/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <avtDataObjectWriter.h>
#include <avtPlot.h>
#include <AnnotationAttributes.h>
#include <AnnotationObjectList.h>
#include <VisualCueList.h>
#include <WindowAttributes.h>
#include <vectortypes.h>
#include <string>
#include <deque>
#include <map>

class AttributeGroup;
class CompactSILRestrictionAttributes;
class LoadBalancer;
class DataNetwork;
class Netnode;
class NetnodeDB;
class ConstructDDFAttributes;
class ExportDBAttributes;
class PickAttributes;
class QueryAttributes;
class QueryOverTimeAttributes;
class MaterialAttributes;
class MeshManagementAttributes;
class VisWindow;
class avtDDF;

typedef struct _EngineVisWinInfo
{
    WindowAttributes            windowAttributes;
    AnnotationAttributes        annotationAttributes;
    AnnotationObjectList        annotationObjectList;
    bool                        visualCuesNeedUpdate;
    VisualCueList               visualCueList;
    std::string                 extentTypeString;
    std::string                 changedCtName;
    int                         frameAndState[7];
    VisWindow                  *viswin;
    std::vector<int>            plotsCurrentlyInWindow;
    std::vector<avtPlot_p>      imageBasedPlots;
    bool                        markedForDeletion;
} EngineVisWinInfo;

typedef void   (*InitializeProgressCallback)(void *, int);
typedef void   (*ProgressCallback)(void *, const char *, const char *,int,int);


// ****************************************************************************
//  Class: NetworkManager
//
//  Purpose:
//      Handles creation and caching of AVT networks.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 29, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Oct 12 12:50:27 PDT 2000
//    Added 'AddOnionPeel'.
//
//    Kathleen Bonnell, Tue Nov  7 15:17:38 PST 2000
//    Added 'AddRelevantPoints'.
//
//    Kathleen Bonnell, Fri Nov 17 16:33:40 PST 2000 
//    Added 'AddFilledBoundary'.
//
//    Jeremy Meredith, Tue Dec 12 13:52:48 PST 2000
//    Added 'AddMaterialSelect'.
//
//    Kathleen Bonnell, Wed Feb 28 15:04:30 PST 2001 
//    Added 'MakeContourPlot'.
//
//    Jeremy Meredith, Thu Mar  1 13:44:46 PST 2001
//    Removed all the AddXXXFilter methods and replaced them with a single
//    AddFilter factory-ish method.
//
//    Jeremy Meredith, Fri Mar  2 13:04:23 PST 2001
//    Replaced all the MakeXXXPlot methods with a single MakePlot
//    factory-ish method.
//
//    Jeremy Meredith, Sun Mar  4 16:59:11 PST 2001
//    Made AddFilter and MakePlot take a string instead of an enumerated
//    type.  This is to use the new PluginManager.
//
//    Jeremy Meredith, Fri Nov  9 10:23:13 PST 2001
//    Added EndNetwork, UseNetwork, and SetWindowAttributes.
//    Added a netMRU since the netPool is persistent.
//
//    Hank Childs, Fri Nov 30 12:37:26 PST 2001
//    Added UpdatePlotAtts.
//
//    Kathleen Bonnell, Mon Nov 19 13:17:52 PST 2001 
//    Added methods StartPickMode, StopPickMode, Pick, and member
//    requireOriginalCells.
//    
//    Hank Childs, Fri Dec 14 17:39:36 PST 2001
//    Added support for a more compact for of sil restrictions.
//
//    Hank Childs, Mon Jan  7 18:14:08 PST 2002
//    Clean up memory better.
//
//    Sean Ahern, Fri Apr 19 13:45:32 PDT 2002
//    Removed ApplyUnaryOperator.  Added ApplyNamedFunction.
//
//    Brad Whitlock, Fri Jun 28 14:02:38 PST 2002
//    Renamed Network class to DataNetwork.
//
//    Hank Childs, Wed Sep 11 09:32:21 PDT 2002
//    Add some support for cleaning up old networks.
//
//    Kathleen Bonnell, Mon Sep 16 14:28:09 PDT 2002   
//    Add Query. 
//
//    Jeremy Meredith, Thu Oct 24 16:03:39 PDT 2002
//    Added material options.
//
//    Mark C. MIller, Mon Dec  9 17:19:02 PST 2002
//    Added VisWindow data member
//
//    Brad Whitlock, Tue Dec 10 14:52:05 PST 2002
//    Overloaded AddDB.
//
//    Sean Ahern, Mon Dec 23 11:23:13 PST 2002
//    Rearranged the AddDB code into GetDBFromCache to reduce code
//    duplication.  Renamed AddDB to StartNetwork.
//
//    Brad Whitlock, Tue Mar 25 13:43:10 PST 2003
//    Added DefineDB.
//
//    Mark C. Miller, 15Jul03
//    Added method to set annotation attributes
//
//    Jeremy Meredith, Mon Sep 15 17:15:09 PDT 2003
//    Removed SetFinalVariableName.
//
//    Hank Childs, Thu Oct  2 16:31:08 PDT 2003
//    Allow queries to involve multiple networks.
//
//    Jeremy Meredith, Thu Nov  6 13:12:15 PST 2003
//    Added a method to cancel the current network.
//
//    Hank Childs, Mon Jan  5 15:54:26 PST 2004
//    Add unique network ids for all networks.  Also add method
//    ClearNetworksWithDatabase.  Also remove networkMRU since it is unused.
//    Populating networkMRU could also lead to problems with dangling pointers.
//
//    Mark C. Miller, Thu Mar  4 12:07:04 PST 2004
//    Added data member dumpRenders and method DumpRenders()
//
//    Hank Childs, Thu Mar 18 16:05:55 PST 2004
//    Keep track of what plots are in the vis window in SR mode, so we don't
//    have to clear out the vis window with every render.
//
//    Eric Brugger, Fri Mar 19 15:21:24 PST 2004
//    Modified the MakePlot rpc to pass the data extents to the engine.
//
//    Hank Childs, Mon Mar 22 11:10:43 PST 2004
//    Allow for the database's file format type to be specified explicitly.
//
//    Jeremy Meredith, Tue Mar 23 10:55:10 PST 2004
//    Added a file format string to StartNetwork.
//
//    Mark C. Miller, Mon Mar 29 14:27:10 PST 200
//    Added bool for doing 3D annots only to Render method
//
//    Kathleen Bonnell, Wed Mar 31 17:23:01 PST 2004 
//    Added CloneNetwork and AddQueryOverTimeFilter. 
//
//    Kathleen Bonnell, Thu Apr 15 14:07:53 PDT 2004 
//    Add int arg to AddQueryOverTimeFilter. 
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added argument for extents type string to SetWindowAttributes
//    Added local variable to store past value for extents type string
//
//    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//    Add GetScalableThreshold method
//
//    Mark C. Miller, Mon May 24 18:36:13 PDT 2004
//    Added SetGlobalCellCount method
//
//    Mark C. Miller, Tue May 25 20:44:10 PDT 2004
//    Added annotationObjectList data member
//    Added arg for annotation object list to SetAnnotationAttributes
//
//    Kathleen Bonnell, Wed Jun  2 09:48:29 PDT 2004 
//    Added bool arg to StartPick.  Added requireOriginalNodes. 
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added visualCueList data member and arg to SetAnnotationAttributes
//
//    Mark C. Miller, Tue Jul 27 15:11:11 PDT 2004
//    Added frameAndState data member and associated arg to
//    SetAnnotationAttributes
//
//    Mark C. Miller, Mon Aug 23 20:27:17 PDT 2004
//    Added argument to GetOutput for cell count multiplier
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Added view extents to SetWindowAttributes
//    Changed bool flag for 3D annotations to integer mode in
//    SetAnnotationAttributes and Render
//
//    Mark C. Miller, Tue Oct 19 20:18:22 PDT 2004
//    Added arg for color table name to SetWindowAttributes
//
//    Hank Childs, Wed Nov 24 17:30:11 PST 2004
//    Added support for image based plots.
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Modified to use a map of EngineVisWindowInfo objects keyed on the
//    window ID
//
//    Mark C. Miller, Wed Jan  5 10:14:21 PST 2005
//    Added NewVisWindow method
//
//    Mark C. Miller, Tue Jan 18 12:44:34 PST 2005
//    Added bool to indicate if visual cues have changed and a new method
//    to UpdateVisualCues. This enables processing of visual cues to be
//    deferred until rendering time after plots have been added to the viswin
//
//    Hank Childs, Mon Feb 28 16:54:31 PST 2005
//    Added Start/StopQueryMode.  This is because the results of certain
//    execution modes (namely DLB) cannot be queried.  So when we do query,
//    the viewer needs to clear plots, set this mode, and then re-execute.
//
//    Hank Childs, Thu May 26 13:34:01 PDT 2005
//    Added ExportDatabase.
//
//    Mark C. Miller, Wed Jun  8 11:03:31 PDT 2005
//    Added HasNonMeshPlots
//
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added GetShouldUseCompression()
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added mesh management attributes to StartNetwork 
//
//    Hank Childs, Sun Dec  4 16:54:05 PST 2005
//    Add progress to SR.
//
//    Hank Childs, Tue Jan  3 14:04:35 PST 2006
//    Add methods to issue progress callbacks.
//
//    Hank Childs, Mon Feb 13 22:25:04 PST 2006
//    Add support for DDFs.
//
//    Hank Childs, Thu Mar  2 10:04:54 PST 2006
//    Add imageBasedPlots to vis win info, remove from network manager.
//
//    Kathleen Bonnell, Tue Mar  7 08:27:25 PST 2006 
//    Add PickForIntersection.
//
// ****************************************************************************

class NetworkManager
{
   typedef std::map<std::string, stringVector> StringVectorMap;
 public:
                  NetworkManager(void);
                 ~NetworkManager(void);

    void          ClearAllNetworks(void);
    void          ClearNetworksWithDatabase(const std::string &);

    NetnodeDB*    GetDBFromCache(const string &filename, int time,
                                 const char * = NULL);
    void          StartNetwork(const std::string&, const std::string &,
                               const std::string &, int,
                               const CompactSILRestrictionAttributes &,
                               const MaterialAttributes &,
                               const MeshManagementAttributes &);
    void          DefineDB(const std::string &, const std::string &,
                           const stringVector &, int, const std::string &);
    void          AddFilter(const std::string&,
                            const AttributeGroup* = NULL,
                            const unsigned int ninputs = 1);
    void          MakePlot(const std::string&, const AttributeGroup*,
                           const std::vector<double> &);
    int           EndNetwork(int windowID);
    void          CancelNetwork();

    void          UseNetwork(int);
    avtPlot_p     GetPlot(void);
    int           GetCurrentNetworkId(void) const;
    int           GetCurrentWindowId(void) const;
    int           GetTotalGlobalCellCounts(int winID) const;
    void          SetGlobalCellCount(int netId, int cellCount);
    int           GetScalableThreshold(int winId) const;
    bool          GetShouldUseCompression(int winId) const;
    void          DoneWithNetwork(int);

    void          UpdatePlotAtts(int, const AttributeGroup *);

    void          SetWindowAttributes(const WindowAttributes&,
                                      const std::string&,
                                      const double*,
                                      const std::string&,
                                      int);
    void          SetAnnotationAttributes(const AnnotationAttributes&,
                                          const AnnotationObjectList&,
                                          const VisualCueList&, 
                                          const int *fns,
                                          int,
                                          int annotMode=1);

    void          SetLoadBalancer(LoadBalancer *lb) {loadBalancer = lb;};

    bool          HasNonMeshPlots(const intVector plotids);
    avtDataObjectWriter_p GetOutput(bool respondWithNullData,
                                    bool calledForRender,
                                    float *cellCountMultiplier);
    avtDataObjectWriter_p Render(intVector networkIds, bool getZBuffer,
                                 int annotMode, int windowID);
 
    void          StartPickMode(const bool);
    void          StopPickMode(void);
    void          StartQueryMode(void);
    void          StopQueryMode(void);

    void          Pick(const int, const int, PickAttributes *);
    void          PickForIntersection(const int, PickAttributes *);
    void          Query(const std::vector<int> &, QueryAttributes*);
    void          ExportDatabase(const int, ExportDBAttributes *);
    void          ConstructDDF(const int, ConstructDDFAttributes *);
    avtDDF       *GetDDF(const char *);

    void          DumpRenders(void) { dumpRenders = true; };

    void          CloneNetwork(const int id);
    void          AddQueryOverTimeFilter(QueryOverTimeAttributes *,
                                         const int clonedFromId);
    static void   RegisterInitializeProgressCallback(
                                           InitializeProgressCallback, void *);
    static void   RegisterProgressCallback(ProgressCallback, void *);

 private:

    void                        UpdateVisualCues(int winID);
    void                        NewVisWindow(int winID);

    std::vector<DataNetwork*>   networkCache;
    std::vector<int>            globalCellCounts;
    std::deque<DataNetwork*>    networkMRU;
    std::vector<NetnodeDB*>     databaseCache;
    StringVectorMap             virtualDatabases;

    std::vector<Netnode*>       workingNetnodeList;
    DataNetwork                *workingNet;
    std::vector<std::string>    nameStack;

    int                         uniqueNetworkId;
    bool                        requireOriginalCells;
    bool                        requireOriginalNodes;
    bool                        inQueryMode;
    LoadBalancer               *loadBalancer;

    std::map<int, EngineVisWinInfo>   viswinMap;

    bool                        dumpRenders;

    std::vector<avtDDF *>       ddf;
    std::vector<std::string>    ddf_names;

    static InitializeProgressCallback
                                initializeProgressCallback;
    static void                *initializeProgressCallbackArgs;
    static ProgressCallback     progressCallback;
    static void                *progressCallbackArgs;

    static void                 CallInitializeProgressCallback(int);
    static void                 CallProgressCallback(const char *, const char*,
                                                     int, int);
};

#endif
