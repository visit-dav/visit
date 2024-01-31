# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  FMS.py
#
#  Tests:      mesh      - high order meshes
#              plots     - Mesh, Pseudocolor
#
#  Programmer: Brad Whitlock
#  Date:       Tue Aug  4 11:25:39 PDT 2020
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def FilterMetaData(s):
    lines = s.split("\n")
    txt = ""
    ignores=("exprList.","#expressions",".enum","simInfo.","blockNameScheme",".missingData","DataExtents =",".rectilinearGrid",".unitCell")
    for line in lines:
        keep = True
        for ig in ignores:
             if ig in line:
                 keep = False
                 break
        if keep:
            txt = txt + (line + "\n")
    return txt

def test_mesh_plus_dofs(prefix, db1, order):
    OpenDatabase(db1)

    # Get the metadata
    md = GetMetaData(db1)
    TestText(prefix+"_00", FilterMetaData(str(md)))

    # Show the mesh (curved)
    AddPlot("Mesh", "mesh")
    m = MeshAttributes()
    m.lineWidth = 2
    SetPlotOptions(m)
    if order != "one":
        AddOperator("MultiresControl")
        mra = MultiresControlAttributes()
        mra.resolution = 10
        mra.maxResolution = 100
        SetOperatorOptions(mra)
    DrawPlots()
    delta = 0.03
    v0 = GetView2D()
    v0.windowCoords = (0.-delta, 1.+delta, 0.-delta, 1.+delta)
    v0.viewportCoords = (0.04, 0.98, 0.03, 0.98)
    v0.fullFrameActivationMode = v0.Auto  # On, Off, Auto
    v0.fullFrameAutoThreshold = 100
    v0.xScale = v0.LINEAR  # LINEAR, LOG
    v0.yScale = v0.LINEAR  # LINEAR, LOG
    v0.windowValid = 1
    SetView2D(v0)
    Test(prefix + "_01")

    # Overlay the dofs
    DefineScalarExpression("X", "coords(dofs)[0]")
    AddPlot("Pseudocolor", "X")
    pc = PseudocolorAttributes()
    pc.pointSizePixels = 12
    pc.colorTableName = "hot_desaturated"
    SetPlotOptions(pc)
    AddOperator("Project")
    AddOperator("Transform")
    tform = TransformAttributes(1)
    tform.doTranslate = 1
    tform.translateZ = 0.01
    SetOperatorOptions(tform)
    DrawPlots()
    Test(prefix + "_02")
    DeleteActivePlots()

    AddPlot("Pseudocolor", "zid")
    SetPlotOptions(pc)
    DrawPlots()
    Test(prefix + "_03")

def domain_test(datapath, prefix, protocol, order):
    db1 = pjoin(datapath,"domains_"+protocol+"_order_"+order+".fms")
    test_mesh_plus_dofs(prefix, db1, order)
    # Cleanup
    DeleteAllPlots()
    CloseDatabase(db1)
    CloseComputeEngine()

def quads_test(datapath, prefix, protocol, order):
    db1 = pjoin(datapath,"quads_"+protocol+"_order_"+order+".fms")
    test_mesh_plus_dofs(prefix, db1, order)

    ChangeActivePlotsVar("r1")
    DrawPlots()
    Test(prefix + "_04")

    ChangeActivePlotsVar("r2")
    DrawPlots()
    Test(prefix + "_05")

    ChangeActivePlotsVar("r3")
    DrawPlots()
    Test(prefix + "_06")

    # Cleanup
    DeleteAllPlots()
    CloseDatabase(db1)
    CloseComputeEngine()

