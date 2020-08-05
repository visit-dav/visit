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
    ignores=("exprList.","#expressions",".enum","simInfo.","blockNameScheme")
    for line in lines:
        keep = True
        for ig in ignores:
             if ig in line:
                 keep = False
                 break
        if keep:
            txt = txt + (line + "\n")
    return txt

def domain_test(datapath, prefix, protocol, order):
    db1 = pjoin(datapath,"domains_"+protocol+"_order_"+order+".fms")
    db2 = pjoin(datapath,"domains_"+protocol+"_order_"+order+".3D")
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
    OpenDatabase(db2)
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
    CloseDatabase(db2)

    AddPlot("Pseudocolor", "zid")
    SetPlotOptions(pc)
    DrawPlots()
    Test(prefix + "_03")

    # FUTURE: show some scalars...

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

def main():
    datapath = data_path("FMS_test_data")
    DefineScalarExpression("zid", "zoneid(mesh)")
    test0(datapath)
    test1(datapath)
    test2(datapath)
    test3(datapath)

main()
Exit()
