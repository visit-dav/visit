# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  filledboundary.py
#
#  Tests:      mesh      - 3D rectilinear, single domain
#              plots     - FilledBoundary
#
#  Defect ID:  none
#
#  Programmer: Kathleen Biagas 
#  Date:       June 11, 2020 
#
#  Modifications:
#
# ----------------------------------------------------------------------------


# singlecolor.py and multicolor.py test the coloring functionality of FB plots,
# so those aspects won't be heavily tested here.

TurnOffAllAnnotations()


OpenDatabase(silo_data_path("rect3d.silo"))
AddPlot("FilledBoundary", "mat1")
fb = FilledBoundaryAttributes()

# test line settings, first with wireframe mode:
fb.wireframe = 1
SetPlotOptions(fb)
DrawPlots()

# want to rotate the view to verify the lines are unlit
v = GetView3D()
viewNormal = (0, 0, -1)
SetView3D(v)
Test("fb_wireframe_01")

fb.lineWidth = 3
SetPlotOptions(fb)
Test("fb_wireframe_02")

# now with lines created by an operator/operator sequence
fb.lineWidth = 1
fb.wireframe = 0
SetPlotOptions(fb)
AddOperator("ThreeSlice")
ts = ThreeSliceAttributes()
ts.x = 0.5;
ts.y = 0.5;
ts.z = 0.5;
SetOperatorOptions(ts)

AddOperator("ExternalSurface")
DrawPlots()
Test("fb_lines_01")

# move ExternalSurface to before ThreeSlice
DemoteOperator(1)
fb.lineWidth = 4
SetPlotOptions(fb)
DrawPlots()
Test("fb_lines_02")

# add another FB plot and play with opacity
AddPlot("FilledBoundary", "mat1")
fb2 = FilledBoundaryAttributes()
fb2.colorType = fb.ColorBySingleColor
fb2.singleColor=(255, 153, 0, 255)
fb2.opacity = 0.5
SetPlotOptions(fb2)

DrawPlots()
Test("fb_lines_03")
DeleteAllPlots()


CloseDatabase(silo_data_path("rect3d.silo"))

# Another way of producing lines in a FB plot: Add Edge operator
OpenDatabase(silo_data_path("noise.silo"))
AddPlot("FilledBoundary", "mat1")
fb2.opacity = 1
SetPlotOptions(fb2)
AddOperator("Edge")
DrawPlots()
ResetView()
v = GetView3D()
v.viewNormal = (0.0368258, 0.737081, -0.674801)
v.viewUp = (0.0382946, 0.673722, -0.737992)
SetView3D(v)
Test("fb_lines_04")

# perhaps a bit contrived, but adding Edge and ThreeSlice operators together
# will generate points and allow testing of point glyphing functionality
AddOperator("ThreeSlice")
SetOperatorOptions(ts)
DrawPlots()
Test("fb_points_01")

fb2.pointType = fb.Tetrahedron
fb2.pointSize = 1
SetPlotOptions(fb2)
Test("fb_points_02")
DeleteAllPlots()
CloseDatabase(silo_data_path("noise.silo"))

Exit()
