# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  PATRAN.py
#
#  Tests:      mesh      - 3D unstructured, single domain
#              plots     - Mesh
#
#  Programmer: Brad Whitlock
#  Date:       Fri Jul 29 17:30:33 PST 2005
#
#  Notes:      The data for these tests are supposed to be PATRAN versions of
#              some of the data in the NASTRAN tests.
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Set up the annotations
a = AnnotationAttributes()
a.userInfoFlag = 0
a.axes3D.visible = 0
a.axes3D.bboxFlag = 0
a.backgroundMode = a.Solid
a.backgroundColor = (180, 180, 180, 255)
a.foregroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)

def AddMeshPlot():
    AddPlot("Mesh", "mesh")
    m = MeshAttributes()
    m.legendFlag = 0
    m.opaqueColor = (153, 204, 255, 255)
    m.backgroundFlag = 0
    m.foregroundFlag = 0
    m.meshColor = (0,0,0,255)
    TurnMaterialsOff("default")
    SetPlotOptions(m)

def AddPseudocolorPlot():
    AddPlot("Pseudocolor", "one")
    p = PseudocolorAttributes()
    p.colorTableName = "rainbow"
    p.opacity = 0.2
    SetPlotOptions(p)
    TurnMaterialsOff("default")
    DrawPlots()

def test1(datapath):
    db = "pipe.neu"
    TestSection(db)
    OpenDatabase(datapath + db)
    AddMeshPlot()
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (-0.786306, 0.379526, 0.487527)
    v.focus = (0.149902, 0.212562, 0.124929)
    v.viewUp = (0.316186, 0.925114, -0.210215)
    v.viewAngle = 30
    v.parallelScale = 0.585963
    v.nearPlane = -1.17193
    v.farPlane = 1.17193
    v.imagePan = (0.0173275, 0.033058)
    v.imageZoom = 1.45734
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.149902, 0.212562, 0.124929)
    SetView3D(v)
    Test("PATRAN1_0")

    v2 = View3DAttributes()
    v2.viewNormal = (-0.786306, 0.379526, 0.487527)
    v2.focus = (0.149902, 0.212562, 0.124929)
    v2.viewUp = (0.316186, 0.925114, -0.210215)
    v2.viewAngle = 30
    v2.parallelScale = 0.585963
    v2.nearPlane = -1.17193
    v2.farPlane = 1.17193
    v2.imagePan = (-0.108127, 0.0971661)
    v2.imageZoom = 6.92887
    v2.perspective = 1
    v2.eyeAngle = 2
    v2.centerOfRotationSet = 0
    v2.centerOfRotation = (0.149902, 0.212562, 0.124929)
    SetView3D(v2)
    Test("PATRAN1_1")

    DeleteActivePlots()
    AddPseudocolorPlot()
    SetView3D(v)
    Test("PATRAN1_2")

    DeleteActivePlots()
    CloseDatabase(datapath + db)

def test2(datapath):
    db = "waterjacket.neu"
    TestSection(db)
    OpenDatabase(datapath + db)
    AddMeshPlot()
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (-0.65317, -0.673371, 0.346324)
    v.focus = (0.03, -0.03, -0.1125)
    v.viewUp = (0.225064, 0.264053, 0.937882)
    v.viewAngle = 30
    v.parallelScale = 0.289925
    v.nearPlane = -0.579849
    v.farPlane = 0.579849
    v.imagePan = (0.0984407, 0.0467479)
    v.imageZoom = 1.40003
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.03, -0.03, -0.1125)
    SetView3D(v)
    Test("PATRAN2_0")

    v2 = View3DAttributes()
    v2.viewNormal = (-0.65317, -0.673371, 0.346324)
    v2.focus = (0.03, -0.03, -0.1125)
    v2.viewUp = (0.225064, 0.264053, 0.937882)
    v2.viewAngle = 30
    v2.parallelScale = 0.289925
    v2.nearPlane = -0.579849
    v2.farPlane = 0.579849
    v2.imagePan = (0.100347, 0.0410472)
    v2.imageZoom = 7.7658
    v2.perspective = 1
    v2.eyeAngle = 2
    v2.centerOfRotationSet = 0
    v2.centerOfRotation = (0.03, -0.03, -0.1125)
    SetView3D(v2)
    Test("PATRAN2_1")

    DeleteActivePlots()
    AddPseudocolorPlot()
    SetView3D(v)
    Test("PATRAN2_2")

    DeleteActivePlots()
    CloseDatabase(datapath + db)

def test3(datapath):
    db = "intake.neu"
    TestSection(db)
    OpenDatabase(datapath + db)
    AddMeshPlot()
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (-0.61876, 0.311053, -0.721375)
    v.focus = (0, 0.011, -0.0935)
    v.viewUp = (0.208418, 0.950364, 0.231021)
    v.viewAngle = 30
    v.parallelScale = 0.198752
    v.nearPlane = -0.397503
    v.farPlane = 0.397503
    v.imagePan = (0.0151275, 0.00167627)
    v.imageZoom = 1.43299
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0.011, -0.0935)
    SetView3D(v)
    Test("PATRAN3_0")

    v2 = View3DAttributes()
    v2.viewNormal = (-0.61876, 0.311053, -0.721375)
    v2.focus = (0, 0.011, -0.0935)
    v2.viewUp = (0.208418, 0.950364, 0.231021)
    v2.viewAngle = 30
    v2.parallelScale = 0.198752
    v2.nearPlane = -0.397503
    v2.farPlane = 0.397503
    v2.imagePan = (-0.101283, 0.0396808)
    v2.imageZoom = 6.28038
    v2.perspective = 1
    v2.eyeAngle = 2
    v2.centerOfRotationSet = 0
    v2.centerOfRotation = (0, 0.011, -0.0935)
    SetView3D(v2)
    Test("PATRAN3_1")

    DeleteActivePlots()
    AddPseudocolorPlot()
    SetView3D(v)
    Test("PATRAN3_2")

    DeleteActivePlots()
    CloseDatabase(datapath + db)

def test4(datapath):
    db = "patout.neu"
    TestSection(db)
    OpenDatabase(datapath + db)
    AddPlot("FilledBoundary", "named_components")
    f = FilledBoundaryAttributes()
    f.legendFlag = 0
    SetPlotOptions(f)
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (-0.765801, 0.0434351, -0.641609)
    v.focus = (-3175.88, -94.6135, -37.6065)
    v.viewUp = (0.625813, -0.179298, -0.759085)
    v.viewAngle = 30
    v.parallelScale = 3328.81
    v.nearPlane = -6657.62
    v.farPlane = 6657.62
    v.imagePan = (-0.0688128, -0.239564)
    v.imageZoom = 92.5361
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (-3175.88, -94.6135, -37.6065)
    SetView3D(v)
    Test("PATRAN4_0")

    DeleteActivePlots()
    AddPlot("Pseudocolor", "elementMats")
    DrawPlots()
    Test("PATRAN4_1")

    ChangeActivePlotsVar("elementIds")
    Test("PATRAN4_2")

def main():
    DefineScalarExpression("one", "(zoneid(mesh) + 1) / (zoneid(mesh) + 1)")
    datapath = "../data/PATRAN_test_data/"
    test1(datapath)
    test2(datapath)
    test3(datapath)
    test4(datapath)

main()
Exit()
