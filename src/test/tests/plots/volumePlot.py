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
#    Brad Whitlock, Thu Mar 12 09:17:57 PDT 2009
#    I changed freeformFlag to opacityMode to reflect changes to the plot
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Hank Childs, Sun Aug 29 16:07:56 PDT 2010
#    Change Log10 to Log, to reflect change in naming.
#
#    Brad Whitlock, Thu May 10 15:21:51 PDT 2018
#    Add test case for sampling types.
#
#    Alister Maguire, Tue Feb  5 14:17:13 PST 2019
#    Updated the aspect test to use a larger multiplier and no shading for
#    better visibility. Updated the scaling test to not use shading (again
#    for better visibility).
#
#    Alister Maguire, Mon Mar 25 11:19:54 PDT 2019
#    Added an opacity test that changes the opacity variable.
#
#    Alister Maguire, Wed Jun  5 11:01:31 PDT 2019
#    Added opacity attenuation test.
#
#    Alister Maguire, Fri Mar 20 15:36:37 PDT 2020
#    Added gradient lighting reduction test.
#
#    Kathleen Biagas, Wed May 25, 2022
#    Added test for command-recorded volume atts.
#
#    Kathleen Biagas, Fri June 3, 2022
#    Renamed TestVolumeColorControlPoints to volume_colors, use new
#    TestAutoName funcationality. Modified TestVolumeSampling to set the
#    ColorControlPoints directly from the retrieved color table rather than
#    calling AddControlPoints (it is now a quick recipe).
#
# ----------------------------------------------------------------------------

def InitAnnotations():
    # Turn off all annotation
    TurnOffAllAnnotations()

def InitAnnotationsLegendOn():
    # Turn off all annotation
    a = AnnotationAttributes()
    TurnOffAllAnnotations(a)
    a.legendInfoFlag = 1
    SetAnnotationAttributes(a)

def TestVolumeScaling():
    OpenDatabase(silo_data_path("rect3d.silo"))

    AddPlot("Volume", "t")
    volAtts = VolumeAttributes()
    volAtts.lightingFlag = 0
    SetPlotOptions(volAtts)
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (1, 0, 0)
    SetView3D(v)

    Test("volumeScaling_01")

    volAtts.scaling = volAtts.Log
    SetPlotOptions(volAtts)
    Test("volumeScaling_02")

    volAtts.scaling = volAtts.Skew
    volAtts.skewFactor = 0.0001
    SetPlotOptions(volAtts)
    Test("volumeScaling_03")

    volAtts.skewFactor = 1000
    SetPlotOptions(volAtts)
    Test("volumeScaling_04")

    volAtts.scaling = volAtts.Linear
    SetPlotOptions(volAtts)
    Test("volumeScaling_05")

    volAtts.scaling = volAtts.Log
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
    volAtts.lightingFlag = 0
    SetPlotOptions(volAtts)
    Test("volumeScaling_09")

    volAtts.scaling = volAtts.Log
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
    OpenDatabase(silo_data_path("noise.silo"))

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
    opac1 = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 136, 144, 152, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 255, 248, 240, 232, 224, 216, 208, 200, 192, 184, 176, 168, 160, 152, 144, 136, 128, 120, 112, 104, 96, 88, 80, 72, 64, 56, 48, 40, 32, 24, 16, 8, 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 136, 144, 152, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 255, 248, 240, 232, 224, 216, 208, 200, 192, 184, 176, 168, 160, 152, 144, 136, 128, 120, 112, 104, 96, 88, 80, 72, 64, 56, 48, 40, 32, 24, 16, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

    volAtts.SetFreeformOpacity(*opac1)
    SetPlotOptions(volAtts)
    Test("volumeOpacity_03")

    # setting opacity via creation of a list
    opac2 = []
    for j in range(32):
        opac2 = opac2 + [j*8]
    for j in range(32):
        opac2 = opac2 + [255-(j*8)]
    for j in range(128):
        opac2 = opac2 + [0]
    for j in range(32):
        opac2 = opac2 + [j*8]
    for j in range(32):
        opac2 = opac2 + [255-(j*8)]

    volAtts.SetFreeformOpacity(opac2)
    SetPlotOptions(volAtts)
    Test("volumeOpacity_04")

    #
    # Make sure we can change out opacity variable.
    #
    DeleteAllPlots()
    OpenDatabase(silo_data_path("globe.silo"))
    AddPlot("Volume", "dz")
    volAtts = VolumeAttributes()
    volAtts.opacityVariable = "v"
    volAtts.lightingFlag = 0
    SetPlotOptions(volAtts)
    DrawPlots()
    Test("volumeOpacity_05")

    DeleteAllPlots()


