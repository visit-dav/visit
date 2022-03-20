# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  NASTRAN.py
#
#  Tests:      mesh      - 3D unstructured, single domain
#              plots     - Mesh
#
#  Programmer: Brad Whitlock
#  Date:       Thu Jul 7 17:46:51 PST 2005
#
#  Modifications:
#    Edward Rusu, Tue Aug 21 10:06:24 PST 2018
#    Added tests for quadratic elements.
#
#    Kathleen Biagas, Wed Feb 16 09:15:45 PST 2022
#    Replace use of meshatts 'foregroundFlag' and 'backgroundFlag' with
#    meshColorSource and opaqueColorSource.
#
# ----------------------------------------------------------------------------

def AddMeshPlot():
    AddPlot("Mesh", "mesh")
    m = MeshAttributes()
    m.legendFlag = 0
    m.opaqueColor = (153, 204, 255, 255)
    m.opaqueColorSource = m.OpaqueCustom
    m.meshColorSource = m.MeshCustom
    m.meshColor = (0,0,0,255)
    SetPlotOptions(m)

def AddPseudocolorPlot():
    AddPlot("Pseudocolor", "one")
    p = PseudocolorAttributes()
    p.colorTableName = "rainbow"
    p.SetOpacityType(p.Constant)
    p.opacity = 0.2
    SetPlotOptions(p)
    DrawPlots()

def test1(datapath):
    db = "pipe.nas"
    TestSection(db)
    OpenDatabase(pjoin(datapath,db))
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
    Test("nastran1_0")

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
    Test("nastran1_1")

    DeleteActivePlots()
    AddPseudocolorPlot()
    SetView3D(v)
    Test("nastran1_2")

    DeleteActivePlots()
    CloseDatabase(pjoin(datapath,db))

def test2(datapath):
    db = "waterjacket.nas"
    TestSection(db)
    OpenDatabase(pjoin(datapath,db))
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
    Test("nastran2_0")

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
    Test("nastran2_1")

    DeleteActivePlots()
    AddPseudocolorPlot()
    SetView3D(v)
    Test("nastran2_2")

    DeleteActivePlots()
    CloseDatabase(pjoin(datapath,db))

def test3(datapath):
    db = "intake.nas"
    TestSection(db)
    OpenDatabase(pjoin(datapath,db))
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
    Test("nastran3_0")

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
    Test("nastran3_1")

    DeleteActivePlots()
    AddPseudocolorPlot()
    SetView3D(v)
    Test("nastran3_2")

    DeleteActivePlots()
    CloseDatabase(pjoin(datapath,db))

def test4(datapath):
    db = "porsche.nas"
    TestSection(db)
    OpenDatabase(pjoin(datapath,db))
    AddMeshPlot()
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (0.518428, 0.424019, 0.74259)
    v.focus = (0, 7.9285, 0)
    v.viewUp = (-0.299854, 0.903405, -0.306506)
    v.viewAngle = 30
    v.parallelScale = 24.7409
    v.nearPlane = -5
    v.farPlane = 49.4817
    v.imagePan = (0.00934164, 0.144131)
    v.imageZoom = 4.02409
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 7.9285, 0)
    SetView3D(v)
    Test("nastran4_0")

    v2 = View3DAttributes()
    v2.viewNormal = (0.518428, 0.424019, 0.74259)
    v2.focus = (0, 7.9285, 0)
    v2.viewUp = (-0.299854, 0.903405, -0.306506)
    v2.viewAngle = 30
    v2.parallelScale = 24.7409
    v2.nearPlane = -5
    v2.farPlane = 49.4817
    v2.imagePan = (0.0898178, 0.146581)
    v2.imageZoom = 19.659
    v2.perspective = 1
    v2.eyeAngle = 2
    v2.centerOfRotationSet = 0
    v2.centerOfRotation = (0, 7.9285, 0)
    SetView3D(v2)
    Test("nastran4_1")

    v3 = View3DAttributes()
    v3.viewNormal = (0.945904, 0.10945, -0.305429)
    v3.focus = (0, 7.9285, 0)
    v3.viewUp = (-0.0962433, 0.993666, 0.0580174)
    v3.viewAngle = 30
    v3.parallelScale = 24.7409
    v3.nearPlane = -8
    v3.farPlane = 49.4817
    v3.imagePan = (-0.000386251, 0.195232)
    v3.imageZoom = 17.0743
    v3.perspective = 1
    v3.eyeAngle = 2
    v3.centerOfRotationSet = 0
    v3.centerOfRotation = (0, 7.9285, 0)
    SetView3D(v3)
    Test("nastran4_2")

    DeleteActivePlots()
    CloseDatabase(pjoin(datapath,db))