def hex_test(datapath, prefix, protocol, order):
    db1 = pjoin(datapath,"hex_"+protocol+"_order_"+order+".fms")
    OpenDatabase(db1)

    # Get the metadata
    md = GetMetaData(db1)
    TestText(prefix+"_00", FilterMetaData(str(md)))

    # Show the mesh (curved)
    AddPlot("Mesh", "mesh")
    m = MeshAttributes()
    m.lineWidth = 2
    SetPlotOptions(m)
    if order != "one":
        AddOperator("MultiresControl")
        mra = MultiresControlAttributes()
        mra.resolution = 7
        mra.maxResolution = 100
        SetOperatorOptions(mra)
    DrawPlots()
    v0 = GetView3D()
    v0.viewNormal = (-0.736972, 0.363225, -0.570035)
    v0.focus = (0.499999, 0.500005, 0.499787)
    v0.viewUp = (0.289138, 0.931697, 0.219863)
    v0.viewAngle = 30
    v0.parallelScale = 0.900499
    v0.nearPlane = -1.801
    v0.farPlane = 1.801
    v0.imagePan = (-0.00987149, 0.0367767)
    v0.imageZoom = 1.15249
    v0.perspective = 1
    v0.eyeAngle = 2
    v0.centerOfRotationSet = 0
    v0.centerOfRotation = (0.499999, 0.500005, 0.499787)
    v0.axis3DScaleFlag = 0
    v0.axis3DScales = (1, 1, 1)
    v0.shear = (0, 0, 1)
    v0.windowValid = 1
    SetView3D(v0)
    Test(prefix + "_01")

    AddPlot("Pseudocolor", "zid")
    pc = PseudocolorAttributes()
    pc.colorTableName = "hot_desaturated"
    SetPlotOptions(pc)
    DrawPlots()
    Test(prefix + "_02")

    ChangeActivePlotsVar("r1")
    Test(prefix + "_03")

    ChangeActivePlotsVar("r2")
    Test(prefix + "_04")

    # Check that order 3 interior dofs look ok.
    AddPlot("Contour", "r3")
    c = ContourAttributes(1)
    c.contourValue = (0.6, 0.8, 0.9, 1, 1.2)
    c.contourMethod = c.Value  # Level, Value, Percent
    SetPlotOptions(c)    
    SetActivePlots((0,1)) # Delete mesh,pc plots
    DeleteActivePlots()
    DrawPlots()   
    Test(prefix + "_05")

    # Cleanup
    DeleteAllPlots()
    CloseDatabase(db1)
    CloseComputeEngine()

def test0(datapath):
    protocol = "ascii"
    TestSection("domains - " + protocol)
    domain_test(datapath, "FMS_0_1", protocol, "one")
    domain_test(datapath, "FMS_0_2", protocol, "two")
    domain_test(datapath, "FMS_0_3", protocol, "three")

def test1(datapath):
    protocol = "yaml"
    TestSection("domains - " + protocol)
    domain_test(datapath, "FMS_1_1", protocol, "one")
    domain_test(datapath, "FMS_1_2", protocol, "two")
    domain_test(datapath, "FMS_1_3", protocol, "three")

def test2(datapath):
    protocol = "json"
    TestSection("domains - " + protocol)
    domain_test(datapath, "FMS_2_1", protocol, "one")
    domain_test(datapath, "FMS_2_2", protocol, "two")
    domain_test(datapath, "FMS_2_3", protocol, "three")

def test3(datapath):
    protocol = "hdf5"
    TestSection("domains - " + protocol)
    domain_test(datapath, "FMS_3_1", protocol, "one")
    domain_test(datapath, "FMS_3_2", protocol, "two")
    domain_test(datapath, "FMS_3_3", protocol, "three")

def test4(datapath):
    protocol = "ascii"
    TestSection("quads - " + protocol)
    quads_test(datapath, "FMS_4_1", protocol, "one")
    quads_test(datapath, "FMS_4_2", protocol, "two")
    quads_test(datapath, "FMS_4_3", protocol, "three")

def test5(datapath):
    protocol = "yaml"
    TestSection("quads - " + protocol)
    quads_test(datapath, "FMS_5_1", protocol, "one")
    quads_test(datapath, "FMS_5_2", protocol, "two")
    quads_test(datapath, "FMS_5_3", protocol, "three")

def test6(datapath):
    protocol = "ascii"
    TestSection("hex - " + protocol)
    hex_test(datapath, "FMS_6_1", protocol, "one")
    hex_test(datapath, "FMS_6_2", protocol, "two")
    hex_test(datapath, "FMS_6_3", protocol, "three")
    hex_test(datapath, "FMS_6_4", protocol, "four")
    hex_test(datapath, "FMS_6_5", protocol, "five")

def test7(datapath):
    protocol = "hdf5"
    TestSection("hex - " + protocol)
    hex_test(datapath, "FMS_7_1", protocol, "one")
    hex_test(datapath, "FMS_7_2", protocol, "two")
    hex_test(datapath, "FMS_7_3", protocol, "three")
    hex_test(datapath, "FMS_7_4", protocol, "four")
    hex_test(datapath, "FMS_7_5", protocol, "five")

