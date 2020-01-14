// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
package llnl.visit;

import java.util.Vector;

// ****************************************************************************
// Class: ViewerState
//
// Purpose:
//   Contains the state objects that make up the Client/Viewer interface.
//
// Notes:      The order that state objects are created must match the order
//             defined in ViewerState.h in the C++ implementation.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 22 16:46:56 PST 2008
//
// Modifications:
//   Brad Whitlock, Fri Jan  9 15:49:54 PST 2009
//   I removed the PlotInfoAttributes from the ViewerState for now. There are
//   now N of them but they contain MapNode, which we don't have in Java yet
//   so we'll just skip over the plot info atts in xfer when they come from
//   the client.
//
//   Brad Whitlock, Thu Aug 26 10:39:25 PDT 2010
//   I added SelectionList.
//
//   Brad Whitlock, Mon Jun 20 15:10:20 PDT 2011
//   I added SelectionProperties and SimulationUIValues.
//
// ****************************************************************************
/**
 * ViewerState contains all of the state objects that make up the
 * client/viewer interface. One sends new state to the viewer by modifying
 * these objects and calling their Notify() method to send their new
 * values back to the viewer.
 *
 * @author Brad Whitlock
 */
public class ViewerState
{
    public ViewerState()
    {
        nPlots = 0;
        nOperators = 0;
        objVector = new Vector();

        // Create the state objects. The order must match ViewerState.h
        attsViewerRPC = new ViewerRPC();
            Register(attsViewerRPC);
        attsPostponedAction = new PostponedAction();
            Register(attsPostponedAction);
        attsSyncAttributes = new SyncAttributes();
            Register(attsSyncAttributes);
        attsMessageAttributes = new MessageAttributes();
            Register(attsMessageAttributes);
        attsStatusAttributes = new StatusAttributes();
            Register(attsStatusAttributes);
        attsDatabaseMetaData = new avtDatabaseMetaData();
            Register(attsDatabaseMetaData);
        attsSILAttributes = new SILAttributes();
            Register(attsSILAttributes);
        attsDBPluginInfoAttributes = new DBPluginInfoAttributes();
            Register(attsDBPluginInfoAttributes);
        attsConstructDataBinningAttributes = new ConstructDataBinningAttributes();
            Register(attsConstructDataBinningAttributes);
        attsClientMethod = new ClientMethod();
            Register(attsClientMethod);
        attsClientInformation = new ClientInformation();
            Register(attsClientInformation);
        attsClientInformationList = new ClientInformationList();
            Register(attsClientInformationList);
        attsSimulationUIValues = new SimulationUIValues();
            Register(attsSimulationUIValues);

        attsExportDBAttributes = new ExportDBAttributes();
            Register(attsExportDBAttributes);
        attsPluginManagerAttributes = new PluginManagerAttributes();
            Register(attsPluginManagerAttributes);
        attsAppearanceAttributes = new AppearanceAttributes();
            Register(attsAppearanceAttributes);
        attsGlobalAttributes = new GlobalAttributes();
            Register(attsGlobalAttributes);
        attsDatabaseCorrelationList = new DatabaseCorrelationList();
            Register(attsDatabaseCorrelationList);
        attsPlotList = new PlotList();
            Register(attsPlotList);
        attsHostProfileList = new HostProfileList();
            Register(attsHostProfileList);
        attsSaveWindowAttributes = new SaveWindowAttributes();
            Register(attsSaveWindowAttributes);
        attsEngineList = new EngineList();
            Register(attsEngineList);
        attsColorTableAttributes = new ColorTableAttributes();
            Register(attsColorTableAttributes);
        attsExpressionList = new ExpressionList();
            Register(attsExpressionList);
        attsAnnotationAttributes = new AnnotationAttributes();
            Register(attsAnnotationAttributes);
        attsSILRestrictionAttributes = new SILRestrictionAttributes();
            Register(attsSILRestrictionAttributes);
        attsViewAxisArrayAttributes = new ViewAxisArrayAttributes();
            Register(attsViewAxisArrayAttributes);
        attsViewCurveAttributes = new ViewCurveAttributes();
            Register(attsViewCurveAttributes);
        attsView2DAttributes = new View2DAttributes();
            Register(attsView2DAttributes);
        attsView3DAttributes = new View3DAttributes();
            Register(attsView3DAttributes);
        attsLightList = new LightList();
            Register(attsLightList);
        attsAnimationAttributes = new AnimationAttributes();
            Register(attsAnimationAttributes);
        attsPickAttributes = new PickAttributes();
            Register(attsPickAttributes);
        attsPrinterAttributes = new PrinterAttributes();
            Register(attsPrinterAttributes);
        attsWindowInformation = new WindowInformation();
            Register(attsWindowInformation);
        attsRenderingAttributes = new RenderingAttributes();
            Register(attsRenderingAttributes);
        attsKeyframeAttributes = new KeyframeAttributes();
            Register(attsKeyframeAttributes);
        attsQueryList = new QueryList();
            Register(attsQueryList);
        attsQueryAttributes = new QueryAttributes();
            Register(attsQueryAttributes);
        attsMaterialAttributes = new MaterialAttributes();
            Register(attsMaterialAttributes);
        attsGlobalLineoutAttributes = new GlobalLineoutAttributes();
            Register(attsGlobalLineoutAttributes);
        attsAnnotationObjectList = new AnnotationObjectList();
            Register(attsAnnotationObjectList);
        attsQueryOverTimeAttributes = new QueryOverTimeAttributes();
            Register(attsQueryOverTimeAttributes);
        attsInteractorAttributes = new InteractorAttributes();
            Register(attsInteractorAttributes);
        attsProcessAttributes = new ProcessAttributes();
            Register(attsProcessAttributes);
        attsMovieAttributes = new MovieAttributes();
            Register(attsMovieAttributes);
        attsMeshManagementAttributes = new MeshManagementAttributes();
            Register(attsMeshManagementAttributes);
        attsLogRPC = new ViewerRPC();
            Register(attsLogRPC);
        attsFileOpenOptions = new FileOpenOptions();
            Register(attsFileOpenOptions);
        attsSelectionProperties = new SelectionProperties();
            Register(attsSelectionProperties);
        attsSelectionList = new SelectionList();
            Register(attsSelectionList);
        attsViewerClientAttributes = new ViewerClientAttributes();
            Register(attsViewerClientAttributes);
        attsViewerClientInformation = new ViewerClientInformation();
            Register(attsViewerClientInformation);
        attsSeedMeAttributes = new SeedMeAttributes();
            Register(attsSeedMeAttributes);
        attsCinemaAttributes = new CinemaAttributes();
            Register(attsCinemaAttributes);
    }

