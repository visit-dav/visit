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
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Cyrus Harrison, Thu Mar 25 09:57:34 PDT 2010
#    Added call(s) to DrawPlots() b/c of changes to the default plot state
#    behavior when an operator is added.
#
#    Kathleen Biagas, Fri Jul 25 15:10:11 MST 2014
#    Added ArbPoly test, which demonstrates use of new att: honorOriginalMesh.
#
#    Kathleen Biagas, Mon Mar 23 17:25:07 PDT 2015
#    Modified AMR test to include the default Subset plot, now that it
#    is fixed.
#
# ----------------------------------------------------------------------------

def TestBigSil():
    OpenDatabase(silo_data_path("bigsil.silo"))


    AddPlot("Subset", "domains")
    AddPlot("Subset", "blocks")
    s = SubsetAttributes()
    s.legendFlag = 0
    s.wireframe = 1
    s.colorType = s.ColorBySingleColor
    s.singleColor = (100,100,100,255)
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
    DrawPlots()
    
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
    OpenDatabase(silo_data_path("multi_ucd3d.silo"))

    AddPlot("Pseudocolor", "d")
    domains = ("domain4", "domain5", "domain6")
    TurnDomainsOff()
    TurnDomainsOn(domains)
    DrawPlots()
    ResetView()
    SetViewExtentsType("actual")
    AddOperator("OnionPeel")
    DrawPlots()
    Test("ops_onionpeel_04")

    DeleteAllPlots()

def TestGlobalZoneId():
    # This is to test that onion peel will work when 
    # a global cell number is specified. 
    OpenDatabase(silo_data_path("global_node.silo"))

    TurnDomainsOn()
    AddPlot("Subset", "domains")
    s = SubsetAttributes()
    s.legendFlag = 0
    s.wireframe = 1
    s.colorType = s.ColorBySingleColor
    s.singleColor = (100,100,100,255)
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
    OpenDatabase(silo_data_path("rect3d.silo"))

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

    OpenDatabase(silo_data_path("globe.silo"))

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
    OpenDatabase(silo_data_path("ucd2d.silo"))

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
    OpenDatabase(silo_data_path("rect3d.silo"))

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
    OpenDatabase(silo_data_path("rect3d.silo"))

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

    OpenDatabase(silo_data_path("globe.silo"))

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

    OpenDatabase(silo_data_path("curv3d.silo"))

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
    OpenDatabase(silo_data_path("rect3d.silo"))

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

    OpenDatabase(silo_data_path("globe.silo"))

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

    OpenDatabase(silo_data_path("curv3d.silo"))

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

def TestArbPoly():
    TestSection("Arbitrary Polyhedra, honor original mesh")
    OpenDatabase(silo_data_path("arbpoly-zoohybrid.silo"))
    DefineScalarExpression("gzid", "global_zoneid(<3D/mesh1>)")
    AddPlot("Mesh", "3D/mesh1")
    AddPlot("Pseudocolor", "gzid")
    pc = PseudocolorAttributes()
    pc.colorTableName="levels"
    SetPlotOptions(pc)
    AddOperator("OnionPeel")
    DrawPlots()
    ResetView()
    op = OnionPeelAttributes()

    SetQueryOutputToObject()
    # there are only 11 zones in original mesh
    # when NumZonesQuery is fixed, can use it instead
    # SetActivePlots(0)
    # numZones = Query("NumZones", use_actual_data=0)['num_zones']
    # SetActivePlots(1)
    numZones = 11 
    for i in range(numZones):
        op.index = i
        SetOperatorOptions(op)
        DrawPlots()
        Test("poly_originalMesh_%02d"%i)

    TestSection("Arbitrary Polyhedra, honor actual mesh")
    op.honorOriginalMesh = 0
    SetActivePlots(0)
    numZones = Query("NumZones", use_actual_data=1)['num_zones']
    print("numZones: ", numZones)
    SetActivePlots(1)
    for i in range (numZones):
        op.index = i
        SetOperatorOptions(op)
        DrawPlots()
        Test("poly_actualMesh%02d"%i)

    DeleteAllPlots()

def TestBigSilMesh():
    # use actual spatial extents
    SetViewExtentsType(1)

    # non-logical index
    OpenDatabase(silo_data_path("bigsil.silo"))
    AddPlot("Mesh", "mesh")
    mesh = MeshAttributes()
    mesh.showInternal = 1
    SetPlotOptions(mesh)

    AddPlot("Label", "mesh")
    label = LabelAttributes()
    label.showNodes = 1
    label.showCells = 1
    label.drawLabelsFacing = label.FrontAndBack
    label.labelDisplayFormat = label.Index
    label.specifyTextColor1 = 1
    label.textColor1 = (255, 0, 0, 0)
    label.specifyTextColor2 = 1
    label.textColor2 = (0, 0, 255, 0)
    label.depthTestMode = label.LABEL_DT_NEVER
    SetPlotOptions(label)

    AddOperator("OnionPeel", 1)
    op = OnionPeelAttributes()
    op.categoryName = "domains"
    op.subsetName = "domain14"
    op.logical = 0
    op.index = (211)
    op.requestedLayer = 0
    op.seedType = op.SeedNode
    SetOperatorOptions(op,0, 1);
    DrawPlots()
    Test("bigsil_mesh_01")

    op.seedType = op.SeedCell
    op.index = (223)
    op.requestedLayer = 1
    SetOperatorOptions(op,0, 1);
    DrawPlots()
    Test("bigsil_mesh_02")
   
    # Blocks/Domains, logical index
    op.logical = 1
    op.index = (3, 2, 3)
    op.requestedLayer = 0
    op.seedType = op.SeedNode
    SetOperatorOptions(op,0, 1);
    DrawPlots()
    Test("bigsil_mesh_03")

    op.seedType = op.SeedCell
    op.index = (6, 3, 4)
    op.requestedLayer = 1
    SetOperatorOptions(op,0, 1);
    DrawPlots()
    Test("bigsil_mesh_04")

    DeleteAllPlots()
    CloseDatabase(silo_data_path("bigsil.silo"))
    # use original spatial extents
    SetViewExtentsType(0)

