# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  legends.py
#
#  Tests:      mesh      - 3D unstructured, single domain 
#              plots     - subset, boundary, filledboundary
#              operators - none
#              selection - material
#
#  Defect ID:  VisIt00002740, VisIt00002737
#
#  Programmer: Kathleen Bonnell 
#  Date:       December 2, 2002 
#
#  Modifications:
#    Brad Whitlock, Thu Dec 12 09:50:31 PDT 2002
#    I fixed the test so it uses the new interface for the SubsetAttributes.
#
#    Kathleen Bonnell, Fri Jul 18 14:04:19 PDT 2003 
#    I added tests for Boundary, FilledBoundary.
#
#    Eric Brugger, Mon Jul 21 12:14:52 PDT 2003
#    I added legend sizing and positioning tests.
#
#    Kathleen Bonnell, Thu Aug 28 14:34:57 PDT 2003
#    Remove compound var name from Subset, Boundary and Filled Boundary plots.
#
#    Kathleen Bonnell, Fri Oct 28 10:00:30 PDT 2005 
#    Add tests for curve plots (legends_09 ... legends_11). 
#
#    Kathleen Bonnell, Fri Oct 28 15:54:37 PDT 2005 
#    Add more tests for curve plots, for testing reading of TIME 
#    (legends_12 & legends_13). 
#
#    Brad Whitlock, Tue Nov 21 10:54:18 PDT 2006
#    I made it use line style enum values instead of ints so the intent
#    is more clear.
#
#    Brad Whitlock, Mon Mar 26 08:54:40 PDT 2007
#    Organized different parts of the test into functions and added a new
#    function that sets the properties for a legend.
#
# ----------------------------------------------------------------------------

# Test the Subset plot with some subsets turned off, and single-color on.
# This test ensures that correct labels are applied to the legend.
def TestLevelsLegend(a):
    TestSection("Test levels legend")
    OpenDatabase("../data/globe.silo")
 
    AddPlot("Subset", "mat1")
    TurnMaterialsOff(("2", "4"))
    subAtts = SubsetAttributes()
    subAtts.colorType = subAtts.ColorBySingleColor
    subAtts.singleColor = (0, 255, 255, 255)
    SetPlotOptions(subAtts)
    DrawPlots()

    Test("legends_01")
    DeleteAllPlots()

    # Test the FilledBoundary and Boundary plots, to ensure that setting
    # their atts works.
    AddPlot("FilledBoundary", "mat1")
    fba = FilledBoundaryAttributes()
    fba.colorType = fba.ColorByMultipleColors
    SetPlotOptions(fba)
    DrawPlots()
    Test("legends_02")
    DeleteAllPlots()

    AddPlot("Boundary", "mat1")
    ba = BoundaryAttributes()
    ba.colorType = ba.ColorByColorTable
    ba.colorTableName = "rainbow"
    SetPlotOptions(ba)
    DrawPlots()
    Test("legends_03")
    DeleteAllPlots()

#
# Test legend sizing and positioning.
#
def TestSizeAndPosition(a):
    TestSection("Test legend default sizing and positioning")
    OpenDatabase("../data/curv2d.silo")

    AddPlot("Boundary", "mat1")
    bndAtts = BoundaryAttributes()
    bndAtts.colorType = bndAtts.ColorBySingleColor
    bndAtts.singleColor = (0, 0, 0, 255)
    SetPlotOptions(bndAtts)
    AddPlot("Contour", "p")
    AddPlot("Mesh", "curvmesh2d")
    AddPlot("FilledBoundary", "mat1")
    DrawPlots()
    Test("legends_04")
    DeleteAllPlots()

    AddPlot("Pseudocolor", "d")
    AddPlot("Vector", "vel")
    AddPlot("Subset", "mat1")
    DrawPlots()
    Test("legends_05")
    DeleteAllPlots()

    AddPlot("Surface", "d")
    AddPlot("Surface", "p")
    DrawPlots()
    Test("legends_06")
    DeleteAllPlots()

    OpenDatabase("../data/globe.silo")

    AddPlot("Volume", "u")
    DrawPlots()
    Test("legends_07")
    DeleteAllPlots()

    OpenDatabase("../data/multi_ucd3d.silo")

    AddPlot("Contour", "d")
    contourAtts = ContourAttributes()
    contourAtts.contourNLevels = 15
    SetPlotOptions(contourAtts)
    AddPlot("FilledBoundary", "domains")
    DrawPlots()
    Test("legends_08")
    DeleteAllPlots()

