# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  batch.py
#
#  Tests:      libsim - batch mode.
#
#  Programmer: Kathleen Biagas 
#  Date:       June 18, 2014 
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def test0():
    TestSection("Slice Export with 3 vertices")
    db = "slice3v_*.vtk database"
    OpenDatabase(db)
    AddPlot("Pseudocolor", "dom")
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (-0.0556426, 0.707794, 0.704224)
    v.focus = (5, 1.53846, 5)
    v.viewUp = (0.048696, 0.706401, -0.706134)
    v.viewAngle = 30
    v.parallelScale = 7.12383
    v.nearPlane = -14.2477
    v.farPlane = 14.2477
    v.imagePan = (-0.021177, 0.0636043)
    v.imageZoom = 1.09666
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (5, 1.53846, 5)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)

    Test("batch_0_00")

    ChangeActivePlotsVar("xc")
    Test("batch_0_01")
    ChangeActivePlotsVar("radius")
    Test("batch_0_02")
    ChangeActivePlotsVar("q")
    Test("batch_0_03")

    TimeSliderSetState(2)
    Test("batch_0_04")
    TimeSliderSetState(4)
    Test("batch_0_05")

    DeleteAllPlots()
    CloseDatabase(db)

def test1():
    TestSection("Slice Export with Origin+Normal")
    db = "sliceON_*.vtk database"
    OpenDatabase(db)
    AddPlot("Pseudocolor", "dom")
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (0, 0, 1)
    v.focus = (5, 5, 5)
    v.viewUp = (0, 1, 0)
    v.viewAngle = 30
    v.parallelScale = 8.66025
    v.nearPlane = -17.3205
    v.farPlane = 17.3205
    v.imagePan = (-0.00113806, 0.0530002)
    v.imageZoom = 1.33438
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (5, 5, 5)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)

    Test("batch_1_00")

    ChangeActivePlotsVar("xc")
    Test("batch_1_01")
    ChangeActivePlotsVar("radius")
    Test("batch_1_02")
    ChangeActivePlotsVar("q")
    Test("batch_1_03")

    TimeSliderSetState(2)
    Test("batch_1_04")
    TimeSliderSetState(4)
    Test("batch_1_05")

    DeleteAllPlots()
    CloseDatabase(db)

def test2():
    TestSection("Slice Export of X")
    db = "sliceX_*.vtk database"
    OpenDatabase(db)
    AddPlot("Pseudocolor", "dom")
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (1, 0, 6.12323e-17)
    v.focus = (0.5, 5, 5)
    v.viewUp = (0, 1, 0)
    v.viewAngle = 30
    v.parallelScale = 7.07107
    v.nearPlane = -14.1421
    v.farPlane = 14.1421
    v.imagePan = (0, 0)
    v.imageZoom = 1.27205
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.5, 5, 5)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)

    Test("batch_2_00")

    ChangeActivePlotsVar("xc")
    Test("batch_2_01")
    ChangeActivePlotsVar("radius")
    Test("batch_2_02")
    ChangeActivePlotsVar("q")
    Test("batch_2_03")

    ChangeActivePlotsVar("radius")
    TimeSliderSetState(2)
    Test("batch_2_04")
    TimeSliderSetState(4)
    Test("batch_2_05")

    DeleteAllPlots()
    CloseDatabase(db)

def test3():
    TestSection("Slice Export of Y")
    db = "sliceY_*.vtk database"
    OpenDatabase(db)
    AddPlot("Pseudocolor", "dom")
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (0, 1, 6.12323e-17)
    v.focus = (5, 2.5, 5)
    v.viewUp = (0, 6.12323e-17, -1)
    v.viewAngle = 30
    v.parallelScale = 7.07107
    v.nearPlane = -14.1421
    v.farPlane = 14.1421
    v.imagePan = (0, 0)
    v.imageZoom = 1.32859
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (5, 2.5, 5)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)

    Test("batch_3_00")

    ChangeActivePlotsVar("xc")
    Test("batch_3_01")
    ChangeActivePlotsVar("radius")
    Test("batch_3_02")
    ChangeActivePlotsVar("q")
    Test("batch_3_03")

    TimeSliderSetState(2)
    Test("batch_3_04")
    TimeSliderSetState(4)
    Test("batch_3_05")

    DeleteAllPlots()
    CloseDatabase(db)

def test4():
    TestSection("Slice Export of Z")
    db = "sliceZ_*.vtk database"
    OpenDatabase(db)
    AddPlot("Pseudocolor", "dom")
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (0, 0, 1)
    v.focus = (5, 5, 5)
    v.viewUp = (0, 1, 0)
    v.viewAngle = 30
    v.parallelScale = 7.07107
    v.nearPlane = -14.1421
    v.farPlane = 14.1421
    v.imagePan = (0, 0)
    v.imageZoom = 1.2647
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (5, 5, 5)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)

    Test("batch_4_00")

    ChangeActivePlotsVar("xc")
    Test("batch_4_01")
    ChangeActivePlotsVar("radius")
    Test("batch_4_02")
    ChangeActivePlotsVar("q")
    Test("batch_4_03")

    TimeSliderSetState(2)
    Test("batch_4_04")
    TimeSliderSetState(4)
    Test("batch_4_05")

    DeleteAllPlots()
    CloseDatabase(db)

