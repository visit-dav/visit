// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H
#include <EngineBase.h>

#include <avtDataObjectWriter.h>
#include <avtImageType.h>
#include <avtPlot.h>
#include <AnnotationAttributes.h>
#include <AnnotationObjectList.h>
#include <SelectionSummary.h>
#include <VisualCueList.h>
#include <WindowAttributes.h>
#include <vectortypes.h>
#include <deque>
#include <map>
#include <string>
#include <vector>
#include <engine_main_exports.h>

class AttributeGroup;
class CompactSILRestrictionAttributes;
class DatabasePluginManager;
class LoadBalancer;
class DataNetwork;
class Netnode;
class NetnodeDB;
class ConstructDataBinningAttributes;
class ExportDBAttributes;
class OperatorPluginManager;
class PickAttributes;
class PlotPluginManager;
class QueryAttributes;
class QueryOverTimeAttributes;
class MaterialAttributes;
class MeshManagementAttributes;
class SelectionProperties;
class VisWindow;
class avtDataBinning;
template <typename T>
class ProgrammableCompositer;

struct EngineVisWinInfo
{
    EngineVisWinInfo() :
        windowAttributes(),
        annotationAttributes(),
        annotationObjectList(),
        visualCuesNeedUpdate(false),
        visualCueList(),
        extentTypeString(),
        changedCtName(),
        frameAndState(),
        viswin(NULL),
        plotsCurrentlyInWindow(0),
        imageBasedPlots(0),
        markedForDeletion(false)
        {}

