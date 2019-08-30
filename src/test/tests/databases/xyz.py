# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  xyz.py
#
#  Tests:      mesh      - 3D Points
#              plots     - Molecule
#
#  Defect ID:  none
#
#  Programmer: Jeremy Meredith
#  Date:       June 14, 2007
#
#  Modifications:
#    Eddie Rusu, Mon Aug 26 08:46:44 PDT 2019
#    Added test for cell-centered non VTK_VERTEX points.
#
# ----------------------------------------------------------------------------


def test_xyz_ascii_output(data_base_name, var, test_output):
    OpenDatabase(silo_data_path(data_base_name))
    AddPlot("Pseudocolor", var, 1, 1)
    DrawPlots()

    # Export results to database
    e = ExportDBAttributes()
    e.db_type = "XYZ"
    e.filename = "test_ex_db"
    ExportDatabase(e)
    time.sleep(1)

    # Stream the ascii text in from the exported database
    file_streamer = open(e.filename+'.xyz', 'r')
    string_to_test = file_streamer.read()
    file_streamer.close()

    # Test the string
    TestText(test_output, string_to_test)
    DeleteAllPlots()
    CloseDatabase(silo_data_path(data_base_name))



# Starting point in this script
OpenDatabase(data_path("xyz_test_data/nanowireTB23K298.xyz"),0, "XYZ_1.0")

AddPlot("Molecule", "element")
DrawPlots()

View3DAtts = GetView3D()
View3DAtts.viewNormal = (-0.882253, 0.0562832, 0.467398)
View3DAtts.focus = (26.3509, 22.1853, 31.5426)
View3DAtts.viewUp = (-0.0613441, 0.970619, -0.232672)
View3DAtts.viewAngle = 30
SetView3D(View3DAtts)

Test("xyz_01")
TimeSliderNextState()
Test("xyz_02")
TimeSliderNextState()
Test("xyz_03")
TimeSliderNextState()
Test("xyz_04")
TimeSliderNextState()
Test("xyz_05")
TimeSliderNextState()
Test("xyz_06")

CloseDatabase(data_path("xyz_test_data/nanowireTB23K298.xyz"))

# Test case where XYZ writer is cell-centered VTK_VERTEX data
# In this case, the baseline text is generated from the point-centered data.
DefineScalarExpression("vz2", 'recenter(vz, "zonal")')
test_xyz_ascii_output('galaxy0000.silo', 'vz2', 'xyz_07')

Exit()
