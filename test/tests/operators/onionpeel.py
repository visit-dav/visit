# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  onionpeel.py
#
#  Tests:      mesh      - 3D rectilinear, mulitple domain, mulitple groups
#              plots     - subset
#              operators - onionpeel
#              selection - none
#
#  Defect ID:  
#
#  Programmer: Kathleen Bonnell 
#  Date:       December 23, 2003 
#
#  Modifications:
#    Kathleen Bonnell, Thu Feb 26 15:31:31 PST 2004
#    Added TestUCD.
#
#    Kathleen Bonnell, Wed Dec 15 15:24:50 PST 2004 
#    Added TestGlobalZoneId.
#
#    Kathleen Bonnell, Tue Jan 18 19:34:41 PST 2005 
#    Added TestMatSelect.
#
#    Kathleen Bonnell, Wed Jan 19 15:45:38 PST 2005 
#    Added TestNodeId.
#
#    Kathleen Bonnell, Tue Jun 14 11:33:39 PDT 2005 
#    Added TestFilledBoundary.
#
#    Kathleen Bonnell, Thu Jul 21 09:09:26 PDT 2005 
#    Modified indices used in TestBigSil, to match corrsponding changes
#    to bigsil.silo. 
#
#    Kathleen Bonnell, Fri Aug 19 16:23:45 PDT 2005 
#    Modified indices used in TestBigSil, more changes to bigsil.silo. 
#
#    Kathleen Bonnell, Thu Sep 22 11:59:42 PDT 2005 
#    Added TestBoundary. 
#
# ----------------------------------------------------------------------------

def InitAnnotations():
    a = AnnotationAttributes()
    a.axesFlag2D = 0
    a.axesFlag = 0
    a.triadFlag = 0
    a.bboxFlag = 0
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    a.backgroundMode = 0
    a.foregroundColor = (0, 0, 0, 255)
    a.backgroundColor = (255, 255, 255, 255)
    SetAnnotationAttributes(a)

def TestBigSil():
    OpenDatabase("../data/bigsil.silo")

    AddPlot("Subset", "domains")
    AddPlot("Subset", "blocks")
    DrawPlots()
    s = SubsetAttributes()
    s.legendFlag = 0
    s.wireframe = 1
    SetPlotOptions(s)

    SetActivePlots(0)
    AddOperator("OnionPeel")
    op = OnionPeelAttributes()
    op.categoryName = "blocks"
    op.subsetName = "block0"
    op.index = (4, 11, 5)
    op.logical = 1
    op.adjacencyType = op.Face
    op.requestedLayer = 3
    SetOperatorOptions(op)

    Test("ops_onionpeel_01")

    op.subsetName = "block1"
    op.index = (18, 4, 5)
    SetOperatorOptions(op)

    Test("ops_onionpeel_02")

    op.subsetName = "block2"
    op.index = (11, 18, 5)
    SetOperatorOptions(op)

    Test("ops_onionpeel_03")

    DeleteAllPlots()

def TestUCD():
    # This is to test that onion peel will work (by default)
    # on the first "On" set -- in this case "domain4".
    OpenDatabase("../data/multi_ucd3d.silo")
    AddPlot("Pseudocolor", "d")
    domains = ("domain4", "domain5", "domain6")
    TurnDomainsOff()
    TurnDomainsOn(domains)
    DrawPlots()
    ResetView()
    SetViewExtentsType("actual")
    AddOperator("OnionPeel")
    Test("ops_onionpeel_04")

    DeleteAllPlots()

def TestGlobalZoneId():
    # This is to test that onion peel will work when 
    # a global cell number is specified. 
    OpenDatabase("../data/global_node.silo")
    TurnDomainsOn()
    AddPlot("Subset", "domains")
    s = SubsetAttributes()
    s.legendFlag = 0
    s.wireframe = 1
    SetPlotOptions(s)
    AddPlot("Pseudocolor", "dist")
    AddOperator("OnionPeel")
    op = OnionPeelAttributes()
    op.categoryName = "domains"
    op.subsetName = "domain5"
    op.index = 372
    op.logical = 0
    op.adjacencyType = op.Face
    op.requestedLayer = 7
    SetOperatorOptions(op)

    DrawPlots()
    ResetView()
    SetViewExtentsType("original")

    Test("ops_onionpeel_05")

    #only change is to specify that seed cell is global
    op.useGlobalId = 1
    SetOperatorOptions(op)
    Test("ops_onionpeel_06")

    DeleteAllPlots()