def TestOpacityAttenuation():

    OpenDatabase(silo_data_path("noise.silo"))
    ResetView()

    #
    # First, test the ray caster without reduced attenuation.
    #
    AddPlot("Volume", "hardyglobal")
    volAtts = VolumeAttributes()
    volAtts.lightingFlag = 0
    volAtts.opacityAttenuation = 1
    volAtts.rendererType = volAtts.RayCasting
    SetPlotOptions(volAtts)
    DrawPlots()
    Test("opacityAttenuation_01")

    #
    # Now reduce attenutation.
    #
    volAtts = VolumeAttributes()
    volAtts.lightingFlag = 0
    volAtts.rendererType = volAtts.RayCasting
    volAtts.opacityAttenuation = .12
    SetPlotOptions(volAtts)
    DrawPlots()
    Test("opacityAttenuation_02")

    DeleteAllPlots()


def TestVolumeAspect():
    OpenDatabase(silo_data_path("noise.silo"))

    orig_atts = VolumeAttributes()
    new_atts  = VolumeAttributes()
    new_atts.lightingFlag = 0

    AddPlot("Volume", "hardyglobal")
    SetPlotOptions(new_atts)

    DefineVectorExpression("disp", "{0,0,-0.9*coord(Mesh)[2]}")
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
    SetPlotOptions(orig_atts)

def volume_colors():
    """Volume Plot color control points"""
    # the next comment and similar below bracket code to be 'literalincluded' in quickrecipes.rst
    # removeControlPoints {
    OpenDatabase(silo_data_path("noise.silo"))

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
    # removeControlPoints }
    TestAutoName()

    # addControlPoints {
    # there are a default of 5 control points, add 3 more and change
    # positions of original  so everything is evenly spaced
    v = VolumeAttributes()
    v.colorControlPoints.GetControlPoints(0).position = 0
    v.colorControlPoints.GetControlPoints(1).position = 0.142857
    v.colorControlPoints.GetControlPoints(2).position = 0.285714
    v.colorControlPoints.GetControlPoints(3).position = 0.428571
    v.colorControlPoints.GetControlPoints(4).position = 0.571429
    tmp = ColorControlPoint()
    tmp.colors = (255,255,0,255)
    tmp.position = 0.714286
    v.GetColorControlPoints().AddControlPoints(tmp)
    tmp.colors = (0,255,0,255)
    tmp.position = 0.857143
    v.GetColorControlPoints().AddControlPoints(tmp)
    tmp.colors = (0,255,255,255)
    tmp.position = 1
    v.GetColorControlPoints().AddControlPoints(tmp)
    SetPlotOptions(v)
    # addControlPoints }
    TestAutoName()

    # setNumControlPoints {
    v = VolumeAttributes()
    # there are a default of 5, this resizes to 6
    v.colorControlPoints.SetNumControlPoints(6)
    v.colorControlPoints.GetControlPoints(4).position = 0.92
    # GetControlPoints(5) will cause a segfault without the call to SetNumControlPoints
    v.colorControlPoints.GetControlPoints(5).position = 1
    v.colorControlPoints.GetControlPoints(5).colors = (128,0,128,255)
    SetPlotOptions(v)
    # setNumControlPoints }
    TestAutoName()

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
    TestAutoName()
    DeleteAllPlots()

def TestVolumeGaussianControlPoints():
    OpenDatabase(silo_data_path("noise.silo"))

    AddPlot("Volume", "hardyglobal")

    v = VolumeAttributes()
    v.opacityMode = v.GaussianMode
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