    //
    // Get methods for the state objects
    //
    public ViewerRPC                 GetViewerRPC() { return attsViewerRPC; }
    public PostponedAction           GetPostponedAction() { return attsPostponedAction; }
    public SyncAttributes            GetSyncAttributes() { return attsSyncAttributes; }
    public MessageAttributes         GetMessageAttributes() { return attsMessageAttributes; }
    public StatusAttributes          GetStatusAttributes() { return attsStatusAttributes; }
    public avtDatabaseMetaData       GetDatabaseMetaData() { return attsDatabaseMetaData; }
    public SILAttributes             GetSILAttributes() { return attsSILAttributes; }
    public DBPluginInfoAttributes    GetDBPluginInfoAttributes() { return attsDBPluginInfoAttributes; }
    public ExportDBAttributes        GetExportDBAttributes() { return attsExportDBAttributes; }
    public ConstructDataBinningAttributes    GetConstructDataBinningAttributes() { return attsConstructDataBinningAttributes; }
    public ClientMethod              GetClientMethod() { return attsClientMethod; }
    public ClientInformation         GetClientInformation() { return attsClientInformation; }
    public ClientInformationList     GetClientInformationList() { return attsClientInformationList; }
    public SimulationUIValues        GetSimulationUIValues() { return attsSimulationUIValues; }

