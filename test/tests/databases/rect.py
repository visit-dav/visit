# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  rect.py 
#
#  Tests:      rect files 
#
#  Programmer: Mark C. Miller 
#  Date:       May 15, 2008 
#
#  Mark C. Miller, Thu Feb 12 01:30:24 PST 2009
#  Added test for rectilinear mesh case. Re-enabled setting of SIL restriction
#  which was disabled due to a bug that was resolved back in May, 2008.
# ----------------------------------------------------------------------------


# test the main mesh
TestSection("Curvilinear grid files")
OpenDatabase("../data/Rect_test_data/curv_data/datafile.rect")
AddPlot("Mesh","Mesh")
DrawPlots()
v=GetView3D()
v.RotateAxis(1, 30.0)
SetView3D(v)
Test("rect_01")

# test a variable
AddPlot("Pseudocolor","pressure")
DrawPlots()
Test("rect_02")

# test going forward in time
TimeSliderNextState()
Test("rect_03")

# test restriction
silr = SILRestriction()
for i in range(silr.NumSets()):
    if silr.SetName(i) == "domain2":
        silr.TurnOffSet(i)
SetPlotSILRestriction(silr)
DrawPlots()
Test("rect_04")

DeleteAllPlots()
CloseDatabase("../data/Rect_test_data/curv_data/datafile.rect")

TestSection("Rectilinear grid files")
OpenDatabase("../data/Rect_test_data/rect_data/datafile.rect")
AddPlot("Mesh","Mesh")
DrawPlots()
ResetView()
Test("rect_05")

AddPlot("Pseudocolor","myTest")
DrawPlots()
Test("rect_06")

DeleteAllPlots()
CloseDatabase("../data/Rect_test_data/rect_data/datafile.rect")

TestSection("1D Datasets and re-interp. as curves.")

OpenDatabase("../data/Rect_test_data/1d/curv_data/datafile.rect")
AddPlot("Curve", "Mesh/pressure")
DrawPlots()
ResetView()
Test("rect_07")
AddPlot("Curve", "Mesh/density")
TimeSliderNextState()
Test("rect_08")

Exit()
