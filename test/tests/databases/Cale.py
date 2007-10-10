# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  Cale.py
#
#  Tests:      mesh      - 2D, structured, single domain
#              plots     - Pseudocolor, FilledBoundary
#
#  Programmer: Brad Whitlock
#  Date:       Wed Oct 10 11:46:45 PDT 2007
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def test0(datapath):
    TestSection("Cale files")
    OpenDatabase(datapath + "jet00405.pdb")
    AddPlot("Pseudocolor", "p")
    DrawPlots()
    Test("Cale_0_00")
    DeleteActivePlots()

    AddPlot("FilledBoundary", "Materials")
    DrawPlots()
    Test("Cale_0_01")

    AddPlot("Mesh", "hydro")
    DrawPlots()
    v0 = View2DAttributes()
    v0.windowCoords = (10.1155, 11.6375, 3.20767, 4.82837)
    v0.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v0.fullFrameActivationMode = v0.Auto  # On, Off, Auto
    v0.fullFrameAutoThreshold = 100
    v0.xScale = v0.LINEAR  # LINEAR, LOG
    v0.yScale = v0.LINEAR  # LINEAR, LOG
    SetView2D(v0)
    Test("Cale_0_02")

    # Do a PC plot with mixvars.
    SetActivePlots(0)
    DeleteActivePlots()
    m = GetMaterialAttributes()
    m.forceMIR = 1
    SetMaterialAttributes(m)
    AddPlot("Pseudocolor", "den")
    DrawPlots()
    Test("Cale_0_03")
    DeleteAllPlots()

    # Test another file.
    OpenDatabase(datapath + "vpgz.pdb")
    AddPlot("FilledBoundary", "Materials")
    DrawPlots()
    ResetView()
    Test("Cale_0_04")
    DeleteActivePlots()

def main():
    a = GetAnnotationAttributes()
    a.databaseInfoFlag = 0
    a. userInfoFlag = 0
    a.xAxisLabels = 0
    a.yAxisLabels = 0
    a.zAxisLabels = 0
    SetAnnotationAttributes(a)

    datapath = "../data/Cale_test_data/"
    test0(datapath)

main()
Exit()
