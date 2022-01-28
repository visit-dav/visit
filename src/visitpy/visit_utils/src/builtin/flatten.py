# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

###############################################################################
# file: flatten.py
# Purpose: Expose tabular mesh data to the CLI
#
#
# Programmer: Chris Laganella
# Creation: Tue Jan 18 11:05:21 EST 2022
#
#
# Modifications:
#
#
###############################################################################
try:
    import visit
except:
    pass

import sys
import socket

def Flatten(vars):
    """Flatten(vars) -> dict

Synopsis:
    Query the active plot for the data at each node/zone for the given
    variables. Data is returned as numpy compatible 2D arrays.

Arguments:

vars:
    The names of the desired variables (tuple of strings).

Returns:

    Flatten returns a dictionary that contains different keys depending
    on the data. If the output contains node centered data then there
    will be 'nodeColumnNames' and 'nodeTable' entries. If the output contains
    zone centered data then there will be 'zoneColumnNames' and 'zoneTable'
    entries. If the query results in no output data, then an empty dictionary
    is returned. The '*Table' entries are compatible with numpy via the
    'numpy.asarray()' function.
    """
    flattenOpts = dict()
    flattenOpts["vars"] = vars

    numPlots = visit.GetNumPlots()
    if numPlots == 0:
        print("VisIt: Error - Flatten() requires an active plot!")
        return dict()

    if (sys.platform.startswith('linux')
            or sys.platform.startswith('darwin')):

        plotList = visit.GetPlotList()
        dbhost = str()
        for i in range(0, numPlots):
            plot = plotList.GetPlots(i)
            if plot.GetActiveFlag() == 1:
                dbname = plot.GetDatabaseName()
                dbhost = dbname.split(':')[0]
                break

        hostname = socket.gethostname()
        if (dbhost == 'localhost'
                or dbhost == '127.0.0.1'
                or dbhost == hostname):
            flattenOpts["useSharedMemory"] = 1

    visit.Query("Flatten", flattenOpts)
    return visit.GetFlattenOutput()