    WindowAttributes            windowAttributes;
    AnnotationAttributes        annotationAttributes;
    AnnotationObjectList        annotationObjectList;
    bool                        visualCuesNeedUpdate;
    VisualCueList               visualCueList;
    std::string                 extentTypeString;
    std::string                 changedCtName;
    int                         frameAndState[7];
    VisWindow                  *viswin;
    bool                        owns;
    std::vector<int>            plotsCurrentlyInWindow;
    std::vector<avtPlot_p>      imageBasedPlots;
    bool                        markedForDeletion;
};

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
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye arg to Render method
//
//    Mark C. Miller, Wed Aug  9 19:40:30 PDT 2006
//    Added SetStereoEnabled
//
//    Brad Whitlock, Wed Mar 21 22:59:34 PST 2007
//    Added plotName to MakePlot.
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added bools to GetDBFromCache and StartNetwork to support to treat
//    all databases as time varying
//
//    Hank Childs, Fri Feb  1 15:44:46 PST 2008
//    Add an arguement to GetDBFromCache for loading plugins.
//
//    Cyrus Harrison, Tue Feb 19 08:42:51 PST 2008
//    Removed dumpRenders (now controled by avtDebugDumpOptions)
//
//    Tom Fogal, Mon Jun  9 09:34:09 EDT 2008
//    Added methods to start splitting up Render().  This works by adding some
//    additional state to the object, via the RenderState struct.
//
//    Tom Fogal, Tue Jun 10 15:50:19 EDT 2008
//    More of the above; `RenderCleanup' today.
//
//    Mark C. Miller, Tue Jun 10 15:57:15 PDT 2008
//    Added bool for ignoring extents to StartNetwork / GetDBFromCache
//
//    Tom Fogal,  Fri Jun 13 11:58:15 EDT 2008
//    Splitting up Render():
//       Added RenderGeometry
//       Added MultipassRendering
//       Added RenderingStages
//       Added RenderTranslucent (required vertical align fixes)
//    Moved methods I'm adding from protected to private.  They won't all stay
//    there, but it's better they are private by default and moved
//    intentionally.
//
//    Tom Fogal, Sat Jun 14 14:59:46 EDT 2008
//    Removed const from RenderingStages and the argument of
//    MultipassRendering, to fix an otherwise-unsolvable compile error.
//
//    Tom Fogal, Mon Jun 16 11:20:52 EDT 2008
//    Splitting up Render():
//       Added RenderDepthCues
//       Added RenderPostProcess
//
//    Tom Fogal, Wed Jun 18 11:31:09 EDT 2008
//    Removed haveImagePlots; done locally.
//
//    Tom Fogal, Wed Jun 18 15:32:51 EDT 2008
//    Changed some arguments to be references in the new methods I added; they
//    used to be all one method, so changes in one place should be visible in
//    all of them (and should aviod copying of large images).
//
//    Tom Fogal, Fri Jun 20 17:21:35 EDT 2008
//    Save windowID in state (needed for IceT child); remove arguments where
//    possible; move methods from private -> protected; made some functions
//    virtual.
//
//    Tom Fogal, Mon Jun 23 10:18:50 EDT 2008
//    Moved some methods + instance variables from private to protected.
//    Made RenderBalance a static method of the class, instead of internal.
//    Add a method to construct the appropriate null DOB to switch to SR.
//
//    Brad Whitlock, Tue Jun 24 15:38:19 PDT 2008
//    Made the plugin managers belong to the Network manager so they are no
//    longer singletons.
//
//    Tom Fogal, Wed Jun 25 17:26:41 EDT 2008
//    Moved CallProgressCallback from private to protected.
//
//    Tom Fogal, Mon Jun 30 15:49:00 EDT 2008
//    private -> protected: MultipassRendering, RenderTranslucent
//
//    Tom Fogal, Mon Jul 14 08:38:58 PDT 2008
//    Use methods to implement the timing infrastructure, so the child can
//    override them.
//
//    Tom Fogal, Wed Jul 16 13:00:14 EDT 2008
//    Made destructor virtual; should've done this earlier.
//
//    Tom Fogal, Fri Jul 18 14:24:35 EDT 2008
//    Methods for determining if rendering effects are enabled.
//
//    Tom Fogal, Sun Aug  3 22:47:59 EDT 2008
//    Added MemoMultipass and ForgetMultipass.
//
//    Tom Fogal, Mon Aug  4 16:52:13 EDT 2008
//    virtualize RenderTranslucent so IceT can use a shorter implementation.
//
//    Kathleen Bonnell, Thu Aug  8 07:54:11 PDT 2008
//    Changed return of ForgetMultipass from bool to void.
//
//    Hank Childs, Wed Aug 13 10:14:21 PDT 2008
//    Added NeedZBufferToCompositeEvenIn2D.
//
//    Hank Childs, Fri Jan 30 09:09:33 PST 2009
//    Added methods for named selections.
//
//    Brad Whitlock, Mon Mar  2 16:56:35 PST 2009
//    I added a SaveWindow method.
//
//    Tom Fogal, Tue Jul 21 19:27:00 MDT 2009
//    Propagate bool for skipping SR threshold down.
//
//    Hank Childs, Sat Aug 21 14:35:47 PDT 2010
//    Rename DDF to DataBinning.
//
//    Dave Pugmire, Tue Aug 24 11:32:12 EDT 2010
//    Add compact domain options.
//
//    Brad Whitlock, Tue Dec 14 14:04:33 PST 2010
//    I made CreateNamedSelection take SelectionProperties.
//
//    Tom Fogal, Wed May 18 12:17:18 MDT 2011
//    Need some debug code in IceT; promoting it to class methods.
//
//    Brad Whitlock, Mon Aug 22 10:23:00 PDT 2011
//    I added a selName argument to StartNetwork.
//
//    Kathleen Biagas, Fri Jul 15 11:10:35 PDT 2011
//    Add GetQueryParameters.
//
//    Eric Brugger, Mon Oct 31 09:52:28 PDT 2011
//    Add a multi resolution display capability for AMR data.
//
//    Brad Whitlock, Fri Sep 28 15:03:06 PDT 2012
//    I added a simpler version of StartNetwork.
//
//    Brad Whitlock, Thu Dec  6 10:52:47 PST 2012
//    Change global cell count variables from int to long.
//
//    Burlen Loring, Thu Aug 20 10:18:50 PDT 2015
//    add constructors to renderState and VisWinInfo classes
//    so these objects get initialized before use. various
//    api updates for order compositing refactor
//
//    Burlen Loring, Mon Sep  7 05:48:16 PDT 2015
//    Use long long for cell counts/number of cells everywhere
//
//    Eric Brugger, Fri Mar 10 13:08:59 PST 2017
//    I created a single implementation for deciding whether to go into
//    scalable rendering mode or not to eliminate a bug where VisIt would
//    go into scalable rendering mode and immediately go back out and
//    a blank image would get displayed.
//
//    Kathleen Biagas, Fri Apr 23 2021
//    Change ExportDatabase atts arg from 'const &' to * so that actual dir
//    name used can be returned in them.
//
// ****************************************************************************

