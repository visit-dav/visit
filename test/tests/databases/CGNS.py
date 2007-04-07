# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  CGNS.py
#
#  Tests:      mesh      - 3D structured, unstructured, single and 
#                          multiple domain
#              plots     - Contour, Mesh, Pseudocolor, Streamline, Subset
#              operators - Clip, Slice
#
#  Programmer: Brad Whitlock
#  Date:       Fri Sep 2 11:19:33 PDT 2005
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def test0(datapath):
    TestSection("Structured files")

    OpenDatabase(datapath + "delta.cgns")
    AddPlot("Mesh", "grid")
    DrawPlots()
    v0 = View3DAttributes()
    v0.viewNormal = (-0.673597, -0.70478, 0.222605)
    v0.focus = (0.5, 0.827196, 0.0589651)
    v0.viewUp = (0.150257, 0.164313, 0.974897)
    v0.viewAngle = 30
    v0.parallelScale = 2.30431
    v0.nearPlane = -4.60862
    v0.farPlane = 4.60862
    v0.imagePan = (0.0171482, 0.0494526)
    v0.imageZoom = 1.09899
    v0.perspective = 1
    v0.eyeAngle = 2
    v0.centerOfRotationSet = 0
    v0.centerOfRotation = (0.5, 0.827196, 0.0589651)
    SetView3D(v0)
    Test("CGNS_0_00")

    DeleteAllPlots()
    AddPlot("Pseudocolor", "Density")
    DrawPlots()
    Test("CGNS_0_01")

    ChangeActivePlotsVar("MomentumX")
    Test("CGNS_0_02")

    AddPlot("Vector", "Momentum")
    v = VectorAttributes()
    v.nVectors = 4000
    v.scale = 0.065
    v.scaleByMagnitude = 0
    v.autoScale = 0
    SetPlotOptions(v)
    DrawPlots()
    v1 = View3DAttributes()
    v1.viewNormal = (-0.59445, 0.745055, 0.302528)
    v1.focus = (0.5, 0.827196, 0.0589651)
    v1.viewUp = (0.177264, -0.245548, 0.953039)
    v1.viewAngle = 30
    v1.parallelScale = 2.30431
    v1.nearPlane = 0
    v1.farPlane = 4.60862
    v1.imagePan = (-0.00797807, 0.021628)
    v1.imageZoom = 2.76008
    v1.perspective = 1
    v1.eyeAngle = 2
    v1.centerOfRotationSet = 0
    v1.centerOfRotation = (0.5, 0.827196, 0.0589651)
    SetView3D(v1)
    Test("CGNS_0_03")
    DeleteAllPlots()

    OpenDatabase(datapath + "5blocks.cgns")
    AddPlot("Subset", "domains")
    AddPlot("Mesh", "grid")
    DrawPlots()
    v2 = View3DAttributes()
    v2.viewNormal = (-0.56714, 0.62864, 0.532131)
    v2.focus = (18.3432, -132.647, 108.59)
    v2.viewUp = (0.353654, -0.397621, 0.846656)
    v2.viewAngle = 30
    v2.parallelScale = 390.48
    v2.nearPlane = -780.96
    v2.farPlane = 780.96
    v2.imagePan = (0.0358739, 0.018554)
    v2.imageZoom = 1.2581
    v2.perspective = 1
    v2.eyeAngle = 2
    v2.centerOfRotationSet = 0
    v2.centerOfRotation = (18.3432, -132.647, 108.59)
    SetView3D(v2)
    Test("CGNS_0_04")
    DeleteAllPlots()

    OpenDatabase(datapath + "multi.cgns")
    AddPlot("Subset", "domains")
    AddOperator("Slice")
    DrawPlots()
    v3 = View2DAttributes()
    v3.windowCoords = (-0.101847, 1.39507, -0.777335, 0.779882)
    v3.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v3.fullFrameActivationMode = v3.Off  # On, Off, Auto
    v3.fullFrameAutoThreshold = 100
    SetView2D(v3)
    Test("CGNS_0_05")
    DeleteAllPlots()

    AddPlot("Pseudocolor", "MomentumZ")
    AddOperator("Slice")
    DrawPlots()
    Test("CGNS_0_06")

    AddPlot("Streamline", "Momentum")
    AddOperator("Slice")
    s = StreamlineAttributes()
    s.sourceType = s.SpecifiedLine
    s.lineStart = (-0.0331738, -0.822069, 0)
    s.lineEnd = (-0.5, 1, 0.)
    s.stepLength = 0.01
    s.maxTime = 1000
    s.pointDensity = 30
    s.coloringMethod = s.Solid
    s.legendFlag = 0
    SetPlotOptions(s)
    DrawPlots()
    Test("CGNS_0_07")
    DeleteAllPlots()