    public PluginManagerAttributes   GetPluginManagerAttributes() { return attsPluginManagerAttributes; }
    public AppearanceAttributes      GetAppearanceAttributes() { return attsAppearanceAttributes; }
    public GlobalAttributes          GetGlobalAttributes() { return attsGlobalAttributes; }
    public DatabaseCorrelationList   GetDatabaseCorrelationList() { return attsDatabaseCorrelationList; }
    public PlotList                  GetPlotList() { return attsPlotList; }
    public HostProfileList           GetHostProfileList() { return attsHostProfileList; }
    public SaveWindowAttributes      GetSaveWindowAttributes() { return attsSaveWindowAttributes; }
    public EngineList                GetEngineList() { return attsEngineList; }
    public ColorTableAttributes      GetColorTableAttributes() { return attsColorTableAttributes; }
    public ExpressionList            GetExpressionList() { return attsExpressionList; }
    public AnnotationAttributes      GetAnnotationAttributes() { return attsAnnotationAttributes; }
    public SILRestrictionAttributes  GetSILRestrictionAttributes() { return attsSILRestrictionAttributes; }
    public ViewAxisArrayAttributes   GetViewAxisArrayAttributes() { return attsViewAxisArrayAttributes; }
    public ViewCurveAttributes       GetViewCurveAttributes() { return attsViewCurveAttributes; }
    public View2DAttributes          GetView2DAttributes() { return attsView2DAttributes; }
    public View3DAttributes          GetView3DAttributes() { return attsView3DAttributes; }
    public LightList                 GetLightList() { return attsLightList; }
    public AnimationAttributes       GetAnimationAttributes() { return attsAnimationAttributes; }
    public PickAttributes            GetPickAttributes() { return attsPickAttributes; }
    public PrinterAttributes         GetPrinterAttributes() { return attsPrinterAttributes; }
    public WindowInformation         GetWindowInformation() { return attsWindowInformation; }
    public RenderingAttributes       GetRenderingAttributes() { return attsRenderingAttributes; }
    public KeyframeAttributes        GetKeyframeAttributes() { return attsKeyframeAttributes; }
    public QueryList                 GetQueryList() { return attsQueryList; }
    public QueryAttributes           GetQueryAttributes() { return attsQueryAttributes; }
    public MaterialAttributes        GetMaterialAttributes() { return attsMaterialAttributes; }
    public GlobalLineoutAttributes   GetGlobalLineoutAttributes() { return attsGlobalLineoutAttributes; }
    public AnnotationObjectList      GetAnnotationObjectList() { return attsAnnotationObjectList; }
    public QueryOverTimeAttributes   GetQueryOverTimeAttributes() { return attsQueryOverTimeAttributes; }
    public InteractorAttributes      GetInteractorAttributes() { return attsInteractorAttributes; }
    public ProcessAttributes         GetProcessAttributes() { return attsProcessAttributes; }
    public MovieAttributes           GetMovieAttributes() { return attsMovieAttributes; }
    public MeshManagementAttributes  GetMeshManagementAttributes() { return attsMeshManagementAttributes; }
    public ViewerRPC                 GetLogRPC() { return attsLogRPC; }
    public FileOpenOptions           GetFileOpenOptions() { return attsFileOpenOptions; }
    public SelectionProperties       GetSelectionProperties() { return attsSelectionProperties; }
    public SelectionList             GetSelectionList() { return attsSelectionList; }
    public ViewerClientAttributes    GetViewerClientAttributes() { return attsViewerClientAttributes; }
    public ViewerClientInformation   GetViewerClientInformation() { return attsViewerClientInformation; }
    public SeedMeAttributes          GetSeedMeAttributes() { return attsSeedMeAttributes; }
    public CinemaAttributes          GetCinemaAttributes() { return attsCinemaAttributes; }

    /**
     * Returns the i'th state object.
     *
     * @param i The index of the state object to return.
     * @return The i'th state object
     */
    public AttributeSubject GetStateObject(int i)
    {
        return (i >= 0 && i < objVector.size()) ?
               (AttributeSubject)objVector.elementAt(i) : null;
    }

    /**
     * Registers a state object.
     *
     * @param obj The state object to register.
     */
    public void Register(AttributeSubject obj)
    {
        objVector.addElement(obj);
    }

    /**
     * Registers a plot attributes state object.
     *
     * @param obj The state object to register.
     */
    public void RegisterPlotAttributes(AttributeSubject obj)
    {
        ++nPlots;
        objVector.addElement(obj);

        objVector.addElement(new PlotInfoAttributes());
    }

    /**
     * Registers an operator attributes state object.
     *
     * @param obj The state object to register.
     */
    public void RegisterOperatorAttributes(AttributeSubject obj)
    {
        ++nOperators;
        objVector.addElement(obj);
    }

    /**
     * Return the number of state objects that have been registered.
     *
     * @return The number of state objects
     */
    public int GetNumStateObjects()
    {
        return objVector.size();
    }

    /**
     * Return the number of plot state objects that have been registered.
     *
     * @return The number of plot state objects
     */
    public int GetNumPlotStateObjects()
    {
        return nPlots;
    }

