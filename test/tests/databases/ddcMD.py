# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  ddcMD.py
#
#  Tests:      mesh      - 2D/3D structured automatically decomposed
#
#  Programmer: Brad Whitlock
#  Date:       Thu Oct 11 17:04:32 PDT 2012
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def GetVariables(db, doExpressions = 1):
    md = GetMetaData(db)
    txt = "Scalar Names:\n"
    s = []
    for i in range(md.GetNumScalars()):
        s = s + [md.GetScalars(i).name]
    scalars = sorted(s)
    for s in scalars:
        txt = txt + s + "\n"
    if doExpressions:
        txt = txt + "\n\nExpressions:\n"
        expr = {}
        for i in range(md.exprList.GetNumExpressions()):
            e = md.exprList.GetExpressions(i)
            expr[e.name] = e.definition
        keys = sorted(expr.keys())
        for k in keys:
            txt = txt + k + " = " + expr[k] + "\n"
    return txt

def test0(datapath):
    TestSection("2D cgrid single files")
    db = pjoin(datapath,"cgrid2d.ddcMD")
    TestText("ddcMD_0_00", GetVariables(db))

    OpenDatabase(db)
    AddPlot("Pseudocolor", "mass")
    DrawPlots()
    Test("ddcMD_0_01")

    ChangeActivePlotsVar("mass_Al")
    Test("ddcMD_0_02")

    ChangeActivePlotsVar("mass_Cu")
    Test("ddcMD_0_03")

    DeleteAllPlots()

def test1(datapath):
    TestSection("2D cgrid multi files")
    db = pjoin(datapath,"cgrid2d_multi.ddcMD")
    TestText("ddcMD_1_00", GetVariables(db))

    OpenDatabase(db)
    AddPlot("Pseudocolor", "Kx")
    DrawPlots()
    Test("ddcMD_1_01")

    ChangeActivePlotsVar("Kx_Al")
    Test("ddcMD_1_02")

    ChangeActivePlotsVar("Kx_Cu")
    Test("ddcMD_1_03")

    DeleteAllPlots()

def test2(datapath):
    TestSection("3D cgrid multi files and multi subname")
    db = pjoin(datapath,"cgrid3d_multi_subname", "snapshot.*.ddcMD database")
    TestText("ddcMD_2_00", GetVariables(db, doExpressions = 0))

    # Plot the gid to see that things are put together properly.
    OpenDatabase(db)
    AddPlot("Pseudocolor", "anatomy/gid")
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (-0.463775, -0.476999, -0.746582)
    v.focus = (12.5,12.5,12.5)
    v.viewUp = (-0.885851, 0.236876, 0.398946)
    v.parallelScale = 21.6506
    v.nearPlane = -43.3013
    v.farPlane = 43.3013
    v.imagePan = (0.0101758, 0.0313539)
    v.imageZoom = 1.01954
    SetView3D(v)
    Test("ddcMD_2_01")

    ChangeActivePlotsVar("state/Vm")

    # Iterate over time.
    for i in range(TimeSliderGetNStates()):
        SetTimeSliderState(i)
        Test("ddcMD_2_%02d" % (2 + i))

    # Lower the transparency to see if domain boundaries are good. I think that
    # currently they are not good.

    DeleteAllPlots()

def main():
    datapath = data_path("ddcmd_test_data")
    test0(datapath)
    test1(datapath)
    test2(datapath)

main()
Exit()
