# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  Dune.py
#
#  Tests:      mesh      - 3D point mesh
#
#  Defect ID:  none
#
#  Programmer: Hank Childs
#  Date:       September 28, 2005
#
# ----------------------------------------------------------------------------

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

# First test that we can open a Dune file.
OpenDatabase("../data/dune_test_data/Dune_6_species_1K_FS.Dat")
AddPlot("Pseudocolor", "mass")
DrawPlots()

Test("Dune_01")

DeleteAllPlots()
AddPlot("FilledBoundary", "fragments")
DrawPlots()
Test("Dune_02")

SetTimeSliderState(50)
Test("Dune_03")

Query("Time")
text = GetQueryOutputString()
TestText("Dune_04", text)

nstates = TimeSliderGetNStates()
text = "The number of states is %d\n" %(nstates)
TestText("Dune_05", text)

Query("NumZones")
text = GetQueryOutputString()
TestText("Dune_06", text)

DeleteAllPlots()
OpenDatabase("../data/dune_test_data/Dune_6_species_1K_out.Dat")
DefineScalarExpression("Z", "coord(particles)[2]")
AddPlot("Pseudocolor", "Z")
DrawPlots()
Test("Dune_07")
QueryOverTime("Max")
SetActiveWindow(2)
SetAnnotationAttributes(a)
Test("Dune_08")

Exit()

