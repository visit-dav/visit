#*****************************************************************************
#
# Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-400142
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# Modifications:
#
#****************************************************************************/

PROJECT(JavaClient Java)

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
avtSILCollectionMetaData.java
avtSILMetaData.java
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
ClientInformation.java 
ClientInformationList.java 
ClientMethod.java 
ColorAttribute.java 
ColorAttributeList.java 
ColorControlPoint.java 
ColorControlPointList.java 
ColorTableAttributes.java 
ConstructDDFAttributes.java 
DBOptionsAttributes.java 
DBPluginInfoAttributes.java 
DatabaseCorrelation.java 
DatabaseCorrelationList.java 
EngineList.java 
ExportDBAttributes.java 
Expression.java 
ExpressionList.java 
FileOpenOptions.java 
FontAttributes.java 
GaussianControlPoint.java 
GaussianControlPointList.java 
GlobalAttributes.java 
GlobalLineoutAttributes.java 
HostProfile.java 
HostProfileList.java 
InteractorAttributes.java 
KeyframeAttributes.java 
LightAttributes.java 
LightList.java 
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
Point.java 
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
SaveWindowAttributes.java 
StatusAttributes.java 
SyncAttributes.java 
View2DAttributes.java 
View3DAttributes.java 
ViewAxisArrayAttributes.java 
ViewCurveAttributes.java 
ViewerRPC.java 
WindowInformation.java
)

SET(PACKAGE_SOURCES
Version.java 
SimpleObserver.java 
CouldNotConnectException.java 
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
Yielder.java
Xfer.java 
SyncNotifier.java
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
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/VersionWriter ${VISIT_SOURCE_DIR}/VERSION ${VISIT_SOURCE_DIR}/java/Version.java
    COMMAND ${CMAKE_Java_COMPILER} ${CMAKE_Java_FLAGS} -d ${VISIT_BINARY_DIR}/java ${PACKAGE_SOURCES} ${PACKAGE_STATE_SOURCES} ${METADATA_SOURCES}
    WORKING_DIRECTORY ${VISIT_BINARY_DIR}/java
)

# Make an empty target that depends on VersionWriter
ADD_CUSTOM_TARGET(JavaClient)
ADD_DEPENDENCIES(JavaClient VersionWriter)

# Add a target that does not get built by default but creates the jar file.
ADD_CUSTOM_TARGET(pack ${CMAKE_Java_ARCHIVE} cvf visit.jar llnl WORKING_DIRECTORY ${VISIT_BINARY_DIR}/java)

# Add a target that compiles all of the main programs.
SET(MAIN_SOURCES
GetMetaData.java 
MakeHostProfile.java 
NIFGUI.java 
OpenGUI.java 
PlotAtts.java 
PlotTypes.java
RunViewer.java 
TryAnnotations.java 
TryLighting.java 
TryQuery.java 
)
ADD_CUSTOM_TARGET(main ${CMAKE_Java_COMPILER} ${CMAKE_Java_FLAGS} -d ${CMAKE_CURRENT_SOURCE_DIR} ${MAIN_SOURCES})