    /**
     * Return the number of operator state objects that have been registered.
     *
     * @return The number of operator state objects
     */
    public int GetNumOperatorStateObjects()
    {
        return nOperators;
    }

    /**
     * Return n'th plot state object
     *
     * @param n Index of the state object to return.
     * @return The n'th plot state objects
     */
    public AttributeSubject GetPlotAttributes(int n)
    {
        int idx = objVector.size()-1 - nOperators - nPlots*2 + n*2;
        return (AttributeSubject)objVector.elementAt(idx);
    }

    /**
     * Return n'th plot information
     *
     * @param n Index of the plot information to return.
     * @return The n'th plot plot information
     */
    public PlotInfoAttributes GetPlotInformation(int n)
    {
        int idx = objVector.size()-1 - nOperators - nPlots*2 + n*2 + 1;
        return (PlotInfoAttributes)objVector.elementAt(idx);
    }

    /**
     * Return n'th operator state object
     *
     * @param n Index of the state object to return.
     * @return The n'th operator state objects
     */
    public AttributeSubject GetOperatorAttributes(int n)
    {
        int idx = objVector.size()-1 - nOperators + n;
        return (AttributeSubject)objVector.elementAt(idx);
    }

    // State Members
    private ViewerRPC                attsViewerRPC;
    private PostponedAction          attsPostponedAction;
    private SyncAttributes           attsSyncAttributes;
    private MessageAttributes        attsMessageAttributes;
    private StatusAttributes         attsStatusAttributes;
    private avtDatabaseMetaData      attsDatabaseMetaData;
    private SILAttributes            attsSILAttributes;
    private DBPluginInfoAttributes   attsDBPluginInfoAttributes;
    private ExportDBAttributes       attsExportDBAttributes;
    private ConstructDataBinningAttributes   attsConstructDataBinningAttributes;
    private ClientMethod             attsClientMethod;
    private ClientInformation        attsClientInformation;
    private ClientInformationList    attsClientInformationList;
    private SimulationUIValues       attsSimulationUIValues;

    private PluginManagerAttributes  attsPluginManagerAttributes;
    private AppearanceAttributes     attsAppearanceAttributes;
    private GlobalAttributes         attsGlobalAttributes;
    private DatabaseCorrelationList  attsDatabaseCorrelationList;
    private PlotList                 attsPlotList;
    private HostProfileList          attsHostProfileList;
    private SaveWindowAttributes     attsSaveWindowAttributes;
    private EngineList               attsEngineList;
    private ColorTableAttributes     attsColorTableAttributes;
    private ExpressionList           attsExpressionList;
    private AnnotationAttributes     attsAnnotationAttributes;
    private SILRestrictionAttributes attsSILRestrictionAttributes;
    private ViewAxisArrayAttributes  attsViewAxisArrayAttributes;
    private ViewCurveAttributes      attsViewCurveAttributes;
    private View2DAttributes         attsView2DAttributes;
    private View3DAttributes         attsView3DAttributes;
    private LightList                attsLightList;
    private AnimationAttributes      attsAnimationAttributes;
    private PickAttributes           attsPickAttributes;
    private PrinterAttributes        attsPrinterAttributes;
    private WindowInformation        attsWindowInformation;
    private RenderingAttributes      attsRenderingAttributes;
    private KeyframeAttributes       attsKeyframeAttributes;
    private QueryList                attsQueryList;
    private QueryAttributes          attsQueryAttributes;
    private MaterialAttributes       attsMaterialAttributes;
    private GlobalLineoutAttributes  attsGlobalLineoutAttributes;
    private AnnotationObjectList     attsAnnotationObjectList;
    private QueryOverTimeAttributes  attsQueryOverTimeAttributes;
    private InteractorAttributes     attsInteractorAttributes;
    private ProcessAttributes        attsProcessAttributes;
    private MovieAttributes          attsMovieAttributes;
    private MeshManagementAttributes attsMeshManagementAttributes;
    private ViewerRPC                attsLogRPC;
    private FileOpenOptions          attsFileOpenOptions;
    private SelectionProperties      attsSelectionProperties;
    private SelectionList            attsSelectionList;
    private ViewerClientAttributes   attsViewerClientAttributes;
    private ViewerClientInformation  attsViewerClientInformation;
    private SeedMeAttributes         attsSeedMeAttributes;
    private CinemaAttributes         attsCinemaAttributes;

    private Vector objVector;
    private int nPlots;
    private int nOperators;
}
