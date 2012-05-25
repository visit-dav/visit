# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  xdmf.py
#
#  Programmer: Brad Whitlock
#  Date:       Tue Apr 27 10:37:01 PDT 2010
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def test0(datapath):
    TestSection("Structured (no hdf5)")
    OpenDatabase(pjoin(datapath,"3DRectMesh.xmf"))
    AddPlot("Pseudocolor", "CellScalars")
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (-0.722705, 0.363868, 0.587621)
    v.focus = (2, 1.5, 1.5)
    v.viewUp = (0.277541, 0.931424, -0.235414)
    v.viewAngle = 30
    v.parallelScale = 2.91548
    v.nearPlane = -5.83095
    v.farPlane = 5.83095
    v.imagePan = (0.0188164, 0.0435196)
    v.imageZoom = 1.09024
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (2, 1.5, 1.5)
    SetView3D(v)
    Test("xdmf_0_00")

    ChangeActivePlotsVar("NodeScalars")
    Test("xdmf_0_01")
    DeleteAllPlots()
    CloseDatabase(pjoin(datapath,"3DRectMesh.xmf"))

def test1(datapath):
    TestSection("Basic topology (no hdf5)")
    OpenDatabase(pjoin(datapath,"BasicTopology.xmf"))
    AddPlot("Pseudocolor", "Hexahedron/Cell Centered Values")
    DrawPlots()

    pc = PseudocolorAttributes(1)
    pc.pointSizePixels = 10
    pc.pointType = pc.Point
    SetPlotOptions(pc)

    v = View3DAttributes()
    v.viewNormal = (-0.722705, 0.363868, 0.587621)
    v.focus = (0.5, 0.5, 2)
    v.viewUp = (0.277541, 0.931424, -0.235414)
    v.viewAngle = 30
    v.parallelScale = 2.12132
    v.nearPlane = -4.24264
    v.farPlane = 4.24264
    v.imagePan = (0.0665142, 0.0326173)
    v.imageZoom = 1.4219
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.5, 0.5, 2)
    SetView3D(v)
    Test("xdmf_1_00")

    ChangeActivePlotsVar("Hexahedron/Node Centered Values")
    Test("xdmf_1_01")

    # Polygon
    ChangeActivePlotsVar("Polygon/Cell Centered Values")
    Test("xdmf_1_02")
    ChangeActivePlotsVar("Polygon/Node Centered Values")
    Test("xdmf_1_03")

    # Polyline
    ChangeActivePlotsVar("Polyline/Cell Centered Values")
    Test("xdmf_1_04")
    ChangeActivePlotsVar("Polyline/Node Centered Values")
    Test("xdmf_1_05")

    # Polyvertex
    ChangeActivePlotsVar("Polyvertex/Cell Centered Values")
    Test("xdmf_1_06")
    ChangeActivePlotsVar("Polyvertex/Node Centered Values")
    Test("xdmf_1_07")

    # Pyramid
    ChangeActivePlotsVar("Pyramid/Cell Centered Values")
    Test("xdmf_1_08")
    ChangeActivePlotsVar("Pyramid/Node Centered Values")
    Test("xdmf_1_09")

    # Quadrilateral
    ChangeActivePlotsVar("Quadrilateral/Cell Centered Values")
    Test("xdmf_1_10")
    ChangeActivePlotsVar("Quadrilateral/Node Centered Values")
    Test("xdmf_1_11")

    # Tetrahedron
    ChangeActivePlotsVar("Tetrahedron/Cell Centered Values")
    Test("xdmf_1_12")
    ChangeActivePlotsVar("Tetrahedron/Node Centered Values")
    Test("xdmf_1_13")

    # Triangle
    ChangeActivePlotsVar("Triangle/Cell Centered Values")
    Test("xdmf_1_14")
    ChangeActivePlotsVar("Triangle/Node Centered Values")
    Test("xdmf_1_15")

    # Wedge
    ChangeActivePlotsVar("Wedge/Cell Centered Values")
    Test("xdmf_1_16")
    ChangeActivePlotsVar("Wedge/Node Centered Values")
    Test("xdmf_1_17")

    DeleteAllPlots()
    CloseDatabase(pjoin(datapath,"BasicTopology.xmf"))

