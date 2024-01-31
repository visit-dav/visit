# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  lines.py
#
#  Tests:      mesh      - 2D lines (unstructured), 3D lines (unstructured)
#              plots     - mesh
#
#  Programmer: Alister Maguire
#  Date:       Tue Mar 17 08:50:32 PDT 2020
#
#  Modifications:
#
#    Mark C. Miller, Mon Jan 11 10:32:17 PST 2021
#    Replace AssertEqual() with TestValueEQ()
# ----------------------------------------------------------------------------

def TestMeshPlot():

    #
    # First, let's make sure that 3d lines are read appropriately.
    #
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)

    OpenDatabase(data_path("lines_test_data/spring.lines"))
    AddPlot("Mesh", "Lines", 1, 1)
    DrawPlots()
    Query("SpatialExtents")

    # Check dimensionality.
    ext_len = len(GetQueryOutputValue())
    TestValueEQ("Verifying 3D lines", ext_len, 6)

    # Check the rendering.
    Test("mesh_plot_00")
    DeleteAllPlots()
    CloseDatabase(data_path("lines_test_data/spring.lines"))

    #
    # Next, let's check 2d lines.
    #
    OpenDatabase(data_path("lines_test_data/2d.lines"))
    AddPlot("Mesh", "Lines", 1, 1)
    DrawPlots()
    Query("SpatialExtents")

    # Check dimensionality.
    ext_len = len(GetQueryOutputValue())
    TestValueEQ("Verifying 2D lines", ext_len, 4)

    # Check the rendering.
    Test("mesh_plot_01")
    DeleteAllPlots()

    CloseDatabase(data_path("lines_test_data/2d.lines"))

    #
    # This test makes sure that consecutive points are only
    # removed from one line at a time.
    #
    OpenDatabase(data_path("lines_test_data/consecutive.lines"))
    AddPlot("Mesh", "Lines", 1, 1)
    DrawPlots()

    # Check the rendering.
    Test("mesh_plot_02")
    DeleteAllPlots()

    CloseDatabase(data_path("lines_test_data/consecutive.lines"))


def main():
    TestMeshPlot()
    Exit()

main()
