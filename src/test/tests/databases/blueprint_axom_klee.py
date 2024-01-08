# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  blueprint_axom_klee.py 
#
#  Tests:      blueprint hdf5 files 
#
#  Programmer: Brad Whitlock
#  Date:       Wed May 31 15:59:22 PDT 2023
#
#  Modifications:
#    Brad Whitlock, Wed Jul 19 15:11:35 PDT 2023
#    I added some tests for an index file that has display_name entries in it.
#
#    Brad Whitlock, Tue Dec 19 17:29:53 PST 2023
#    I added a new test to make sure refining a low-order field happens by
#    making sure there is no message about adding extra 0's.
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("Blueprint")
from os.path import join as pjoin

def bj_test_helper_mats(rootName, prefix, sectionText, extraPlots = None):
    TestSection(sectionText)
    db = data_path(pjoin("axom_klee_test_data", rootName))
    OpenDatabase(db)
    AddPlot("FilledBoundary", "shaping_mesh_material")
    fb = FilledBoundaryAttributes(1)
    fb.SetMultiColor(0, (255,0,0,255))
    fb.SetMultiColor(1, (0,255,0,255))
    fb.SetMultiColor(2, (0,0,255,255))
    fb.SetMultiColor(3, (0,255,255,255))
    SetPlotOptions(fb)
    AddPlot("Mesh", "shaping_mesh")
    DrawPlots()
    v = GetView2D()
    v.windowCoords = (7.83773, 12.304, 8.64959, 13.1412)
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    SetView2D(v)
    SetActivePlots((0,1))

    Test(prefix + "_00")

    # Increase the resolution.
    AddOperator("MultiresControl", 1)
    m = MultiresControlAttributes()
    m.resolution = 4
    m.maxResolution = 10
    SetOperatorOptions(m)
    DrawPlots()
    Test(prefix + "_01")

    v2 = GetView2D()
    v2.windowCoords = (9.50025, 10.5163, 10.8769, 11.9097)
    v2.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    SetView2D(v2)
    Test(prefix + "_02")

    SetActivePlots(0)
    DeleteActivePlots()

    # Possibly make some extra plots
    if extraPlots is not None:
        extraPlots(prefix, v)

    DeleteAllPlots()
    CloseDatabase(db)

def bj_test_helper(datadir, prefix, sectionText):
    def pc_plots(prefix, v):
        # Look at one of the volume fractions. It should be refined
        AddPlot("Pseudocolor", "shaping_mesh/vol_frac_steel", 1, 1)
        DrawPlots()
        SetView2D(v)
        Test(prefix + "_03")
    bj_test_helper_mats(datadir, prefix, sectionText, pc_plots)

def test0():
    TestSection("P0 Material")
    db = data_path(pjoin("axom_klee_test_data", "heroic_roses_o0", "shaping.root"))
    OpenDatabase(db)

    AddPlot("FilledBoundary", "shaping_mesh_material")
    fb = FilledBoundaryAttributes(1)
    fb.SetMultiColor(0, (0,0,0,255))
    fb.SetMultiColor(1, (80,80,220,255))
    fb.SetMultiColor(2, (93,241,160,255))
    fb.SetMultiColor(3, (0,120,0,255))
    fb.SetMultiColor(4, (90,100,50,255))
    fb.SetMultiColor(5, (241,132,171,255))
    fb.SetMultiColor(6, (184,158,241,255))
    fb.SetMultiColor(7, (240,0,0,255))
    fb.SetMultiColor(8, (255,153,0,255))
    SetPlotOptions(fb)
    DrawPlots()
    ResetView()
    Test("blueprint_axom_klee_0_00")

    DeleteAllPlots()
    CloseDatabase(db)

def test1():
    bj_test_helper("balls_and_jacks_q7o2/shaping.root", "blueprint_axom_klee_1", "P2 Material")

