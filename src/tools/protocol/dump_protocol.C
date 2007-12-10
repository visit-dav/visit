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
#include <ExtentsAttributes.h>
#include <GaussianControlPoint.h>
#include <GaussianControlPointList.h>
#include <GlobalAttributes.h>
#include <GlobalLineoutAttributes.h>
#include <HostProfile.h>
#include <HostProfileList.h>
#include <InteractorAttributes.h>
#include <KeepAliveRPC.h>
#include <KeyframeAttributes.h>
#include <LightAttributes.h>
#include <LightList.h>
#include <Line.h>
#include <MaterialAttributes.h>
#include <MeshManagementAttributes.h>
#include <MessageAttributes.h>
#include <MovieAttributes.h>
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
#include <ResampleAttributes.h>
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
#include <avtSILCollectionMetaData.h>
#include <avtSILMetaData.h>
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

// Include plot attributes that we regularly build
#include <../../plots/Boundary/BoundaryAttributes.h>
#include <../../plots/Contour/ContourAttributes.h>
#include <../../plots/Curve/CurveAttributes.h>
#include <../../plots/FilledBoundary/FilledBoundaryAttributes.h>
#include <../../plots/Histogram/HistogramAttributes.h>
#include <../../plots/Label/LabelAttributes.h>
#include <../../plots/Mesh/MeshAttributes.h>
#include <../../plots/Molecule/MoleculeAttributes.h>
#include <../../plots/ParallelAxis/ParallelAxisAttributes.h>
#include <../../plots/Pseudocolor/PseudocolorAttributes.h>
#include <../../plots/Scatter/ScatterAttributes.h>
#include <../../plots/Spreadsheet/SpreadsheetAttributes.h>
#include <../../plots/Streamline/StreamlineAttributes.h>
#include <../../plots/Subset/SubsetAttributes.h>
#include <../../plots/Surface/SurfaceAttributes.h>
#include <../../plots/Tensor/TensorAttributes.h>
#include <../../plots/Truecolor/TruecolorAttributes.h>
#include <../../plots/Vector/VectorAttributes.h>
#include <../../plots/Volume/VolumeAttributes.h>

// Include the operator attributes that we are REQUIRED to build
#include <../../operators/BoundaryOp/BoundaryOpAttributes.h>
#include <../../operators/Box/BoxAttributes.h>
#include <../../operators/Clip/ClipAttributes.h>
#include <../../operators/Cone/ConeAttributes.h>
#include <../../operators/CoordSwap/CoordSwapAttributes.h>
#include <../../operators/CracksClipper/CracksClipperAttributes.h>
#include <../../operators/CreateBonds/CreateBondsAttributes.h>
#include <../../operators/Cylinder/CylinderAttributes.h>
#include <../../operators/Decimate/DecimateAttributes.h>
#include <../../operators/DeferExpression/DeferExpressionAttributes.h>
#include <../../operators/Displace/DisplaceAttributes.h>
#include <../../operators/Elevate/ElevateAttributes.h>
#include <../../operators/ExternalSurface/ExternalSurfaceAttributes.h>
#include <../../operators/IndexSelect/IndexSelectAttributes.h>
#include <../../operators/InverseGhostZone/InverseGhostZoneAttributes.h>
#include <../../operators/Isosurface/IsosurfaceAttributes.h>
#include <../../operators/Isovolume/IsovolumeAttributes.h>
#include <../../operators/Lineout/LineoutAttributes.h>
#include <../../operators/Merge/MergeOperatorAttributes.h>
#include <../../operators/OnionPeel/OnionPeelAttributes.h>
#include <../../operators/PDF/PDFAttributes.h>
#include <../../operators/Project/ProjectAttributes.h>
#include <../../operators/Reflect/ReflectAttributes.h>
#include <../../operators/Replicate/ReplicateAttributes.h>
#include <../../operators/Resample/ResamplePluginAttributes.h>
#include <../../operators/Revolve/RevolveAttributes.h>
#include <../../operators/Slice/SliceAttributes.h>
#include <../../operators/Smooth/SmoothOperatorAttributes.h>
#include <../../operators/SphereSlice/SphereSliceAttributes.h>
#include <../../operators/ThreeSlice/ThreeSliceAttributes.h>
#include <../../operators/Threshold/ThresholdAttributes.h>
#include <../../operators/Transform/TransformAttributes.h>
#include <../../operators/Tube/TubeAttributes.h>
#include <../../operators/ZoneDump/ZoneDumpAttributes.h>


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
    MAKE_OBJECT(ExtentsAttributes);
    MAKE_OBJECT(GaussianControlPoint);
    MAKE_OBJECT(GaussianControlPointList);
    MAKE_OBJECT(GlobalAttributes);
    MAKE_OBJECT(GlobalLineoutAttributes);
    MAKE_OBJECT(HostProfile);
    MAKE_OBJECT(HostProfileList);
    MAKE_OBJECT(InteractorAttributes);
    MAKE_OBJECT(KeepAliveRPC);
    MAKE_OBJECT(KeyframeAttributes);
    MAKE_OBJECT(LightAttributes);
    MAKE_OBJECT(LightList);
    MAKE_OBJECT(Line);
    MAKE_OBJECT(MaterialAttributes);
    MAKE_OBJECT(MeshManagementAttributes);
    MAKE_OBJECT(MessageAttributes);
    MAKE_OBJECT(MovieAttributes);
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
    MAKE_OBJECT(ResampleAttributes);
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
    MAKE_OBJECT(avtSILCollectionMetaData);
    MAKE_OBJECT(avtSILMetaData);
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
CreatePlotStateObjects(StateObjectMap &attributes)
{
    MAKE_OBJECT(BoundaryAttributes);
    MAKE_OBJECT(ContourAttributes);
    MAKE_OBJECT(CurveAttributes);
    MAKE_OBJECT(FilledBoundaryAttributes);
    MAKE_OBJECT(HistogramAttributes);
    //MAKE_OBJECT(KerbelAttributes);
    MAKE_OBJECT(LabelAttributes);
    MAKE_OBJECT(MeshAttributes);
    MAKE_OBJECT(MoleculeAttributes);
    MAKE_OBJECT(ParallelAxisAttributes);
    MAKE_OBJECT(PseudocolorAttributes);
    MAKE_OBJECT(ScatterAttributes);
    MAKE_OBJECT(SpreadsheetAttributes);
    MAKE_OBJECT(StreamlineAttributes);
    MAKE_OBJECT(SubsetAttributes);
    MAKE_OBJECT(SurfaceAttributes);
    MAKE_OBJECT(TensorAttributes);
    //MAKE_OBJECT(TopologyAttributes);
    MAKE_OBJECT(TruecolorAttributes);
    MAKE_OBJECT(VectorAttributes);
    MAKE_OBJECT(VolumeAttributes);
}

