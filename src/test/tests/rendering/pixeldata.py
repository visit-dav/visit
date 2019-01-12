# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  openexr.py 
#
#  Tests:      saving pixel data, transparent backgrounds in saved images.
#
#  Programmer: Brad Whitlock
#  Date:       Thu Feb 15 14:16:45 PST 2018
#
#  Modifications:
# 
# ----------------------------------------------------------------------------

# Saves pixel data.
def SavePixelData(filename, alpha, depth, luminance, value):
    s = GetSaveWindowAttributes()
    s.outputToCurrentDirectory = 1
    s.family = 0
    s.fileName = filename
    s.width  = TestEnv.params["width"]
    s.height = TestEnv.params["height"]
    s.screenCapture = 0
    s.format = s.PNG
    s.pixelData = 1
    if alpha:
        s.pixelData = s.pixelData | 2
    if luminance:
        s.pixelData = s.pixelData | 4
    if value:
        s.pixelData = s.pixelData | 8
    if depth:
        s.pixelData = s.pixelData | 16
    SetSaveWindowAttributes(s)
    SaveWindow()

def test0():
    TestSection("Save transparent background image")
    db = silo_data_path("noise.silo")
    OpenDatabase(db)
    AddPlot("Contour", "hardyglobal")
    c = GetPlotOptions()
    c.SetMultiColor(2, (0, 0, 255, 100))
    c.SetMultiColor(3, (0, 255, 255, 100))
    c.SetMultiColor(4, (255, 0, 255, 100))
    SetPlotOptions(c)
    DrawPlots()
    v3 = GetView3D()
    v3.viewNormal = (-0.569008, 0.313204, 0.760351)
    v3.focus = (0, 0, 0)
    v3.viewUp = (0.185488, 0.949682, -0.252384)
    v3.viewAngle = 30
    v3.parallelScale = 17.3205
    v3.nearPlane = -34.641
    v3.farPlane = 34.641
    v3.imagePan = (-0.0120947, 0.0574413)
    v3.imageZoom = 1.13829
    v3.perspective = 1
    v3.eyeAngle = 2
    v3.centerOfRotationSet = 0
    v3.centerOfRotation = (0, 0, 0)
    v3.axis3DScaleFlag = 0
    v3.axis3DScales = (1, 1, 1)
    v3.shear = (0, 0, 1)
    v3.windowValid = 1
    SetView3D(v3)
    Test("pixeldata_0_00")
    # Save a partially transparent Contour plot on a transparent background.
    SavePixelData("pd0", 1, 0, 0, 0)

    # Save a partially transparent Contour plot on a transparent background -- with axes
    a = GetAnnotationAttributes()
    a.axes3D.bboxFlag = 1
    SetAnnotationAttributes(a)
    Test("pixeldata_0_01")
    SavePixelData("pd1", 1, 0, 0, 0)

    # Test that we get the transparent background when we have a gradient bg.
    a = GetAnnotationAttributes()
    a.backgroundMode = a.Gradient
    a.gradientColor1 = (180, 100, 180, 255)
    a.gradientColor2 = (0, 0, 0, 255)
    a.axes3D.bboxFlag = 0
    a.foregroundColor = (255,255,255,255)
    SetAnnotationAttributes(a)
    Test("pixeldata_0_02")
    SavePixelData("pd2", 1, 0, 0, 0)

    # Test that we get the transparent background when we have a gradient bg. with axes
    a.axes3D.bboxFlag = 1
    SetAnnotationAttributes(a)
    Test("pixeldata_0_03")
    SavePixelData("pd3", 1, 0, 0, 0)

    # Restore solid bg, no axes
    a.backgroundMode = a.Solid
    a.axes3D.bboxFlag = 0
    a.foregroundColor = (0,0,0,255)
    SetAnnotationAttributes(a)
    DeleteAllPlots()
    CloseDatabase(db)

    # Make some expressions that let us plot a transparent image with a checker background.
    DefineScalarExpression("i", "floor(coords(ImageMesh)[0] / 20)")
    DefineScalarExpression("j", "floor(coords(ImageMesh)[1] / 20)")
    DefineScalarExpression("checker", "mod(i+j, 2)")
    DefineScalarExpression("gray", "100+checker*80")
    DefineScalarExpression("t", "alpha / 255.")
    DefineScalarExpression("omt", "1 - t")
    DefineVectorExpression("blend", "color4(t*red + omt*gray,t*green + omt*gray,t*blue + omt*gray, 255)")
    DefineVectorExpression("comp", "if(lt(alpha, 255), blend, color4(red, green, blue, 255))")

    # Open the files we made.
    idx = 4
    for db in ("pd0.png", "pd1.png", "pd2.png", "pd3.png"):
        OpenDatabase(db)
        AddPlot("Pseudocolor", "alpha")
        DrawPlots()
        v = GetView2D()
        v.viewportCoords = (0, 1, 0, 1)
        SetView2D(v)
        Test("pixeldata_0_%02d" % idx)
        DeleteAllPlots()
        AddPlot("Truecolor", "comp")
        DrawPlots()
        Test("pixeldata_0_%02d" % (idx+1))
        DeleteAllPlots()
        CloseDatabase(db)
        idx = idx + 2

