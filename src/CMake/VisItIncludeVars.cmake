# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
#  Purpose:  Sets up some Include vars for use with normal development builds
#            and building plugins against an installed version of VisIt.
#
#  Vars created:
#     VISIT_COMMON_INCLUDES:    used everywhere in VisIt
#     VISIT_DATABASE_INCLUDES:  used by database plugins
#     VISIT_OPERATOR_INCLUDES:  used by operator plugins
#     VISIT_PLOT_INCLUDES:      used by plot plugins
#
#  Modifications:
#    Kathleen Biagas, Thu Oct 27, 2022
#    CMake modernization: Remove src/common from VISIT_COMMON_INCLUDES,
#    they are now part of the interface for visitcommon.
#
#*****************************************************************************

# Set up easy to use includes for the common directory
set(VISIT_COMMON_INCLUDES
    ${VISIT_BINARY_DIR}/include
    ${VISIT_INCLUDE_DIR}/include
    ${ZLIB_INCLUDE_DIR}
    )

# Set up easy to use includes for plugins

set(VISIT_PLUGIN_INCLUDES
    ${VISIT_COMMON_INCLUDES}
    ${VISIT_INCLUDE_DIR}/avt/DBAtts/MetaData
    ${VISIT_INCLUDE_DIR}/avt/DBAtts/SIL
    ${VISIT_INCLUDE_DIR}/avt/Database/Database
    ${VISIT_INCLUDE_DIR}/avt/FileWriter
    ${VISIT_INCLUDE_DIR}/avt/Filters
    ${VISIT_INCLUDE_DIR}/avt/Math
    ${VISIT_INCLUDE_DIR}/avt/Pipeline/AbstractFilters
    ${VISIT_INCLUDE_DIR}/avt/Pipeline/Data
    ${VISIT_INCLUDE_DIR}/avt/Pipeline/Pipeline
    ${VISIT_INCLUDE_DIR}/avt/Pipeline/Sinks
    ${VISIT_INCLUDE_DIR}/avt/Pipeline/Sources
    ${VISIT_INCLUDE_DIR}/avt/VisWindow/VisWindow
    ${VISIT_INCLUDE_DIR}/visit_vtk/full
    ${VISIT_INCLUDE_DIR}/visit_vtk/lightweight
    ${VTK_INCLUDE_DIRS}
)


set(VISIT_DATABASE_INCLUDES
    ${VISIT_PLUGIN_INCLUDES}
    ${VISIT_INCLUDE_DIR}/avt/Database/Formats
    ${VISIT_INCLUDE_DIR}/avt/Database/Ghost
    ${VISIT_INCLUDE_DIR}/avt/MIR/Base
    ${VISIT_INCLUDE_DIR}/avt/MIR/Tet
    ${VISIT_INCLUDE_DIR}/avt/MIR/Zoo
)

if(VISIT_PYTHON_SCRIPTING)
    set(VISIT_PLUGIN_PY_INCLUDES ${PYTHON_INCLUDE_PATH} ${VISIT_INCLUDE_DIR}/visitpy/common ${VISIT_INCLUDE_DIR}/visitpy/visitpy)
endif()

set(VISIT_PLOT_OPERATOR_INCLUDES
    ${VISIT_PLUGIN_INCLUDES}
    ${VISIT_INCLUDE_DIR}/avt/IVP
    ${VISIT_INCLUDE_DIR}/avt/Plotter
    ${VISIT_INCLUDE_DIR}/avt/QtVisWindow
    ${VISIT_INCLUDE_DIR}/avt/View
    ${VISIT_INCLUDE_DIR}/avt/VisWindow/Colleagues
    ${VISIT_INCLUDE_DIR}/avt/VisWindow/Interactors
    ${VISIT_INCLUDE_DIR}/avt/VisWindow/Proxies
    ${VISIT_INCLUDE_DIR}/avt/VisWindow/Tools
    ${VISIT_INCLUDE_DIR}/gui
    ${VISIT_INCLUDE_DIR}/viewer/core
    ${VISIT_INCLUDE_DIR}/viewer/main
    ${VISIT_INCLUDE_DIR}/viewer/main/ui
    ${VISIT_INCLUDE_DIR}/viewer/proxy
    ${VISIT_INCLUDE_DIR}/viewer/rpc
    ${VISIT_INCLUDE_DIR}/winutil
    ${VISIT_PLUGIN_PY_INCLUDES}
)

set(VISIT_OPERATOR_INCLUDES
    ${VISIT_PLOT_OPERATOR_INCLUDES}
    ${VISIT_INCLUDE_DIR}/avt/Expressions/Abstract
    ${VISIT_INCLUDE_DIR}/avt/Expressions/Abstract
    ${VISIT_INCLUDE_DIR}/avt/Expressions/CMFE
    ${VISIT_INCLUDE_DIR}/avt/Expressions/Conditional
    ${VISIT_INCLUDE_DIR}/avt/Expressions/Derivations
    ${VISIT_INCLUDE_DIR}/avt/Expressions/General
    ${VISIT_INCLUDE_DIR}/avt/Expressions/ImageProcessing
    ${VISIT_INCLUDE_DIR}/avt/Expressions/Management
    ${VISIT_INCLUDE_DIR}/avt/Expressions/Math
    ${VISIT_INCLUDE_DIR}/avt/Expressions/MeshQuality
    ${VISIT_INCLUDE_DIR}/avt/Expressions/TimeIterators
    ${VISIT_INCLUDE_DIR}/mdserver/proxy
    ${VISIT_INCLUDE_DIR}/mdserver/rpc
)

set(VISIT_PLOT_INCLUDES
    ${VISIT_PLOT_OPERATOR_INCLUDES}
)


