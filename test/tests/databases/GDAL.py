# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  GDAL.py
#
#  Tests:      mesh      - 2D structured, 3D unstructured
#              plots     - Contour, Pseudocolor
#
#  Programmer: Brad Whitlock
#  Date:       Thu Sep 8 16:35:17 PST 2005
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def test0_and_1(db, prefix):
    OpenDatabase(db)
    AddPlot("Pseudocolor", "lower_res/resolution_0067x0086/height")
    AddPlot("Contour", "lower_res/resolution_0067x0086/height")
    c = ContourAttributes()
    c.colorType = c.ColorBySingleColor
    c.singleColor = (0,0,0,255)
    c.legendFlag = 0
    SetPlotOptions(c)
    DrawPlots()
    ResetView()
    v = View2DAttributes()
    v.windowCoords = (478545, 489295, 4.38614e+06, 4.40004e+06)
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v.fullFrameActivationMode = v.Off  # On, Off, Auto
    v.fullFrameAutoThreshold = 100
    SetView2D(v)
    Test(prefix + "00")

    # Increase the resolution a couple times.
    SetActivePlots((0,1))
    ChangeActivePlotsVar("lower_res/resolution_0268x0347/height")
    Test(prefix + "01")
    ChangeActivePlotsVar("height")
    Test(prefix + "02")

    # Switch to the elevated mesh
    ChangeActivePlotsVar("elevated/lower_res/resolution_0067x0086/height")
    v0 = View3DAttributes()
    v0.viewNormal = (0.514425, -0.612921, 0.599745)
    v0.focus = (483920, 4.39318e+06, 2025.5)
    v0.viewUp = (-0.383293, 0.461288, 0.800187)
    v0.viewAngle = 30
    v0.parallelScale = 8729.59
    v0.nearPlane = -17459.2
    v0.farPlane = 17459.2
    v0.imagePan = (0, 0)
    v0.imageZoom = 1.16907
    v0.perspective = 1
    v0.eyeAngle = 2
    v0.centerOfRotationSet = 0
    v0.centerOfRotation = (483920, 4.39318e+06, 2025.5)
    SetView3D(v0)
    Test(prefix + "03")

    v1 = View3DAttributes()
    v1.viewNormal = (0.514425, -0.612921, 0.599745)
    v1.focus = (483920, 4.39312e+06, 2031)
    v1.viewUp = (-0.383293, 0.461288, 0.800187)
    v1.viewAngle = 30
    v1.parallelScale = 8777.87
    v1.nearPlane = -17555.7
    v1.farPlane = 17555.7
    v1.imagePan = (0.106418, 0.0224564)
    v1.imageZoom = 2.73653
    v1.perspective = 1
    v1.eyeAngle = 2
    v1.centerOfRotationSet = 0
    v1.centerOfRotation = (483920, 4.39312e+06, 2031)
    SetView3D(v1)
    Test(prefix + "04")

    SetActivePlots(1)
    DeleteActivePlots()
    ChangeActivePlotsVar("elevated/lower_res/resolution_0268x0347/height")
    Test(prefix + "05")
    ChangeActivePlotsVar("elevated/mesh/height")
    Test(prefix + "06")
    DeleteAllPlots()

def test0(datapath):
    TestSection("ArcInfo files")
    test0_and_1(datapath + "ADF/w001001.adf", "GDAL_0_")

def test1(datapath):
    TestSection("DEM files")
    test0_and_1(datapath + "DEM/Morrison.dem", "GDAL_1_")

def main():
    a = GetAnnotationAttributes()
    a.databaseInfoFlag = 0
    a. userInfoFlag = 0
    a.axesFlag = 0
    a.bboxFlag = 0
    SetAnnotationAttributes(a)

    # Draw antialiased lines
    r = GetRenderingAttributes()
    r.antialiasing = 1
    SetRenderingAttributes(r)

    datapath = "../data/GDAL_test_data/"
    test0(datapath)
    test1(datapath)

main()
Exit()