def test5():
    TestSection("Iso Export")
    dbs = ("iso_0000.visit","iso_0001.visit","iso_0002.visit","iso_0003.visit","iso_0004.visit")
    index = 0
    for db in dbs:
        OpenDatabase(db)
        AddPlot("Pseudocolor", "dom")
        DrawPlots()

        v = GetView3D()
        v.viewNormal = (-0.433402, 0.101212, 0.895499)
        v.focus = (5, 5, 5)
        v.viewUp = (0.0882645, 0.993663, -0.0695889)
        v.viewAngle = 30
        v.parallelScale = 8.66025
        v.nearPlane = -17.3205
        v.farPlane = 17.3205
        v.imagePan = (0.00394299, 0.0327202)
        v.imageZoom = 1.34799
        v.perspective = 1
        v.eyeAngle = 2
        v.centerOfRotationSet = 0
        v.centerOfRotation = (5, 5, 5)
        v.axis3DScaleFlag = 0
        v.axis3DScales = (1, 1, 1)
        v.shear = (0, 0, 1)
        v.windowValid = 1
        SetView3D(v)

        Test("batch_5_%02d" % index)

        ChangeActivePlotsVar("xc")
        Test("batch_5_%02d" % (index+1))
        ChangeActivePlotsVar("radius")
        Test("batch_5_%02d" % (index+2))
        ChangeActivePlotsVar("q")
        Test("batch_5_%02d" % (index+3))
        index = index + 4

        DeleteAllPlots()
        CloseDatabase(db)

def test6():
    TestSection("Image Rendering")
    db = "batch*.png database"
    OpenDatabase(db)
    AddPlot("Truecolor", "color")
    AddOperator("Box")
    box = BoxAttributes()
    box.amount = box.Some  # Some, All
    box.minx = 230
    box.maxx = 1000
    box.miny = 45
    box.maxy = 1000
    box.minz = 0
    box.maxz = 1
    box.inverse = 0
    SetOperatorOptions(box)
    DrawPlots()

    v = GetView2D()
    v.windowCoords = (0, 960, 0, 540)
    v.viewportCoords = (0.01, 0.99, 0.22, 0.99)
    v.fullFrameActivationMode = v.Auto  # On, Off, Auto
    v.fullFrameAutoThreshold = 100
    v.xScale = v.LINEAR  # LINEAR, LOG
    v.yScale = v.LINEAR  # LINEAR, LOG
    v.windowValid = 1
    SetView2D(v)

    Test("batch_6_00")
    TimeSliderSetState(1)
    Test("batch_6_01")
    TimeSliderSetState(2)
    Test("batch_6_02")
    TimeSliderSetState(3)
    Test("batch_6_03")
    TimeSliderSetState(4)
    Test("batch_6_04")

    DeleteAllPlots()
    CloseDatabase(db)

def test7():
    TestSection("Streamline Export")
    db = "streamline_*.vtk database"
    OpenDatabase(db)
    AddPlot("Pseudocolor", "colorVar")
    pc = PseudocolorAttributes(1)
    pc.lineWidth = 3
    SetPlotOptions(pc)
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (0, 0, 1)
    v.focus = (5.2835, 5.2835, 5.2562)
    v.viewUp = (0, 1, 0)
    v.viewAngle = 30
    v.parallelScale = 8.26953
    v.nearPlane = -16.5391
    v.farPlane = 16.5391
    v.imagePan = (0, 0)
    v.imageZoom = 1.33131
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (5.2835, 5.2835, 5.2562)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)
    Test("batch_7_00")

    ChangeActivePlotsVar("xc")
    Test("batch_7_01")
    ChangeActivePlotsVar("radius")
    Test("batch_7_02")
    ChangeActivePlotsVar("q")
    Test("batch_7_03")

    TimeSliderSetState(2)
    Test("batch_7_04")
    TimeSliderSetState(4)
    Test("batch_7_05")

    DeleteAllPlots()
    CloseDatabase(db)

def main():
    # Create our simulation object.
    sim = TestBatchSimulation("batch")
    sim.addargument("-format")
    sim.addargument("VTK_1.0")
    sim.addargument("-maxcycles")
    sim.addargument("5")
    sim.addargument("-render")
    sim.addargument("1")

    # Test that we can start the simulation.
    sim.startsim()
    # This will wait for the simulation to complete.
    sim.endsim()

    # Test the simulation outputs.
    test0()
    test1()
    test2()
    test3()
    test4()
    test5()
    test6()
    test7()

main()
Exit()

