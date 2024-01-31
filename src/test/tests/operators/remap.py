# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  remap.py
#
#  Tests:      remap operator
#
#  Defect ID:  none
#
#  Programmer: Eddie Rusu
#  Date:       Fri Feb  1 11:16:24 PST 2019
#
#  Modifications:
#     Eddie Rusu, Tue Apr 13 12:08:59 PDT 2021
#     Changed Pseudocolor CurrentPlot to ActualData.
# ----------------------------------------------------------------------------

def remap(cells, type = 0):
    AddOperator("Remap", 1)
    RemapAtts = RemapAttributes()    
    RemapAtts.cellsX = cells  
    RemapAtts.cellsY = cells
    RemapAtts.cellsZ = cells
    if type == 1:
        RemapAtts.variableType = RemapAtts.extrinsic
        setPseudoOptions()
    SetOperatorOptions(RemapAtts, 1)
    DrawPlots()

def setPseudoOptions():
    PseudocolorAtts = PseudocolorAttributes()
    PseudocolorAtts.limitsMode = PseudocolorAtts.ActualData
    SetPlotOptions(PseudocolorAtts)
    
def set3DView():
    View3DAtts = View3DAttributes()
    View3DAtts.viewNormal = (0.5, 0.5, 0.7)
    View3DAtts.focus = (1.5, 1.5, 0.5)
    View3DAtts.viewUp = (-0.0429563, 0.998733, -0.0262205)
    View3DAtts.viewAngle = 30
    View3DAtts.parallelScale = 2.17945
    View3DAtts.nearPlane = -4.3589
    View3DAtts.farPlane = 4.3589
    View3DAtts.imagePan = (0, 0)
    View3DAtts.imageZoom = 1
    View3DAtts.perspective = 1
    View3DAtts.eyeAngle = 2
    View3DAtts.centerOfRotationSet = 0
    View3DAtts.centerOfRotation = (1.5, 1.5, 0.5)
    View3DAtts.axis3DScaleFlag = 0
    View3DAtts.axis3DScales = (1, 1, 1)
    View3DAtts.shear = (0, 0, 1)
    View3DAtts.windowValid = 1
    SetView3D(View3DAtts)
    
def plotVariables(varName, saveName, cells = 10):
    AddPlot("Pseudocolor", varName, 1, 1)
    DrawPlots()
    
    remap(cells)
    Test(saveName + "_int")
    
    RemoveOperator(0, 1)
    remap(cells, 1)
    Test(saveName + "_ext")
    
    DeleteAllPlots()
    
def ghost1():
    OpenDatabase(silo_data_path("ghost1.silo"))
    plotVariables("zvar", "ghost_zvar")
    CloseDatabase(silo_data_path("ghost1.silo"))
    
def globalNode():
    OpenDatabase(silo_data_path("global_node.silo"))
    set3DView()
    plotVariables("p", "globalNode_p", 4) # Wrong results.
    ResetView()
    CloseDatabase(silo_data_path("global_node.silo"))
    
def mCurve2():
    OpenDatabase(silo_data_path("multi_curv2d.silo"))
    plotVariables("d", "mCurve2_d")
    plotVariables("d_dup", "mCurve2_d_dup")
    plotVariables("nmats", "mCurve2_nmats")
    plotVariables("p", "mCurve2_p")
    CloseDatabase(silo_data_path("multi_curv2d.silo"))
    
def mCurve3():
    OpenDatabase(silo_data_path("multi_curv3d.silo"))
    set3DView()
    plotVariables("d", "mCurve3_d", 4)
    plotVariables("d_dup", "mCurve3_d_dup", 4)
    plotVariables("nmats", "mCurve3_nmats", 4)
    plotVariables("p", "mCurve3_p", 4)
    ResetView()
    CloseDatabase(silo_data_path("multi_curv3d.silo"))
    
def mRect2():
    OpenDatabase(silo_data_path("multi_rect2d.silo"))
    plotVariables("d", "mRect2_d")
    plotVariables("d_dup", "mRect2_d_dup")
    plotVariables("nmats", "mRect2_nmats")
    plotVariables("p", "mRect2_p")
    CloseDatabase(silo_data_path("multi_rect2d.silo"))
    
def mRect3():
    OpenDatabase(silo_data_path("multi_rect3d.silo"))
    set3DView()
    plotVariables("d", "mRect3_d", 4)
    plotVariables("d_dup", "mRect3_d_dup", 4)
    plotVariables("nmats", "mRect3_nmats", 4)
    plotVariables("p", "mRect3_p", 4)
    ResetView()
    CloseDatabase(silo_data_path("multi_rect3d.silo"))

def ucd2():
    OpenDatabase(silo_data_path("ucd2d.silo"))
    plotVariables("p", "ucd2_p")
    CloseDatabase(silo_data_path("ucd2d.silo"))

def Main():
    ghost1()
    globalNode()
    mCurve2()
    mCurve3()
    mRect2()
    mRect3()
    ucd2()
    
Main()
Exit()
















