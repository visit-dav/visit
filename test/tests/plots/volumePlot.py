# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  volumePlot.py
#
#  Tests:      mesh      - 3D rectilinear, one domain
#              plots     - volume 
#              operators - none
#              selection - no 
#
#  Programmer: Kathleen Bonnell 
#  Date:       March 4, 2005 
#
#  Modifications:
#    Kathleen Bonnell, Fri Oct 14 10:12:06 PDT 2005
#    Added InitAnnotationsLegendOn and TestVolumeOpacity per '6670.
#
#    Hank Childs, Fri May 26 10:11:52 PDT 2006
#    Add test for extremely high aspect ratio meshes.  ['7250]
#
#    Brad Whitlock, Wed Mar 14 17:18:05 PST 2007
#    Added tests for color control points and gaussian control points.
#
# ----------------------------------------------------------------------------

def InitAnnotations():
    # Turn off all annotation
    a = AnnotationAttributes()
    a.axesFlag2D = 0
    a.axesFlag = 0
    a.triadFlag = 0
    a.bboxFlag = 0
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    a.legendInfoFlag = 0
    a.backgroundMode = 0
    a.foregroundColor = (0, 0, 0, 255)
    a.backgroundColor = (255, 255, 255, 255)
    SetAnnotationAttributes(a)

def InitAnnotationsLegendOn():
    # Turn off all annotation
    a = AnnotationAttributes()
    a.axesFlag2D = 0
    a.axesFlag = 0
    a.triadFlag = 0
    a.bboxFlag = 0
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    a.legendInfoFlag = 1
    a.backgroundMode = 0
    a.foregroundColor = (0, 0, 0, 255)
    a.backgroundColor = (255, 255, 255, 255)
    SetAnnotationAttributes(a)

def TestVolumeScaling():
    OpenDatabase("../data/rect3d.silo")
    AddPlot("Volume", "t")
    volAtts = VolumeAttributes()
    volAtts.rendererType = volAtts.Splatting
    SetPlotOptions(volAtts)
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (1, 0, 0)
    SetView3D(v)

    Test("volumeScaling_01")

    volAtts.scaling = volAtts.Log10
    SetPlotOptions(volAtts)
    Test("volumeScaling_02")

    volAtts.scaling = volAtts.Skew
    volAtts.skewFactor = 0.0001
    SetPlotOptions(volAtts)
    Test("volumeScaling_03")

    volAtts.skewFactor = 1000
    SetPlotOptions(volAtts)
    Test("volumeScaling_04")

    volAtts.rendererType = volAtts.Texture3D
    volAtts.scaling = volAtts.Linear
    SetPlotOptions(volAtts)
    Test("volumeScaling_05")

    volAtts.scaling = volAtts.Log10
    SetPlotOptions(volAtts)
    Test("volumeScaling_06")

    volAtts.scaling = volAtts.Skew
    volAtts.skewFactor = 0.0001
    SetPlotOptions(volAtts)
    Test("volumeScaling_07")

    volAtts.skewFactor = 1000
    SetPlotOptions(volAtts)
    Test("volumeScaling_08")

    volAtts.rendererType = volAtts.RayCasting
    volAtts.scaling = volAtts.Linear
    SetPlotOptions(volAtts)
    Test("volumeScaling_09")

    volAtts.scaling = volAtts.Log10
    SetPlotOptions(volAtts)
    Test("volumeScaling_10")

    volAtts.scaling = volAtts.Skew
    volAtts.skewFactor = 0.0001
    SetPlotOptions(volAtts)
    Test("volumeScaling_11")

    volAtts.skewFactor = 1000
    SetPlotOptions(volAtts)
    Test("volumeScaling_12")

    DeleteAllPlots()

