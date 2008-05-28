# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  length.py
#  Tests:      queries     - total length
#
#  Defect ID:  VisIt00007530
#
#  Programmer: Hank Childs
#  Date:       November 8, 2006
#
#  Modifications:
#
#    Hank Childs, Mon Nov 27 09:27:06 PST 2006
#    Forgot to remove annotations.
#
# ----------------------------------------------------------------------------

a = AnnotationAttributes()
TurnOffAllAnnotations(a)
a.axesFlag2D = 1
SetAnnotationAttributes(a)

# Test that we can do an l2norm of a degenerate ultra file.
OpenDatabase("../data/rect2d.silo")
AddPlot("Contour", "u")
DrawPlots()
Query("Total Length")
text = GetQueryOutputString()
TestText("length_01", text)

DeleteAllPlots()
OpenDatabase("../data/curve.visit")
AddPlot("Curve", "going_up") 
DrawPlots()

Query("Total Length")
text = GetQueryOutputString()
TestText("length_02", text)

q = GetQueryOverTimeAttributes()
q.timeType = q.Timestep
SetQueryOverTimeAttributes(q)
QueryOverTime("Total Length")
SetActiveWindow(2)
SetAnnotationAttributes(a)
Test("length_03")

Exit()
