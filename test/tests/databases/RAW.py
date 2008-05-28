# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  RAW.py
#
#  Tests:      mesh      - 3D unstructured
#              plots     - Mesh, Pseudocolor, FilledBoundary
#
#  Programmer: Brad Whitlock
#  Date:       Wed Oct 10 12:05:17 PDT 2007
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def test0(datapath):
    TestSection("RAW files")
    OpenDatabase(datapath + "visitrocks.raw")
    AddPlot("Pseudocolor", "x")
    DrawPlots()
    v0 = View3DAttributes()
    v0.viewNormal = (-0.567094, 0.226914, 0.791779)
    v0.focus = (0.000999987, -0.1635, 0)
    v0.viewUp = (0.130227, 0.973912, -0.18584)
    v0.viewAngle = 30
    v0.parallelScale = 1.5462
    v0.nearPlane = -3.0924
    v0.farPlane = 3.0924
    v0.imagePan = (0.0382352, 0.0259835)
    v0.imageZoom = 1.29479
    v0.perspective = 1
    v0.eyeAngle = 2
    v0.centerOfRotationSet = 0
    v0.centerOfRotation = (0.000999987, -0.1635, 0)
    SetView3D(v0)
    Test("RAW_0_00")
    DeleteAllPlots()

    OpenDatabase(datapath + "visit1.7.raw")
    AddPlot("Pseudocolor", "z")
    DrawPlots()
    v1 = View3DAttributes()
    v1.viewNormal = (0.548073, -0.470773, 0.691367)
    v1.focus = (-3, 2.5, -3.5)
    v1.viewUp = (-0.467318, 0.513182, 0.719902)
    v1.viewAngle = 30
    v1.parallelScale = 26.3154
    v1.nearPlane = -52.6308
    v1.farPlane = 52.6308
    v1.imagePan = (-0.00129203, 0.0275781)
    v1.imageZoom = 1.17533
    v1.perspective = 1
    v1.eyeAngle = 2
    v1.centerOfRotationSet = 0
    v1.centerOfRotation = (-3, 2.5, -3.5)
    SetView3D(v1)
    Test("RAW_0_01")
    DeleteAllPlots()

    OpenDatabase(datapath + "llnl-logo.raw")
    AddPlot("Subset", "domains")
    silr = SILRestriction()
    silr.SuspendCorrectnessChecking()
    silr.TurnOffAll()
    for silSet in (1,2,77):
        silr.TurnOnSet(silSet)
    silr.EnableCorrectnessChecking()
    SetPlotSILRestriction(silr)
    DrawPlots()
    v2 = View3DAttributes()
    v2.viewNormal = (0.373419, 0.166874, 0.91253)
    v2.focus = (0.108829, 9.95567, 5.85852)
    v2.viewUp = (-0.0717588, 0.985936, -0.150933)
    v2.viewAngle = 30
    v2.parallelScale = 91.211
    v2.nearPlane = -182.422
    v2.farPlane = 182.422
    v2.imagePan = (-0.00129203, 0.0275781)
    v2.imageZoom = 1.17145
    v2.perspective = 1
    v2.eyeAngle = 2
    v2.centerOfRotationSet = 0
    v2.centerOfRotation = (0.108829, 9.95567, 5.85852)
    SetView3D(v2)
    Test("RAW_0_02")
    DeleteAllPlots()

def main():
    a = AnnotationAttributes()
    a.axes3D.xAxis.label.visible = 0
    a.axes3D.yAxis.label.visible = 0
    a.axes3D.zAxis.label.visible = 0
    a.databaseInfoFlag = 0
    a.userInfoFlag = 0
    SetAnnotationAttributes(a)

    datapath = "../data/RAW_test_data/"
    test0(datapath)

main()
Exit()
