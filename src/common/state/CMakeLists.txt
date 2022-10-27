# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Note: This was moved from common/CMakeLists.txt and modified to use more
#       modern CMake syntax.
#
# Modifications:
#
#****************************************************************************


# Add XML Code Gen Targets

# Note: These names are in alphabetical order


####################
# Common State Objects that we can't Code-Gen, or have customizations:
#####################
#
# ExpressionList
#

foreach(obj_name
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
    AxisRestrictionAttributes
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
    CompactSILRestrictionAttributes
    ConstructDataBinningAttributes
    ContourOpAttributes
    DBOptionsAttributes
    DBPluginInfoAttributes
    DatabaseAttributes
    DatabaseCorrelation
    DatabaseCorrelationList
    EngineList
    EngineProperties
    ExportDBAttributes
    Expression
    FileOpenOptions
    FontAttributes
    GaussianControlPoint
    GaussianControlPointList
    GlobalAttributes
    GlobalLineoutAttributes
    HostProfileList
    InteractorAttributes
    InternalResampleAttributes
    KeyframeAttributes
    LaunchProfile
    LightAttributes
    LightList
    Line
    LineGlyphAttributes
    LinearTransformAttributes
    MachineProfile
    MaterialAttributes
    MeshManagementAttributes
    MessageAttributes
    MovieAttributes
    NameschemeAttributes
    NamespaceAttributes
    PickAttributes
    PickVarInfo
    PlaneAttributes
    Plot
    PlotInfoAttributes
    PlotList
    PlotQueryInfo
    PluginManagerAttributes
    PointAttributes
    PointGlyphAttributes
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
    SaveSubWindowAttributes
    SaveSubWindowsAttributes
    SaveWindowAttributes
    SelectionList
    SelectionProperties
    SelectionSummary
    SelectionVariableSummary
    SimilarityTransformAttributes
    SimulationCommand
    SimulationUIValues
    SphereAttributes
    StatusAttributes
    SurfaceFilterAttributes
    SyncAttributes
    ThresholdOpAttributes
    TimeFormat
    View2DAttributes
    View3DAttributes
    ViewAttributes
    ViewAxisArrayAttributes
    ViewCurveAttributes
    ViewerClientAttributes
    ViewerClientInformation
    ViewerClientInformationElement
    VisualCueInfo
    VisualCueList
    WindowAttributes
    WindowInformation)

    ADD_CPP_GEN_TARGET(${obj_name}
                       ${CMAKE_CURRENT_SOURCE_DIR}/
                       ${CMAKE_CURRENT_SOURCE_DIR}/)
endforeach()


# add to visitcommon library

target_include_directories(visitcommon PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

if(NOT VISIT_HEADERS_SKIP_INSTALL)
    target_include_directories(visitcommon PUBLIC
      $<INSTALL_INTERFACE:include/visit/common/state>)
endif()

target_sources(visitcommon PRIVATE
  AnimationAttributes.C
  AnnotationAttributes.C
  AnnotationObject.C
  AnnotationObjectList.C
  AppearanceAttributes.C
  AttributeGroup.C
  AttributeSubject.C
  AttributeSubjectMap.C
  AttributeSubjectSerialize.C
  Axes2D.C
  Axes3D.C
  AxesArray.C
  AxisAttributes.C
  AxisLabels.C
  AxisRestrictionAttributes.C
  AxisTickMarks.C
  AxisTitles.C
  BoxExtents.C
  CinemaAttributes.C
  ClientInformation.C
  ClientInformationList.C
  ClientMethod.C
  ColorAttribute.C
  ColorAttributeList.C
  ColorControlPoint.C
  ColorControlPointList.C
  ColorTableAttributes.C
  ColorTableManager.C
  CompactSILRestrictionAttributes.C
  ConfigManager.C
  ConstructDataBinningAttributes.C
  ContourOpAttributes.C
  DatabaseAttributes.C
  DatabaseCorrelation.C
  DatabaseCorrelationList.C
  DataNode.C
  DBOptionsAttributes.C
  DBPluginInfoAttributes.C
  EngineList.C
  EngineProperties.C
  ExportDBAttributes.C
  Expression.C
  ExpressionList.C
  FileOpenOptions.C
  FontAttributes.C
  GaussianControlPoint.C
  GaussianControlPointList.C
  GlobalAttributes.C
  GlobalLineoutAttributes.C
  GlyphTypes.C
  LaunchProfile.C
  LaunchRPC.C
  MachineProfile.C
  HostProfileList.C
  InteractorAttributes.C
  InternalResampleAttributes.C
  KeepAliveRPC.C
  KeyframeAttributes.C
  LightAttributes.C
  LightList.C
  Line.C
  LinearTransformAttributes.C
  LineGlyphAttributes.C
  MapNode.C
  MaterialAttributes.C
  MeshManagementAttributes.C
  MessageAttributes.C
  MovieAttributes.C
  NamespaceAttributes.C
  NameschemeAttributes.C
  Observer.C
  ObserverToCallback.C
  PickAttributes.C
  PickVarInfo.C
  PlaneAttributes.C
  Plot.C
  PlotInfoAttributes.C
  PlotList.C
  PlotQueryInfo.C
  PluginManagerAttributes.C
  PointAttributes.C
  PointGlyphAttributes.C
  PrinterAttributes.C
  ProcessAttributes.C
  QueryAttributes.C
  QueryList.C
  QueryOverTimeAttributes.C
  QuitRPC.C
  RenderingAttributes.C
  SaveWindowAttributes.C
  SaveSubWindowAttributes.C
  SaveSubWindowsAttributes.C
  SelectionList.C
  SelectionProperties.C
  SelectionSummary.C
  SelectionVariableSummary.C
  SimulationUIValues.C
  SILArrayAttributes.C
  SILAttributes.C
  SILMatrixAttributes.C
  SILRestrictionAttributes.C
  SimilarityTransformAttributes.C
  SimpleObserver.C
  SimulationCommand.C
  SingleAttributeConfigManager.C
  SphereAttributes.C
  StatusAttributes.C
  Subject.C
  SurfaceFilterAttributes.C
  SyncAttributes.C
  ThresholdOpAttributes.C
  TimeFormat.C
  Variant.C
  View2DAttributes.C
  View3DAttributes.C
  ViewAttributes.C
  ViewAxisArrayAttributes.C
  ViewCurveAttributes.C
  ViewerClientAttributes.C
  ViewerClientInformation.C
  ViewerClientInformationElement.C
  VisItRPC.C
  VisualCueInfo.C
  VisualCueList.C
  WindowAttributes.C
  WindowInformation.C
  Xfer.C
  XMLNode.C
  JSONNode.C
)

