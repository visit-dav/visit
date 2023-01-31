# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#   Tom Fogal, Wed Aug  4 18:54:25 MDT 2010
#   Add multires operator.
#
#   Hank Childs, Sat Aug  7 03:39:33 PDT 2010
#   Add DataBinning operator.
#
#   Hank Childs, Fri Sep 24 08:16:10 PDT 2010
#   Add SurfaceNormal, Flux, and ConnectedComponents operators.
#
#   Brad Whitlock, Wed Jun 22 15:25:18 PDT 2011
#   I added the Extrude operator.
#
#   Marc Durant, Tue Sep 6 2011
#   Added the FFT operator
#
#   Hank Childs, Mon Oct 31 09:38:27 PDT 2011
#   Add ToroidalPoloidalProjection on behalf of Hari Krishnan.
#
#   Cyrus Harrison, Tue Sep 25 12:08:29 PDT 2012
#   Added CartographicProjection (from Jean Favre)
#
#   Kathleen Biagas, Thu Oct 11 11:48:27 PDT 2012
#   Make CartographicProjection dependent upon existence of GEOVIS vtk kit.
#
#   Kathleen Biagas, Mon May 12 12:59:25 PDT 2014
#   Prefer VISIT_SELECTED_OPERATOR_PLUGINS if defined.  Test for existence of
#   subdir before adding it.  Call VISIT_SELECTED_PLUGIN_ERROR macro if
#   necessary.
#
#   Kevin Griffin, Wed July 22 18:00:10 PDT 2015
#   Add SPH Resample Operator 
#
#   Eric Brugger, Wed Mar 22 10:34:24 PDT 2017
#   I moved the SubdivideQuads operator from optional to required.
#
#   Kathleen Biagas, Tue Oct  9 12:48:07 PDT 2018
#   Removed ProgrammableOp, since it doesn't compile and has been commented
#   out for a while.
#
#   Eric Brugger, Mon Jul 20 16:07:05 PDT 2020
#   I added the Tessellate operator.
#
#    Kathleen Biagas, Mon Jan 30, 2023
#    Moved setting of RUNTIME_OUTPUT_DIRECTORY and LIBRARY_OUTPUT_DIRECTORY
#    to macro VISIT_PLUGIN_TARGET_OUTPUT_DIR.
#
#****************************************************************************

include(${VISIT_SOURCE_DIR}/CMake/PluginMacros.cmake)


SET(REQUIRED_OPERATOR_PLUGINS
AMRStitchCell
AxisAlignedSlice4D
BoundaryOp
Box
Clip
Cone
ConnectedComponents
CoordSwap
CracksClipper
CreateBonds
Cylinder
DataBinning
Decimate
Delaunay
DeferExpression
Displace
DualMesh
Edge
Elevate
EllipsoidSlice
ExternalSurface
ExtractPointFunction2D
Extrude
Explode
FFT
FiveFoldTetSubdivision
Flux
LCS
IndexSelect
IntegralCurve
InverseGhostZone
Isosurface
Isovolume
Lagrangian
LimitCycle
Lineout
LineSampler
Merge
ModelFit
MultiresControl
OnionPeel
PDF
PersistentParticles
Project
Remap
Reflect
Replicate
Resample
RadialResample
Revolve
Slice
Smooth
SphereSlice
SPHResample
Stagger
StatisticalTrends
SubdivideQuads
SurfaceNormal
Tessellate
ThreeSlice
Threshold
ToroidalPoloidalProjection
Transform
TriangulateRegularPoints
Tube
ZoneDump
)

# Do not include Poincare on BlueGene/Q because it has a std::map symbol
# that prevents static linking of libpami.a which is needed by MPI on 
# that system.
IF(NOT BLUEGENEQ)
   SET(REQUIRED_OPERATOR_PLUGINS ${REQUIRED_OPERATOR_PLUGINS} Poincare)
ENDIF(NOT BLUEGENEQ)

#adding programmable operator only if Python is available.
#IF(VISIT_PYTHON_FILTERS OR VISIT_PYTHON_SCRIPTING)
#  IF(NOT DEFINED VISIT_SELECTED_OPERATOR_PLUGINS)
#    LIST(APPEND REQUIRED_OPERATOR_PLUGINS Script)
#  ENDIF()
#ENDIF(VISIT_PYTHON_FILTERS OR VISIT_PYTHON_SCRIPTING)

IF(TARGET vtkGeovisCore)
    LIST(APPEND REQUIRED_OPERATOR_PLUGINS CartographicProjection)
