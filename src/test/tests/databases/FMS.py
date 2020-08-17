# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  FMS.py
#
#  Tests:      mesh      - high order meshes
#              plots     - Mesh, Pseudocolor, FilledBoundary
#
#  Programmer: Brad Whitlock
#  Date:       Tue Aug  4 11:25:39 PDT 2020
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def FilterMetaData(s):
    lines = string.split(s, "\n")
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

main()
Exit()
