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
# ----------------------------------------------------------------------------

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'

OpenDatabase("../data/curv2d.silo")

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

