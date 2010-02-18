#include <stdio.h>
#include <map>

// All of the state objects in common/state
#include <AttributeGroup.h>
#include <AnimationAttributes.h>
#include <AnnotationAttributes.h>
#include <AnnotationObject.h>
#include <AnnotationObjectList.h>
#include <AppearanceAttributes.h>
#include <BoxExtents.h>
#include <ClientInformation.h>
#include <ClientInformationList.h>
#include <ClientMethod.h>
#include <ColorAttribute.h>
#include <ColorAttributeList.h>
#include <ColorControlPoint.h>
#include <ColorControlPointList.h>
#include <ColorTableAttributes.h>
#include <CompactSILRestrictionAttributes.h>
#include <ConstructDDFAttributes.h>
#include <ContourOpAttributes.h>
#include <DBOptionsAttributes.h>
#include <DBPluginInfoAttributes.h>
#include <DatabaseAttributes.h>
#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <EngineList.h>
#include <ExportDBAttributes.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <GaussianControlPoint.h>
#include <GaussianControlPointList.h>
#include <GlobalAttributes.h>
#include <GlobalLineoutAttributes.h>
#include <HostProfileList.h>
#include <InteractorAttributes.h>
#include <InternalResampleAttributes.h>
#include <KeepAliveRPC.h>
#include <KeyframeAttributes.h>
#include <LaunchProfile.h>
#include <LightAttributes.h>
#include <LightList.h>
#include <Line.h>
#include <MachineProfile.h>
#include <MaterialAttributes.h>
#include <MeshManagementAttributes.h>
#include <MessageAttributes.h>
#include <MovieAttributes.h>
#include <NameschemeAttributes.h>
#include <NamespaceAttributes.h>
#include <PickAttributes.h>
#include <PickVarInfo.h>
#include <PlaneAttributes.h>
#include <Plot.h>
#include <PlotInfoAttributes.h>
#include <PlotList.h>
#include <PlotQueryInfo.h>
#include <PluginManagerAttributes.h>
#include <Point.h>
#include <PointAttributes.h>
#include <PrinterAttributes.h>
#include <ProcessAttributes.h>
#include <QueryAttributes.h>
#include <QueryList.h>
#include <QueryOverTimeAttributes.h>
#include <QuitRPC.h>
#include <RenderingAttributes.h>
#include <SILAttributes.h>
#include <SILMatrixAttributes.h>
#include <SILRestrictionAttributes.h>
#include <SaveWindowAttributes.h>
#include <SimilarityTransformAttributes.h>
#include <SimulationCommand.h>
#include <SphereAttributes.h>
#include <StatusAttributes.h>
#include <SurfaceFilterAttributes.h>
#include <SyncAttributes.h>
#include <TimeFormat.h>
#include <View2DAttributes.h>
#include <View3DAttributes.h>
#include <ViewAttributes.h>
#include <ViewCurveAttributes.h>
#include <VisualCueInfo.h>
#include <VisualCueList.h>
#include <WindowAttributes.h>
#include <WindowInformation.h>

// Metadata includes
#include <avtArrayMetaData.h>
#include <avtCurveMetaData.h>
#include <avtDatabaseMetaData.h>
#include <avtDefaultPlotMetaData.h>
#include <avtLabelMetaData.h>
#include <avtMatSpeciesMetaData.h>
#include <avtMaterialMetaData.h>
#include <avtMeshMetaData.h>
#include <avtScalarMetaData.h>
#include <avtSimulationCommandSpecification.h>
#include <avtSimulationInformation.h>
#include <avtSpeciesMetaData.h>
#include <avtSymmetricTensorMetaData.h>
#include <avtTensorMetaData.h>
#include <avtVectorMetaData.h>

// Include proxy classes
#include <LauncherProxy.h>
#include <EngineProxy.h>
#include <MDServerProxy.h>

// Includes for plot and operator plugins
#include <VisItInit.h>
#include <PlotPluginManager.h>
#include <OperatorPluginManager.h>
#include <PlotPluginInfo.h>
#include <OperatorPluginInfo.h>

