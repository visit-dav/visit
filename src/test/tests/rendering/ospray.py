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
#    Added a test for OSPRay domain boundaries.
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

    Test("ospray_window_save", pixdiff=0.2, avgdiff=0.01)
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
    # We need to first reset, otherwise the previous test ends up affecting
    # the view here.
    #
    ResetView()
    View3DAtts = View3DAttributes()
    View3DAtts.viewNormal = (0.171986, 0.984799, 0.0243329)
    View3DAtts.viewUp = (-0.187983, 0.0570564, -0.980514)
    SetView3D(View3DAtts)

    #
    # Add our plot and set up the ospray renderer.
    #
    AddPlot("Volume", "Regular/temperature", 1, 1)
    SetActivePlots(0)

    VolumeAtts = VolumeAttributes()
    VolumeAtts.lightingFlag = 1
    VolumeAtts.opacityMode = VolumeAtts.FreeformMode
    VolumeAtts.freeformOpacity = (255,) * 256
    VolumeAtts.samplesPerRay = 100
    VolumeAtts.rendererType = VolumeAtts.Parallel
    VolumeAtts.resampleType = VolumeAtts.OnlyIfRequired
    VolumeAtts.OSPRayEnabledFlag = 1
    SetPlotOptions(VolumeAtts)
    DrawPlots()

    #
    # NOTE: This test always has a slight pixel diff (~0.01%).
    #
    Test("ospray_domain_boundaries_00", pixdiff=0.1, avgdiff=0.01)

    DeleteAllPlots()
    CloseDatabase(data_path("vtk_test_data/poiseuille_rayleigh_benard_3D_00000600.visit"))


def main():
    TestOsprayWindowSave()
    TestOsprayVolumeDomainBoundaries()

    Exit()

main()