def TestVolumeOpacity():
    OpenDatabase("../data/noise.silo")
    AddPlot("Volume", "hardyglobal")
    volAtts = VolumeAttributes()
    SetPlotOptions(volAtts)
    DrawPlots()
    ResetView()
    # default opacity
    Test("volumeOpacity_01")

    # setting opacity via individual index and value
    for i in range(150):
        volAtts.SetFreeformOpacity(i, 0) 

    SetPlotOptions(volAtts)
    Test("volumeOpacity_02")

    # setting opacity via creation of a tuple
    opac1 = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 136, 144, 152, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 256, 248, 240, 232, 224, 216, 208, 200, 192, 184, 176, 168, 160, 152, 144, 136, 128, 120, 112, 104, 96, 88, 80, 72, 64, 56, 48, 40, 32, 24, 16, 8, 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 136, 144, 152, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 256, 248, 240, 232, 224, 216, 208, 200, 192, 184, 176, 168, 160, 152, 144, 136, 128, 120, 112, 104, 96, 88, 80, 72, 64, 56, 48, 40, 32, 24, 16, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    volAtts.SetFreeformOpacity(opac1)
    SetPlotOptions(volAtts)
    Test("volumeOpacity_03")

    # setting opacity via creation of a list
    opac2 = []
    for j in range(32):
        opac2 = opac2 + [j*8]
    for j in range(32):
        opac2 = opac2 + [256-(j*8)]
    for j in range(128):
        opac2 = opac2 + [0]
    for j in range(32):
        opac2 = opac2 + [j*8]
    for j in range(32):
        opac2 = opac2 + [256-(j*8)]

    volAtts.SetFreeformOpacity(opac2)
    SetPlotOptions(volAtts)
    Test("volumeOpacity_04")

    DeleteAllPlots()

def TestVolumeAspect():
    OpenDatabase("../data/noise.silo")
    AddPlot("Volume", "hardyglobal")
    DefineVectorExpression("disp", "{0,0,-0.9999*coord(Mesh)[2]}")
    AddOperator("Displace")
    d = DisplaceAttributes()
    d.variable = "disp"
    SetOperatorOptions(d)
    DrawPlots()
    ResetView()
    v = GetView3D()
    v.viewNormal = (-0.324974, 0.839345, 0.435765)
    SetView3D(v)
    Test("volumeAspect_01")
    DeleteAllPlots()

def TestVolumeColorControlPoints():
    OpenDatabase("../data/noise.silo")
    AddPlot("Volume", "hardyglobal")

    # Modify colors. The default color table has 5 control points. Delete
    # all but 2 of them and then change their colors.
    v = VolumeAttributes()
    v.colorControlPoints.RemoveControlPoints(4)
    v.colorControlPoints.RemoveControlPoints(3)
    v.colorControlPoints.RemoveControlPoints(2)
    v.colorControlPoints.GetControlPoints(0).colors = (255,0,0,255)
    v.colorControlPoints.GetControlPoints(0).position = 0.
    v.colorControlPoints.GetControlPoints(1).colors = (0,0,255,255)
    v.colorControlPoints.GetControlPoints(1).position = 1.
    SetPlotOptions(v)
    DrawPlots()
    ResetView()
    Test("volumeColors_01")

    # Start over with the colors.
    v.GetColorControlPoints().ClearControlPoints()
    tmp = ColorControlPoint()
    npts = 10
    for i in range(npts):
        t = 0.
        if i < npts/2:
            t = float(i) / float(npts/2 - 1)
            r = 255
            g = int(t * 255.)
            b = 0
        else:
            t = 1. - (float(i-(npts/2)) / float(npts/2 - 1))
            r = int((1-t) * 255.)
            g = int((1-t) * 255.)
            b = int(t * 255.)
        tmp.colors = (r,g,b,255)
        tmp.position = float(i) / float(npts-1)
        v.GetColorControlPoints().AddControlPoints(tmp)
    SetPlotOptions(v)
    Test("volumeColors_02")
    DeleteAllPlots()

def TestVolumeGaussianControlPoints():
    OpenDatabase("../data/noise.silo")
    AddPlot("Volume", "hardyglobal")

    v = VolumeAttributes()
    v.freeformFlag = 0
    v.opacityControlPoints.ClearControlPoints()
    tmp = GaussianControlPoint()
    # Design gaussians
    tmp.x = 0.818182
    tmp.height = 0.787879
    tmp.width = 0.147059
    tmp.xBias = 0
    tmp.yBias = 0
    v.GetOpacityControlPoints().AddControlPoints(tmp)

    tmp.x = 0.203209
    tmp.height = 0.590909
    tmp.width = 0.0588235
    tmp.xBias = 0.0026738
    tmp.yBias = 2
    v.GetOpacityControlPoints().AddControlPoints(tmp)

    tmp.x = 0.47861
    tmp.height = 0.19697
    tmp.width = 0.0721925
    tmp.xBias = -0.00802138
    tmp.yBias = 0
    v.GetOpacityControlPoints().AddControlPoints(tmp)

    SetPlotOptions(v)
    DrawPlots()
    ResetView()
    Test("volumeGaussian_01")
    DeleteAllPlots()

InitAnnotationsLegendOn()
TestVolumeColorControlPoints()
TestVolumeGaussianControlPoints()
TestVolumeAspect()
TestVolumeOpacity()
InitAnnotations()
TestVolumeScaling()
Exit()
