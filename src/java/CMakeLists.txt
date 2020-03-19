# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#   Hank Childs, Thu Aug 19 12:26:29 PDT 2010
#   Rename DDF to DataBinning.
#
#   Brad Whitlock, Tue Jan 11 10:22:33 PST 2011
#   Add DualClients.java
#
#   Brad Whitlock, Fri Jan 28 11:13:40 PST 2011
#   Add EventLoop object.
#
#   Brad Whitlock, Mon Jun 20 15:44:41 PDT 2011
#   Add SelectionVariableSummary and SelectionSummary.
#
#   Brad Whitlock, Thu Feb  2 10:48:50 PST 2012
#   Adding in initial Variant, MapNode classes to rehab the Java client.
#
#   Kathleen Biagas, Mon Jul  8 12:11:38 PDT 2013
#   Added ThresholdOpAttributes.java.
#
#   Kathleen Biagas, Wed Nov  6 17:30:46 PST 2013
#   Reorganized so out-of-source builds will work.  Added creation of docs,
#   'pack' target now will create examples and docs (if not already built), 
#   as well as creating the jar file and the jvisit tarball for distribution.
#
#   Kathleen Biagas, Fri Mar 31 12:56:33 PDT 2017
#   Added TryThreshold and TryPointGlyphing examples.
#
#   Cyrus Harrison, Mon Dec  9 12:07:55 PST 2019
#   Add code gen targets, change logic to guard java interface build.
#
#****************************************************************************

PROJECT(JavaClient)


#########################################################
# BEGIN Logic for XML Code Gen Targets
#########################################################

#####################
# Metadata Objects
#####################
SET(JAVA_GEN_METADATA_OBJECTS
    avtArrayMetaData
    avtBaseVarMetaData
    avtCurveMetaData
    avtDatabaseMetaData
    avtDefaultPlotMetaData
    avtLabelMetaData
    avtMatSpeciesMetaData
    avtMaterialMetaData
    avtMeshMetaData
    avtScalarMetaData
    avtSimulationCommandSpecification
    avtSimulationInformation
    avtSpeciesMetaData
    avtSubsetsMetaData
    avtSymmetricTensorMetaData
    avtTensorMetaData
    avtVarMetaData
    avtVectorMetaData
    )

#####################
# State Objects
#####################
SET(JAVA_GEN_STATE_OBJECTS
    AnimationAttributes 
    AnnotationAttributes 
    AnnotationObject 
    AnnotationObjectList 
    AppearanceAttributes 
    Axes2D 
    Axes3D 
    AxesArray 
    AxisAttributes 
    AxisLabels 
    AxisTickMarks 
    AxisTitles 
    BoxExtents 
    CinemaAttributes
    ClientInformation 
    ClientInformationList 
    ClientMethod 
    ColorAttribute 
    ColorAttributeList 
    ColorControlPoint 
    ColorControlPointList 
    ColorTableAttributes 
    ConstructDataBinningAttributes 
    DBOptionsAttributes 
    DBPluginInfoAttributes 
    DatabaseCorrelation 
    DatabaseCorrelationList 
    EngineList 
    EngineProperties 
    ExportDBAttributes 
    Expression 
    ExpressionList 
    FileOpenOptions 
    FontAttributes 
    GaussianControlPoint 
    GaussianControlPointList 
    GlobalAttributes 
    GlobalLineoutAttributes 
    HostProfileList 
    InteractorAttributes 
    KeyframeAttributes 
    LaunchProfile
    LightAttributes 
    LightList 
    MachineProfile
    MapNode
    MapNodePair
    MaterialAttributes 
    MeshManagementAttributes 
    MessageAttributes
    MovieAttributes 
    NamespaceAttributes 
    NameschemeAttributes
    PickAttributes 
    PickVarInfo 
    Plot 
    PlotInfoAttributes 
    PlotList
    PluginManagerAttributes 
    PostponedAction 
    PrinterAttributes 
    ProcessAttributes 
    QueryAttributes 
    QueryList 
    QueryOverTimeAttributes 
    RenderingAttributes 
    SILArrayAttributes 
    SILAttributes 
    SILMatrixAttributes 
    SILRestrictionAttributes 
    SimulationUIValues
    SaveSubWindowAttributes
    SaveSubWindowsAttributes
    SaveWindowAttributes 
    SeedMeAttributes
    SelectionList
    SelectionProperties
    SelectionSummary
    SelectionVariableSummary
    StatusAttributes 
    SyncAttributes 
    ThresholdOpAttributes 
    Variant
    View2DAttributes 
    View3DAttributes 
    ViewAxisArrayAttributes 
    ViewerClientAttributes
    ViewerClientInformation
    ViewerClientInformationElement
    ViewCurveAttributes 
    WindowInformation)

#####################
# RPC Objects
#####################
SET(JAVA_GEN_RPC_OBJECTS ViewerRPC)


