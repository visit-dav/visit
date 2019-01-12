# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  missingdata.py
#
#  Tests:      missing data
#
#  Programmer: Brad Whitlock
#  Date:       Thu Jan 19 09:49:15 PST 2012
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def SetTheView():
    v = GetView2D()
    v.viewportCoords = (0.02, 0.98, 0.25, 1)
    SetView2D(v)

def test0(datapath):
    TestSection("Missing data")
    OpenDatabase(pjoin(datapath,"earth.nc"))
    AddPlot("Pseudocolor", "height")
    DrawPlots()
    SetTheView()
    Test("missingdata_0_00")
    ChangeActivePlotsVar("carbon_particulates")
    Test("missingdata_0_01")
    ChangeActivePlotsVar("seatemp")
    Test("missingdata_0_02")
    ChangeActivePlotsVar("population")
    Test("missingdata_0_03")

    # Pick on higher zone numbers to make sure pick works.
    PickByNode(domain=0, element=833621)
    TestText("missingdata_0_04", GetPickOutput())

    DeleteAllPlots()

def test1(datapath):
    TestSection("Expressions and missing data")
    OpenDatabase(pjoin(datapath,"earth.nc"))
    DefineScalarExpression("meaningless", "carbon_particulates + seatemp")
    AddPlot("Pseudocolor", "meaningless")
    DrawPlots()
    SetTheView()
    Test("missingdata_1_00")
    DeleteAllPlots()

    DefineVectorExpression("color", "color(red,green,blue)")
    AddPlot("Truecolor", "color")
    DrawPlots()
    ResetView()
    SetTheView()
    Test("missingdata_1_01")

    DefineVectorExpression("color2", "color(population*0.364,green,blue)")
    ChangeActivePlotsVar("color2")
    v1 = GetView2D()
    v1.viewportCoords = (0.02, 0.98, 0.02, 0.98)
    v1.windowCoords = (259.439, 513.299, 288.93, 540) #25.466)
    SetView2D(v1)
    Test("missingdata_1_02")

def main():
    datapath = data_path("netcdf_test_data")
    test0(datapath)
    test1(datapath)

main()
Exit()
