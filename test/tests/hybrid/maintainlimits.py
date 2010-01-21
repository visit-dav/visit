# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  maintainlimits.py
#
#  Tests:      mesh      - 2D curvilinear
#              plots     - pseudocolor, contour
#
#  Defect ID:  none
#
#  Programmer: Eric Brugger
#  Date:       April 8, 2004
#
#  Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------


OpenDatabase("../data/silo_%s_test_data/curv2d.silo"%SILO_MODE)

# Test maintain data with the pseudocolor plot.
ToggleMaintainDataMode()
AddPlot("Pseudocolor", "p")
DrawPlots()

ChangeActivePlotsVar("u")
Test("maintaindata1")

ToggleMaintainDataMode()
Test("maintaindata2")

# Test maintain data with the contour plot.
DeleteAllPlots()
ToggleMaintainDataMode()
AddPlot("Contour", "p")
DrawPlots()

ChangeActivePlotsVar("u")
Test("maintaindata3")

ToggleMaintainDataMode()
Test("maintaindata4")

Exit()

