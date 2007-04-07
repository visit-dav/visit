# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  renderpoints.py
#
#  Tests:      mesh      - 3D point
#              plots     - pseudocolor, subset, mesh
#
#  Defect ID:  none
#
#  Programmer: Brad Whitlock
#  Date:       Fri Aug 26 13:39:07 PST 2005
#
#  Modificatons:
#
# ----------------------------------------------------------------------------

#
# General testing function
#
def TestPlot(plotName, plotVar, atts, cases):
    AddPlot(plotName, plotVar)
    if atts == None:
        atts = eval(plotName + "Attributes()")
    atts.pointType = atts.Point
    atts.pointSizePixels = 1
    SetPlotOptions(atts)
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (-0.51943, 0.317321, 0.79341)
    v.focus = (0.499552, 0.500362, 0.499909)
    v.viewUp = (0.172325, 0.948317, -0.266458)
    v.viewAngle = 30
    v.parallelScale = 0.865387
    v.nearPlane = -1.73077
    v.farPlane = 1.73077
    v.imagePan = (-0.0139929, 0.0505193)
    v.imageZoom = 1.25645
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.499552, 0.500362, 0.499909)
    SetView3D(v)
    Test("renderpoint" + cases[0])

    atts.pointSizePixels = 5
    SetPlotOptions(atts)
    Test("renderpoint" + cases[1])

    # Make sure that it remains set after regenerating the plot.
    TurnDomainsOff(("domain3", "domain4", "domain7", "domain8"))
    Test("renderpoint" + cases[2])
    TurnDomainsOn()
    DeleteActivePlots()

#
# Test plots that can render their points using GL points.
#
def test0():
    TestSection("Set pointSizePixels for plots that support it")
    # Test Mesh
    atts = MeshAttributes()
    atts.meshColor = (100,150,255)
    atts.foregroundFlag = 0
    TestPlot("Mesh", "StarMesh", atts, ("_0_00", "_0_01", "_0_02"))

    # Test Pseudocolor
    TestPlot("Pseudocolor", "Matnos", None, ("_0_03", "_0_04", "_0_05"))

    # Test Scatter
    atts = ScatterAttributes()
    atts.var2 = "vy"
    atts.var3 = "vz"
    atts.var3Role = atts.Coordinate2
    atts.var4 = "Matnos"
    atts.var4Role = atts.Color
    TestPlot("Scatter", "vx", atts, ("_0_06", "_0_07", "_0_08"))

    # Test FilledBoundary
    TestPlot("FilledBoundary", "stars", None, ("_0_09", "_0_10", "_0_11"))

    # Test Subset
    TestPlot("Subset", "domains", None, ("_0_12", "_0_13", "_0_14"))

#
# General testing function #2
#
def TestPlot2(plotName, plotVar, atts, cases):
    AddPlot(plotName, plotVar)
    if atts == None:
        atts = eval(plotName + "Attributes()")
    atts.pointType = atts.Point
    atts.pointSizePixels = 5
    SetPlotOptions(atts)
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (-0.51943, 0.317321, 0.79341)
    v.focus = (0.499552, 0.500362, 0.499909)
    v.viewUp = (0.172325, 0.948317, -0.266458)
    v.viewAngle = 30
    v.parallelScale = 0.865387
    v.nearPlane = -1.73077
    v.farPlane = 1.73077
    v.imagePan = (-0.0139929, 0.0505193)
    v.imageZoom = 1.25645
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.499552, 0.500362, 0.499909)
    SetView3D(v)
    Test("renderpoint" + cases[0])

    atts.pointType = atts.Box
    SetPlotOptions(atts)
    Test("renderpoint" + cases[1])
    DeleteActivePlots()

#
# Test that we can switch to other glyph types after starting up
# in point glyph mode, which does not use the glyph filters.
#
def test1():
    TestSection("Check points to glyph transition")
    # Test Pseudocolor
    TestPlot2("Pseudocolor", "Matnos", None, ("_1_00", "_1_01"))
    # Test FilledBoundary
    TestPlot2("FilledBoundary", "stars", None, ("_1_02", "_1_03"))

#
# General testing function #3
#
def TestPlot3(plotName, plotVar, atts, cases):
    AddPlot(plotName, plotVar)
    if atts == None:
        atts = eval(plotName + "Attributes()")
    atts.pointType = atts.Sphere
    atts.pointSizePixels = 10
    SetPlotOptions(atts)
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (-0.51943, 0.317321, 0.79341)
    v.focus = (0.499552, 0.500362, 0.499909)
    v.viewUp = (0.172325, 0.948317, -0.266458)
    v.viewAngle = 30
    v.parallelScale = 0.865387
    v.nearPlane = -1.73077
    v.farPlane = 1.73077
    v.imagePan = (-0.0139929, 0.0505193)
    v.imageZoom = 1.25645
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.499552, 0.500362, 0.499909)
    SetView3D(v)
    Test("renderpoint" + cases[0])

    atts.pointSizePixels = 20
    SetPlotOptions(atts)
    Test("renderpoint" + cases[1])
    DeleteActivePlots()

#
# Test that all plots that support sphere point texturing can do it.
#
def test2():
    TestSection("Check points drawn as spheres")

    # Test Mesh
    atts = MeshAttributes()
    atts.meshColor = (100,150,255)
    atts.foregroundFlag = 0
    TestPlot3("Mesh", "StarMesh", atts, ("_2_00", "_2_01"))

    # Test Pseudocolor
    TestPlot3("Pseudocolor", "Matnos", None, ("_2_02", "_2_03"))

    # Test Scatter
    atts = ScatterAttributes()
    atts.var2 = "vy"
    atts.var3 = "vz"
    atts.var3Role = atts.Coordinate2
    atts.var4 = "Matnos"
    atts.var4Role = atts.Color
    TestPlot3("Scatter", "vx", atts, ("_2_04", "_2_05"))

    # Test FilledBoundary
    TestPlot3("FilledBoundary", "stars", None, ("_2_06", "_2_07"))

    # Test Subset
    TestPlot3("Subset", "domains", None, ("_2_08", "_2_09"))


def main():
    # Turn off all annotation
    a = AnnotationAttributes()
    a.axesFlag2D = 0
    a.axesFlag = 0
    a.triadFlag = 0
    a.bboxFlag = 0
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    a.legendInfoFlag = 0
    a.backgroundMode = 0
    a.foregroundColor = (0, 0, 0, 255)
    a.backgroundColor = (255, 255, 255, 255)
    SetAnnotationAttributes(a)

    OpenDatabase("../data/galaxy0000.silo")

    test0()
    test1()
    test2()

main()
Exit()
