# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  bov.py
#
#  Tests:      mesh      - 3D rectilinear, multiple domain
#              plots     - Pseudocolor, Subset, Label, Contour
#              operators - Slice
#
#  Programmer: Brad Whitlock
#  Date:       Fri Mar 17 14:37:45 PST 2006
#
#  Modifications:
#    Brad Whitlock, Thu May 4 14:02:29 PST 2006
#    Added testing of INT and DOUBLE BOV files.
#
# ----------------------------------------------------------------------------

def SaveTestImage(name):
    # Save these images somewhat larger than a regular test case image
    # since the images contain a lot of text.
    backup = GetSaveWindowAttributes()
    swa = SaveWindowAttributes()
    swa.width = 500
    swa.height = 500
    swa.screenCapture = 0
    Test(name, swa)
    SetSaveWindowAttributes(backup)

def TestBOVDivide(prefix, db, doSubset):
    # Take a picture to make sure that the division took. There will be
    # a lot of bricks.
    OpenDatabase(db)

    if doSubset:
        AddPlot("Subset", "bricks")
        subAtts = SubsetAttributes()
        subAtts.legendFlag = 0
        SetPlotOptions(subAtts)
    else:
        AddPlot("Pseudocolor", "myvar")
    DrawPlots()
    v = View3DAttributes()
    v.viewNormal = (0.534598, 0.40012, 0.744385)
    v.focus = (15, 15, 15)
    v.viewUp = (-0.228183, 0.916444, -0.32873)
    v.viewAngle = 30
    v.parallelScale = 8.66025
    v.nearPlane = -17.3205
    v.farPlane = 17.3205
    v.imagePan = (0, 0)
    v.imageZoom = 1
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (15, 15, 15)
    SetView3D(v)
    Test(prefix + "00")

    # Make sure there are the right number of zones.
    Query("NumZones")
    TestText(prefix + "01", GetQueryOutputString())

    # Let's slice a few times to make sure that crucial areas have the
    # right values
    AddPlot("Mesh", "mesh")
    AddPlot("Label", "myvar")
    L = LabelAttributes()
    L.textHeight1 = 0.03
    L.textHeight2 = 0.03
    SetPlotOptions(L)
    SetActivePlots((0,1,2))
    AddOperator("Slice")
    s = SliceAttributes()
    s.originType = s.Intercept  # Point, Intercept, Percent, Zone, Node
    s.originIntercept = 10.001
    s.normal = (0, 0, 1)
    s.axisType = s.ZAxis  # XAxis, YAxis, ZAxis, Arbitrary
    s.upAxis = (0, 1, 0)
    s.project2d = 1
    SetOperatorOptions(s)
    DrawPlots()
    v2 = GetView2D()
    v2.windowCoords = (12.0201, 13.0004, 9.99781, 10.9888)
    v2.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v2.fullFrameActivationMode = v2.Auto  # On, Off, Auto
    v2.fullFrameAutoThreshold = 100
    SetView2D(v2)
    SaveTestImage(prefix+"02")

    # Move to another slice on the far edge that will have the max zone #
    s.originIntercept = 19.998
    SetOperatorOptions(s)
    v3 = View2DAttributes()
    v3.windowCoords = (19.2017, 20.0179, 19.1966, 20.0217)
    v3.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v3.fullFrameActivationMode = v3.Auto  # On, Off, Auto
    v3.fullFrameAutoThreshold = 100
    SetView2D(v3)
    SaveTestImage(prefix+"03")

    # Move to another slice in the middle.
    s.originIntercept = 15.01
    SetOperatorOptions(s)
    v4 = View2DAttributes()
    v4.windowCoords = (14.6419, 15.361, 15.638, 16.365)
    v4.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v4.fullFrameActivationMode = v4.Auto  # On, Off, Auto
    v4.fullFrameAutoThreshold = 100
    SetView2D(v4)
    SaveTestImage(prefix+"04")
    DeleteAllPlots()

    # Test that ghost zones are right.
    AddPlot("Pseudocolor", "myvar")
    p = PseudocolorAttributes()
    p.opacity = 0.25
    SetPlotOptions(p)
    DrawPlots()
    v5 = View3DAttributes()
    v5.viewNormal = (0.772475, 0.402431, 0.491255)
    v5.focus = (15, 15, 15)
    v5.viewUp = (-0.355911, 0.915018, -0.18992)
    v5.viewAngle = 30
    v5.parallelScale = 8.66025
    v5.nearPlane = -17.3205
    v5.farPlane = 17.3205
    v5.imagePan = (-0.0253114, 0.0398304)
    v5.imageZoom = 1.20806
    v5.perspective = 1
    v5.eyeAngle = 2
    v5.centerOfRotationSet = 0
    v5.centerOfRotation = (15, 15, 15)
    SetView3D(v5)
    Test(prefix+"05")

    # Zoom in on a contour plot to make sure that there are no tears.
    # This means that the ghost zones were created properly.
    ClearWindow()
    p.opacity = 1.
    SetPlotOptions(p)
    AddOperator("Isosurface")
    iso = IsosurfaceAttributes()
    iso.variable = "radial"
    SetOperatorOptions(iso)
    DrawPlots()
    v6 = View3DAttributes()
    v6.viewNormal = (0.373168, 0.412282, 0.831125)
    v6.focus = (15, 15, 15)
    v6.viewUp = (-0.181836, 0.910964, -0.370244)
    v6.viewAngle = 30
    v6.parallelScale = 8.66025
    v6.nearPlane = -17.3205
    v6.farPlane = 17.3205
    v6.imagePan = (0.0994254, 0.0810457)
    v6.imageZoom = 1.94126
    v6.perspective = 1
    v6.eyeAngle = 2
    v6.centerOfRotationSet = 0
    v6.centerOfRotation = (15, 15, 15)
    SetView3D(v6)
    Test(prefix+"06")
    DeleteAllPlots()

    CloseDatabase(db)

def TestBOVType(bovtype, prefixes):
    # Test the original BOV file without it being divided.
    TestSection("Reading BOV file of %s" % bovtype)
    TestBOVDivide(prefixes[0], "../data/%s_indices.bov" % bovtype, 0)

    #
    # Test 2 BOV files that are being subdivided into smaller bricks
    # by the BOV plugin so that there are multiple domains that
    # can be processed in parallel.
    #
    TestSection("Decomposing BOV of %s into smaller bricks" % bovtype)
    TestBOVDivide(prefixes[1], "../data/%s_indices_div.bov" % bovtype, 1)

    TestSection("Decomposing BOV of %s with small header into smaller bricks" % bovtype)
    TestBOVDivide(prefixes[2], "../data/%s_indices_div_with_header.bov" % bovtype, 1)


def main():
    a = AnnotationAttributes()
    TurnOffAllAnnotations(a)
    a.axes2D.visible = 1
    SetAnnotationAttributes(a)

    # Define some expressions
    DefineScalarExpression("x", "coord(mesh)[0]")
    DefineScalarExpression("y", "coord(mesh)[1]")
    DefineScalarExpression("z", "coord(mesh)[2]")
    DefineScalarExpression("dx", "x - 15.")
    DefineScalarExpression("dy", "y - 15.")
    DefineScalarExpression("dz", "z - 15.")
    DefineScalarExpression("radial", "sqrt(dx*dx + dy*dy + dz*dz)")

    TestBOVType("FLOAT",  ("bov_0_", "bov_1_", "bov_2_"))
    TestBOVType("DOUBLE", ("bov_3_", "bov_4_", "bov_5_"))
    TestBOVType("INT",    ("bov_6_", "bov_7_", "bov_8_"))

    Exit()

main()