def test1(datapath):
    TestSection("Unstructured files")

    OpenDatabase(datapath + "StaticMixer.cgns")
    AddPlot("Mesh", "grid")
    AddPlot("Pseudocolor", "Pressure")
    DrawPlots()
    v0 = View3DAttributes()
    v0.viewNormal = (-0.791423, -0.34473, 0.504788)
    v0.focus = (0, 0, 6.65045e-11)
    v0.viewUp = (0.453682, 0.222176, 0.863024)
    v0.viewAngle = 30
    v0.parallelScale = 4.1231
    v0.nearPlane = -8.24621
    v0.farPlane = 8.24621
    v0.imagePan = (-0.00919414, -0.0394234)
    v0.imageZoom = 1.4092
    v0.perspective = 1
    v0.eyeAngle = 2
    v0.centerOfRotationSet = 0
    v0.centerOfRotation = (0, 0, 0)
    SetView3D(v0)
    Test("CGNS_1_00")

    ChangeActivePlotsVar("VelocityX")
    Test("CGNS_1_01")

    SetActivePlots((0,1))
    AddOperator("Clip")
    c = ClipAttributes()
    c.plane2Status = 1
    c.plane1Normal = (-1.,0.,0.)
    c.plane2Normal = (0.,-1.,0.)
    SetOperatorOptions(c)
    Test("CGNS_1_02")
    DeleteAllPlots()

    OpenDatabase(datapath + "jet.cgns")
    AddPlot("Pseudocolor", "CoefPressure")
    AddPlot("Contour", "CoefPressure")
    c = ContourAttributes()
    c.colorType = c.ColorBySingleColor
    c.contourNLevels = 30
    c.singleColor=(0,0,0,255)
    c.legendFlag = 0
    SetPlotOptions(c)
    DrawPlots()
    v1 = View3DAttributes()
    v1.viewNormal = (0.89973, 0.23533, 0.367566)
    v1.focus = (1.82694, 0, 4.12744)
    v1.viewUp = (-0.354779, -0.0961553, 0.929992)
    v1.viewAngle = 30
    v1.parallelScale = 47.9514
    v1.nearPlane = -95.9031
    v1.farPlane = 95.9031
    v1.imagePan = (-0.0544089, 0.00587157)
    v1.imageZoom = 1.62963
    v1.perspective = 1
    v1.eyeAngle = 2
    v1.centerOfRotationSet = 0
    v1.centerOfRotation = (1.82694, 0, 4.12744)
    SetView3D(v1)
    Test("CGNS_1_03")
    DeleteAllPlots()

    OpenDatabase(datapath + "yf17.cgns")
    AddPlot("Mesh", "grid")
    AddPlot("Pseudocolor", "Density")
    DrawPlots()
    v2 = View3DAttributes()
    v2.viewNormal = (-0.79815, 0.489358, 0.351405)
    v2.focus = (0, 31.7504, 0)
    v2.viewUp = (0.286761, -0.204385, 0.935946)
    v2.viewAngle = 30
    v2.parallelScale = 95.2512
    v2.nearPlane = 0
    v2.farPlane = 190.502
    v2.imagePan = (-0.129267, -0.0352136)
    v2.imageZoom = 15.4243
    v2.perspective = 1
    v2.eyeAngle = 2
    v2.centerOfRotationSet = 0
    v2.centerOfRotation = (0, 31.7504, 0)
    SetView3D(v2)
    Test("CGNS_1_04")

    v3 = View3DAttributes()
    v3.viewNormal = (-0.79815, 0.489358, 0.351405)
    v3.focus = (0, 31.7504, 0)
    v3.viewUp = (0.286761, -0.204385, 0.935946)
    v3.viewAngle = 30
    v3.parallelScale = 95.2512
    v3.nearPlane = 0
    v3.farPlane = 190.502
    v3.imagePan = (-0.130871, -0.033492)
    v3.imageZoom = 66.7586
    v3.perspective = 1
    v3.eyeAngle = 2
    v3.centerOfRotationSet = 0
    v3.centerOfRotation = (0, 31.7504, 0)
    SetView3D(v3)
    Test("CGNS_1_05")

    SetActivePlots(0)
    DeleteActivePlots()
    SetView3D(v2)
    Test("CGNS_1_06")

    AddPlot("Vector", "Velocity")
    v = VectorAttributes()
    v.nVectors = 4000
    v.scale = 0.2
    v.scaleByMagnitude = 0
    v.autoScale = 0
    SetPlotOptions(v)
    DrawPlots()
    Test("CGNS_1_07")
    DeleteAllPlots()
    

