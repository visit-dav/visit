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
    CloseDatabase(silo_data_path("noise.silo"))


def main():
    TestOsprayWindowSave()
    Exit()

main()

