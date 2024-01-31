# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  multi_format.py 
#
#  Tests:      Using multiple file formats types in the same engine.
#
#  Programmer: Hank Childs
#  Creation:   March 22, 2004
#
#  Modifications:
#    Brad Whitlock, Wed Mar 9 09:15:30 PDT 2005
#    Removed deprecated functions.
#
#    Tom Fogal, Tue Dec 15 17:34:00 MST 2009
#    Fix path to data.
#
# ----------------------------------------------------------------------------


db = (data_path("silo_pdb_test_data/wave*.silo database"),
      data_path("pdb_test_data/allinone00.pdb"))

# Create a Pseudocolor plot of wave.
OpenDatabase(db[0])
AddPlot("Pseudocolor", "pressure")
DrawPlots()

# Set the view.
v0 = View3DAttributes()
v0.viewNormal = (-0.661743, 0.517608, 0.542382)
v0.focus = (5, 0.757692, 2.5)
v0.viewUp = (0.370798, 0.854716, -0.363277)
v0.viewAngle = 30
v0.parallelScale = 5.63924
v0.nearPlane = -11.2785
v0.farPlane = 11.2785
v0.imagePan = (0.00100868, 0.0454815)
v0.imageZoom = 1.17935
v0.perspective = 1
SetView3D(v0)
Test("multi_format_01")

# Create another window and make sure that it has the wave database as its
# active source.
CloneWindow()
SetActiveWindow(2)
DeleteAllPlots()

# Open the allinone database and make sure that it becomes the active source.
OpenDatabase(db[1])

# Create a plot from it.
AddPlot("FilledBoundary", "material(mesh)")
DrawPlots()
Test("multi_format_02")
# note: py3 div creates float
SetTimeSliderState(int(TimeSliderGetNStates() / 2))
Test("multi_format_03")

# Switch back to window 1.
SetActiveWindow(1)
Test("multi_format_04")

Exit()

