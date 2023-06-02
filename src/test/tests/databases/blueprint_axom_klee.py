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
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("Blueprint")
from os.path import join as pjoin

def bj_test_helper(datadir, prefix, sectionText):
    TestSection(sectionText)
    db = data_path(pjoin("axom_klee_test_data", datadir, "shaping.root"))
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

    # Look at one of the volume fractions. It should be refined
    SetActivePlots(0)
    DeleteActivePlots()
    AddPlot("Pseudocolor", "shaping_mesh/vol_frac_steel", 1, 1)
    DrawPlots()
    SetView2D(v)
    Test(prefix + "_03")

    DeleteAllPlots()
    CloseDatabase(db)

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
    bj_test_helper("balls_and_jacks_q7o2", "blueprint_axom_klee_1", "P2 Material")

def test2():
    bj_test_helper("balls_and_jacks_q7o5", "blueprint_axom_klee_2", "P5 Material")

def main():
    test0()
    test1()
    test2()

main()
Exit()