def test8(datapath):
    TestSection("hex - time varying")
    prefix = "FMS_8_"
    db1 = pjoin(datapath,"hex*.fms database")
    OpenDatabase(db1)

    # Get the metadata
    md = GetMetaData(db1)
    TestText(prefix+"_00", FilterMetaData(str(md)))

    # Show the mesh (curved)
    AddPlot("Mesh", "mesh")
    m = MeshAttributes()
    m.lineWidth = 2
    SetPlotOptions(m)
    AddOperator("MultiresControl")
    mra = MultiresControlAttributes()
    mra.resolution = 7
    mra.maxResolution = 100
    SetOperatorOptions(mra)
    DrawPlots()
    v0 = GetView3D()
    v0.viewNormal = (-0.722008, 0.525589, -0.449957)
    v0.focus = (0.500085, 0.501459, 0.49554)
    v0.viewUp = (0.465249, 0.850161, 0.246515)
    v0.viewAngle = 30
    v0.parallelScale = 0.989501
    v0.nearPlane = -1.979
    v0.farPlane = 1.979
    v0.imagePan = (0, 0)
    v0.imageZoom = 1.12555
    v0.perspective = 1
    v0.eyeAngle = 2
    v0.centerOfRotationSet = 0
    v0.centerOfRotation = (0.500085, 0.501459, 0.49554)
    v0.axis3DScaleFlag = 0
    v0.axis3DScales = (1, 1, 1)
    v0.shear = (0, 0, 1)
    v0.windowValid = 1
    SetView3D(v0)

    AddPlot("Pseudocolor", "r3")
    pc = PseudocolorAttributes()
    pc.colorTableName = "hot_desaturated"
    SetPlotOptions(pc)
    DrawPlots()
    Test(prefix + "_01")

    SetTimeSliderState(9)
    Test(prefix + "_02")

    # Cleanup
    DeleteAllPlots()
    CloseDatabase(db1)
    CloseComputeEngine()

def test9(datapath):
    TestSection("Root file")
    prefix = "FMS_9_"
    db1 = pjoin(datapath,"multidom.fms_root")
    OpenDatabase(db1)

    # Get the metadata
    md = GetMetaData(db1)
    TestText(prefix+"_00", FilterMetaData(str(md)))

    # Show the mesh (curved)
    AddPlot("Mesh", "mesh")
    m = MeshAttributes()
    m.lineWidth = 2
    SetPlotOptions(m)
    DrawPlots()
    AddPlot("Pseudocolor", "coords_magnitude")
    pc = PseudocolorAttributes()
    pc.colorTableName = "hot_desaturated"
    SetPlotOptions(pc)
    DrawPlots()
    ResetView()
    Test(prefix + "_01")

    DeleteActivePlots()
    AddPlot("Subset", "domains")
    DrawPlots()
    Test(prefix + "_02")

    # Cleanup
    DeleteAllPlots()
    CloseDatabase(db1)
    CloseComputeEngine()

def plot_converted_data(prefix, db, var, v0, resolution, meshName, dodof):
    OpenDatabase(db)

    # Get the metadata
    md = GetMetaData(db)
    TestText(prefix+"_00", FilterMetaData(str(md)))

    AddPlot("Pseudocolor", var)
    pc = PseudocolorAttributes()
    pc.colorTableName = "hot_desaturated"
    SetPlotOptions(pc)
    AddOperator("MultiresControl")
    mra = MultiresControlAttributes()
    mra.resolution = resolution
    mra.maxResolution = 100
    SetOperatorOptions(mra)
    DrawPlots()

    # Try setting the view.
    try:
        SetView3D(v0)
    except:
        try:
            SetView2D(v0)
        except:
            ResetView()


    Test(prefix + "_01")

    # Add a mesh plot of the boundaries and refine them.
    if meshName != "":
        AddPlot("Mesh", meshName, 1, 1)
        m = MeshAttributes(1)
        m.lineWidth = 1
        SetPlotOptions(m)
        DrawPlots()
        Test(prefix + "_02")

    # Add a mesh plot of the dofs.
    if dodof:
        AddPlot("Mesh", "dofs", 0, 0)
        m2 = MeshAttributes(1)
        m2.meshColor = (255, 0, 0, 255)
        m2.meshColorSource = m2.MeshCustom  # Foreground, MeshCustom, MeshRandom
        m2.pointSize = 0.075
        m2.opaqueColor = (255, 255, 255, 255)
        m2.pointType = m2.SphereGeometry  # Box, Axis, Icosahedron, Octahedron, Tetrahedron, SphereGeometry, Point, Sphere
        m2.pointSizePixels = 10
        m2.opacity = 1
        SetPlotOptions(m2)
        DrawPlots()
        Test(prefix + "_03")

    DeleteAllPlots()
    CloseDatabase(db)
    CloseComputeEngine()

