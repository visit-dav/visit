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
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Kathleen Biagas, Wed Aug 28 09:04:00 MST 2019
#    Turn off cycling of colors for all Curve plot tests.  Set the colors
#    individually to match current baseline results.
#
# ----------------------------------------------------------------------------

# Test that we can do an l2norm of a degenerate ultra file.
OpenDatabase(silo_data_path("rect2d.silo"))

AddPlot("Contour", "u")
DrawPlots()
Query("Total Length")
text = GetQueryOutputString()
TestText("length_01", text)

DeleteAllPlots()
OpenDatabase(data_path("curve_test_data/curve.visit"))

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
TurnOffAllAnnotations()
c = CurveAttributes()
c.curveColorSource = c.Custom
c.curveColor = (0, 255, 0, 255)
SetPlotOptions(c)
Test("length_03")

Exit()