def TestAMR():
    # AMR Mesh
    OpenDatabase(data_path("samrai_test_data/sil_changes/dumps.visit"))
    # use actual spatial extents
    SetViewExtentsType(1)

    AddPlot("Mesh", "amr_mesh")
    mesh = MeshAttributes()
    mesh.showInternal = 1
    SetPlotOptions(mesh)

    AddPlot("Label", "amr_mesh")
    label = LabelAttributes()
    label.showNodes = 1
    label.showCells = 1
    label.drawLabelsFacing = label.FrontAndBack
    label.labelDisplayFormat = label.Index
    label.specifyTextColor1 = 1
    label.textColor1 = (255, 0, 0, 0)
    label.specifyTextColor2 = 1
    label.textColor2 = (0, 0, 255, 0)
    label.depthTestMode = label.LABEL_DT_NEVER
    SetPlotOptions(label)

    AddOperator("OnionPeel", 1)
    op = OnionPeelAttributes()
    op.categoryName = "levels"
    op.subsetName = "level2"
    op.logical = 1
    op.index = (11, 15, 8)
    op.seedType = op.SeedNode
    SetOperatorOptions(op,0, 1);
    DrawPlots()
    Test("amr_3d_01")

    op.seedType = op.SeedCell
    op.index = (10, 15, 8)
    op.requestedLayer = 1
    SetOperatorOptions(op,0, 1);
    Test("amr_3d_02")

    DeleteAllPlots()
    CloseDatabase(data_path("samrai_test_data/sil_changes/dumps.visit"))

    # Test Vector plot
    OpenDatabase(data_path("samrai_test_data/mats-par3/dumps.visit"))

    AddPlot("Mesh", "amr_mesh")
    AddPlot("Vector", "Velocity")
    TimeSliderNextState()
    AddOperator("OnionPeel", 1)
    op = OnionPeelAttributes()
    op.categoryName = "levels"
    op.subsetName = "level1"
    op.logical = 1
    op.index = (12, 15, 10)
    op.seedType = op.SeedCell
    op.requestedLayer = 1
    SetOperatorOptions(op,0, 1);
    DrawPlots()
    Test("amr_3d_03")

    op.seedType = op.SeedNode
    op.index = (13, 15, 10)
    op.requestedLayer = 0
    SetOperatorOptions(op,0, 1);
    Test("amr_3d_04")
   
    DeleteAllPlots()
    CloseDatabase(data_path("samrai_test_data/mats-par3/dumps.visit"))

    # 2D AMR Mesh
    OpenDatabase(data_path("samrai_test_data/ale2d/dumps.visit"))

    # use actual spatial extents
    SetViewExtentsType(1)

    AddPlot("Mesh", "amr_mesh")
    AddPlot("Contour", "Density")

    AddOperator("OnionPeel", 1)
    op = OnionPeelAttributes()
    op.categoryName = "levels"
    op.subsetName = "level2"
    op.logical = 1
    op.index = (384, 490)
    op.seedType = op.SeedCell
    op.requestedLayer = 1 
    SetOperatorOptions(op,0, 1);
    DrawPlots()
    Test("amr_2d_01")

    DeleteAllPlots()
    CloseDatabase(data_path("samrai_test_data/ale2d/dumps.visit"))

    OpenDatabase(data_path("AMRStitchCell_test_data/AMRStitchCellTest.1.no_ghost.phi.2d.hdf5"))
    AddPlot("Subset", "levels")
    AddPlot("Mesh", "Mesh")
    AddPlot("Contour", "phi")
    AddPlot("Label", "Mesh")
    label = LabelAttributes()
    label.showNodes = 1
    label.showCells = 1
    label.drawLabelsFacing = label.FrontAndBack
    label.labelDisplayFormat = label.Index
    label.specifyTextColor1 = 1
    label.textColor1 = (255, 0, 0, 0)
    label.specifyTextColor2 = 1
    label.textColor2 = (0, 0, 255, 0)
    label.depthTestMode = label.LABEL_DT_NEVER
    SetPlotOptions(label)

    AddOperator("OnionPeel", 1)
    op = OnionPeelAttributes()
    op.categoryName = "levels"
    op.subsetName = "level2"
    op.logical = 1
    op.seedType = op.SeedCell
    op.requestedLayer = 1
    op.index = (24, 35)
    SetOperatorOptions(op, 0, 1)
    DrawPlots()
    Test("amr_2d_02")
    # use original spatial extents
    SetViewExtentsType(0)

def Main():
    TestBigSil()
    TestUCD()
    TestGlobalZoneId()
    TestMatSelect()
    TestNodeId()
    TestFilledBoundary()
    TestBoundary()
    TestArbPoly()
    TestBigSilMesh()
    TestAMR()

Main()
Exit()