def test2(datapath):
    TestSection("Mixed (no hdf5)")
    # Open the mixed topology file
    OpenDatabase(pjoin(datapath,"Mixed.xmf"))
    DeleteAllPlots()
    AddPlot("Mesh", "Mixed")
    m = MeshAttributes()
    m.legendFlag = 1
    m.lineStyle = m.SOLID  # SOLID, DASH, DOT, DOTDASH
    m.lineWidth = 1
    m.meshColor = (255, 0, 0, 255)
    m.outlineOnlyFlag = 0
    m.errorTolerance = 0.01
    m.meshColorSource = m.MeshCustom  # Foreground, MeshCustom
    m.opaqueColorSource = m.OpaqueCustom  # Background, OpaqueCustom
    m.opaqueMode = m.Auto  # Auto, On, Off
    m.pointSize = 0.05
    m.opaqueColor = (192, 192, 192, 255)
    m.smoothingLevel = m.None  # None, Fast, High
    m.pointSizeVarEnabled = 0
    m.pointSizeVar = "default"
    m.pointType = m.Point  # Box, Axis, Icosahedron, Point, Sphere
    m.showInternal = 0
    m.pointSizePixels = 2
    m.opacity = 1
    SetPlotOptions(m)
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (-0.611582, 0.210354, 0.762705)
    v.focus = (0.5, 0.5, 2)
    v.viewUp = (0.145916, 0.97746, -0.15258)
    v.viewAngle = 30
    v.parallelScale = 2.12132
    v.nearPlane = -4.24264
    v.farPlane = 4.24264
    v.imagePan = (-0.0635688, 0.0486989)
    v.imageZoom = 1.17908
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.5, 0.5, 2)
    SetView3D(v)
    Test("xdmf_2_00")

    DeleteAllPlots()
    CloseDatabase(datapath + "Mixed.xmf")

def test3(datapath):
    TestSection("Polygon (no hdf5)")
    OpenDatabase(pjoin(datapath,"PolygonOctagon.xmf"))
    AddPlot("Mesh", "PolygonOctagon")
    DrawPlots()
    ResetView()
    Test("xdmf_3_00")

    AddPlot("Pseudocolor", "CellScalar")
    DrawPlots()
    Test("xdmf_3_01")

    ChangeActivePlotsVar("NodeScalar")
    Test("xdmf_3_02")
    DeleteAllPlots()
    CloseDatabase(pjoin(datapath,"PolygonOctagon.xmf"))

def test4(datapath):
    TestSection("Quadratic topology (no hdf5)")
    OpenDatabase(pjoin(datapath,"QuadraticTopology.xmf"))
    AddPlot("Pseudocolor", "Hexahedron_20/Node Centered Values")
    DrawPlots()
    v = View3DAttributes()
    v.viewNormal = (-0.703184, 0.393286, 0.592333)
    v.focus = (0.5, 0.5, 0.5)
    v.viewUp = (0.293186, 0.919352, -0.262361)
    v.viewAngle = 30
    v.parallelScale = 0.866025
    v.nearPlane = -1.73205
    v.farPlane = 1.73205
    v.imagePan = (0.0122989, 0.0367562)
    v.imageZoom = 1.02726
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.5, 0.5, 0.5)
    SetView3D(v)
    Test("xdmf_4_00")

    ChangeActivePlotsVar("Pyramid_13/Node Centered Values")
    Test("xdmf_4_01")

    ChangeActivePlotsVar("Quadrilateral_8/Node Centered Values")
    Test("xdmf_4_02")

    ChangeActivePlotsVar("Tetrahedron_10/Node Centered Values")
    Test("xdmf_4_03")

    ChangeActivePlotsVar("Triangle_6/Node Centered Values")
    Test("xdmf_4_04")

    ChangeActivePlotsVar("Wedge_15/Node Centered Values")
    Test("xdmf_4_05")

    DeleteAllPlots()
    CloseDatabase(pjoin(datapath,"QuadraticTopology.xmf"))

def test5(datapath):
    TestSection("Multiple grids (no hdf5)")
    OpenDatabase(pjoin(datapath,"TetrahedronMultipleGrids.xmf"))
    AddPlot("Pseudocolor", "Tetrahedron 1/CellScalar")
    AddPlot("Pseudocolor", "Tetrahedron 2/CellScalar")
    DrawPlots()
    v = View3DAttributes()
    v.viewNormal = (-0.0158313, 0.998128, -0.0590748)
    v.focus = (0, 0.5, 0)
    v.viewUp = (-0.0096669, -0.0592323, -0.998197)
    v.viewAngle = 30
    v.parallelScale = 1.5
    v.nearPlane = -3
    v.farPlane = 3
    v.imagePan = (0, 0)
    v.imageZoom = 1
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0.5, 0)
    SetView3D(v)

    Test("xdmf_5_00")

    DeleteAllPlots()
    CloseDatabase(pjoin(datapath,"TetrahedronMultipleGrids.xmf"))