// ****************************************************************************
// Function: CreateOperatorStateObjects
//
// Purpose: 
//   Creates the operator state objects whose definitions we'll print.
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
CreateOperatorStateObjects(StateObjectMap &attributes)
{
    MAKE_OBJECT(BoundaryOpAttributes);
    MAKE_OBJECT(BoxAttributes);
    MAKE_OBJECT(ClipAttributes);
    MAKE_OBJECT(ConeAttributes);
    MAKE_OBJECT(CoordSwapAttributes);
    MAKE_OBJECT(CracksClipperAttributes);
    MAKE_OBJECT(CreateBondsAttributes);
    MAKE_OBJECT(CylinderAttributes);
    MAKE_OBJECT(DecimateAttributes);
    MAKE_OBJECT(DeferExpressionAttributes);
    MAKE_OBJECT(DisplaceAttributes);
    MAKE_OBJECT(ElevateAttributes);
    MAKE_OBJECT(ExternalSurfaceAttributes);
    MAKE_OBJECT(IndexSelectAttributes);
    MAKE_OBJECT(InverseGhostZoneAttributes);
    MAKE_OBJECT(IsosurfaceAttributes);
    MAKE_OBJECT(IsovolumeAttributes);
    MAKE_OBJECT(LineoutAttributes);
    MAKE_OBJECT(MergeOperatorAttributes);
    MAKE_OBJECT(OnionPeelAttributes);
    MAKE_OBJECT(PDFAttributes);
    MAKE_OBJECT(ProjectAttributes);
    MAKE_OBJECT(ReflectAttributes);
    MAKE_OBJECT(ReplicateAttributes);
    MAKE_OBJECT(ResamplePluginAttributes);
    MAKE_OBJECT(RevolveAttributes);
    MAKE_OBJECT(SliceAttributes);
    MAKE_OBJECT(SmoothOperatorAttributes);
    MAKE_OBJECT(SphereSliceAttributes);
    MAKE_OBJECT(ThreeSliceAttributes);
    MAKE_OBJECT(ThresholdAttributes);
    MAKE_OBJECT(TransformAttributes);
    MAKE_OBJECT(TubeAttributes);
    MAKE_OBJECT(ZoneDumpAttributes);
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
//   
// ****************************************************************************

int
main(int argc, char *argv[])
{
    StateObjectMap commonState, metadata, engine, launcher, plots, operators;
    FILE *f = stdout;

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
    CreatePlotStateObjects(plots);
    WriteObjectDefinitions(f, plots);
    DeleteStateObjects(plots);

    PrintHeader(f, "Operator plugin state objects");
    CreateOperatorStateObjects(operators);
    WriteObjectDefinitions(f, operators);
    DeleteStateObjects(operators);

    return 0;
}
