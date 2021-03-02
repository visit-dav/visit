# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  CGNS.py
#
#  Tests:      mesh      - 3D structured, unstructured, single and 
#                          multiple domain
#              plots     - Contour, Mesh, Pseudocolor, Subset
#              operators - Clip, Slice
#
#  Programmer: Brad Whitlock
#  Date:       Fri Sep 2 11:19:33 PDT 2005
#
#  Modifications:
#    Brad Whitlock, Mon Apr 21 10:55:14 PDT 2008
#    Updated variable names to compensate for changes to CGNS reader.
#
#    Jeremy Meredith, Fri Aug  8 11:23:29 EDT 2008
#    Updated streamline settings to match new attribute fields.
#
#    Cyrus Harrison, Thu Mar 25 09:57:34 PDT 2010
#    Added call(s) to DrawPlots() b/c of changes to the default plot state
#    behavior when an operator is added.
#
#    Cyrus Harrison, Wed Aug 25 14:28:14 PDT 2010
#    Variable name change due to changes with SIL generation.
#
#    Brad Whitlock, Tue Sep 11 14:38:37 PDT 2012
#    Test file that has boundaries.
#
#    Alister Maguire, Tue Mar  2 10:29:37 PST 2021
#    Added tests for NGon and NFace elements.
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("CGNS")

def test0(datapath):
    TestSection("Structured files")

    OpenDatabase(pjoin(datapath,"delta.cgns"))
    AddPlot("Mesh", "Zone___1")
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

    OpenDatabase(pjoin(datapath,"5blocks.cgns"))
    AddPlot("Subset", "zones")
    AddPlot("Mesh", "BASE_1")
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

    OpenDatabase(pjoin(datapath,"multi.cgns"))
    AddPlot("Subset", "zones")
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

    DeleteAllPlots()


def test1(datapath):
    TestSection("Unstructured files")

    OpenDatabase(pjoin(datapath,"StaticMixer.cgns"))
    AddPlot("Mesh", "StaticMixer")
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
    DrawPlots()
    Test("CGNS_1_02")
    DeleteAllPlots()

    OpenDatabase(pjoin(datapath,"jet.cgns"))
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

    OpenDatabase(pjoin(datapath,"yf17.cgns"))
    AddPlot("Mesh", "Zone1")
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

    OpenDatabase(pjoin(datapath,"HeatingCoil.cgns"))
    AddPlot("Subset", "zones(Base)")
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
    AddPlot("Mesh", "subgrid/Base001")
    DrawPlots()
    Test("CGNS_2_04")
    DeleteAllPlots()

    # Draw the 1 variable that exists in both domains.
    AddPlot("Pseudocolor","Temperature")
    DrawPlots()
    Test("CGNS_2_05")
    DeleteAllPlots()

def test3(datapath):
    TestSection("File with boundaries")

    OpenDatabase(pjoin(datapath,"with_boundary_faces.cgns"))
    AddPlot("Pseudocolor", "Density")
    DrawPlots()
    ResetView()
    v = GetView2D()          
    v.fullFrameActivationMode = v.On
    SetView2D(v)

    Test("CGNS_3_00")
    DeleteAllPlots()

def testNGonElements(datapath):
    TestSection("Arbitrary polygons")

    OpenDatabase(pjoin(datapath, "spherePolyMesh.cgns"))
    AddPlot("Mesh", "sphere")
    DrawPlots()
    ResetView()

    Test("CGNS_NGon_Elements_00")
    DeleteAllPlots()
    CloseDatabase(pjoin(datapath, "spherePolyMesh.cgns"))

def testNFaceElements(datapath):
    TestSection("Arbitrary polyhedra")

    #
    # First, test the simple case. This mesh has 2 sections,
    # one for NGon and one for NFace.
    #
    OpenDatabase(pjoin(datapath, "trappedvtx_ngon.cgns"))
    AddPlot("Mesh", "ComputeBase")
    MeshAtts = MeshAttributes()
    DrawPlots()
    ResetView()

    Test("CGNS_NFace_Elements_00")
    DeleteAllPlots()
    CloseDatabase(pjoin(datapath, "trappedvtx_ngon.cgns"))

    #
    # Next, test a more complicated case. This mesh has 5 sections,
    # 4 of which are NGon. The single NFace section references all
    # 4 NGon sections.
    #
    OpenDatabase(pjoin(datapath, "spherePolyMesh3D.cgns"))
    AddPlot("Mesh", "sphere")
    MeshAtts = MeshAttributes()
    MeshAtts.opacity = 0.4
    SetPlotOptions(MeshAtts)

    AddPlot("Pseudocolor", "Density")
    PseudocolorAtts = PseudocolorAttributes()
    PseudocolorAtts.opacityType = PseudocolorAtts.Constant
    PseudocolorAtts.opacity = 0.4
    SetPlotOptions(PseudocolorAtts)

    DrawPlots()
    ResetView()

    Test("CGNS_NFace_Elements_01")
    DeleteAllPlots()
    CloseDatabase(pjoin(datapath, "spherePolyMesh3D.cgns"))

def main():
    # Draw antialiased lines
    r = GetRenderingAttributes()
    r.antialiasing = 1
    SetRenderingAttributes(r)

    datapath = data_path("CGNS_test_data")
    test0(datapath)
    test1(datapath)
    test2(datapath)
    test3(datapath)
    testNGonElements(datapath)
    testNFaceElements(datapath)

main()
Exit()
