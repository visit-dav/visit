# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  pathlines.py
#
#  Tests:      streamline - 3D rectilinear
#              plots      - streamline
#
#  Defect ID:  -
#
#  Programmer: Hank Childs
#  Date:       March 11, 2012
# ----------------------------------------------------------------------------

OpenDatabase(data_path("pics_test_data/pathline_test.pics"))


AddPlot("Streamline", "velocity", 1, 0)

View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (0.270729, 0.624198, 0.732859)
View3DAtts.focus = (0.496062, 0.99603, 0.496062)
View3DAtts.viewUp = (-0.0922782, 0.774611, -0.62567)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 12.1829
View3DAtts.nearPlane = -24.3658
View3DAtts.farPlane = 24.3658
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 1
View3DAtts.perspective = 1
View3DAtts.eyeAngle = 2
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
SetView3D(View3DAtts)

s = StreamlineAttributes()
s.sourceType = s.SpecifiedPoint
s.pointSource = (0.5, 0.1, 0.1)
s.pathlines = 1
s.integrationType = s.Euler
s.maxStepLength = 0.01
s.legendMinFlag = 1
s.legendMin = 1.5
s.legendMaxFlag = 1
s.legendMax = 5.7
s.pathlinesCMFE = s.CONN_CMFE
SetPlotOptions(s)
DrawPlots()
Test("pathlines_01")

s.terminateByTime = 1
s.termTime = 3
SetPlotOptions(s)
Test("pathlines_02")

TimeSliderSetState(6)
e = GetLastError()
TestText("pathlines_03", e)

s.integrationDirection = s.Backward
SetPlotOptions(s)
DrawPlots()
Test("pathlines_04")

s.terminateByTime = 0
SetPlotOptions(s)
Test("pathlines_05")

s.pathlinesOverrideStartingTimeFlag = 1
s.pathlinesOverrideStartingTime = 3.5
SetPlotOptions(s)
Test("pathlines_06")

TimeSliderSetState(0)
s.pathlinesCMFE = s.POS_CMFE
SetPlotOptions(s)
Test("pathlines_07")

s.pathlinesCMFE = s.POS_CMFE
SetPlotOptions(s)
Test("pathlines_08")

s.pathlinesOverrideStartingTimeFlag = 0
SetPlotOptions(s)
e = GetLastError()
TestText("pathlines_09", e)

Exit()