def escher_view():
    v0 = GetView3D()
    v0.viewNormal = (0.301943, 0.349059, 0.887124)
    v0.focus = (0.015155, 0.00931501, 0.00220501)
    v0.viewUp = (-0.108572, 0.937093, -0.331767)
    v0.viewAngle = 30
    v0.parallelScale = 2.69081
    v0.nearPlane = -5.38163
    v0.farPlane = 5.38163
    v0.imagePan = (-0.011426, -0.0216873)
    v0.imageZoom = 1.62652
    v0.perspective = 1
    v0.eyeAngle = 2
    v0.centerOfRotationSet = 0
    v0.centerOfRotation = (0.015155, 0.00931501, 0.00220501)
    v0.axis3DScaleFlag = 0
    v0.axis3DScales = (1, 1, 1)
    v0.shear = (0, 0, 1)
    v0.windowValid = 1
    return v0

def test10(datapath):
    TestSection("Converted MFEM data: escher-p3-0ref")
    db = pjoin(datapath,"converted-data/escher-p3-0ref/Example5_000000.fms")

    v0 = escher_view()
    plot_converted_data("FMS_10", db, "pressure", v0, 5, "boundary", True)

def test11(datapath):
    TestSection("Converted MFEM data: escher-p3")
    db = pjoin(datapath,"converted-data/escher-p3/Example5_000000.fms")

    v0 = escher_view()
    plot_converted_data("FMS_11", db, "pressure", v0, 4, "boundary", False)

def test12(datapath):
    TestSection("Converted MFEM data: Example15")
    db = pjoin(datapath,"converted-data/Example15/Example15_000000.fms")

    v0 = GetView2D()
    v0.windowCoords = (-1.74594, 1.76718, -1.7, 1.55366)
    v0.viewportCoords = (0.01, 0.99, 0.01, 0.99)
    v0.fullFrameActivationMode = v0.Auto  # On, Off, Auto
    v0.fullFrameAutoThreshold = 100
    v0.xScale = v0.LINEAR  # LINEAR, LOG
    v0.yScale = v0.LINEAR  # LINEAR, LOG
    v0.windowValid = 1

    plot_converted_data("FMS_12", db, "solution", v0, 8, "boundary", True)

def test13(datapath):
    TestSection("Converted MFEM data: Example9")
    db = pjoin(datapath,"converted-data/Example9/Example9_000000.fms")

    v0 = GetView2D()
    v0.windowCoords = (-1, 1, -0.966025, 0.866025)
    v0.viewportCoords = (0.01, 0.99, 0.01, 0.99)
    v0.fullFrameActivationMode = v0.Auto  # On, Off, Auto
    v0.fullFrameAutoThreshold = 100
    v0.xScale = v0.LINEAR  # LINEAR, LOG
    v0.yScale = v0.LINEAR  # LINEAR, LOG
    v0.windowValid = 1

    plot_converted_data("FMS_13", db, "solution", v0, 8, "", False)

def fichera_view():
    v0 = GetView3D()
    v0.viewNormal = (0.76587, 0.438546, -0.470235)
    v0.focus = (0.00682861, 0.00298607, -0.00561833)
    v0.viewUp = (-0.368491, 0.898664, 0.237945)
    v0.viewAngle = 30
    v0.parallelScale = 1.82615
    v0.nearPlane = -3.6523
    v0.farPlane = 3.6523
    v0.imagePan = (0.00522255, 0.0292194)
    v0.imageZoom = 1.09494
    v0.perspective = 1
    v0.eyeAngle = 2
    v0.centerOfRotationSet = 0
    v0.centerOfRotation = (0.00682861, 0.00298607, -0.00561833)
    v0.axis3DScaleFlag = 0
    v0.axis3DScales = (1, 1, 1)
    v0.shear = (0, 0, 1)
    v0.windowValid = 1
    return v0

