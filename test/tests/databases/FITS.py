# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  FITS.py
#
#  Tests:      mesh      - 2D structured, 3D structured
#              plots     - Pseudocolor, TrueColor
#
#  Programmer: Brad Whitlock
#  Date:       Thu Jul 20 14:07:49 PST 2006
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def test0(datapath):
    TestSection("2D truecolor images")

    # Set up the plots.
    db = datapath + "mandrill.fits"
    OpenDatabase(db)
    AddPlot("Truecolor", "MANDRILL")
    DrawPlots()

    Test("FITS_0_00")

    DeleteAllPlots()
    CloseDatabase(db)


def test1(datapath):
    TestSection("2D pseudocolor images")

    # Set up the plots.
    db = datapath + "y2000m2.fits"
    OpenDatabase(db)
    AddPlot("Pseudocolor", "NGC7083 V 550nm {Intens}")
    pc = PseudocolorAttributes()
    pc.minFlag = 1
    pc.scaling = pc.Log  # Linear, Log, Skew
    pc.min = 0.001
    pc.colorTableName = "bluehot"
    SetPlotOptions(pc)
    DrawPlots()
    Test("FITS_1_00")
    DeleteAllPlots()
    CloseDatabase(db)

    # Set up the plots.
    db = datapath + "04May04_standard_*.fits database"
    OpenDatabase(db)
    AddPlot("Pseudocolor", "HD43583")
    DrawPlots()
    Test("FITS_1_01")

    SetTimeSliderState(1)
    Test("FITS_1_02")

    SetTimeSliderState(2)
    Test("FITS_1_03")

    DeleteAllPlots()
    CloseDatabase(db)


def test2(datapath):
    TestSection("3D pseudocolor data")

    # Set up the plots.
    db = datapath + "ngc6503.fits"
    OpenDatabase(db)
    AddPlot("Pseudocolor", "NGC6503")
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (0.406249, -0.543192, 0.734782)
    v.focus = (128, 128, 15.5)
    v.viewUp = (-0.273573, 0.694941, 0.664993)
    v.viewAngle = 30
    v.parallelScale = 181.682
    v.nearPlane = -363.363
    v.farPlane = 363.363
    v.imagePan = (0.0160165, 0.0533909)
    v.imageZoom = 1.05918
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (128, 128, 15.5)
    SetView3D(v)
    Test("FITS_2_00")
    DeleteAllPlots()

    AddPlot("Contour", "NGC6503")
    ca = ContourAttributes()
    ca.contourValue = (0.0075)
    ca.contourMethod = ca.Value  # Level, Value, Percent
    SetPlotOptions(ca)
    DrawPlots()
    Test("FITS_2_01")
    DeleteAllPlots()

    CloseDatabase(db)

def main():
    a = GetAnnotationAttributes()
    a.databaseInfoFlag = 0
    a. userInfoFlag = 0
    a.axesFlag = 0
    a.bboxFlag = 0
    a.legendInfoFlag = 0
    a.triadFlag = 0
    SetAnnotationAttributes(a)

    InvertBackgroundColor()

    datapath = "../data/FITS_test_data/"
    test0(datapath)
    test1(datapath)
    test2(datapath)

    InvertBackgroundColor()

main()
Exit()