#define MAKE_OBJECT(T) \
    { T *obj = new T; \
      attributes[obj->TypeName()] = obj;\
    }

typedef std::map<std::string, AttributeSubject *> StateObjectMap;

// ****************************************************************************
// Function: CreateCommonStateObjects
//
// Purpose: 
//   Creates the common state objects whose definitions we'll print.
//
// Arguments:
//   attributes : The map that will contain the state objects.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 10 13:52:02 PST 2007
//
// Modifications:
//   
//    Hank Childs, Wed Dec 31 13:51:24 PST 2008
//    Rename ResampleAtts as InternalResampleAtts.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.
//
// ****************************************************************************

void
CreateCommonStateObjects(StateObjectMap &attributes)
{
    // Populate the attributes that we'll test.
    MAKE_OBJECT(AnimationAttributes);
    MAKE_OBJECT(AnnotationAttributes);
    MAKE_OBJECT(AnnotationObject);
    MAKE_OBJECT(AnnotationObjectList);
    MAKE_OBJECT(AppearanceAttributes);
    MAKE_OBJECT(BoxExtents);
    MAKE_OBJECT(ClientInformation);
    MAKE_OBJECT(ClientInformationList);
    MAKE_OBJECT(ClientMethod);
    MAKE_OBJECT(ColorAttribute);
    MAKE_OBJECT(ColorAttributeList);
    MAKE_OBJECT(ColorControlPoint);
    MAKE_OBJECT(ColorControlPointList);
    MAKE_OBJECT(ColorTableAttributes);
    MAKE_OBJECT(CompactSILRestrictionAttributes);
    MAKE_OBJECT(ConstructDDFAttributes);
    MAKE_OBJECT(ContourOpAttributes);
    MAKE_OBJECT(DBOptionsAttributes);
    MAKE_OBJECT(DBPluginInfoAttributes);
    MAKE_OBJECT(DatabaseAttributes);
    MAKE_OBJECT(DatabaseCorrelation);
    MAKE_OBJECT(DatabaseCorrelationList);
    MAKE_OBJECT(EngineList);
    MAKE_OBJECT(ExportDBAttributes);
    MAKE_OBJECT(Expression);
    MAKE_OBJECT(ExpressionList);
    MAKE_OBJECT(GaussianControlPoint);
    MAKE_OBJECT(GaussianControlPointList);
    MAKE_OBJECT(GlobalAttributes);
    MAKE_OBJECT(GlobalLineoutAttributes);
    MAKE_OBJECT(HostProfileList);
    MAKE_OBJECT(InteractorAttributes);
    MAKE_OBJECT(InternalResampleAttributes);
    MAKE_OBJECT(KeepAliveRPC);
    MAKE_OBJECT(KeyframeAttributes);
    MAKE_OBJECT(LaunchProfile);
    MAKE_OBJECT(LightAttributes);
    MAKE_OBJECT(LightList);
    MAKE_OBJECT(Line);
    MAKE_OBJECT(MachineProfile);
    MAKE_OBJECT(MaterialAttributes);
    MAKE_OBJECT(MeshManagementAttributes);
    MAKE_OBJECT(MessageAttributes);
    MAKE_OBJECT(MovieAttributes);
    MAKE_OBJECT(NameschemeAttributes);
    MAKE_OBJECT(NamespaceAttributes);
    MAKE_OBJECT(PickAttributes);
    MAKE_OBJECT(PickVarInfo);
    MAKE_OBJECT(PlaneAttributes);
    MAKE_OBJECT(Plot);
    MAKE_OBJECT(PlotInfoAttributes);
    MAKE_OBJECT(PlotList);
    MAKE_OBJECT(PlotQueryInfo);
    MAKE_OBJECT(PluginManagerAttributes);
    MAKE_OBJECT(Point);
    MAKE_OBJECT(PointAttributes);
    MAKE_OBJECT(PrinterAttributes);
    MAKE_OBJECT(ProcessAttributes);
    MAKE_OBJECT(QueryAttributes);
    MAKE_OBJECT(QueryList);
    MAKE_OBJECT(QueryOverTimeAttributes);
    MAKE_OBJECT(QuitRPC);
    MAKE_OBJECT(RenderingAttributes);
    MAKE_OBJECT(SILAttributes);
    MAKE_OBJECT(SILMatrixAttributes);
    MAKE_OBJECT(SILRestrictionAttributes);
    MAKE_OBJECT(SaveWindowAttributes);
    MAKE_OBJECT(SimilarityTransformAttributes);
    MAKE_OBJECT(SimulationCommand);
    MAKE_OBJECT(SphereAttributes);
    MAKE_OBJECT(StatusAttributes);
    MAKE_OBJECT(SurfaceFilterAttributes);
    MAKE_OBJECT(SyncAttributes);
    MAKE_OBJECT(TimeFormat);
    MAKE_OBJECT(View2DAttributes);
    MAKE_OBJECT(View3DAttributes);
    MAKE_OBJECT(ViewAttributes);
    MAKE_OBJECT(ViewCurveAttributes);
    MAKE_OBJECT(VisualCueInfo);
    MAKE_OBJECT(VisualCueList);
    MAKE_OBJECT(WindowAttributes);
    MAKE_OBJECT(WindowInformation);
}

