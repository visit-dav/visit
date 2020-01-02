# Tom Fogal, originally Sun Aug  5 13:44:41 PDT 2007
# Revamped Mon Jul 14 13:49:23 EDT 2008
import os
import sys

rootdir=os.environ['V_DIR']
# where are the data files?
datadir=rootdir + "/data"

def SetSRMode():
    ra = GetRenderingAttributes()
    ra.scalableActivationMode = ra.Always
    SetRenderingAttributes(ra)

def InitialSetup():
    SetSRMode()
    swa = GetSaveWindowAttributes()
    swa.format = swa.PNG
    swa.width, swa.height = 320, 240
    swa.quality=100
    swa.screenCapture = 1
    SetSaveWindowAttributes(swa)

def MyRender(fn):
    s = SaveWindowAttributes()
    s.format = s.PNG
    s.screenCapture = 1
    s.quality=100
    s.fileName = fn
    SetSaveWindowAttributes(s)
    DrawPlots()
    SaveWindow()

def DeleteAddRender(p, v):
    print("Rendering ", p, ":", v)
    DeleteAllPlots()
    AddPlot(p, v)
    if p is "Pseudocolor":
        print("Setting 60% transparency ...")
        pcAtts = PseudocolorAttributes()
        pcAtts.opacity = 0.60
        SetPlotOptions(pcAtts)
    MyRender("result")

def PlotVars(plots, vars):
    """Takes a list of plots and a list of variables, and renders all
       combinations of the two."""
    for p in plots:
        for v in vars:
            DeleteAddRender(p, v)

def RunTestsWithDB(db_name, plots, vars):
    OpenDatabase(db_name)
    save_view = GetView3D()
    PlotVars(plots, vars)

    view=GetView3D()
    view.viewNormal=(-0.528567, 0.374238, 0.761946)
    view.viewUp=(0.200044, 0.927212, -0.316639)
    SetView3D(view)
    PlotVars(plots, vars)

    DeleteAllPlots()
    CloseDatabase(db_name)
    SetView3D(save_view)

InitialSetup()
RunTestsWithDB(datadir + "/hist_ucd3d_0000",
               ["Vector"],
               ["vec"])
if True:
    RunTestsWithDB(datadir + "/multi_ucd3d.silo",
                   ["Contour", "Pseudocolor"],
                   ["p","sum","u","v","mag","hist"])

sys.exit(0)
