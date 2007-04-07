# ---------------------------------------------------------------------------- 
#  CLASSES: nightly
#
#  Test Case:  pickarray.py
#  Tests:      queries     - Pick on array variables
#
#  Defect ID:  '6862, '3191, '7323, '7324
#
#  Programmer: Hank Childs
#  Date:       May 25, 2006
#
#  Modifications:
#
#    Hank Childs, Tue Sep  5 17:08:59 PDT 2006
#    Add more tests for indexing bug ['7324].
#
#    Kathleen Bonnell, Wed Oct 18 11:43:27 PDT 2006 
#    Add more tests for bug '7498.
#
#    Hank Childs, Fri Jan  5 13:51:41 PST 2007
#    Add tests for time varying databases ['7323].
#
#    Hank Childs, Fri Jan 12 17:31:15 PST 2007
#    Added tests for array variables with bin widths.
#
# ----------------------------------------------------------------------------

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

OpenDatabase("../data/rect2d.silo")
AddPlot("Pseudocolor","d")
DrawPlots()

DefineArrayExpression("arr", "array_compose(d, p)")
Pick((0.5, 0.5), "arr")
text = GetPickOutput()
TestText("pickarray_01", text)

SetActiveWindow(2)
SetAnnotationAttributes(a)

Test("pickarray_02")

# Now make sure future picks cause the window to update.
SetActiveWindow(1)
Pick((0.5, 1.0), "arr")
text = GetPickOutput()
TestText("pickarray_03", text)

SetActiveWindow(2)
Test("pickarray_04")

SetActiveWindow(1)
DeleteAllPlots()
OpenDatabase("../data/wave.visit")
AddPlot("Pseudocolor", "pressure")
DrawPlots()
DefineArrayExpression("arr2", "array_compose(u, v)")
ZonePick((0.29, 0.47, 5.0), "arr2")
text = GetPickOutput()
TestText("pickarray_05", text)

#  bug '7498.
DeleteAllPlots()
OpenDatabase("../data/globe.silo")
DefineArrayExpression("arr3", "array_compose(dx, dy, dz)")
AddPlot("Pseudocolor", "u")
DrawPlots()

vars = ("u", "arr3")
Pick(128, 107, vars)
text = GetPickOutput() + "\n"

e = ExportDBAttributes()
e.db_type = "VTK"
e.filename = "current/globe_temp"
e.variables = ("u", "dx", "dy")
ExportDatabase(e)
ReplaceDatabase("current/globe_temp.vtk")
Pick(128, 107, vars)
text += GetPickOutput()
text += "\n"

DeleteExpression("arr3")
DefineArrayExpression("arr3", "array_compose(dx, dy)")
Pick(128, 107, vars)
text += GetPickOutput()
TestText("pickarray_06", text)

#  bug '7323
SetActiveWindow(1)
DeleteAllPlots()
SetActiveWindow(2)
DeleteAllPlots()
SetActiveWindow(1)
OpenDatabase("../data/wave.visit")
AddPlot("Pseudocolor", "pressure")
TimeSliderSetState(34)
DrawPlots()
ZonePick((0.29, 0.47, 5.0), "arr2")
SetActiveWindow(2)
Test("pickarray_07")
Query("Cycle")
str = GetQueryOutputString()
TestText("pickarray_08", str)

DeleteAllPlots()
SetActiveWindow(1)
DeleteAllPlots()
DefineArrayExpression("arr4", "array_compose_with_bins(dx, dy, [0, 0.5, 1.5])")
OpenDatabase("../data/globe.silo")
AddPlot("Pseudocolor", "u")
DrawPlots()
vars = ("u", "arr4")
Pick(128, 107, vars)
SetActiveWindow(2)
ResetView()
Test("pickarray_09")

h = HistogramAttributes()
h.useBinWidths = 0
h.basedOn = h.ManyVarsForSingleZone
SetPlotOptions(h)
ResetView()
Test("pickarray_10")


Exit()