// ****************************************************************************
// Function: CreateMetaDataObjects
//
// Purpose: 
//   Creates the metadata state objects whose definitions we'll print.
//
// Arguments:
//   attributes : The map that will contain the state objects.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 10 13:52:02 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
CreateMetaDataObjects(StateObjectMap &attributes)
{
    MAKE_OBJECT(avtArrayMetaData);
    MAKE_OBJECT(avtCurveMetaData);
    MAKE_OBJECT(avtDatabaseMetaData);
    MAKE_OBJECT(avtDefaultPlotMetaData);
    MAKE_OBJECT(avtLabelMetaData);
    MAKE_OBJECT(avtMatSpeciesMetaData);
    MAKE_OBJECT(avtMaterialMetaData);
    MAKE_OBJECT(avtMeshMetaData);
    MAKE_OBJECT(avtScalarMetaData);
    MAKE_OBJECT(avtSimulationCommandSpecification);
    MAKE_OBJECT(avtSimulationInformation);
    MAKE_OBJECT(avtSpeciesMetaData);
    MAKE_OBJECT(avtSymmetricTensorMetaData);
    MAKE_OBJECT(avtTensorMetaData);
    MAKE_OBJECT(avtVectorMetaData);
}

// ****************************************************************************
// Function: CreatePlotStateObjects
//
// Purpose: 
//   Creates the plot state objects whose definitions we'll print.
//
// Arguments:
//   plotPlugins : The plot plugin manager.
//   attributes  : The map that will contain the state objects.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 10 13:52:02 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Jun 25 09:55:13 PDT 2008
//   Pass in the plugin manager.
//
// ****************************************************************************

void
CreatePlotStateObjects(PlotPluginManager &plotPlugins, StateObjectMap &attributes)
{
    // Enable all plugins
    for(int i = 0; i < plotPlugins.GetNAllPlugins(); ++i)
    {
        // Get a pointer to the GUI portion of the plot plugin information.
        plotPlugins.EnablePlugin(plotPlugins.GetAllID(i));
    }

    // Load the plugins
    plotPlugins.LoadPluginsNow();

    // Create a state object for each plugin.
    for(int i = 0; i < plotPlugins.GetNAllPlugins(); ++i)
    {
        // Get a pointer to the GUI portion of the plot plugin information.
        EnginePlotPluginInfo *info = plotPlugins.
            GetEnginePluginInfo(plotPlugins.GetAllID(i));
        if(info != 0)
        {
            AttributeSubject *atts = info->AllocAttributes();
            attributes[atts->TypeName()] = atts;
        }
    }
}

// ****************************************************************************
// Function: CreateOperatorStateObjects
//
// Purpose: 
//   Creates the operator state objects whose definitions we'll print.
//
// Arguments:
//   operatorPlugins : The operator plugin manager.
//   attributes      : The map that will contain the state objects.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 10 13:52:02 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Jun 25 09:55:13 PDT 2008
//   Pass in the plugin manager.
//   
// ****************************************************************************

