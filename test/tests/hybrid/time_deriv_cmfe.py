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
