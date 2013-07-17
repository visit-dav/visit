# ----------------------------------------------------------------------------
#  MODES: serial
#  CLASSES: nightly
#
#  Test Case:  axistitles.py 
#
#  Tests:      Tests setting axis titles and units.
#
#  Programmer: Brad Whitlock
#  Date:       Thu Jul 28 11:07:57 PDT 2005
#
#  Modifications:
#    Brad Whitlock, Wed Apr 2 16:44:26 PST 2008
#    Modified the 3D test since setting the 3D font scale now actually works.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Kathleen Biagas, Thu Jul 11 08:36:38 PDT 2013
#    Remove legacy style annotation settings.
#
# ----------------------------------------------------------------------------

def SaveTestImage(name):
    # Save these images somewhat larger than a regular test case image
    # since the images contain a lot of text.
    swa = SaveWindowAttributes()
    swa.width = 500
    swa.height = 500
    swa.screenCapture = 0
    Test(name, swa)

#
# Test replacing 2D titles and units.
#
def Test2D():
    TestSection("Setting axis titles in 2D")
    OpenDatabase(silo_data_path("noise2d.silo"))

    AddPlot("Pseudocolor", "hardyglobal")
    DrawPlots()

    v = GetView2D()
    v.viewportCoords = (0.35, 0.95, 0.15, 0.95)
    SetView2D(v)

    a = GetAnnotationAttributes()
    a.axes2D.visible = 1
    a.axes2D.xAxis.label.visible = 0
    a.axes2D.yAxis.label.visible = 0
    a.axes2D.xAxis.title.visible = 1
    a.axes2D.yAxis.title.visible = 1
    a.axes2D.xAxis.title.font.scale = 2 #TitleFontHeight2D = 0.04
    a.axes2D.yAxis.title.font.scale = 2 #TitleFontHeight2D = 0.04
    SetAnnotationAttributes(a)
    SaveTestImage("axistitles_0_00")

    a.axes2D.xAxis.title.title = "New X Title"
    a.axes2D.xAxis.title.userTitle = 1
    SetAnnotationAttributes(a)
    SaveTestImage("axistitles_0_01")

    a.axes2D.yAxis.title.title = "New Y Title"
    a.axes2D.yAxis.title.userTitle = 1
    SetAnnotationAttributes(a)
    SaveTestImage("axistitles_0_02")

    a.axes2D.xAxis.title.units = "New X Units"
    a.axes2D.xAxis.title.userUnits = 1
    SetAnnotationAttributes(a)
    SaveTestImage("axistitles_0_03")

    a.axes2D.yAxis.title.units = "New Y Units"
    a.axes2D.yAxis.title.userUnits = 1
    SetAnnotationAttributes(a)
    SaveTestImage("axistitles_0_04")
    DeleteAllPlots()


#
# Test replacing 3D titles and units.
#
def Test3D():
    TestSection("Setting axis titles in 3D")
    OpenDatabase(silo_data_path("noise.silo"))

    AddPlot("Pseudocolor", "hardyglobal")
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (-0.749133, -0.494511, 0.440747)
    v.focus = (0, 0, 0)
    v.viewUp = (-0.588718, 0.802033, -0.10077)
    v.viewAngle = 30
    v.parallelScale = 17.3205
    v.nearPlane = -34.641
    v.farPlane = 34.641
    v.imagePan = (0, 0)
    v.imageZoom = 1
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    SetView3D(v)

    a = GetAnnotationAttributes()
    a.axes3D.visible = 1
    a.axes3D.xAxis.title.visible = 1
    a.axes3D.yAxis.title.visible = 1
    a.axes3D.zAxis.title.visible = 1
    a.axes3D.xAxis.title.font.scale = 2
    a.axes3D.yAxis.title.font.scale = 2
    a.axes3D.zAxis.title.font.scale = 2
    SetAnnotationAttributes(a)
    SaveTestImage("axistitles_1_00")

    a.axes3D.xAxis.title.title = "New X Title"
    a.axes3D.xAxis.title.userTitle = 1
    SetAnnotationAttributes(a)
    SaveTestImage("axistitles_1_01")

    a.axes3D.yAxis.title.title = "New Y Title"
    a.axes3D.yAxis.title.userTitle = 1
    SetAnnotationAttributes(a)
    SaveTestImage("axistitles_1_02")

    a.axes3D.zAxis.title.title = "New Z Title"
    a.axes3D.zAxis.title.userTitle = 1
    SetAnnotationAttributes(a)
    SaveTestImage("axistitles_1_03")

    a.axes3D.xAxis.title.units = "New X Units"
    a.axes3D.xAxis.title.userUnits = 1
    SetAnnotationAttributes(a)
    SaveTestImage("axistitles_1_04")

    a.axes3D.yAxis.title.units = "New Y Units"
    a.axes3D.yAxis.title.userUnits = 1
    SetAnnotationAttributes(a)
    SaveTestImage("axistitles_1_05")

    a.axes3D.zAxis.title.units = "New Z Units"
    a.axes3D.zAxis.title.userUnits = 1
    SetAnnotationAttributes(a)
    SaveTestImage("axistitles_1_06")
    DeleteAllPlots()

def main():
    Test2D()
    Test3D()

# Run all of the tests
main()

# Exit the test
Exit()
