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

def Flatten(vars, fillValue=0., nodeIds=True, zoneIds=True, nodeIJK=True,
                zoneIJK=True, zoneCenters=False, maxDataSize=1.024,
                forceNoSharedMemory=False):
    """Flatten


Synopsis:

Flatten(vars) -> dictionary

Flatten(vars, fillValue, nodeIds, zoneIds, nodeIJK, zoneIJK, zoneCenters,
            maxDataSize, forceNoSharedMemory) -> dictionary


Returns:

    Flatten returns a dictionary that contains different keys depending
    on the data. If the output contains node centered data then there
    will be 'nodeColumnNames' and 'nodeTable' entries. If the output contains
    zone centered data then there will be 'zoneColumnNames' and 'zoneTable'
    entries. If the query results in no output data, then an empty dictionary
    is returned. The '*Table' entries are compatible with numpy via the
    'numpy.asarray()' function.
vars
    The names of the desired variables (tuple of strings).
fillValue
    The default value for a column if no data is present (float, default = 0.)
nodeIds
    Whether or not the nodeIds should be included in the output table.
    (bool, default = True)
zoneIds
    Whether or not the zoneIds should be included in the output table.
    (bool, default = True)
nodeIJK
    Whether or not the nodeIJK should be included in the output table.
    (bool, default = True)
zoneIJK
    Whether or not the zoneIJK should be included in the output table.
    (bool, default = True)
zoneCenters
    Whether or not to add the central coordinates of each zone.
    (bool, default = False)
maxDataSize
    The maximum output data size when not using shared memory, expressed in GB.
    This parameters exists because the default method of returning query
    results does not scale well up to large sizes.
    (float, default=1.024)
forceNoSharedMemory
    An override that makes sure the function will NOT use shared memory
    to transport the output data to the VisIt CLI, even if the
    environment seems to support it.
    (bool, default = False)


Description:

Query the active plot for the data at each node/zone for the given
variables. Data is returned as numpy compatible 2D arrays using
numpy.asarray().


Example:

#% visit -cli
db = "/usr/gapps/visit/data/rect2d.silo"
OpenDatabase(db)
AddPlot("Pseudocolor", "d")
DrawPlots()
data = Flatten(("p", "d"))
if "nodeTable" in data:
  print(numpy.asarray(data["nodeTable"]))
if zoneTable in data:
  print(numpy.asarray(data["zoneTable"]))"""
    flattenOpts = dict()
    flattenOpts['vars'] = vars

    numPlots = visit.GetNumPlots()
    if numPlots == 0:
        print("VisIt: Error - Flatten() requires an active plot!")
        return dict()

    if ((sys.platform.startswith('linux')
            or sys.platform.startswith('darwin'))
            and not forceNoSharedMemory):

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
            flattenOpts['useSharedMemory'] = 1

    flattenOpts['fillValue'] = float(fillValue)
    flattenOpts['maxDataSize'] = float(maxDataSize)
    # NOTE: Was having problems parsing these as bool in C++,
    #  using int now instead.
    flattenOpts['nodeIds'] = int(nodeIds)
    flattenOpts['zoneIds'] = int(zoneIds)
    flattenOpts['nodeIJK'] = int(nodeIJK)
    flattenOpts['zoneIJK'] = int(zoneIJK)
    flattenOpts['zoneCenters'] = int(zoneCenters)

    visit.Query('Flatten', flattenOpts)
    retval = visit.GetFlattenOutput()

    # NOTE: If desired we could just return the data as numpy arrays
    #  (would have to import numpy in this file)
    #if 'nodeTable' in retval:
    #  retval['nodeTable'] = numpy.fromarray(retval['nodeTable'])
    #if 'zoneTable' in retval:
    #  retval['zoneTable'] = numpy.fromarray(retval['zoneTable'])
    return retval