def test1():
    TestSection("Save image plus depth")
    db = silo_data_path("multi_rect3d.silo")
    OpenDatabase(db)
    AddPlot("Contour", "u")
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (-0.483301, 0.412135, 0.772376)
    v.focus = (0.5, 0.5, 0.5)
    v.viewUp = (0.218992, 0.911123, -0.349139)
    v.viewAngle = 30
    v.parallelScale = 0.866025
    v.nearPlane = -1.73205
    v.farPlane = 1.73205
    v.imagePan = (0, 0)
    v.imageZoom = 1
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.5, 0.5, 0.5)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)

    # Save against a solid background.
    Test("pixeldata_1_00")
    SavePixelData("pdz0", 0, 1, 0, 0)

    # Save against a solid background adding axes
    a = GetAnnotationAttributes()
    a.axes3D.bboxFlag = 1
    SetAnnotationAttributes(a)
    Test("pixeldata_1_01")
    SavePixelData("pdz1", 0, 1, 0, 0)

    # Save against a gradient background 
    a.backgroundMode = a.Gradient
    a.gradientColor1 = (180, 100, 180, 255)
    a.gradientColor2 = (0, 0, 0, 255)
    a.axes3D.bboxFlag = 0
    a.foregroundColor = (255,255,255,255)
    SetAnnotationAttributes(a)
    Test("pixeldata_1_02")
    SavePixelData("pdz2", 0, 1, 0, 0)

    # Save against a gradient background adding axes
    a.backgroundMode = a.Gradient
    a.gradientColor1 = (180, 100, 180, 255)
    a.gradientColor2 = (0, 0, 0, 255)
    a.axes3D.bboxFlag = 1
    a.foregroundColor = (255,255,255,255)
    SetAnnotationAttributes(a)
    Test("pixeldata_1_03")
    SavePixelData("pdz3", 0, 1, 0, 0)

    DeleteAllPlots()
    CloseDatabase(db)

    # Open the files we made.
    idx = 4
    for db in ("pdz0.png", "pdz1.png", "pdz2.png", "pdz3.png"):
        OpenDatabase(db)
        AddPlot("Pseudocolor", "depth")
        pc = GetPlotOptions()
        pc.max = 220.
        pc.maxFlag = 1
        SetPlotOptions(pc)
        DrawPlots()
        v = GetView2D()
        v.viewportCoords = (0, 1, 0, 1)
        SetView2D(v)
        Test("pixeldata_1_%02d" % idx)
        DeleteAllPlots()
        CloseDatabase(db)
        idx = idx + 1

def main():
    test0()
    test1()

main()
Exit()
