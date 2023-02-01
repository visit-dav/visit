# ----------------------------------------------------------------------------
#  CLASSES: nightly
#  LIMIT: 900
#  Test Case:  mfem.py 
#
#  Tests:      mfem files 
#
#  Programmer: Cyrus Harrison
#  Date:       Thu Jun 12 14:58:03 PDT 2014
#
#  Modifications:
#    Brad Whitlock, Wed Sep  2 20:56:13 PDT 2020
#    The MFEM reader now provides original cells so artificial mesh lines
#    get removed. There's less of a point to seeing them here. Keep min/max
#    refinement levels only. 
# 
#    Justin Privitera, Mon Oct 17 17:33:30 PDT 2022
#    Added new tests for the new LOR settings. All prior tests use the legacy
#    LOR setting, while new tests use a mix of both.
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("MFEM")

readOptions = GetDefaultFileOpenOptions("MFEM")
readOptions["MFEM LOR Setting"] = "Legacy LOR"
SetDefaultFileOpenOptions("MFEM", readOptions)

mfem_roots  = glob.glob(data_path("mfem_test_data/*.mfem_root"))
input_meshs  = [ f for f in mfem_roots if f.count("ex0") == 0]
ex01_results = [ f for f in mfem_roots if f.count("ex01") == 1]
ex02_results = [ f for f in mfem_roots if f.count("ex02") == 1]

def set_test_view():
    v = View3DAttributes()
    v.viewNormal = (-0.510614, 0.302695, 0.804767)
    v.focus = (0, 0, 0)
    v.viewUp = (-0.0150532, 0.932691, -0.360361)
    v.viewAngle = 30
    v.parallelScale = 17.3205
    v.nearPlane = -34.641
    v.farPlane = 34.641
    v.imagePan = (0, 0)
    v.imageZoom = 1
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)

TestSection("Input Mesh Files")
for f in input_meshs:
    base = os.path.splitext(os.path.basename(f))[0]
    DeleteAllPlots()
    OpenDatabase(f)
    AddPlot("Pseudocolor","element_coloring")
    #pc_atts = PseudocolorAttributes()
    #pc_atts.colorTableName = "PuBuGn"
    #SetPlotOptions(pc_atts)
    AddOperator("MultiresControl")
    AddPlot("Mesh","main")
    AddOperator("MultiresControl")
    SetActivePlots((0,1))
    levels = [0,20]
    if f.count("amr") > 0:
        levels = [0,5]
    for mres in levels:
        mc_atts  = MultiresControlAttributes()
        mc_atts.resolution = mres
        SetOperatorOptions(mc_atts)
        DrawPlots()
        Test("input_mesh_%s_mres_%04d" % (base,mres))
    DeleteAllPlots()
    CloseDatabase(f)

TestSection("ex01 results")
for f in ex01_results:
    base = os.path.splitext(os.path.basename(f))[0]
    DeleteAllPlots()
    OpenDatabase(f)
    AddPlot("Pseudocolor","gf")
    #AddPlot("Mesh","main")
    DrawPlots()
    Test("ex01_%s" % (base))
    DeleteAllPlots()
    CloseDatabase(f)

TestSection("ex02 results")
for f in ex02_results:
    base = os.path.splitext(os.path.basename(f))[0]
    DeleteAllPlots()
    OpenDatabase(f)
    AddPlot("Pseudocolor","element_attribute")
    #AddPlot("Mesh","main")
    DrawPlots()
    Test("ex02_element_attribute_%s" % (base))
    ChangeActivePlotsVar("gf_magnitude");
    DrawPlots()
    Test("ex02_gf_mag_%s" % (base))
    DeleteAllPlots()
    CloseDatabase(f)

TestSection("MFEM Expressions")
OpenDatabase(data_path("mfem_test_data/ex02-beam-tet.mfem_root"))
AddPlot("Pseudocolor","mag-gf")
DrawPlots()
Test("mfem_expressions_1")
DeleteAllPlots()
AddPlot("Pseudocolor","comp0")
DrawPlots()
Test("mfem_expressions_2")
DeleteAllPlots()
AddPlot("Vector","curl-gf")
DrawPlots()
Test("mfem_expressions_3")
DeleteAllPlots()
CloseDatabase(data_path("mfem_test_data/ex02-beam-tet.mfem_root"))

