# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  cracksclipper.py
#
#  Tests:      operators - CracksClipper
#
#  Programmer: Kathleen Biagas
#  Date:       August 14, 2012
#
#  Modifications:
# ----------------------------------------------------------------------------

OpenDatabase(data_path("vtk_cracked_test_data/cracked_*.vtk database"))
AddPlot("Pseudocolor", "operators/CracksClipper/mesh/den")
pcAtts = PseudocolorAttributes()
pcAtts.minFlag = 1
pcAtts.maxFlag = 1
pcAtts.min = 1.9
pcAtts.max = 3.853
SetPlotOptions(pcAtts)


v = GetView3D()
v.viewNormal = (-0.507948, 0.663707, 0.549074)
v.focus = (0.5, 0.5, 0.5)
v.viewUp = (0.388198, 0.745409, -0.541911)
v.parallelScale = 0.866025
v.nearPlane = -1.73205
v.farPlane = 1.73205
SetView3D(v)

DrawPlots()

# Changing time states shows the cracks developing, and demonstrates
# the density calculation
Test("CracksClipper_00")
SetTimeSliderState(1)
Test("CracksClipper_01")
SetTimeSliderState(3)
Test("CracksClipper_02")
SetTimeSliderState(5)
Test("CracksClipper_03")
SetTimeSliderState(12)
Test("CracksClipper_04")

SetTimeSliderState(14)
# Show that normal vars can be used.
ChangeActivePlotsVar("ems")
# 
Test("CracksClipper_05")

Exit()
