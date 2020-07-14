# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  contour.py
#
#  Tests:      mesh      - 3D structured, single domain
#                        - 3D rectilinear, multiple domain
#                        - 2D curvilinear, single domain
#              plots     - Contour
#
#  Programmer: Kathleen Biagas 
#  Date:       October 15, 2014
#
#  Modifications:
#    Brad Whitlock, Wed Sep 23 17:06:37 PDT 2015
#    Add a test for bad contour values to make sure the plot can handle it.
#
# ----------------------------------------------------------------------------

def ExerciseAtts():
    OpenDatabase(silo_data_path("noise.silo"))
    AddPlot("Contour", "hardyglobal")
    c = ContourAttributes()
    c.contourMethod = c.Percent
    c.contourPercent = (5, 78, 52, 13, 36, 42)
    SetPlotOptions(c)
    DrawPlots()
    Test("contour_contourMethod_00")
    c.contourMethod = c.Value
    c.contourValue= (2.0, 3.3, 5.01)
    SetPlotOptions(c)
    Test("contour_contourMethod_01")
    c.contourMethod = c.Level
    c.contourNLevels = 7
    SetPlotOptions(c)
    Test("contour_contourMethod_02")

    c.wireframe = 1
    c.lineWidth = 3
    SetPlotOptions(c)
    Test("contour_wireframe_00")
    ChangeActivePlotsVar("shepardglobal")
    # Get new default atts
    c = ContourAttributes()
    c.contourNLevels = 3
    SetPlotOptions(c)
    DrawPlots()
    Test("contour_minMax_00")
    c.minFlag = 1
    c.maxFlag = 1
    c.max = 5
    for i in range(1,5):
        c.min = i
        SetPlotOptions(c)
        Test("contour_minMax_%02d"%i)


    DeleteAllPlots()

    OpenDatabase(silo_data_path("curv2d.silo"))
    AddPlot("Contour", "d")
    c = ContourAttributes()
    c.contourNLevels = 4
    SetPlotOptions(c)
    DrawPlots()
    for i in range(10):
        c.lineWidth = i
        SetPlotOptions(c)
        Test("contour_lineWidth_%02d"%i)

    c.lineWidth = 4

    DeleteAllPlots()
    OpenDatabase(silo_data_path("multi_rect3d.silo"))
    AddPlot("Contour", "d")
    c = ContourAttributes()
    c.contourNLevels = 5
    c.colorType = c.ColorByColorTable 
    c.colorTableName = "rainbow"
    SetPlotOptions(c)
    DrawPlots()
    Test("contour_colorMethod_00")
    c.invertColorTable = 1
    SetPlotOptions(c)
    Test("contour_colorMethod_01")
    
    c.colorType = c.ColorBySingleColor
    c.singleColor = (87, 170, 0, 255)
    SetPlotOptions(c)
    Test("contour_colorMethod_02")
    c.singleColor = (87, 170, 0, 120)
    SetPlotOptions(c)
    Test("contour_colorMethod_03")
   
    c.colorType = c.ColorByMultipleColors
    SetPlotOptions(c)
    Test("contour_colorMethod_04")
    for i in range(5,0,-1):
        levelColor = list(c.GetMultiColor(i-1))
        levelColor[3] = 65
        c.SetMultiColor((i-1),tuple(levelColor))
        SetPlotOptions(c)
        Test("contour_colorMethod_%02d"%(10-i))
 
    DeleteAllPlots()   

    OpenDatabase(silo_data_path("wave.visit"))
    AddPlot("Contour", "pressure")
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (-0.561907, 0.653269, 0.507444)
    v.viewUp = (0.388352, 0.749975, -0.535463)
    SetView3D(v)
    Test("contour_linearScale")
    c = ContourAttributes()
    c.scaling = c.Log
    SetPlotOptions(c)
    Test("contour_logScale")
    
    DeleteAllPlots()   

def TestMultiColor():
    # Bug 2013, indexing issues with SetMultiColor when num levels reduces.
    OpenDatabase(silo_data_path("noise.silo"))
    AddPlot("Contour", "hardyglobal")
    DrawPlots()

    Sets = [[1,5.5,11],[1,5.5,11],[1,5.5,8]]
    testNum = 0
    for r in Sets:
        p = ContourAttributes()
        p.contourMethod = p.Level
        p.contourNLevels=r[2]
        p.colorType = p.ColorByMultipleColors
        p.singleColor = (255, 0, 0, 25)
        p.SetMultiColor(0, (255,0,255,64))
        p.SetMultiColor(1, (0,255,0,64))
        p.SetMultiColor(2, (255, 135, 0, 255))
        p.SetMultiColor(3, (255,255,0,64))
        p.SetMultiColor(4, (255, 0, 135, 255))
        p.SetMultiColor(5, (0,0,255,64))
        p.SetMultiColor(6, (0,255,255,64))
        p.SetMultiColor(7, (255,0,0,64))
        if r[2] > 8 :
            p.SetMultiColor(8, (99, 255, 99, 255))
            p.SetMultiColor(9, (168, 168, 168, 255))
            p.SetMultiColor(10, (255, 68, 68, 255))

        p.minFlag=1
        p.maxFlag=1
        p.min=r[0]
        p.max=r[1]
        SetPlotOptions(p)
        Test("contour_multicolor_%02d"%testNum)
        testNum = testNum + 1
    DeleteAllPlots()

def TestBadValue():
    OpenDatabase(silo_data_path("noise.silo"))
    AddPlot("Contour", "hardyglobal")
    ContourAtts = ContourAttributes()
    ContourAtts.contourValue = (0)
    ContourAtts.contourMethod = ContourAtts.Value  # Level, Value, Percent
    ContourAtts.minFlag = 0
    ContourAtts.maxFlag = 0
    SetPlotOptions(ContourAtts)
    DrawPlots()

    # Test that the plot is not in the error state and that we have an engine. (i.e. the engine did not crash)
    pl = GetPlotList()
    txt = "Plot Completed: " + str(pl.GetPlots(0).stateType == pl.GetPlots(0).Completed) + "\n"
    txt = txt + "Number of compute engines: " + str(len(GetEngineList())) + "\n"
    TestText("contour_bad_value", txt)

ExerciseAtts()
TestMultiColor()
TestBadValue()

Exit()