ELSE()
    IF(DEFINED VISIT_SELECTED_OPERATOR_PLUGINS)
        VISIT_SELECTED_PLUGIN_ERROR(OPERATOR
          "CartographicProjection"
          "library vtkGeovisCore not found")
    ELSE()
        MESSAGE(STATUS "CartographicProjection requires vtkGeovisCore")
    ENDIF()
ENDIF()

SET(OPTIONAL_OPERATOR_PLUGINS
Context
ConnCompReduce
MetricThreshold
RemoveCells
SurfCompPrep
)

SET(MINIMAL_OPERATOR_PLUGINS
Lineout
Slice
Threshold
)

# Select the plugins to build
IF(DEFINED VISIT_SELECTED_OPERATOR_PLUGINS)
    SET(BUILD_PLUGINS ${VISIT_SELECTED_OPERATOR_PLUGINS})
ELSEIF(VISIT_BUILD_ALL_PLUGINS)
    SET(BUILD_PLUGINS ${REQUIRED_OPERATOR_PLUGINS} ${OPTIONAL_OPERATOR_PLUGINS})
ELSE()
    IF(VISIT_BUILD_MINIMAL_PLUGINS)
        SET(BUILD_PLUGINS ${MINIMAL_OPERATOR_PLUGINS})
    ELSE(VISIT_BUILD_MINIMAL_PLUGINS)
        SET(BUILD_PLUGINS ${REQUIRED_OPERATOR_PLUGINS})
    ENDIF(VISIT_BUILD_MINIMAL_PLUGINS)
ENDIF()

# Add the plugin directories
FOREACH(operator ${BUILD_PLUGINS})
    # (EXISTS test requires full path)
    IF(EXISTS ${VISIT_SOURCE_DIR}/operators/${operator})
        ADD_SUBDIRECTORY(${operator})
    ELSE()
        IF(DEFINED VISIT_SELECTED_OPERATOR_PLUGINS)
            Message(FATAL_ERROR "Selected operator (${operator}) does not exist.")
        ELSEIF(VISIT_STATIC)
            # for code below, remove anything we aren't actually building
            Message("${operator} removed from build because it does not exist")
            LIST(REMOVE_ITEM BUILD_PLUGINS ${operator})
        ENDIF()
    ENDIF()
ENDFOREACH(operator)

# If we're building statically, make various main programs depend on plugins
# being built
IF(VISIT_STATIC)
    # Write a file containing the list of plugins such that we can include it
    # into C++ code.
    FOREACH(operator ${BUILD_PLUGINS})
        SET(operatorCode "${operatorCode}PLUGIN_VERB(${operator})\n")
    ENDFOREACH(operator)
    FILE(WRITE ${VISIT_BINARY_DIR}/include/enabled_operators.h "${operatorCode}")

    IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY)
        CREATE_PLUGIN_DEPENDENCIES(gui_exe "I" "Operator" ${BUILD_PLUGINS})
        CREATE_PLUGIN_DEPENDENCIES(gui_exe "G" "Operator" ${BUILD_PLUGINS})

        CREATE_PLUGIN_DEPENDENCIES(viewer_exe "I" "Operator" ${BUILD_PLUGINS})
        CREATE_PLUGIN_DEPENDENCIES(viewer_exe "V" "Operator" ${BUILD_PLUGINS})

        IF(VISIT_PYTHON_SCRIPTING)
            CREATE_PLUGIN_DEPENDENCIES(cli "I" "Operator" ${BUILD_PLUGINS})
            CREATE_PLUGIN_DEPENDENCIES(cli "S" "Operator" ${BUILD_PLUGINS})
        ENDIF(VISIT_PYTHON_SCRIPTING)
    ENDIF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY)

    CREATE_PLUGIN_DEPENDENCIES(engine_ser_exe "I" "Operator" ${BUILD_PLUGINS})
    CREATE_PLUGIN_DEPENDENCIES(engine_ser_exe "E" "Operator_ser" ${BUILD_PLUGINS})

    IF(VISIT_PARALLEL)
        CREATE_PLUGIN_DEPENDENCIES(engine_par_exe "I" "Operator" ${BUILD_PLUGINS})
        CREATE_PLUGIN_DEPENDENCIES(engine_par_exe "E" "Operator_par" ${BUILD_PLUGINS})
    ENDIF(VISIT_PARALLEL)
ENDIF(VISIT_STATIC)
