# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  WData.py 
#
#  Programmer: Kathleen Biagas 
#  Date:       September 22, 2021 
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def Test1():
    OpenDatabase(data_path("WData_test_data/WData-test.wtxt"))

    AddPlot("Contour", "density_a")
    AddPlot("Vector", "current_a")
    DrawPlots()

    v = GetView3D()
    v.viewNormal=(0.363715, 0.43572, 0.822339)
    v.viewUp=(-0.158317, 0.898996, -0.40834)
    SetView3D(v)

    Test("wdata_1_01")

    TimeSliderSetState(5)
    Test("wdata_1_02")

    DeleteAllPlots()
    AddPlot("Volume", "density_b")
    DrawPlots()
    Test("wdata_1_03")

    DeleteAllPlots()

    AddPlot("Pseudocolor", "density_b")
    DrawPlots()

    SetQueryOutputToObject()
    se = Query("SpatialExtents")["extents"]
    AddOperator("ThreeSlice")
    ts = ThreeSliceAttributes()
    ts.x = (se[1] - se[0]) * 0.5
    ts.y = (se[3] - se[2]) * 0.5
    ts.z = (se[5] - se[4]) * 0.5
    SetOperatorOptions(ts)
    DrawPlots()
   
    Test("wdata_1_04")

    DeleteAllPlots()
    CloseDatabase(data_path("WData_test_data/WData-test.wtxt"))

Test1()

Exit()
