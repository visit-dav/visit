# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  revolved_surface_area.py
#  Tests:      queries     - Revolved surface area of 2D plot.
#
#  Defect ID:  VisIt00004666, '3260, '3261
#
#  Programmer: Hank Childs
#  Date:       March 30, 2004
#
#  Modifications:
#    Kathleen Bonnell, Thu Apr 29 07:40:58 PDT 2004 
#    Slice defaults atts have changed, update accordingly. 
#
#    Hank Childs, Thu Jan 12 15:57:52 PST 2006
#    Added additional tests ('3260, '3261).
#
# ----------------------------------------------------------------------------


# Test for topological dimension 2.
TurnOnAllAnnotations()
OpenDatabase("../data/rect2d.silo")
AddPlot("Pseudocolor", "d")
DrawPlots()

Query("Revolved surface area")
text = GetQueryOutputString()
TestText("revolved_surface_area_01", text)


# Test with isosurface operator.
DeleteAllPlots()
AddPlot("Pseudocolor", "d")
AddOperator("Isosurface")
iso_atts = IsosurfaceAttributes();
iso_atts.contourValue = (.25)
iso_atts.contourMethod = iso_atts.Value
SetOperatorOptions(iso_atts)
DrawPlots()

Query("Revolved surface area")
text = GetQueryOutputString()
TestText("revolved_surface_area_02", text)


# Test for boundary plot.
DeleteAllPlots()
AddPlot("Boundary", "mat1")
DrawPlots()

Query("Revolved surface area")
text = GetQueryOutputString()
TestText("revolved_surface_area_03", text)


# Test with 3D, isosurface, slice.
DeleteAllPlots()
OpenDatabase("../data/rect3d.silo")
AddPlot("Pseudocolor", "d")
AddOperator("Isosurface")
SetOperatorOptions(iso_atts)
AddOperator("Slice")
s = SliceAttributes()
s.originIntercept = 0.5
s.axisType = s.YAxis
SetOperatorOptions(s)
DrawPlots()

Query("Revolved surface area")
text = GetQueryOutputString()
TestText("revolved_surface_area_04", text)

RemoveLastOperator()
Query("Revolved surface area")
text = GetLastError()
TestText("revolved_surface_area_05", text)

Exit()
