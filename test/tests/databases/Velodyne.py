# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  Velodyne.py
#
#  Tests:      mesh      - 3D unstructured
#
#  Programmer: Brad Whitlock
#  Date:       Wed Feb 10 14:51:33 PDT 2010
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def InitializeView():
    v0 = View3DAttributes()
    v0.viewNormal = (0.715595, -0.551955, 0.4281)
    v0.focus = (11.5, 2.39999, 1.04954)
    v0.viewUp = (-0.34794, 0.249768, 0.903633)
    v0.viewAngle = 30
    v0.parallelScale = 11.8686
    v0.nearPlane = -23.7373
    v0.farPlane = 23.7373
    v0.imagePan = (-0.0579851, 0.0689456)
    v0.imageZoom = 1.2342
    v0.perspective = 1
    v0.eyeAngle = 2
    v0.centerOfRotationSet = 0
    v0.centerOfRotation = (11.5, 2.39999, 1.04954)
    SetView3D(v0)

def test0(datapath):
    TestSection("Solid")
    db = pjoin(datapath,"pelh.*.vld database")
    OpenDatabase(db)
    md = GetMetaData(db)
    TestText("Velodyne_0_00", str(md))
    AddPlot("Pseudocolor", "Solid/Pressure")
    DrawPlots()
    InitializeView()

    Test("Velodyne_0_01")
    TimeSliderSetState(11)
    Test("Velodyne_0_02")
    TimeSliderSetState(22)
    Test("Velodyne_0_03")
    TimeSliderSetState(0)
    DeleteAllPlots()
    
    AddPlot("Pseudocolor", "Solid/Velocity_0")
    DrawPlots()
    InitializeView()
    Test("Velodyne_0_04")
    TimeSliderSetState(11)
    Test("Velodyne_0_05")
    TimeSliderSetState(22)
    Test("Velodyne_0_06")
    TimeSliderSetState(0)
    DeleteAllPlots()
    CloseDatabase(db)

def test1(datapath):
    TestSection("Surface")

    OpenDatabase(pjoin(datapath,"pelh.1.vld"))
    AddPlot("Pseudocolor", "Surface/Velocity_0")
    DrawPlots()
    InitializeView()
    Test("Velodyne_1_00")
    DeleteAllPlots()

def main():
    datapath = data_path("Velodyne_test_data")
    test0(datapath)
    test1(datapath)

main()
Exit()