#########################################################
# Add our code gen targets
#########################################################
# Metadata Objects
foreach(obj_name ${JAVA_GEN_METADATA_OBJECTS})
    ADD_JAVA_GEN_TARGET(${obj_name}
                        ${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
                        ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

# State Objects
foreach(obj_name ${JAVA_GEN_STATE_OBJECTS})
    ADD_JAVA_GEN_TARGET(${obj_name}
                        ${VISIT_SOURCE_DIR}/common/state/
                        ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

# RPC Objects
foreach(obj_name ${JAVA_GEN_RPC_OBJECTS})
    ADD_JAVA_GEN_TARGET(${obj_name}
                        ${VISIT_SOURCE_DIR}/viewer/rpcstate/
                        ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()
#########################################################
# END Logic for XML Code Gen Targets
#########################################################


############################################
# - Begin Java Interface CMake logic
############################################
# (only added if java support is enabled)
############################################
IF(VISIT_JAVA)
############################################ 

enable_language(Java)

SET(METADATA_SOURCES
avtArrayMetaData.java
avtBaseVarMetaData.java
avtCurveMetaData.java
avtDatabaseMetaData.java
avtDefaultPlotMetaData.java
avtLabelMetaData.java
avtMatSpeciesMetaData.java
avtMaterialMetaData.java
avtMeshMetaData.java
avtScalarMetaData.java
avtSimulationCommandSpecification.java
avtSimulationInformation.java
avtSpeciesMetaData.java
avtSubsetsMetaData.java
avtSymmetricTensorMetaData.java
avtTensorMetaData.java
avtVarMetaData.java
avtVectorMetaData.java
)

SET(PACKAGE_STATE_SOURCES
AnimationAttributes.java 
AnnotationAttributes.java 
AnnotationObject.java 
AnnotationObjectList.java 
AppearanceAttributes.java 
Axes2D.java 
Axes3D.java 
AxesArray.java 
AxisAttributes.java 
AxisLabels.java 
AxisTickMarks.java 
AxisTitles.java 
BoxExtents.java 
CinemaAttributes.java
ClientInformation.java 
ClientInformationList.java 
ClientMethod.java 
ColorAttribute.java 
ColorAttributeList.java 
ColorControlPoint.java 
ColorControlPointList.java 
ColorTableAttributes.java 
ConstructDataBinningAttributes.java 
DBOptionsAttributes.java 
DBPluginInfoAttributes.java 
DatabaseCorrelation.java 
DatabaseCorrelationList.java 
EngineList.java 
EngineProperties.java 
ExportDBAttributes.java 
Expression.java 
ExpressionList.java 
FileOpenOptions.java 
FontAttributes.java 
GaussianControlPoint.java 
GaussianControlPointList.java 
GlobalAttributes.java 
GlobalLineoutAttributes.java 
HostProfileList.java 
InteractorAttributes.java 
KeyframeAttributes.java 
LaunchProfile.java
LightAttributes.java 
LightList.java 
MachineProfile.java
MapNode.java
MapNodePair.java
MaterialAttributes.java 
MeshManagementAttributes.java 
MessageAttributes.java
MovieAttributes.java 
NamespaceAttributes.java 
NameschemeAttributes.java
PickAttributes.java 
PickVarInfo.java 
Plot.java 
PlotInfoAttributes.java 
PlotList.java
PluginManagerAttributes.java 
PostponedAction.java 
PrinterAttributes.java 
ProcessAttributes.java 
QueryAttributes.java 
QueryList.java 
QueryOverTimeAttributes.java 
RenderingAttributes.java 
SILArrayAttributes.java 
SILAttributes.java 
SILMatrixAttributes.java 
SILRestrictionAttributes.java 
SimulationUIValues.java
SaveSubWindowAttributes.java
SaveSubWindowsAttributes.java
SaveWindowAttributes.java 
SeedMeAttributes.java
SelectionList.java
SelectionProperties.java
SelectionSummary.java
SelectionVariableSummary.java
StatusAttributes.java 
SyncAttributes.java 
ThresholdOpAttributes.java 
Variant.java
View2DAttributes.java 
View3DAttributes.java 
ViewAxisArrayAttributes.java 
ViewerClientAttributes.java
ViewerClientInformation.java
ViewerClientInformationElement.java
ViewCurveAttributes.java 
ViewerRPC.java 
WindowInformation.java
)

SET(PACKAGE_SOURCES
SimpleObserver.java 
CouldNotConnectException.java 
EventLoop.java
LostConnectionException.java 
IncompatibleSecurityTokenException.java 
IncompatibleVersionException.java 
CommunicationHeader.java 
CommunicationBuffer.java 
AttributeSubject.java 
Connection.java 
MessageObserver.java 
Plugin.java 
PluginManager.java 
RemoteProcess.java 
ViewerState.java 
ProxyInterface.java
ViewerMethods.java 
SyncNotifier.java
Xfer.java
Yielder.java
ViewerProxy.java 
)

# Make the VersionWriter executable to create Version.java.
ADD_EXECUTABLE(VersionWriter VersionWriter.C)

# Add a custom build step that runs VersionWriter after it is built and then 
# build all the Java sources. This has a weakness in that Java sources won't be
# recompiled when they change unless we "make clean" in this directory first but
# it's not all that different than before and it's WAY easier than keeping the
# per-file source dependencies.
ADD_CUSTOM_COMMAND(TARGET VersionWriter POST_BUILD 
    COMMAND ${VISIT_EXECUTABLE_DIR}/VersionWriter ${VISIT_SOURCE_DIR}/VERSION ${JavaClient_BINARY_DIR}/Version.java
)

# custom command/target that ensures java class files are comiled
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/java_client_out.txt
    COMMAND ${Java_JAVAC_EXECUTABLE} 
        ${VISIT_Java_FLAGS} 
        -sourcepath ${JavaClient_BINARY_DIR}
        -d ${JavaClient_BINARY_DIR}
        ${JavaClient_BINARY_DIR}/Version.java ${PACKAGE_SOURCES} ${PACKAGE_STATE_SOURCES} ${METADATA_SOURCES}
    COMMAND ${CMAKE_COMMAND} -E  copy ${JavaClient_BINARY_DIR}/Version.java ${JavaClient_BINARY_DIR}/src
    COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/java_client_out.txt
    DEPENDS ${PACKAGE_SOURCES} ${PACKAGE_STATE_SOURCES} ${METADATA_SOURCES} ${JavaClient_BINARY_DIR}/Version.java
    WORKING_DIRECTORY ${JavaClient_SOURCE_DIR}
    COMMENT "Compiling Java sources"
)

ADD_CUSTOM_TARGET(JavaClient ALL DEPENDS VersionWriter
${CMAKE_CURRENT_BINARY_DIR}/java_client_out.txt ${JavaClient_BINARY_DIR}/Version.java)

# Add a target that compiles all of the main programs.
SET(MAIN_SOURCES
DualClients.java
GetMetaData.java 
MakeHostProfile.java 
NIFGUI.java 
OpenGUI.java 
PlotAtts.java 
PlotTypes.java
RunViewer.java 
TryAnnotations.java 
TryLighting.java 
TryPointGlyphing.java 
TryQuery.java 
TryThreshold.java 
)

# Some preliminary work to aid in creating the tarball
FILE(MAKE_DIRECTORY 
    ${JavaClient_BINARY_DIR}/src/plots 
    ${JavaClient_BINARY_DIR}/src/operators
    ${JavaClient_BINARY_DIR}/llnl/visit/plots
    ${JavaClient_BINARY_DIR}/llnl/visit/operators
    ${JavaClient_BINARY_DIR}/images
)

FILE(COPY ${PACKAGE_SOURCES} ${PACKAGE_STATE_SOURCES} ${METADATA_SOURCES} ${MAIN_SOURCES} 
    DESTINATION ${JavaClient_BINARY_DIR}/src)
FILE(COPY ${JavaClient_SOURCE_DIR}/images/noimage.jpeg
    DESTINATION ${JavaClient_BINARY_DIR}/images)

# Custom command/target pair for building the examples, but not as part of the
# normal build.  'make main' will build the examples
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/java_examples_out.txt 
    COMMAND ${Java_JAVAC_EXECUTABLE} 
        ${VISIT_Java_FLAGS} 
        -classpath  ${JavaClient_BINARY_DIR}
        -sourcepath ${JavaClient_BINARY_DIR}
        -d ${JavaClient_BINARY_DIR}
        ${MAIN_SOURCES}
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/java_examples_out.txt
    DEPENDS ${MAIN_SOURCES}
    WORKING_DIRECTORY ${JavaClient_SOURCE_DIR}
    COMMENT "Compiling Java examples"
)

ADD_CUSTOM_TARGET(main DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/java_examples_out.txt) 


# Custom target for building the docs
ADD_CUSTOM_TARGET(visit_java_docs 
    COMMAND ${CMAKE_COMMAND} -E make_directory ${JavaClient_BINARY_DIR}/docs
    COMMAND ${Java_JAVADOC_EXECUTABLE} -d docs ${JavaClient_SOURCE_DIR}/*.java
    COMMAND ${CMAKE_COMMAND} -E copy ${JavaClient_SOURCE_DIR}/stylesheet.css ${JavaClient_BINARY_DIR}/docs
    WORKING_DIRECTORY ${JavaClient_BINARY_DIR}
)

# Custom target for creating the jar
ADD_CUSTOM_TARGET(pack ${Java_JAR_EXECUTABLE} cvf visit.jar llnl 
    DEPENDS JavaClient main visit_java_docs
    WORKING_DIRECTORY ${JavaClient_BINARY_DIR}
    COMMENT "Creating visit.jar"
)

# Add a command that will create the tarball after the jar file is finished.
ADD_CUSTOM_COMMAND(TARGET pack POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E tar "zcf" jvisit${VISIT_VERSION}.tar.gz visit.jar src docs *.class images
    WORKING_DIRECTORY ${JavaClient_BINARY_DIR}
    COMMENT "Creating jvisit${VISIT_VERSION}.tar.gz"
)


############################################
# - End Java Interface CMake logic
############################################
ENDIF()
############################################