class ENGINE_MAIN_API NetworkManager : public EngineBase
{
 public:
                  NetworkManager(void);
    virtual      ~NetworkManager(void);

    // Set the plugin managers.
    void SetDatabasePluginManager(DatabasePluginManager *mgr);
    void SetPlotPluginManager(PlotPluginManager *mgr);
    void SetOperatorPluginManager(OperatorPluginManager *mgr);

    // Get the plugin managers
    DatabasePluginManager *GetDatabasePluginManager() const;
    OperatorPluginManager *GetOperatorPluginManager() const;
    PlotPluginManager     *GetPlotPluginManager() const;

    void          ClearAllNetworks(void);
    void          ClearNetworksWithDatabase(const std::string &);

    stringVector  GetOpenDatabases() const;
    NetnodeDB*    GetDBFromCache(const std::string &filename, int time,
                                 const char * = NULL, bool=false,
                                 bool=false, bool=false);
    void          StartNetwork(const std::string &format,
                               const std::string &filename,
                               const std::string &var,
                               int time);
    void          StartNetwork(const std::string&, const std::string &,
                               const std::string &, int,
                               const CompactSILRestrictionAttributes &,
                               const MaterialAttributes &,
                               const MeshManagementAttributes &,
                               bool, bool,
                               const std::string &selName,
                               int windowID);
    void          DefineDB(const std::string &, const std::string &,
                           const stringVector &, int, const std::string &);
    void          AddFilter(const std::string&,
                            const AttributeGroup* = NULL,
                            const unsigned int ninputs = 1);
    void          MakePlot(const std::string &plotName,
                           const std::string &pluginID,
                           const AttributeGroup*,
                           const std::vector<double> &);
    int           EndNetwork(int windowID);
    void          CancelNetwork();

    void          UseNetwork(int);
    avtPlot_p     GetPlot(void);
    int           GetCurrentNetworkId(void) const;
    int           GetCurrentWindowId(void) const;
    long long     GetTotalGlobalCellCounts(int winID) const;
    void          SetGlobalCellCount(int netId, long long cellCount);
    int           GetScalableThreshold(int winId) const;
    int           GetCompactDomainsThreshold(int winId) const;
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
    bool          NeedZBufferToCompositeEvenIn2D(const intVector plotids);

    avtDataObjectWriter_p GetOutput(bool respondWithNullData,
                                    bool calledForRender,
                                    float *cellCountMultiplier);

    virtual avtDataObject_p Render(avtImageType imgT, bool getZBuffer,
                                   intVector networkIds, 
                                   bool checkThreshold,  int annotMode,
                                   int windowID, bool leftEye,
                                   // out arguments
                                   int &outImgWidth, int &outImgHeight);

    avtDataObject_p RenderValues(intVector plotIds, bool getZBuffer, int windowID, bool leftEye);

    avtDataObjectWriter_p CreateNullDataWriter() const;

    void          StartPickMode(const bool);
    void          StopPickMode(void);
    void          StartQueryMode(void);
    void          StopQueryMode(void);

