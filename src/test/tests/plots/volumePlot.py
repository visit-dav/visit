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

def TestVolumeColorControlPoints():
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
    OpenDatabase(silo_data_path("noise.silo"))
    AddPlot("Volume", "hardyglobal")
    v = VolumeAttributes()
    v.lightingFlag = 0
    v.rendererType = v.RayCasting
    v.sampling = v.KernelBased
    ct = GetColorTable("hot_desaturated")
    v.GetColorControlPoints().ClearControlPoints()
    for i in xrange(ct.GetNumControlPoints()):
        v.GetColorControlPoints().AddControlPoints(ct.GetControlPoints(i))
    SetPlotOptions(v)

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

#FIXME: For some reason, if you render using the ray caster, 
#       attempting to render using the default renderer afterwards
#       will result in a blank test result. I have not been able
#       to reproduce this outside of the test suite. I created 
#       issue #3608 to track this. 

InitAnnotationsLegendOn()
TestVolumeColorControlPoints()
TestVolumeGaussianControlPoints()
TestVolumeAspect()
TestVolumeOpacity()
InitAnnotations()
TestVolumeScaling()
TestVolumeSampling()
TestOpacityAttenuation()

Exit()