void
CreateOperatorStateObjects(OperatorPluginManager &operatorPlugins, 
    StateObjectMap &attributes)
{
    // Enable all plugins
    for(int i = 0; i < operatorPlugins.GetNAllPlugins(); ++i)
    {
        // Get a pointer to the GUI portion of the Operator plugin information.
        operatorPlugins.EnablePlugin(operatorPlugins.GetAllID(i));
    }

    // Load the plugins
    operatorPlugins.LoadPluginsNow();

    // Create a state object for each plugin.
    for(int i = 0; i < operatorPlugins.GetNAllPlugins(); ++i)
    {
        // Get a pointer to the GUI portion of the Operator plugin information.
        EngineOperatorPluginInfo *info = operatorPlugins.
            GetEnginePluginInfo(operatorPlugins.GetAllID(i));
        if(info != 0)
        {
            AttributeSubject *atts = info->AllocAttributes();
            attributes[atts->TypeName()] = atts;
        }
    }
}

// ****************************************************************************
// Function: WriteObjectDefinitions
//
// Purpose: 
//   Prints out the state object fields for all of the state objects in the map.
//
// Arguments:
//   f          : The output file.
//   attributes:  The map of state objects to print.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 10 13:51:09 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
WriteObjectDefinitions(FILE *f, StateObjectMap &attributes)
{
    std::string space(" ");
    for(std::map<std::string, AttributeSubject *>::iterator pos = 
        attributes.begin(); pos != attributes.end(); ++pos)
    {
        fprintf(f, "%s\n", pos->second->TypeName().c_str());
        stringVector tNames;
        int maxNameLen = 0;
        for(int attr = 0; attr < pos->second->NumAttributes(); ++attr)
        {
            std::string typeName(pos->second->GetFieldTypeName(attr));

            // If we have one of the following field types then we should
            // use the base class implementation of GetFieldTypeName since
            // it can do better in some instances.
            AttributeGroup::FieldType f = pos->second->GetFieldType(attr);
            bool baseClassImpl = (
                f == AttributeGroup::FieldType_intArray ||
                f == AttributeGroup::FieldType_floatArray ||
                f == AttributeGroup::FieldType_doubleArray ||
                f == AttributeGroup::FieldType_ucharArray ||
                typeName.find("UNKNOWN") != std::string::npos ||
                typeName == "att" ||
                typeName == "attVector");

            // If we get a junk answer then call the base class impl.
            if(baseClassImpl)
                typeName = pos->second->AttributeGroup::GetFieldTypeName(attr);

            tNames.push_back(typeName);
            if(typeName.size() > maxNameLen)
                maxNameLen = typeName.size();
        }

        for(int attr = 0; attr < pos->second->NumAttributes(); ++attr)
        {
            if(pos->second->GetFieldName(attr).find("<UNKNOWN") == std::string::npos)
            {
                while(tNames[attr].size() < maxNameLen)
                tNames[attr] += space;
           
                fprintf(f, "\t%s %s\n", tNames[attr].c_str(),
                    pos->second->GetFieldName(attr).c_str());
            }
            else
            {
                fprintf(f, "\t%s\n", tNames[attr].c_str());
            }
        }
        fprintf(f, "\n");
    }
}

// ****************************************************************************
// Function: DeleteStateObjects
//
// Purpose: 
//   Deletes the state objects in the passed in map.
//
// Arguments:
//   attributes : The map whose objects will be deleted.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 10 13:50:38 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
DeleteStateObjects(StateObjectMap &attributes)
{
    for(std::map<std::string, AttributeSubject *>::iterator pos = 
        attributes.begin(); pos != attributes.end(); ++pos)
    {
        delete pos->second;
    }
    attributes.clear();
}

// ****************************************************************************
// Function: ExtractProxyBaseInterface
//
// Purpose: 
//   Extracts all of the state objects from a proxy class and adds them to
//   the state object map.
//
// Arguments:
//   proxy : The proxy whose state objects will be extracted.
//   objs  : A map that will contain the objects.
//
// Returns:    The map will contain pointers to the state objects.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 10 13:49:24 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
ExtractProxyBaseInterface(RemoteProxyBase *proxy, StateObjectMap &objs)
{
    char key[100];
    for(int i = 0; i < proxy->GetXfer().GetNumSubjects(); ++i)
    {
        sprintf(key, "%05d", i);
        objs[std::string(key)] = (AttributeSubject *)
            proxy->GetXfer().GetSubject(i);
    }
}

