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
    db = pjoin(datapath,"velodyne.20.vld")
    OpenDatabase(db,0,"ParallelVelodyne_1")
    md = GetMetaData(db)
    TestText("PVLD_solid", str(md))
    AddPlot("Pseudocolor", "Solid/Pressure")
    DrawPlots()
    InitializeView()

    Test("PVLD_solid_pressure")
    DeleteAllPlots()
    
    AddPlot("Pseudocolor", "Solid/Velocity_0")
    DrawPlots()
    InitializeView()
    Test("PVLD_solid_velocity")
    DeleteAllPlots()
    CloseDatabase(db)

def test1(datapath):
    TestSection("Surface")

    OpenDatabase(pjoin(datapath,"velodyne.20.vld"),0,"ParallelVelodyne_1")
    AddPlot("Pseudocolor", "Surface/Velocity_0")
    DrawPlots()
    InitializeView()
    Test("PVLD_surface_velocity")
    DeleteAllPlots()

def test2(datapath):
    TestSection("SPH")

    OpenDatabase(pjoin(datapath,"velodyne.20.vld"),0,"ParallelVelodyne_1")
    AddPlot("Pseudocolor", "SPH/Density")
    DrawPlots()
    InitializeView()
    Test("PVLD_SPH_density")
    DeleteAllPlots()

def main():
    datapath = data_path("PVLD_test_data")
    test0(datapath)
    test1(datapath)
    test2(datapath)

main()
Exit()