def test5(datapath):
    db = "Rohr_01.nas"
    TestSection(db)
    OpenDatabase(pjoin(datapath,db))
    AddMeshPlot()
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (0.528785, -0.54024, -0.654619)
    v.focus = (4308, -300.408, 1264.41)
    v.viewUp = (0.848128, 0.365992, 0.383052)
    v.viewAngle = 30
    v.parallelScale = 80.5714
    v.nearPlane = -161.143
    v.farPlane = 161.143
    v.imagePan = (0.00712918, 0.0383046)
    v.imageZoom = 1.48788
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (4308, -300.408, 1264.41)
    SetView3D(v)
    Test("nastran5_0")

    DeleteActivePlots()
    AddPseudocolorPlot()
    Test("nastran5_1")

    DeleteActivePlots()
    CloseDatabase(pjoin(datapath,db))

def test6(datapath):
    db = "BMSP4E.nas"
    TestSection(db)
    OpenDatabase(pjoin(datapath,db))
    AddMeshPlot()
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (0.489205, 0.349064, 0.79927)
    v.focus = (0, 45.5, 0)
    v.viewUp = (-0.206708, 0.93671, -0.282569)
    v.viewAngle = 30
    v.parallelScale = 87.3806
    v.nearPlane = -174.761
    v.farPlane = 174.761
    v.imagePan = (-0.0102947, 0.0427356)
    v.imageZoom = 1.29632
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 45.5, 0)
    SetView3D(v)
    Test("nastran6_0")

    DeleteActivePlots()
    CloseDatabase(pjoin(datapath,db))

def test7(datapath):
    db = "HAM6D.nas"
    TestSection(db)
    OpenDatabase(pjoin(datapath,db))
    AddMeshPlot()
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (0.470653, -0.53132, 0.704403)
    v.focus = (23.6, -6.3125, 0)
    v.viewUp = (0.882043, 0.303259, -0.360602)
    v.viewAngle = 30
    v.parallelScale = 74.2944
    v.nearPlane = -148.589
    v.farPlane = 148.589
    v.imagePan = (0.0131053, 0.00691564)
    v.imageZoom = 1.18535
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (23.6, -6.3125, 0)
    SetView3D(v)
    Test("nastran7_0")

    v.viewNormal = (0.347772, 0.646944, -0.678615)
    v.focus = (23.6, -6.3125, 0)
    v.viewUp = (0.937573, -0.242724, 0.249084)
    v.viewAngle = 30
    v.parallelScale = 74.2944
    v.nearPlane = -148.589
    v.farPlane = 148.589
    v.imagePan = (-0.0229218, 0.0211742)
    v.imageZoom = 1.18535
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (23.6, -6.3125, 0)
    SetView3D(v)
    Test("nastran7_1")

    DeleteActivePlots()
    CloseDatabase(pjoin(datapath,db))

def test8(datapath):
    db = "simpleCQUAD8.nas"
    TestSection(db)
    OpenDatabase(pjoin(datapath,db))
    AddMeshPlot()
    DrawPlots()
    Test("nastran8_0")

    DeleteActivePlots()
    CloseDatabase(pjoin(datapath,db))

def test9(datapath):
    db = "simpleQuadSphere.nas"
    TestSection(db)
    OpenDatabase(pjoin(datapath,db))
    AddMeshPlot()
    DrawPlots()

    View3DAtts = View3DAttributes()
    View3DAtts.viewNormal = (0, 0, 1)
    View3DAtts.focus = (2.88777, -1.45671, -0.629586)
    View3DAtts.viewUp = (0, 1, 0)
    View3DAtts.viewAngle = 30
    View3DAtts.parallelScale = 6.98115
    View3DAtts.nearPlane = -13.9623
    View3DAtts.farPlane = 13.9623
    View3DAtts.imagePan = (0.204513, -0.111237)
    View3DAtts.imageZoom = 5.50207
    View3DAtts.perspective = 1
    View3DAtts.eyeAngle = 2
    View3DAtts.centerOfRotationSet = 0
    View3DAtts.centerOfRotation = (2.88777, -1.45671, -0.629586)
    View3DAtts.axis3DScaleFlag = 0
    View3DAtts.axis3DScales = (1, 1, 1)
    View3DAtts.shear = (0, 0, 1)
    View3DAtts.windowValid = 1
    SetView3D(View3DAtts)
    Test("nastran9_0")

    DeleteActivePlots()
    CloseDatabase(pjoin(datapath,db))