# reset default
readOptions = GetDefaultFileOpenOptions("MFEM")
readOptions["MFEM LOR Setting"] = "MFEM LOR"
SetDefaultFileOpenOptions("MFEM", readOptions)

def test_mfem_lor_mesh(tag_name, dbfile):
    ResetView()
    base = os.path.splitext(os.path.basename(dbfile))[0]

    # get default options
    readOptions = GetDefaultFileOpenOptions("MFEM")
    readOptions["MFEM LOR Setting"] = "MFEM LOR"
    SetDefaultFileOpenOptions("MFEM", readOptions)
    OpenDatabase(dbfile)

    # we want to test a picture of a wireframe
    # new LOR should only have the outer edge
    AddPlot("Subset", "main")
    SubsetAtts = SubsetAttributes()
    SubsetAtts.wireframe = 1
    SetPlotOptions(SubsetAtts)
    set_test_view()
    DrawPlots()
    Test(tag_name + "_" + base + "_lor")
    DeleteAllPlots()
    ResetView()
    CloseDatabase(dbfile)

    ##############################

    # examine legacy
    readOptions = GetDefaultFileOpenOptions("MFEM")
    readOptions["MFEM LOR Setting"] = "Legacy LOR"
    SetDefaultFileOpenOptions("MFEM", readOptions)
    OpenDatabase(dbfile)

    # old LOR leaves a busy wireframe
    AddPlot("Subset", "main")
    SubsetAtts = SubsetAttributes()
    SubsetAtts.wireframe = 1
    SetPlotOptions(SubsetAtts)
    set_test_view()
    DrawPlots()
    Test(tag_name + "_" + base + "_legacy_lor")
    DeleteAllPlots()
    ResetView()
    CloseDatabase(dbfile)

    # restore default
    readOptions = GetDefaultFileOpenOptions("MFEM")
    readOptions["MFEM LOR Setting"] = "MFEM LOR"
    SetDefaultFileOpenOptions("MFEM", readOptions)

TestSection("Legacy and New LOR")
for dbfile in input_meshs:
    test_mfem_lor_mesh("LOR", dbfile)

def test_mfem_lor_field(tag_name, dbfile):
    ResetView()
    base = os.path.splitext(os.path.basename(dbfile))[0]

    readOptions = GetDefaultFileOpenOptions("MFEM")
    readOptions["MFEM LOR Setting"] = "MFEM LOR"
    SetDefaultFileOpenOptions("MFEM", readOptions)
    OpenDatabase(dbfile)

    AddPlot("Pseudocolor","gf")
    AddOperator("MultiresControl", 1)
    SetActivePlots(0)
    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 3
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    set_test_view()
    DrawPlots()
    Test(tag_name + "_" + base + "_pseudocolor_gf_lor")
    DeleteAllPlots()
    ResetView()

    CloseDatabase(dbfile)

    ##############################

    # examine legacy
    readOptions = GetDefaultFileOpenOptions("MFEM")
    readOptions["MFEM LOR Setting"] = "Legacy LOR"
    SetDefaultFileOpenOptions("MFEM", readOptions)
    OpenDatabase(dbfile)

    AddPlot("Pseudocolor","gf")
    AddOperator("MultiresControl", 1)
    SetActivePlots(0)
    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 3
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    set_test_view()
    DrawPlots()
    Test(tag_name + "_" + base + "_pseudocolor_gf_legacy_lor")
    DeleteAllPlots()
    ResetView()

    # restore default
    readOptions = GetDefaultFileOpenOptions("MFEM")
    readOptions["MFEM LOR Setting"] = "MFEM LOR"
    SetDefaultFileOpenOptions("MFEM", readOptions)

TestSection("Legacy and New LOR Fields")
for dbfile in ex01_results:
    test_mfem_lor_field("LOR_Fields", dbfile)

Exit()