def test2():
    bj_test_helper("balls_and_jacks_q7o5/shaping.root", "blueprint_axom_klee_2", "P5 Material")

def test3():
    TestSection("matvf on HO materials")
    db = data_path(pjoin("axom_klee_test_data", "3mat_q12o12", "shaping.root"))
    OpenDatabase(db)

    AddPlot("FilledBoundary", "shaping_mesh_material")
    AddOperator("MultiresControl")
    op = MultiresControlAttributes()
    op.resolution = 16
    SetOperatorOptions(op)
    DrawPlots()
    ResetView()
    Test("blueprint_axom_klee_3_00")
    DeleteAllPlots()

    DefineScalarExpression("vf_inner", 'matvf(shaping_mesh_material, "inner")')
    DefineScalarExpression("vf_middle", 'matvf(shaping_mesh_material, "middle")')
    DefineScalarExpression("vf_outer", 'matvf(shaping_mesh_material, "outer")')
    AddPlot("Pseudocolor", "vf_inner")
    AddOperator("MultiresControl")
    op = MultiresControlAttributes()
    op.resolution = 3
    op.maxResolution = 20
    SetOperatorOptions(op)
    DrawPlots()
    Test("blueprint_axom_klee_3_01")
    op.resolution = 20
    SetOperatorOptions(op)
    Test("blueprint_axom_klee_3_02")

    ChangeActivePlotsVar("vf_middle")
    op.resolution = 3
    SetOperatorOptions(op)
    Test("blueprint_axom_klee_3_03")
    op.resolution = 20
    SetOperatorOptions(op)
    Test("blueprint_axom_klee_3_04")

    ChangeActivePlotsVar("vf_outer")
    op.resolution = 3
    SetOperatorOptions(op)
    Test("blueprint_axom_klee_3_05")
    op.resolution = 20
    SetOperatorOptions(op)
    Test("blueprint_axom_klee_3_06")

    DeleteAllPlots()
    CloseDatabase(db)

def test4():
    def pc_plots(prefix, v):
        # Look at the volume fraction fields, whose names were changed
        # in the index using display_name.
        SetActivePlots(0)
        DeleteActivePlots()
        AddPlot("Pseudocolor", "volume_fractions/air", 1, 1)
        DrawPlots()
        ResetView()
        Test(prefix + "_03")
        ChangeActivePlotsVar("volume_fractions/rubber")
        Test(prefix + "_04")
        ChangeActivePlotsVar("volume_fractions/steel")
        Test(prefix + "_05")
    # Make sure the material still plots, even though its constituent volume
    # fraction arrays have been renamed in the index file using display_name.
    bj_test_helper_mats("balls_and_jacks_q7o2/shaping_mod.root", "blueprint_axom_klee_4", "Testing display_name", pc_plots)

def test5():
    TestSection("Refine low order field")
    db = data_path(pjoin("axom_klee_test_data", "balls_and_jacks_q7o5", "shaping.root"))
    OpenDatabase(db)

    # Plot a low-order field that would normally *not* refine with MultiresControl.
    AddPlot("Pseudocolor", "shaping_mesh/mesh_material_attribute")
    AddOperator("MultiresControl")
    m = MultiresControlAttributes()
    m.maxResolution = 10
    m.resolution = 2
    SetOperatorOptions(m)
    msg, severity = GetLastMessage(1)
    DrawPlots()

    # Executing the plot would have issued a warning about VisIt having to add
    # 0's. Make sure that message no longer happens.
    msg, severity = GetLastMessage(1)
    if msg.find("Extra 0.\'s were added") != -1:
        txt = msg
    else:
        txt = "Padding the field was not necessary."
    TestText("blueprint_axom_klee_5_00", txt)
    Test("blueprint_axom_klee_5_01")

    DeleteAllPlots()
    CloseDatabase(db)

def main():
    test0()
    test1()
    test2()
    test3()
    test4()
    test5()

main()
Exit()
