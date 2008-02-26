// ****************************************************************************
//
// Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
// Produced at the Lawrence Livermore National Laboratory
// LLNL-CODE-400142
// All rights reserved.
//
// This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
// full copyright notice is contained in the file COPYRIGHT located at the root
// of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
//
// Redistribution  and  use  in  source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of  source code must  retain the above  copyright notice,
//    this list of conditions and the disclaimer below.
//  - Redistributions in binary form must reproduce the above copyright notice,
//    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
//    documentation and/or other materials provided with the distribution.
//  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
//    be used to endorse or promote products derived from this software without
//    specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
// ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
// LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
// DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
// SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
// CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
// OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ****************************************************************************
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
        attsExportDBAttributes = new ExportDBAttributes();
            Register(attsExportDBAttributes);
        attsConstructDDFAttributes = new ConstructDDFAttributes();
            Register(attsConstructDDFAttributes);
        attsClientMethod = new ClientMethod();
            Register(attsClientMethod);
        attsClientInformation = new ClientInformation();
            Register(attsClientInformation);
        attsClientInformationList = new ClientInformationList();
            Register(attsClientInformationList);
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
        attsPlotInfoAttributes = new PlotInfoAttributes();
            Register(attsPlotInfoAttributes);
        attsFileOpenOptions = new FileOpenOptions();
            Register(attsFileOpenOptions);
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
    public ConstructDDFAttributes    GetConstructDDFAttributes() { return attsConstructDDFAttributes; }
    public ClientMethod              GetClientMethod() { return attsClientMethod; }
    public ClientInformation         GetClientInformation() { return attsClientInformation; }
    public ClientInformationList     GetClientInformationList() { return attsClientInformationList; }
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
    public PlotInfoAttributes        GetPlotInfoAttributes() { return attsPlotInfoAttributes; }
    public FileOpenOptions           GetFileOpenOptions() { return attsFileOpenOptions; }

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
        int idx = objVector.size()-1 - nOperators - nPlots + n;
        return (AttributeSubject)objVector.elementAt(idx);
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
    private ConstructDDFAttributes   attsConstructDDFAttributes;
    private ClientMethod             attsClientMethod;
    private ClientInformation        attsClientInformation;
    private ClientInformationList    attsClientInformationList;
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
    private PlotInfoAttributes       attsPlotInfoAttributes;
    private FileOpenOptions          attsFileOpenOptions;

    private Vector objVector;
    private int nPlots;
    private int nOperators;
}