def test10(datapath):
    db = "simpleQuadSphereBlock.nas"
    TestSection(db)
    OpenDatabase(pjoin(datapath,db))
    AddMeshPlot()
    DrawPlots()

    View3DAtts = View3DAttributes()
    View3DAtts.viewNormal = (0, 0, 1)
    View3DAtts.focus = (0, -1.38778, -3.66334)
    View3DAtts.viewUp = (0, 1, 0)
    View3DAtts.viewAngle = 30
    View3DAtts.parallelScale = 12.6655
    View3DAtts.nearPlane = -25.3309
    View3DAtts.farPlane = 25.3309
    View3DAtts.imagePan = (-0.00429799, -0.061086)
    View3DAtts.imageZoom = 9.89552
    View3DAtts.perspective = 1
    View3DAtts.eyeAngle = 2
    View3DAtts.centerOfRotationSet = 0
    View3DAtts.centerOfRotation = (0, -1.38778, -3.66334)
    View3DAtts.axis3DScaleFlag = 0
    View3DAtts.axis3DScales = (1, 1, 1)
    View3DAtts.shear = (0, 0, 1)
    View3DAtts.windowValid = 1
    SetView3D(View3DAtts)
    Test("nastran10_0")

    DeleteActivePlots()
    CloseDatabase(pjoin(datapath,db))

def test11(datapath):
    db = "simpleQuadTet.nas"
    TestSection(db)
    OpenDatabase(pjoin(datapath,db))
    AddMeshPlot()
    DrawPlots()

    View3DAtts = View3DAttributes()
    View3DAtts.viewNormal = (0.585895, 0.392177, 0.709172)
    View3DAtts.focus = (0.5, 0.5, 0.5)
    View3DAtts.viewUp = (-0.133518, 0.909858, -0.39285)
    View3DAtts.viewAngle = 30
    View3DAtts.parallelScale = 0.866025
    View3DAtts.nearPlane = -1.73205
    View3DAtts.farPlane = 1.73205
    View3DAtts.imagePan = (0, 0)
    View3DAtts.imageZoom = 1
    View3DAtts.perspective = 1
    View3DAtts.eyeAngle = 2
    View3DAtts.centerOfRotationSet = 0
    View3DAtts.centerOfRotation = (0.5, 0.5, 0.5)
    View3DAtts.axis3DScaleFlag = 0
    View3DAtts.axis3DScales = (1, 1, 1)
    View3DAtts.shear = (0, 0, 1)
    View3DAtts.windowValid = 1
    SetView3D(View3DAtts)
    Test("nastran11_0")

    DeleteActivePlots()
    CloseDatabase(pjoin(datapath,db))

def test12(datapath):
    db = "simpleQuadWedge.nas"
    TestSection(db)
    OpenDatabase(pjoin(datapath,db))
    AddMeshPlot()
    DrawPlots()

    View3DAtts = View3DAttributes()
    View3DAtts.viewNormal = (-0.468638, 0.237166, 0.850959)
    View3DAtts.focus = (0.5, 0.5, 0.5)
    View3DAtts.viewUp = (0.128356, 0.971346, -0.20003)
    View3DAtts.viewAngle = 30
    View3DAtts.parallelScale = 0.866025
    View3DAtts.nearPlane = -1.73205
    View3DAtts.farPlane = 1.73205
    View3DAtts.imagePan = (0, 0)
    View3DAtts.imageZoom = 1
    View3DAtts.perspective = 1
    View3DAtts.eyeAngle = 2
    View3DAtts.centerOfRotationSet = 0
    View3DAtts.centerOfRotation = (0.5, 0.5, 0.5)
    View3DAtts.axis3DScaleFlag = 0
    View3DAtts.axis3DScales = (1, 1, 1)
    View3DAtts.shear = (0, 0, 1)
    View3DAtts.windowValid = 1
    SetView3D(View3DAtts)
    Test("nastran12_0")

    DeleteActivePlots()
    CloseDatabase(pjoin(datapath,db))

def main():
    # Set the window background color
    a = GetAnnotationAttributes()
    a.backgroundMode = a.Solid
    a.backgroundColor = (180, 180, 180, 255)
    a.foregroundColor = (255, 255, 255, 255)
    SetAnnotationAttributes(a)

    DefineScalarExpression("one", "(zoneid(mesh) + 1) / (zoneid(mesh) + 1)")

    datapath = data_path("NASTRAN_test_data")
    test1(datapath)
    test2(datapath)
    test3(datapath)
    test4(datapath)
    test5(datapath)
    test6(datapath)
    test7(datapath)
    test8(datapath)
    test9(datapath)
    test10(datapath)
    test11(datapath)
    test12(datapath)

main()
Exit()