#
# TEST LEGEND FOR CURVE PLOTS
#
def TestCurveLegend(a):
    TestSection("Test Curve plot legend")
    OpenDatabase("../data/c033.curve")
    # Test legend on
    AddPlot("Curve", "parabolic")
    curveAtts = CurveAttributes()
    curveAtts.color = (255, 0, 0, 255)
    curveAtts.lineStyle = curveAtts.SOLID  # was 4 -- invalid but evals to SOLID
    curveAtts.lineWidth = 1
    SetPlotOptions(curveAtts)
    DrawPlots()
    Test("legends_09")

    # Test legend off
    curveAtts.showLegend = 0
    SetPlotOptions(curveAtts)
    Test("legends_10")

    curveAtts.showLegend = 1
    SetPlotOptions(curveAtts)

    # Tests multiple plots
    OpenDatabase("../data/c044.curve")
    AddPlot("Curve", "parabolic")
    curveAtts.color = (0, 255, 0, 255)
    curveAtts.lineStyle = curveAtts.DOT  # was 2
    curveAtts.lineWidth = 5
    SetPlotOptions(curveAtts)
    DrawPlots()

    OpenDatabase("../data/c055.curve")
    AddPlot("Curve", "parabolic")
    curveAtts.color = (0, 0, 255, 255)
    curveAtts.lineStyle = curveAtts.DOTDASH # was 3
    curveAtts.lineWidth = 2
    SetPlotOptions(curveAtts)
    DrawPlots()

    Test("legends_11")

    # Add DatabaseInfo
    a.databaseInfoFlag = 1
    SetAnnotationAttributes(a)
    Test("legends_12")

    DeleteAllPlots()

    OpenDatabase("../data/distribution.ultra")
    AddPlot("Curve", "Laplace Distribution")
    DrawPlots()
    Test("legends_13")
    DeleteAllPlots()

#
# Test setting legend properties. Note that we currently just test the
# avtVariableLegend but others work pretty much the same way.
#
def TestLegendProperties(a):
    TestSection("Test setting legend properties")
    OpenDatabase("../data/noise.silo")
    AddPlot("Pseudocolor", "hardyglobal")
    DrawPlots()
    v0 = View3DAttributes()
    v0.viewNormal = (-0.778207, 0.3577, 0.516183)
    v0.focus = (0, 0, 0)
    v0.viewUp = (0.283417, 0.933512, -0.219613)
    v0.viewAngle = 30
    v0.parallelScale = 17.3205
    v0.nearPlane = -34.641
    v0.farPlane = 34.641
    v0.imagePan = (0.0768749, 0.057219)
    v0.imageZoom = 0.863307
    v0.perspective = 1
    v0.eyeAngle = 2
    v0.centerOfRotationSet = 0
    v0.centerOfRotation = (0, 0, 0)
    SetView3D(v0)
    Test("legends_14")

    # Get the plot's legend
    legend = GetAnnotationObject(GetPlotList().GetPlots(0).plotName)

    # See if we can scale the legend.
    legend.xScale = 3.
    Test("legends_15")
    legend.yScale = 3.
    Test("legends_16")

    # Test the bounding box.
    legend.drawBoundingBox = 1
    Test("legends_17")
    legend.boundingBoxColor = (180,180,180,230)
    Test("legends_18")

    # Test moving the legend
    legend.managePosition = 0
    legend.position = (0.55,0.9)
    Test("legends_19")

    # Test text color
    InvertBackgroundColor()
    Test("legends_20")
    InvertBackgroundColor()
    legend.useForegroundForTextColor = 0
    legend.textColor = (255, 0, 0, 255)
    Test("legends_21")

    # Test number format
    legend.numberFormat = "%1.4e"
    Test("legends_22")

    # Test the font.
    legend.fontFamily = legend.Courier
    Test("legends_23")
    legend.fontFamily = legend.Times
    Test("legends_24")
    legend.fontFamily = legend.Arial
    legend.fontBold = 1
    Test("legends_25")
    legend.fontBold = 0
    legend.fontItalic = 1
    Test("legends_26")

    # Test turning off the labels.
    legend.fontItalic = 0
    legend.drawLabels = 0
    Test("legends_27")

    # Test turning off the title.
    legend.drawTitle = 0
    Test("legends_28")

    # Add a plot and then delete plot 0 to see that the legend disappears
    # in the list of annotation objects. Note that plot names are created
    # using an increasing integer. If this test is executed out of the order
    # from when it was baselined then the number will change and the test
    # will need to be rebaselined.
    text = "Before: " + str(GetAnnotationObjectNames()) + "\n"
    AddPlot("Mesh", "Mesh")
    DrawPlots()
    SetActivePlots(0)
    DeleteActivePlots()
    text = text + "After: " + str(GetAnnotationObjectNames()) + "\n"
    TestText("legends_29", text)
    DeleteAllPlots()

