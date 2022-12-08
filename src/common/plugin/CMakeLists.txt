# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

target_sources(visitcommon PRIVATE
    DatabasePluginInfo.C
    DatabasePluginManager.C
    InvalidDirectoryException.C
    InvalidPluginException.C
    OperatorPluginManager.C
    PlotPluginManager.C
    PluginBroadcaster.C
    PluginManager.C)

target_include_directories(visitcommon PUBLIC .)

