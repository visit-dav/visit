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
#    Mark C. Miller, Wed Jan 21 10:00:10 PST 2009
#    Removed silly comment regarding global annotation object 'a'
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Kathleen Biagas, Thu Dec 15 09:16:39 PST 2016
#    Added tests for other glyph types, scaling by variable and restoring
#    session files.
#
#    Kathleen Biagas, Wed Feb 16 09:15:45 PST 2022
#    Replace use of meshatts 'foregroundFlag' with meshColorSource.
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
    atts.meshColorSource = atts.MeshCustom
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
def TestPlot1(plotName, plotVar, atts, cases):
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
    TestPlot1("Pseudocolor", "Matnos", None, ("_1_00", "_1_01"))
    # Test FilledBoundary
    TestPlot1("FilledBoundary", "stars", None, ("_1_02", "_1_03"))

#
# General testing function #3
#
def TestPlot2(plotName, plotVar, atts, cases):
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
    atts.meshColorSource = atts.MeshCustom
    TestPlot2("Mesh", "StarMesh", atts, ("_2_00", "_2_01"))

    # Test Pseudocolor
    TestPlot2("Pseudocolor", "Matnos", None, ("_2_02", "_2_03"))

    # Test Scatter
    atts = ScatterAttributes()
    atts.var2 = "vy"
    atts.var3 = "vz"
    atts.var3Role = atts.Coordinate2
    atts.var4 = "Matnos"
    atts.var4Role = atts.Color
    TestPlot2("Scatter", "vx", atts, ("_2_04", "_2_05"))

    # Test FilledBoundary
    TestPlot2("FilledBoundary", "stars", None, ("_2_06", "_2_07"))

    # Test Subset
    TestPlot2("Subset", "domains", None, ("_2_08", "_2_09"))

def TestPlot3(plotName, plotVar, atts, cases):
    AddPlot(plotName, plotVar)
    if atts == None:
        atts = eval(plotName + "Attributes()")
    atts.pointType = atts.Axis
    SetPlotOptions(atts)
    DrawPlots()

    Test("renderpoint" + cases[0])

    atts.pointType = atts.Icosahedron
    atts.pointSize = 0.1
    SetPlotOptions(atts)
    Test("renderpoint" + cases[1])

    atts.pointType = atts.Octahedron
    SetPlotOptions(atts)
    Test("renderpoint" + cases[2])

    atts.pointSize = 0.05
    atts.pointType = atts.Tetrahedron
    SetPlotOptions(atts)
    Test("renderpoint" + cases[3])

    atts.pointType = atts.SphereGeometry
    SetPlotOptions(atts)
    Test("renderpoint" + cases[4])

    DeleteActivePlots()

#
# Test that all plots that support sphere point texturing can do it.
#
def test3():
    TestSection("Check glyph types for Mesh plot")
    atts = MeshAttributes()
    atts.meshColor = (152,203,0)
    atts.meshColorSource = atts.MeshCustom
    TestPlot3("Mesh", "StarMesh", atts, ("_3_00", "_3_01", "_3_02", "_3_03", "_3_04"))

    TestSection("Check glyph types for Pseudocolor plot")
    # Test Pseudocolor
    TestPlot3("Pseudocolor", "Matnos", None, ("_3_05", "_3_06", "_3_07", "_3_08", "_3_09"))

    TestSection("Check glyph types for Scatter plot")
    # Test Scatter
    atts = ScatterAttributes()
    atts.var2 = "vy"
    atts.var3 = "vz"
    atts.var3Role = atts.Coordinate2
    atts.var4 = "Matnos"
    atts.var4Role = atts.Color
    TestPlot3("Scatter", "vx", atts, ("_3_10", "_3_11", "_3_12", "_3_13", "_3_14"))

    TestSection("Check glyph types for FilledBoundary plot")
    # Test FilledBoundary
    TestPlot3("FilledBoundary", "stars", None, ("_3_15", "_3_16", "_3_17", "_3_18", "_3_19"))

    # Test Subset
    TestSection("Check all glyph types for Subset plot")
    TestPlot3("Subset", "domains", None, ("_3_20", "_3_21", "_3_22", "_3_23", "_3_24"))

def TestPlot4(plotName, plotVar, atts, case):
    AddPlot(plotName, plotVar)
    if atts == None:
        atts = eval(plotName + "Attributes()")
    atts.pointType = atts.SphereGeometry
    atts.pointSize = 0.007
    atts.pointSizeVarEnabled = 1
    atts.pointSizeVar = "Matnos"
    SetPlotOptions(atts)
    DrawPlots()

    Test("renderpoint" + case)

    DeleteActivePlots()

#
# Test that all plots that support scaling glyphs by a variable can do it.
#
def test4():
    TestSection("Check scaling glyphs by a variable")
    atts = MeshAttributes()
    atts.meshColor = (152,203,0)
    atts.meshColorSource = atts.MeshCustom
    TestPlot4("Mesh", "StarMesh", atts, "_4_00")

    # Test Pseudocolor
    TestPlot4("Pseudocolor", "Matnos", None, "_4_01")

    # Test FilledBoundary
    TestPlot4("FilledBoundary", "stars", None, "_4_02")

    # Test Subset
    TestPlot4("Subset", "domains", None, "_4_03")

#
# Test that glyphed plots can be restored from session files
#
def test5():
    TestSection("Check glyphed plots restored from session files")
    DeleteAllPlots()
    RestoreSessionWithDifferentSources(tests_path("rendering", "filledBoundaryStars.session"), 0, silo_data_path("galaxy0000.silo"))
    Test("renderpoint_5_00")

    DeleteAllPlots()
    RestoreSessionWithDifferentSources(tests_path("rendering", "meshNoisePoint.session"), 0, silo_data_path("noise.silo"))
    Test("renderpoint_5_01")

    DeleteAllPlots()
    RestoreSessionWithDifferentSources(tests_path("rendering", "pcNoisePoint.session"), 0, silo_data_path("noise.silo"))
    Test("renderpoint_5_02")

    DeleteAllPlots()
    RestoreSessionWithDifferentSources(tests_path("rendering", "scatterNoiseHG.session"), 0, silo_data_path("noise.silo"))
    Test("renderpoint_5_03")

    DeleteAllPlots()
    RestoreSessionWithDifferentSources(tests_path("rendering", "subsetNoisePoint.session"), 0, silo_data_path("noise.silo"))
    Test("renderpoint_5_04")


def main():
    # Turn off all annotation
    TurnOffAllAnnotations()

    OpenDatabase(silo_data_path("galaxy0000.silo"))

    test0()
    test1()
    test2()
    test3()
    test4()

    CloseDatabase(silo_data_path("galaxy0000.silo"))

    test5()

main()
Exit()
