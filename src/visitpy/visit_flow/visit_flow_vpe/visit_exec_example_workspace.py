# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: visit_exec_flow_workspace.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 1/04/2012
 description:
   Driver script that executes a flow workspace on 'rect2d.silo'.

"""

import os
import visit
from os.path import join as pjoin
from visit_flow_vpe import *

def save_window():
    visit.ResetView()
    v = visit.GetView3D()
    v.RotateAxis(0,-90)
    visit.SetView3D(v)
    swatts= visit.SaveWindowAttributes()
    swatts.outputToCurrentDirectory = 1
    swatts.outputDirectory = "."
    swatts.fileName = "chunk_render"
    swatts.family = 0
    swatts.format = swatts.PNG
    swatts.width = 1024
    swatts.height = 1024
    swatts.screenCapture = 0
    swatts.saveTiled = 0
    swatts.quality = 100
    swatts.progressive = 0
    swatts.binary = 0
    swatts.stereo = 0
    swatts.compression = swatts.PackBits
    swatts.forceMerge = 0
    swatts.resConstraint = swatts.NoConstraint
    swatts.advancedMultiWindowSave = 0
    visit.SetSaveWindowAttributes(swatts)
    visit.SaveWindow()

def main():
    args   = Argv()
    wfile  = args[0]
    dbfile = pjoin("tests","_data","rect2d.silo")
    fset = "pyocl_ops"
    plat = 0
    dev  = 0
    if len(args) > 1:
        dbfile = args[1]
    if len(args) > 2:
        fset = args[2]
    if len(args) > 3:
        plat  = int(args[3])
    if len(args) > 4:
        dev   = int(args[4])
    visit.OpenDatabase(dbfile)
    define_flow_vpe("flow",
                    expr=wfile,
                    filter_set=fset,
                    platform_id=plat,
                    device_id=dev)
    visit.AddPlot("Pseudocolor","flow")
    visit.DrawPlots()
    #visit.ClearAllWindows()
    #visit.DrawPlots()

if __visit_script_file__ == __visit_source_file__:
    main()
    if "-save" in Argv():
        save_window()
    if "-nowin" in sys.argv:
        sys.exit(0)