def test6(datapath):
    TestSection("Structured with hdf5")
    db = pjoin(datapath,"rect2d.xmf")
    OpenDatabase(db)
    AddPlot("Mesh", "mesh")
    AddPlot("Pseudocolor", "Pressure")
    DrawPlots()
    ResetView()
    Test("xdmf_6_00")
    ChangeActivePlotsVar("VelocityX")
    Test("xdmf_6_01")

    db = pjoin(datapath,"corect2d.xmf")
    ReplaceDatabase(db)
    DrawPlots()
    ResetView()
    Test("xdmf_6_02")

    DeleteAllPlots()
    CloseDatabase(db)

    db = pjoin(datapath,"rect3d.xmf")
    OpenDatabase(db)
    AddPlot("Mesh", "mesh")
    AddPlot("Pseudocolor", "VelocityZ")
    DrawPlots()
    ResetView()
    Test("xdmf_6_03")

    db = pjoin(datapath,"corect3d.xmf")
    ReplaceDatabase(db)
    DrawPlots()
    ResetView()
    Test("xdmf_6_04")

    DeleteAllPlots()
    CloseDatabase(db)

    db = pjoin(datapath,"xdmf2d.xmf")
    OpenDatabase(db)
    AddPlot("Mesh", "mesh1")
    AddPlot("Pseudocolor", "Pressure")
    DrawPlots()
    Test("xdmf_6_05")

    ChangeActivePlotsVar("VelocityX")
    Test("xdmf_6_06")

    DeleteAllPlots()
    CloseDatabase(db)

def test7(datapath):
    TestSection("Quadratic topology (no hdf5)")
    OpenDatabase(pjoin(datapath,"singleHex24.xmf"))
    AddPlot("Mesh", "singleHex24")
    AddPlot("Pseudocolor", "GlobalNodeId")
    DrawPlots()
    v = View3DAttributes()
    v.viewNormal = (-0.690415, 0.541158, 0.480078)
    v.focus = (0, 0, 0)
    v.viewUp = (0.123212, -0.565967, 0.815169)
    v.viewAngle = 30
    v.parallelScale = 1.73205
    v.nearPlane = -3.4641
    v.farPlane = 3.4641
    v.imagePan = (0, 0.0352717)
    v.imageZoom = 1
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    SetView3D(v)

    Test("xdmf_7_00")

    DeleteAllPlots()
    CloseDatabase(pjoin(datapath,"singleHex24.xmf"))

    OpenDatabase(pjoin(datapath,"singleHex27.xmf"))
    AddPlot("Mesh", "singleHex27")
    AddPlot("Pseudocolor", "GlobalNodeId")
    DrawPlots()

    Test("xdmf_7_01")

    DeleteAllPlots()
    CloseDatabase(pjoin(datapath,"singleHex27.xmf"))

    OpenDatabase(pjoin(datapath,"singleQuad6.xmf"))
    AddPlot("Mesh", "singleQuad6")
    AddPlot("Pseudocolor", "GlobalNodeId")
    DrawPlots()

    Test("xdmf_7_02")

    DeleteAllPlots()
    CloseDatabase(pjoin(datapath,"singleQuad6.xmf"))

    OpenDatabase(pjoin(datapath,"singleQuad9.xmf"))
    AddPlot("Mesh", "singleQuad9")
    AddPlot("Pseudocolor", "GlobalNodeId")
    DrawPlots()

    Test("xdmf_7_03")

    DeleteAllPlots()
    CloseDatabase(pjoin(datapath,"singleQuad9.xmf"))

    OpenDatabase(pjoin(datapath,"singleTri7.xmf"))
    AddPlot("Mesh", "singleTri7")
    AddPlot("Pseudocolor", "GlobalNodeId")
    DrawPlots()

    Test("xdmf_7_04")

    DeleteAllPlots()
    CloseDatabase(pjoin(datapath, "singleTri7.xmf"))

    OpenDatabase(pjoin(datapath, "singleWedge12.xmf"))
    AddPlot("Mesh", "singleWedge12")
    AddPlot("Pseudocolor", "GlobalNodeId")
    DrawPlots()
    v = View3DAttributes()
    v.viewNormal = (0.69918, -0.142559, 0.700588)
    v.focus = (0, 0, 0.5)
    v.viewUp = (-0.216074, 0.89196, 0.397139)
    v.viewAngle = 30
    v.parallelScale = 1.5
    v.nearPlane = -3
    v.farPlane = 3
    v.imagePan = (0, 0)
    v.imageZoom = 1
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    SetView3D(v)

    Test("xdmf_7_05")

    DeleteAllPlots()
    CloseDatabase(pjoin(datapath,"singleWedge12.xmf"))

    OpenDatabase(pjoin(datapath,"singleWedge18.xmf"))
    AddPlot("Mesh", "singleWedge18")
    AddPlot("Pseudocolor", "GlobalNodeId")
    DrawPlots()

    Test("xdmf_7_06")

    DeleteAllPlots()
    CloseDatabase(pjoin(datapath,"singleWedge18.xmf"))

def main():
    datapath = data_path("xdmf_test_data")
    test0(datapath)
    test1(datapath)
    test2(datapath)
    test3(datapath)
    test4(datapath)
    test5(datapath)

    test6(datapath)
    test7(datapath)

main()
Exit()


#fails:
# rect2d.xmf
#              Crashes the engine cuz we don't give a Z coordinate
#
# corect2d.xml 
#              I can plot the mesh but it is in the YZ plane, which doesn't seem right.
#              Does not expose Pressure or VelocityX.
#              
# rect3d.xmf
#              Only exposes VelocityZ (where is Pressure?)
#
# corect3d.xmf
#              Does not expose Pressure or VelocityZ