def TestMatSelect():
    OpenDatabase("../data/rect3d.silo")
    AddPlot("FilledBoundary", "mat1")
    AddOperator("OnionPeel")
    op = OnionPeelAttributes()
    op.index = 8
    op.requestedLayer = 5
    op.adjacencyType = op.Face
    SetOperatorOptions(op)
    DrawPlots()
    SetViewExtentsType("actual")
    Test("ops_onionpeel_07")

    TurnMaterialsOff("1")
    Test("ops_onionpeel_08")

    DeleteAllPlots()

    OpenDatabase("../data/globe.silo")
    AddPlot("Pseudocolor", "u")
    TurnMaterialsOff("4")
    AddOperator("OnionPeel")
    op.requestedLayer = 2
    op.index = 27
    SetOperatorOptions(op)
    DrawPlots()
    Test("ops_onionpeel_09")
    TurnMaterialsOn()
    DeleteAllPlots()

def TestNodeId():
    OpenDatabase("../data/ucd2d.silo")
    AddPlot("Mesh", "ucdmesh2d")
    AddPlot("Pseudocolor", "d")
    AddOperator("OnionPeel")
    op = OnionPeelAttributes()
    op.index = 0
    op.seedType = op.SeedCell
    SetOperatorOptions(op)
    DrawPlots()
    Test("ops_onionpeel_10")

    op.seedType = op.SeedNode
    SetOperatorOptions(op)
    Test("ops_onionpeel_11")

    op.requestedLayer = 2
    op.adjacencyType = op.Face
    SetOperatorOptions(op)
    Test("ops_onionpeel_12")

    DeleteAllPlots()
    OpenDatabase("../data/rect3d.silo")
    AddPlot("Mesh", "quadmesh3d")
    AddPlot("Pseudocolor", "d")
    SetActivePlots((0, 1))
    TurnMaterialsOff("1")
    AddOperator("OnionPeel")
    op.index = 44
    op.requestedLayer = 0
    SetOperatorOptions(op)
    DrawPlots()
    SetViewExtentsType("actual")
    v = GetView3D()
    v.viewNormal = (0, 0, -1)
    SetView3D(v)
    Test("ops_onionpeel_13")
    op.requestedLayer = 12
    SetOperatorOptions(op)
    Test("ops_onionpeel_14")
    TurnMaterialsOn()
    DeleteAllPlots()

def TestFilledBoundary():
    # need to test index on high-side
    OpenDatabase("../data/rect3d.silo")
    AddPlot("Mesh", "quadmesh3d")
    AddPlot("FilledBoundary", "mat1")
    SetActivePlots((0,1))
    AddOperator("OnionPeel")
    op = OnionPeelAttributes()
    op.index = 15735
    op.requestedLayer = 5
    op.adjacencyType = op.Face
    SetOperatorOptions(op)
    DrawPlots()
    ResetView()
    SetViewExtentsType("actual")
    Test("ops_onionpeel_15")

    op.seedType = op.SeedNode
    op.index = 32713
    SetOperatorOptions(op)
    Test("ops_onionpeel_16")
    DeleteAllPlots()

    OpenDatabase("../data/globe.silo")
    AddPlot("Mesh", "mesh1")
    AddPlot("FilledBoundary", "mat1")
    SetActivePlots((0,1))
    AddOperator("OnionPeel")
    op.seedType = op.SeedCell
    op.requestedLayer = 2
    op.index = 742 
    SetOperatorOptions(op)
    DrawPlots()
    Test("ops_onionpeel_17")
    op.index = 1117 
    SetOperatorOptions(op)
    DrawPlots()
    ResetView()
    Test("ops_onionpeel_18")

    op.seedType = op.SeedNode
    op.requestedLayer = 6
    op.index = 965
    SetOperatorOptions(op)
    Test("ops_onionpeel_19")
    DeleteAllPlots()

    OpenDatabase("../data/curv3d.silo")
    AddPlot("Mesh", "curvmesh3d")
    AddPlot("FilledBoundary", "mat1")
    SetActivePlots((0, 1))
    AddOperator("OnionPeel")
    op.seedType = op.SeedCell
    op.requestedLayer = 5
    op.index = 19412
    SetOperatorOptions(op)
    DrawPlots()
    ResetView()
    v = GetView3D()
    v.viewNormal = (0, 0, 1)
    v.focus = (-2.29977, 0.966088, 16.5)
    v.viewUp = (0, 1, 0)
    v.viewAngle = 30
    v.parallelScale = 5.61745
    v.nearPlane = -11.2349
    v.farPlane = 11.2349
    v.imagePan = (-0.00484124, 0.00104964)
    v.imageZoom = 4.08886
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (-2.29977, 0.966088, 16.5)
    SetView3D(v)
    Test("ops_onionpeel_20")
    DeleteAllPlots()