def test2(datapath):
    TestSection("Variables on only some of the domains")

    OpenDatabase(datapath + "HeatingCoil.cgns")
    AddPlot("Subset", "domains(grid)")
    DrawPlots()
    v0 = View3DAttributes()
    v0.viewNormal = (0.27344, 0.876709, 0.395743)
    v0.focus = (0, 0, 1.125)
    v0.viewUp = (-0.0932919, -0.385311, 0.918059)
    v0.viewAngle = 30
    v0.parallelScale = 1.8071
    v0.nearPlane = -3.61421
    v0.farPlane = 3.61421
    v0.imagePan = (-0.00158217, 0.0267093)
    v0.imageZoom = 1.11866
    v0.perspective = 1
    v0.eyeAngle = 2
    v0.centerOfRotationSet = 0
    v0.centerOfRotation = (0, 0, 1.125)
    SetView3D(v0)
    Test("CGNS_2_00")

    TurnDomainsOff("FluidZone")
    Test("CGNS_2_01")

    TurnDomainsOn()
    TurnDomainsOff("SolidZone")
    Test("CGNS_2_02")
    TurnDomainsOn()
    DeleteAllPlots()

    # Make sure we can plot variables that exist on only 1 domain.
    AddPlot("Pseudocolor", "Pressure")
    DrawPlots()
    Test("CGNS_2_03")
    ChangeActivePlotsVar("VelocityZ")
    AddPlot("Mesh", "subgrid/grid001")
    DrawPlots()
    Test("CGNS_2_04")
    DeleteAllPlots()
    
    # Draw the 1 variable that exists in both domains.
    AddPlot("Pseudocolor","Temperature")
    DrawPlots()
    Test("CGNS_2_05")
    DeleteAllPlots()


def main():
    a = GetAnnotationAttributes()
    a.databaseInfoFlag = 0
    a. userInfoFlag = 0
    a.axesFlag = 0
    a.bboxFlag = 0
    SetAnnotationAttributes(a)

    # Draw antialiased lines
    r = GetRenderingAttributes()
    r.antialiasing = 1
    SetRenderingAttributes(r)

    datapath = "../data/CGNS_test_data/"
    test0(datapath)
    test1(datapath)
    test2(datapath)

main()
Exit()
