# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  ospray.py
#
#  Tests:      mesh      - 3D rectilinear, one domain
#              plots     - pseudocolor
#              operators - Isovolume
#
#  Programmer: Alister Maguire
#  Date:       May 18, 2020
#
#  Modifications:
#
#    Alister Maguire, Fri Sep 11 16:16:49 PDT 2020
#    Added a test for OSPRay domain boundaries. Note that I've commented
#    it out until we can set a pixel diff tolerance.
#
# ----------------------------------------------------------------------------


def TestOsprayWindowSave():
    #
    # This is testing a resolution of github issue #4286 where ospray
    # was not being used when SaveWindow was called, regardless of
    # whether or not we enabeld it.
    #
    OpenDatabase(silo_data_path("noise.silo"))

    #
    # OSPRay looks pretty similar, so let's make sure that we
    # enable shadows and can see them in the output image.
    #
    AddPlot("Pseudocolor", "hardyglobal", 1, 1)
    AddOperator("Isovolume", 1)
    SetActivePlots(0)
    IsovolumeAtts = IsovolumeAttributes()
    IsovolumeAtts.lbound = 4.1
    IsovolumeAtts.ubound = 4.9
    IsovolumeAtts.variable = "default"
    SetOperatorOptions(IsovolumeAtts, 0, 1)
    DrawPlots()

    RenderingAtts = RenderingAttributes()
    RenderingAtts.osprayRendering = 1
    RenderingAtts.osprayShadows = 1
    SetRenderingAttributes(RenderingAtts)

    Test("ospray_window_save")
    DeleteAllPlots()

    RenderingAtts = RenderingAttributes()
    RenderingAtts.osprayRendering = 0
    RenderingAtts.osprayShadows = 0
    SetRenderingAttributes(RenderingAtts)

    CloseDatabase(silo_data_path("noise.silo"))


def TestOsprayVolumeDomainBoundaries():
    OpenDatabase(data_path("vtk_test_data/poiseuille_rayleigh_benard_3D_00000600.visit"))

    #
    # Change the view so that we get a good look at the domain boundaries.
    #
    ResetView()
    View3DAtts = View3DAttributes()
    View3DAtts.viewNormal = (0.171986, 0.984799, 0.0243329)
    View3DAtts.focus = (75, 40, 40)
    View3DAtts.viewUp = (-0.187983, 0.0570564, -0.980514)
    View3DAtts.viewAngle = 30
    View3DAtts.parallelScale = 93.9415
    View3DAtts.nearPlane = -187.883
    View3DAtts.farPlane = 187.883
    View3DAtts.imagePan = (0, 0)
    View3DAtts.imageZoom = 1
    View3DAtts.perspective = 1
    View3DAtts.eyeAngle = 2
    View3DAtts.centerOfRotationSet = 0
    View3DAtts.centerOfRotation = (75, 40, 40)
    View3DAtts.axis3DScaleFlag = 0
    View3DAtts.axis3DScales = (1, 1, 1)
    View3DAtts.shear = (0, 0, 1)
    View3DAtts.windowValid = 1
    SetView3D(View3DAtts)

    #
    # Add our plot and set up the ospray renderer.
    #
    AddPlot("Volume", "Regular/temperature", 1, 1)
    SetActivePlots(0)

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
    VolumeAtts.lightingFlag = 0
    VolumeAtts.opacityAttenuation = 1
    VolumeAtts.opacityMode = VolumeAtts.FreeformMode
    VolumeAtts.freeformOpacity = (255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                  255, 255, 255, 255, 255, 255)
    VolumeAtts.samplesPerRay = 100
    VolumeAtts.rendererType = VolumeAtts.RayCastingOSPRay
    SetPlotOptions(VolumeAtts)
    DrawPlots()

    Test("ospray_domain_boundaries_00")

    DeleteAllPlots()
    CloseDatabase(data_path("vtk_test_data/poiseuille_rayleigh_benard_3D_00000600.visit"))


def main():
    TestOsprayWindowSave()

    #FIXME: this test always has a slight pixel diff (~0.01%). We
    # need to add the ability to set pixel diff tolerance for
    # individual tests before we can run this in our nightly.
    # Currently, it's in the skip list.
    TestOsprayVolumeDomainBoundaries()
    Exit()

main()