def TestBoundary():
    # need to test index on high-side
    OpenDatabase("../data/rect3d.silo")
    AddPlot("Mesh", "quadmesh3d")
    AddPlot("Boundary", "mat1")
    SetActivePlots((0,1))
    AddOperator("OnionPeel")
    op = OnionPeelAttributes()
    op.index = 15735
    op.requestedLayer = 5
    op.adjacencyType = op.Face
    SetOperatorOptions(op)
    DrawPlots()
    ResetView()
    SetViewExtentsType("actual")
    Test("ops_onionpeel_21")

    op.seedType = op.SeedNode
    op.index = 1370
    SetOperatorOptions(op)
    Test("ops_onionpeel_22")
    DeleteAllPlots()

    OpenDatabase("../data/globe.silo")
    AddPlot("Mesh", "mesh1")
    AddPlot("Boundary", "mat1")
    SetActivePlots((0,1))
    AddOperator("OnionPeel")
    op.seedType = op.SeedCell
    op.requestedLayer = 2
    op.index = 17 
    SetOperatorOptions(op)
    DrawPlots()
    ResetView()
    v = GetView3D()
    v.viewNormal = (1, 0, 0)
    v.viewUp = (0, 1, 0)
    SetView3D(v) 
    Test("ops_onionpeel_23")
    op.index = 117 
    SetOperatorOptions(op)
    DrawPlots()
    ResetView()
    Test("ops_onionpeel_24")

    op.seedType = op.SeedNode
    op.requestedLayer = 6
    op.index = 13
    SetOperatorOptions(op)
    Test("ops_onionpeel_25")
    DeleteAllPlots()

    OpenDatabase("../data/curv3d.silo")
    AddPlot("Mesh", "curvmesh3d")
    AddPlot("Boundary", "mat1")
    SetActivePlots((0, 1))
    AddOperator("OnionPeel")
    op.seedType = op.SeedCell
    op.requestedLayer = 5
    op.index = 19472
    SetOperatorOptions(op)
    DrawPlots()
    ResetView()
    v = GetView3D()
    v.viewNormal = (0, 0, 1)
    v.focus = (-2.29977, 0.966088, 16.5)
    v.viewUp = (0, 1, 0)
    v.viewAngle = 30
    v.parallelScale = 5.61745
    v.nearPlane = -11.2349
    v.farPlane = 11.2349
    v.imagePan = (-0.00484124, 0.00104964)
    v.imageZoom = 4.08886
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (-2.29977, 0.966088, 16.5)
    SetView3D(v)
    Test("ops_onionpeel_26")
    DeleteAllPlots()


def Main():
    InitAnnotations()
    TestBigSil()
    TestUCD()
    TestGlobalZoneId()
    TestMatSelect()
    TestNodeId()
    TestFilledBoundary()
    TestBoundary()

Main()
Exit()