    void          Pick(const int, const int, PickAttributes *);
    void          PickForIntersection(const int, PickAttributes *);
    void          Query(const std::vector<int> &, QueryAttributes*);
    std::string   GetQueryParameters(const std::string &qName);
    void          ExportDatabases(const intVector &, ExportDBAttributes *,
                                  const std::string &timeSuffix);
    void          ConstructDataBinning(const int, ConstructDataBinningAttributes *);
    avtDataBinning *GetDataBinning(const char *);

    SelectionSummary CreateNamedSelection(int, const SelectionProperties &);
    void          DeleteNamedSelection(const std::string &);
    void          LoadNamedSelection(const std::string &);
    void          SaveNamedSelection(const std::string &);

    void          CloneNetwork(const int id);
    void          AddQueryOverTimeFilter(QueryOverTimeAttributes *,
                                         const int clonedFromId);
    static void   RegisterInitializeProgressCallback(
                                           InitializeProgressCallback, void *);
    static void   RegisterProgressCallback(ProgressCallback, void *);

    static void   SetStereoEnabled();

    static void   CalculateCellCountTotal(std::vector<long long> &,
                                          const std::vector<float> &,
                                          std::vector<long long> &,
                                          long long &);

    void          SetCreateVisWindow(void (*cb)(int, VisWindow *&, bool &, void *), void *cbdata);

 protected:
    bool               ValidNetworkId(int id) const;
    void               ExportSingleDatabase(int, const ExportDBAttributes &);

    virtual
    avtDataObject_p    RenderInternal();

    void               RenderSetup(avtImageType imgT, int windowId, intVector& networkIds, 
                                   bool getZBuffer, int annotMode, bool leftEye, 
                                   bool checkSRThreshold);
    virtual
    avtImage_p         RenderGeometry();

    void               RenderShadows(avtImage_p& input) const;
    void               RenderDepthCues(avtImage_p& input) const;

    virtual
    avtImage_p         RenderTranslucent(avtImage_p& input);

    void               RenderPostProcess(avtImage_p& input);

    void               RenderCleanup();

    int                RenderingStages();

    virtual void       StartTimer();
    virtual void       StopTimer();

    void               DumpImage(avtDataObject_p img, const char* prefix) const;
    void               DumpImage(const avtImage_p, const char *fmt) const;

    virtual void       FormatDebugImage(char*, size_t, const char*) const;

    static double      RenderBalance(int numTrianglesIHave);
    static void        CallInitializeProgressCallback(int);
    static void        CallProgressCallback(const char *, const char*,
                                            int, int);

    int                GetScalableThreshold(const RenderingAttributes &renderAtts) const;

    void (*CreateVisWindowCB)(int, VisWindow *&, bool &, void *);
    void *CreateVisWindowCBData;
 private:
    struct RenderState
    {
        RenderState() :
            origWorkingNet(NULL),
            windowID(0),
            window(NULL),
            windowInfo(NULL),
            needToSetUpWindowContents(false),
            cellCounts(),
            cellCountTotal(0),
            haveCells(),
            onlyRootHasCells(false),
            renderOnViewer(false),
            stereoType(0),
            timer(0),
            annotMode(0),
            threeD(true),
            twoD(false),
            gradientBg(false),
            getZBuffer(true),
            getAlpha(false),
            zBufferComposite(true),
            allReducePass1(false),
            allReducePass2(false),
            restoreAnnotations(false),
            handledCues(false),
            transparency(false),
            transparencyInPass1(false),
            transparencyInPass2(false),
            orderComposite(false),
            compositeOrder(),
            viewportedMode(false),
            needZBufferToCompositeEvenIn2D(false),
            shadowMap(false),
            depthCues(false),
            imageBasedPlots(false),
            imageType(ColorRGBImage)
            {}