def TestVolumeSampling():
    # setFromColorTable {
    OpenDatabase(silo_data_path("noise.silo"))
    AddPlot("Volume", "hardyglobal")
    v = VolumeAttributes()
    v.lightingFlag = 0
    v.rendererType = v.RayCasting
    v.sampling = v.KernelBased
    ct = GetColorTable("hot_desaturated")
    v.SetColorControlPoints(ct)
    SetPlotOptions(v)
    # setFromColorTable }

    view = GetView3D()
    view.viewNormal = (-1, 0, 0)
    view.focus = (0, 0, 0)
    view.viewUp = (0, 1, 0)
    view.viewAngle = 30
    view.parallelScale = 17.3205
    view.nearPlane = -34.641
    view.farPlane = 34.641
    view.imagePan = (0.0720459, -0.00108509)
    view.imageZoom = 1.85429
    view.perspective = 1
    view.eyeAngle = 2
    view.centerOfRotationSet = 0
    view.centerOfRotation = (0, 0, 0)
    view.axis3DScaleFlag = 0
    view.axis3DScales = (1, 1, 1)
    view.shear = (0, 0, 1)
    view.windowValid = 1
    DrawPlots()
    SetView3D(view)
    Test("volumeSampling_01")

    v.sampling = v.Rasterization
    SetPlotOptions(v)
    Test("volumeSampling_02")

    v.sampling = v.Trilinear
    SetPlotOptions(v)
    Test("volumeSampling_03")

    DeleteAllPlots()


def TestGradientLightingReduction():

    view = GetView3D()
    view.viewNormal = (0.746662961825451, 0.1647201021100829, 0.6444856161303283)
    SetView3D(view)

    OpenDatabase(silo_data_path("globe.silo"))
    AddPlot("Volume", "v")
    DrawPlots()

    #
    # Trilinear ray casting used to have very harsh results with
    # gradient lighting reduction. Let's make sure they're better
    # now.
    #
    v = VolumeAttributes()
    v.lightingFlag = 1
    v.rendererType = v.RayCasting
    v.sampling     = v.Trilinear

    v.lowGradientLightingReduction = v.Lower
    SetPlotOptions(v)
    Test("graidentLighting_00")

    v.lowGradientLightingReduction = v.Medium
    SetPlotOptions(v)
    Test("graidentLighting_01")

    v.lowGradientLightingReduction = v.Higher
    SetPlotOptions(v)
    Test("graidentLighting_02")

    DeleteAllPlots()
    CloseDatabase(silo_data_path("globe.silo"))

