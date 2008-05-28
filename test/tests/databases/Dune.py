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
TurnOffAllAnnotations()
Test("Dune_08")

Exit()