def test14(datapath):
    TestSection("Converted MFEM data: fichera-q2-0ref")
    db = pjoin(datapath,"converted-data/fichera-q2-0ref/Example5_000000.fms")

    v0 = fichera_view()
    plot_converted_data("FMS_14", db, "pressure", v0, 8, "boundary", True)

def test15(datapath):
    TestSection("Converted MFEM data: fichera-q2")
    db = pjoin(datapath,"converted-data/fichera-q2/Example5_000000.fms")

    v0 = fichera_view()
    plot_converted_data("FMS_15", db, "pressure", v0, 4, "boundary", False)

def test16(datapath):
    TestSection("Converted MFEM data: star-q3-0ref")
    db = pjoin(datapath,"converted-data/star-q3-0ref/Example5_000000.fms")

    v0 = GetView2D()
    v0.windowCoords = (-1.8181, 1.8181, -1.7, 1.58418)
    v0.viewportCoords = (0.01, 0.99, 0.01, 0.99)
    v0.fullFrameActivationMode = v0.Auto  # On, Off, Auto
    v0.fullFrameAutoThreshold = 100
    v0.xScale = v0.LINEAR  # LINEAR, LOG
    v0.yScale = v0.LINEAR  # LINEAR, LOG
    v0.windowValid = 1

    plot_converted_data("FMS_16", db, "pressure", v0, 8, "mesh", True)

def test17(datapath):
    TestSection("Converted MFEM data: star-q3")
    db = pjoin(datapath,"converted-data/star-q3/Example5_000000.fms")

    v0 = GetView2D()
    v0.windowCoords = (-1.8181, 1.8181, -1.7, 1.58418)
    v0.viewportCoords = (0.01, 0.99, 0.01, 0.99)
    v0.fullFrameActivationMode = v0.Auto  # On, Off, Auto
    v0.fullFrameAutoThreshold = 100
    v0.xScale = v0.LINEAR  # LINEAR, LOG
    v0.yScale = v0.LINEAR  # LINEAR, LOG
    v0.windowValid = 1

    plot_converted_data("FMS_17", db, "pressure", v0, 4, "boundary", False)

def toroid_view():
    v0 = GetView3D()
    v0.viewNormal = (0, 0, 1)
    v0.focus = (-0.0503261, 0, 0)
    v0.viewUp = (0, 1, 0)
    v0.viewAngle = 30
    v0.parallelScale = 1.96586
    v0.nearPlane = -3.93172
    v0.farPlane = 3.93172
    v0.imagePan = (0, 0)
    v0.imageZoom = 1.29234
    v0.perspective = 1
    v0.eyeAngle = 2
    v0.centerOfRotationSet = 0
    v0.centerOfRotation = (-0.0503261, 0, 0)
    v0.axis3DScaleFlag = 0
    v0.axis3DScales = (1, 1, 1)
    v0.shear = (0, 0, 1)
    v0.windowValid = 1
    return v0

def test18(datapath):
    TestSection("Converted MFEM data: toroid-hex-0ref")
    db = pjoin(datapath,"converted-data/toroid-hex-0ref/Example5_000000.fms")

    v0 = toroid_view()
    plot_converted_data("FMS_18", db, "pressure", v0, 8, "mesh", True)

def test19(datapath):
    TestSection("Converted MFEM data: toroid-hex")
    db = pjoin(datapath,"converted-data/toroid-hex/Example5_000000.fms")

    v0 = toroid_view()
    plot_converted_data("FMS_19", db, "pressure", v0, 4, "boundary", False)

def main():
    RequiredDatabasePlugin("FMS")

    datapath = data_path("FMS_test_data")
    DefineScalarExpression("zid", "zoneid(mesh)")

    # domains
    test0(datapath)
    test1(datapath)
    test2(datapath)
    test3(datapath)
    # quads
    test4(datapath)
    test5(datapath)
    # hex
    test6(datapath)
    test7(datapath)
    # time varying data
    test8(datapath)
    # root
    test9(datapath)
    # Datasets that were converted from MFEM to FMS
    test10(datapath)
    test11(datapath)
    test12(datapath)
    test13(datapath)
    test14(datapath)
    test15(datapath)
    test16(datapath)
    test17(datapath)
    test18(datapath)
    test19(datapath)

main()
Exit()
