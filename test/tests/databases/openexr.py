# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  openexr.py 
#
#  Tests:      openexr reading 
#
#  Programmer: Brad Whitlock
#  Date:       Thu Feb 15 14:16:45 PST 2018
#
#  Modifications:
# 
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("OpenEXR")

def test0():
    DefineScalarExpression("masked_value", "if(gt(A, 0), value, 0)")

    db = data_path("OpenEXR_test_data/noise_rgbazlv.exr")
    OpenDatabase(db)
    AddPlot("Pseudocolor", "R")
    DrawPlots()
    v = GetView2D()
    v.viewportCoords = (0, 1, 0, 1)
    SetView2D(v)
    Test("openexr_0_R")

    ChangeActivePlotsVar("G")
    Test("openexr_0_G")

    ChangeActivePlotsVar("B")
    Test("openexr_0_B")

    ChangeActivePlotsVar("A")
    Test("openexr_0_A")

    ChangeActivePlotsVar("Z")
    Test("openexr_0_Z")

    ChangeActivePlotsVar("L")
    Test("openexr_0_L")

    ChangeActivePlotsVar("masked_value")
    Test("openexr_0_value")

    DeleteAllPlots()
    CloseDatabase(db)

def main():
    test0()

main()
Exit()
