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

from itertools import starmap
from pydoc import plain
import sys

def flatten(vars):
    """flatten(vars) -> dict

Synopsis:
    Query the active plot for the data at each node/zone for the given
    variables. Data is returned in as numpy compatible 2D arrays.

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

    if (sys.platform.startswith('linux')
            or sys.platform.startswith('darwin')):

        if visit.GetPlotList().GetPlots(0).GetDatabaseName().startswith('localhost'):
            flattenOpts["useSharedMemory"] = 1

    visit.Query("Flatten", flattenOpts)
    return visit.GetFlattenOutput()
