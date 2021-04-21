# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  mesh_quality.py
#
#  Tests:      mesh      - 3D single domain
#              plots     - pseudocolor
#
#  Programmer: Alister Maguire
#  Date: Wed Apr 21 10:55:48 PDT 2021
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def TestSideVolume():
    #
    # This ensures that a bug fix is still working well.
    #
    TestSection("Side Volume")

    OpenDatabase(silo_data_path("wave.visit"))

    AddPlot("Pseudocolor", "mesh_quality/min_side_volume")
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (0.3, 0.8, 0.6)
    SetView3D(v)
    SetTimeSliderState(10)

    Test("min_side_volume_00")
    DeleteAllPlots()

    AddPlot("Pseudocolor", "mesh_quality/max_side_volume")
    DrawPlots()

    Test("max_side_volume_00")
    DeleteAllPlots()

    CloseDatabase(silo_data_path("wave.visit"))


def Main():
    TestSideVolume()

Main()
Exit()