        DataNetwork *origWorkingNet;     // saves this->workingNet
        int windowID;                    // id of window we're rendering.
        VisWindow *window;               // window we are rendering
        EngineVisWinInfo *windowInfo;    // info struct of window we are rendering
        bool needToSetUpWindowContents;
        std::vector<long long> cellCounts; // total num of cells per plot * plot sr multiplier
        long long cellCountTotal;        // total number of cells rendered
        std::vector<int> haveCells;      // true if this rank has any cells
        bool onlyRootHasCells;           // true if rank 0 is the only one with data
        bool renderOnViewer;             // below the scalabale renderiung threshold?
        int stereoType;                  // for push/popping stereo rendering
        int timer;                       // overall render time
        int annotMode;
        bool threeD;                     // window modes
        bool twoD;
        bool gradientBg;                 // background mode is gradient
        bool getZBuffer;                 // should we readback Z too?
        bool getAlpha;                   // should we read back alpha too?
        bool zBufferComposite;           // opaque composite operation (because 2d may/may not need it)
        bool allReducePass1;             // ensure all ranks have the composited image
        bool allReducePass2;             // ensure all ranks have the composited image
        bool restoreAnnotations;         // Do we need to restore annotations after rendering?
        bool handledCues;                //
        bool transparency;               // some trasnparent geometry will be rendered
        bool transparencyInPass1;        // handle both opaque and translucent in one pass (serial)
        bool transparencyInPass2;        // split opaque and translucent into two passes (parallel)
        bool orderComposite;             // can we use ordered alpha compositing?
        std::vector<int> compositeOrder; // order for alpha compositing
        bool viewportedMode;
        bool needZBufferToCompositeEvenIn2D;
        bool shadowMap;                  // will use a shadow map pass
        bool depthCues;                  // will use a depth que pass
        bool imageBasedPlots;            // has image based plots
        avtImageType imageType;
    };
    friend ostream &operator<<(ostream &os, const RenderState &rs);

 protected:
    AnnotationAttributes  AnnotationAttributesForRender(
                              const AnnotationAttributes &atts,
                              int annotMode) const;
    AnnotationObjectList  AnnotationObjectListForRender(
                              const AnnotationObjectList &aolist,
                              int annotMode) const;
    void                  ApplyAnnotations(VisWindow *, 
                              const AnnotationAttributes &atts, 
                              const AnnotationObjectList &aolist);


    DataNetwork                      *workingNet;
    std::map<int, EngineVisWinInfo>   viswinMap;
    RenderState                       renderState;

 private:
    void            UpdateVisualCues();
    void            NewVisWindow(int winID);
    bool            PlotsNeedUpdating(const intVector &plots,
                                      const intVector &plotsInWindow) const;

    bool            ViewerExecute(const VisWindow * const viswin,
                                  const intVector &plots,
                                  const WindowAttributes &windowAttributes);

    void            SetUpWindowContents(const intVector &plotIds,
                                        bool forceViewerExecute);

    void            SetWindowAttributes(EngineVisWinInfo &viswinInfo,
                        const WindowAttributes &atts, const std::string& extstr,
                        const double *vexts, const std::string& ctName);

    // The plugin managers
    DatabasePluginManager      *databasePlugins;
    OperatorPluginManager      *operatorPlugins;
    PlotPluginManager          *plotPlugins;

    std::vector<DataNetwork*>   networkCache;
    std::vector<long long>      globalCellCounts;
    std::deque<DataNetwork*>    networkMRU;
    std::vector<NetnodeDB*>     databaseCache;

    typedef std::map<std::string, stringVector> StringVectorMap;
    StringVectorMap             virtualDatabases;

    std::vector<Netnode*>       workingNetnodeList;
    std::vector<std::string>    nameStack;

    int                         uniqueNetworkId;
    bool                        requireOriginalCells;
    bool                        requireOriginalNodes;
    bool                        inQueryMode;
    LoadBalancer               *loadBalancer;

    std::vector<avtDataBinning *>  dataBinnings;
    std::vector<std::string>       dataBinningNames;

    bool                        initialized;

    ProgrammableCompositer<unsigned char> *zcomp;
    ProgrammableCompositer<float> *acomp;

    static InitializeProgressCallback
                                initializeProgressCallback;
    static void                *initializeProgressCallbackArgs;
    static ProgressCallback     progressCallback;
    static void                *progressCallbackArgs;
};

#endif
