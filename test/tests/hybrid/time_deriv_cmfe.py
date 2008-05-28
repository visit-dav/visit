# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  time_deriv_cmfe.py
#
#  Defect ID:  '2715, '6897
#
#  Programmer: Hank Childs
#  Date:       January 12, 2006
#
# ----------------------------------------------------------------------------

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'


OpenDatabase("../data/wave.visit")


# Test general capability.
AddPlot("Pseudocolor", "time_derivative/conn_based/pressure")
DrawPlots()
Test("time_deriv_cmfe_01")

TimeSliderSetState(50)
Test("time_deriv_cmfe_02")

ChangeActivePlotsVar("time_derivative/pos_based/pressure")
Test("time_deriv_cmfe_03")

Exit()