#
# Test how legends get copied to new windows.
#
def TestLegendCopying(a):
    TestSection("Test legend copying")
    OpenDatabase("../data/noise.silo")
    AddPlot("Pseudocolor", "hardyglobal")
    DrawPlots()

    # Customize the legend.
    legend = GetAnnotationObject(GetPlotList().GetPlots(0).plotName)
    legend.xScale = 3.
    legend.yScale = 3.
    legend.drawBoundingBox = 1
    legend.boundingBoxColor = (50,50,100,255)

    # Create another annotation object.
    text2d = CreateAnnotationObject("Text2D", "text_obj")
    text2d.position = (0.45, 0.5)
    text2d.width = 0.5
    text2d.textColor = (255, 0, 0, 255)
    text2d.useForegroundForTextColor = 0
    text2d.text = "Text annotation"
    Test("legends_30")

    # Clone the window and make sure that it has the right annotation objects
    # and that their properties have been inherited from window 1.
    CloneWindow()
    SetActiveWindow(2)
    DrawPlots()
    Test("legends_31")
    DeleteWindow()

    # Test clone on first reference.
    SetCloneWindowOnFirstRef(1)
    AddWindow()
    DrawPlots()
    Test("legends_32")
    TestText("legends_33", str(GetAnnotationObjectNames()))

    # Test it clone on first reference again via SetActiveWindow
    DeleteWindow()
    SetWindowLayout(2)
    SetActiveWindow(2)
    DrawPlots()
    Test("legends_34")
    TestText("legends_35", str(GetAnnotationObjectNames()))

    # Now that we're in window 2, delete the text object.
    w2text = GetAnnotationObject("text_obj")
    w2text.Delete()
    # Customize the legend in window 2 so we'll know if copying window 1's
    # attributes over to window 2 messed it up.
    legend2 = GetAnnotationObject(GetPlotList().GetPlots(0).plotName)
    legend2.boundingBoxColor = (200,0,0,255)
    Test("legends_36")
    CopyAnnotationsToWindow(1, 2)
    RedrawWindow()
    Test("legends_37")

    # Clean up
    DeleteAllPlots()
    DeleteWindow()
    text2d.Delete()
    DeleteAllPlots()


def main():
    # Turn off all annotation except the legend.
    a = AnnotationAttributes()
    TurnOffAllAnnotations(a)
    a.legendInfoFlag = 1
    SetAnnotationAttributes(a)

    TestLevelsLegend(a)
    TestSizeAndPosition(a)
    TestCurveLegend(a)
    TestLegendProperties(a)
    TestLegendCopying(a)

    # reset DatabaseInfo for future tests.
    a.databaseInfoFlag = 0
    SetAnnotationAttributes(a)

main()
Exit()