def TestCommandRecording():
    OpenDatabase(silo_data_path("globe.silo"))
    AddPlot("Volume", "u")
    DrawPlots()
    view3D = GetView3D()
    view3D.viewNormal = (0.0173089, 0.999712, 0.0165968)
    view3D.viewUp = (0.230674, 0.0121587, -0.972955)
    SetView3D(view3D)

    # Setting up of Volume plot atts from command recording
    VolumeAtts = VolumeAttributes()
    VolumeAtts.osprayShadowsEnabledFlag = 0
    VolumeAtts.osprayUseGridAcceleratorFlag = 0
    VolumeAtts.osprayPreIntegrationFlag = 0
    VolumeAtts.ospraySingleShadeFlag = 0
    VolumeAtts.osprayOneSidedLightingFlag = 0
    VolumeAtts.osprayAoTransparencyEnabledFlag = 0
    VolumeAtts.ospraySpp = 1
    VolumeAtts.osprayAoSamples = 0
    VolumeAtts.osprayAoDistance = 100000
    VolumeAtts.osprayMinContribution = 0.001
    VolumeAtts.legendFlag = 1
    VolumeAtts.lightingFlag = 1
    VolumeAtts.colorControlPoints.SetNumControlPoints(14)
    VolumeAtts.colorControlPoints.GetControlPoints(0).colors = (0, 0, 255, 255)
    VolumeAtts.colorControlPoints.GetControlPoints(0).position = 0
    VolumeAtts.colorControlPoints.GetControlPoints(1).colors = (0, 255, 255, 255)
    VolumeAtts.colorControlPoints.GetControlPoints(1).position = 0.0769231
    VolumeAtts.colorControlPoints.GetControlPoints(2).colors = (0, 255, 0, 255)
    VolumeAtts.colorControlPoints.GetControlPoints(2).position = 0.153846
    VolumeAtts.colorControlPoints.GetControlPoints(3).colors = (255, 255, 0, 255)
    VolumeAtts.colorControlPoints.GetControlPoints(3).position = 0.230769
    VolumeAtts.colorControlPoints.GetControlPoints(4).colors = (255, 255, 0, 255)
    VolumeAtts.colorControlPoints.GetControlPoints(4).position = 0.307692
    VolumeAtts.colorControlPoints.GetControlPoints(5).colors = (0, 255, 0, 255)
    VolumeAtts.colorControlPoints.GetControlPoints(5).position = 0.384615
    VolumeAtts.colorControlPoints.GetControlPoints(6).colors = (0, 255, 255, 255)
    VolumeAtts.colorControlPoints.GetControlPoints(6).position = 0.461538
    VolumeAtts.colorControlPoints.GetControlPoints(7).colors = (0, 0, 255, 255)
    VolumeAtts.colorControlPoints.GetControlPoints(7).position = 0.538462
    VolumeAtts.colorControlPoints.GetControlPoints(8).colors = (255, 0, 255, 255)
    VolumeAtts.colorControlPoints.GetControlPoints(8).position = 0.615385
    VolumeAtts.colorControlPoints.GetControlPoints(9).colors = (0, 0, 0, 255)
    VolumeAtts.colorControlPoints.GetControlPoints(9).position = 0.692308
    VolumeAtts.colorControlPoints.GetControlPoints(10).colors = (255, 255, 255, 255)
    VolumeAtts.colorControlPoints.GetControlPoints(10).position = 0.769231
    VolumeAtts.colorControlPoints.GetControlPoints(11).colors = (255, 0, 0, 255)
    VolumeAtts.colorControlPoints.GetControlPoints(11).position = 0.846154
    VolumeAtts.colorControlPoints.GetControlPoints(12).colors = (255, 255, 0, 255)
    VolumeAtts.colorControlPoints.GetControlPoints(12).position = 0.923077
    VolumeAtts.colorControlPoints.GetControlPoints(13).colors = (255, 0, 0, 255)
    VolumeAtts.colorControlPoints.GetControlPoints(13).position = 1
    VolumeAtts.colorControlPoints.smoothing = VolumeAtts.colorControlPoints.Linear
    VolumeAtts.colorControlPoints.equalSpacingFlag = 0
    VolumeAtts.colorControlPoints.discreteFlag = 0
    VolumeAtts.colorControlPoints.categoryName = ""
    VolumeAtts.opacityAttenuation = 1
    VolumeAtts.opacityMode = VolumeAtts.GaussianMode
    VolumeAtts.opacityControlPoints.SetNumControlPoints(20)
    VolumeAtts.opacityControlPoints.GetControlPoints(0).x = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(0).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(0).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(0).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(0).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(1).x = 0.075
    VolumeAtts.opacityControlPoints.GetControlPoints(1).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(1).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(1).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(1).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(2).x = 0.125
    VolumeAtts.opacityControlPoints.GetControlPoints(2).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(2).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(2).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(2).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(3).x = 0.175
    VolumeAtts.opacityControlPoints.GetControlPoints(3).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(3).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(3).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(3).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(4).x = 0.225
    VolumeAtts.opacityControlPoints.GetControlPoints(4).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(4).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(4).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(4).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(5).x = 0.275
    VolumeAtts.opacityControlPoints.GetControlPoints(5).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(5).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(5).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(5).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(6).x = 0.325
    VolumeAtts.opacityControlPoints.GetControlPoints(6).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(6).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(6).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(6).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(7).x = 0.375
    VolumeAtts.opacityControlPoints.GetControlPoints(7).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(7).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(7).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(7).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(8).x = 0.425
    VolumeAtts.opacityControlPoints.GetControlPoints(8).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(8).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(8).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(8).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(9).x = 0.475
    VolumeAtts.opacityControlPoints.GetControlPoints(9).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(9).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(9).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(9).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(10).x = 0.525
    VolumeAtts.opacityControlPoints.GetControlPoints(10).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(10).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(10).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(10).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(11).x = 0.575
    VolumeAtts.opacityControlPoints.GetControlPoints(11).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(11).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(11).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(11).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(12).x = 0.625
    VolumeAtts.opacityControlPoints.GetControlPoints(12).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(12).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(12).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(12).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(13).x = 0.675
    VolumeAtts.opacityControlPoints.GetControlPoints(13).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(13).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(13).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(13).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(14).x = 0.725
    VolumeAtts.opacityControlPoints.GetControlPoints(14).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(14).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(14).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(14).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(15).x = 0.775
    VolumeAtts.opacityControlPoints.GetControlPoints(15).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(15).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(15).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(15).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(16).x = 0.825
    VolumeAtts.opacityControlPoints.GetControlPoints(16).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(16).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(16).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(16).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(17).x = 0.875
    VolumeAtts.opacityControlPoints.GetControlPoints(17).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(17).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(17).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(17).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(18).x = 0.925
    VolumeAtts.opacityControlPoints.GetControlPoints(18).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(18).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(18).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(18).yBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(19).x = 0.975
    VolumeAtts.opacityControlPoints.GetControlPoints(19).height = 0.5
    VolumeAtts.opacityControlPoints.GetControlPoints(19).width = 0.025
    VolumeAtts.opacityControlPoints.GetControlPoints(19).xBias = 0
    VolumeAtts.opacityControlPoints.GetControlPoints(19).yBias = 0
    VolumeAtts.resampleFlag = 1
    VolumeAtts.resampleTarget = 1000000
    VolumeAtts.opacityVariable = "default"
    VolumeAtts.compactVariable = "default"
    VolumeAtts.freeformOpacity = (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255)
    VolumeAtts.useColorVarMin = 0
    VolumeAtts.colorVarMin = 0
    VolumeAtts.useColorVarMax = 0
    VolumeAtts.colorVarMax = 0
    VolumeAtts.useOpacityVarMin = 0
    VolumeAtts.opacityVarMin = 0
    VolumeAtts.useOpacityVarMax = 0
    VolumeAtts.opacityVarMax = 0
    VolumeAtts.smoothData = 0
    VolumeAtts.samplesPerRay = 500
    VolumeAtts.rendererType = VolumeAtts.Default
    VolumeAtts.gradientType = VolumeAtts.SobelOperator
    VolumeAtts.scaling = VolumeAtts.Linear
    VolumeAtts.skewFactor = 1
    VolumeAtts.limitsMode = VolumeAtts.OriginalData
    VolumeAtts.sampling = VolumeAtts.Rasterization
    VolumeAtts.rendererSamples = 3
    VolumeAtts.lowGradientLightingReduction = VolumeAtts.Lower
    VolumeAtts.lowGradientLightingClampFlag = 0
    VolumeAtts.lowGradientLightingClampValue = 1
    VolumeAtts.materialProperties = (0.4, 0.75, 0, 15)
    SetPlotOptions(VolumeAtts)

    Test("vol_commandRecorded")
    ResetView()

    DeleteAllPlots()
    CloseDatabase(silo_data_path("globe.silo"))


#FIXME: For some reason, if you render using the ray caster,
#       attempting to render using the default renderer afterwards
#       will result in a blank test result. I have not been able
#       to reproduce this outside of the test suite. I created
#       issue #3608 to track this.

InitAnnotationsLegendOn()
volume_colors()
TestVolumeGaussianControlPoints()
# This test hangs in parallel.
if TestEnv.params["serial"]:
    TestVolumeAspect()
TestVolumeOpacity()
TestCommandRecording()
InitAnnotations()
TestVolumeScaling()
TestVolumeSampling()
TestOpacityAttenuation()
TestGradientLightingReduction()

Exit()