// ****************************************************************************
// Function: PrintHeader
//
// Purpose: 
//   Prints a header to the output.
//
// Arguments:
//   f      : The output file.
//   header : The header to print.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 10 13:48:17 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
PrintHeader(FILE *f, const char *header)
{
    fprintf(f, "========================================================================\n");
    fprintf(f, "%s\n", header);
    fprintf(f, "========================================================================\n");
}

// ****************************************************************************
// Function: main
//
// Purpose: 
//   This is the program's entry point.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 10 13:48:17 PST 2007
//
// Modifications:
//    Mark C. Miller, Thu Apr  3 14:36:48 PDT 2008
//    Moved setting of component name to before Initialize
//
//    Brad Whitlock, Wed Jun 25 09:52:50 PDT 2008
//    Changed how plugin managers are accessed.
//
// ****************************************************************************

int
main(int argc, char *argv[])
{
    StateObjectMap commonState, metadata, engine, launcher, plots, operators;
    FILE *f = stdout;

    // Initialize
    VisItInit::SetComponentName("visitprotocol");
    VisItInit::Initialize(argc, argv, 0, 1, true, true);

    // Initialize the plugin managers and load the info plugins.
    PlotPluginManager     plotPlugins;
    OperatorPluginManager operatorPlugins;
    plotPlugins.Initialize(PlotPluginManager::Engine, false);
    operatorPlugins.Initialize(OperatorPluginManager::Engine, false);

    // Print out header and program description.
    PrintHeader(f, "VisIt protocol report");
    fprintf(f, "Command line: \n");
    for(int i = 0; i < argc; ++i)
        fprintf(f, "\t%s\n", argv[i]);
    fprintf(f, "\n");
    fprintf(f, "VisIt contains state objects that are used to communicate data from\n"
               "one VisIt component to another. These state objects are described by\n"
               "XML files and the XML files are used to automatically generate source\n"
               "code representing the state object. This tool prints out a description\n"
               "of all of the state objects in use for a version of VisIt. The output\n"
               "of this program can be used to determine whether 2 versions of VisIt\n"
               "have the same state object definitions, thus having the same binary\n"
               "protocol for communication. You can redirect the output of this program\n"
               "to a file on the 2 systems being compared and then you can use a text\n"
               "differencing program to look for changes that make the protocols\n"
               "incompatible.\n\n");


    // Test the attributes in common/state.
    PrintHeader(f, "Attributes in common/state");
    CreateCommonStateObjects(commonState);
    WriteObjectDefinitions(f, commonState);
    DeleteStateObjects(commonState);

    PrintHeader(f, "Metadata objects");
    CreateMetaDataObjects(metadata);
    WriteObjectDefinitions(f, metadata);
    DeleteStateObjects(metadata);

    PrintHeader(f, "Launcher RPC interface");
    LauncherProxy L;
    ExtractProxyBaseInterface(&L, launcher);
    WriteObjectDefinitions(f, launcher);

    PrintHeader(f, "Metadata server RPC interface");
    MDServerProxy mds;
    ExtractProxyBaseInterface(&mds, metadata);
    WriteObjectDefinitions(f, metadata);

    PrintHeader(f, "Compute engine RPC interface");
    EngineProxy e;
    ExtractProxyBaseInterface(&e, engine);
    WriteObjectDefinitions(f, engine);

    PrintHeader(f, "Plot plugin state objects");
    CreatePlotStateObjects(plotPlugins, plots);
    WriteObjectDefinitions(f, plots);
    DeleteStateObjects(plots);

    PrintHeader(f, "Operator plugin state objects");
    CreateOperatorStateObjects(operatorPlugins, operators);
    WriteObjectDefinitions(f, operators);
    DeleteStateObjects(operators);

    // Prepare for exit.
    VisItInit::Finalize();

    return 0;
}
