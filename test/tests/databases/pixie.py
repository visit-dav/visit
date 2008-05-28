# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  pixie.py 
#
#  Programmer: Mark C. Miller 
#  Date:       April 4, 2005 
#
#  Modifications:
#    Jeremy Meredith, Wed Sep  7 12:06:04 PDT 2005
#    Allowed spaces in variable names.
#
# ----------------------------------------------------------------------------

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'

# test a 2D pixie database
OpenDatabase("../data/pixie_test_data/pixie3d3.h5")

#
# Test simple read and display of a variable 
#
AddPlot("Pseudocolor","Car variables/Bx")
DrawPlots()
Test("pixie_01")

AddPlot("Mesh","curvemesh_33x33x2")
DrawPlots()
Test("pixie_02")

# really zoom in on the center of this mesh
v=GetView2D()
v.windowCoords = (-5.84435e-05, 5.84435e-05, -5.12076e-05, 5.12076e-05)
SetView2D(v)
Test("pixie_03")

DeleteAllPlots()

# test a 3D pixie database
OpenDatabase("../data/pixie_test_data/pixie3d4.h5")
AddPlot("Contour","Car variables/Vx")
AddPlot("Mesh","curvemesh_33x33x33")
v=GetView3D()
v.viewNormal=(-0.5, 0, 0.866025)
SetView3D(v)
DrawPlots()
Test("pixie_04")
DeleteAllPlots()
ResetView()

# test a pixie expression
OpenDatabase("../data/pixie_test_data/pixie_expressions.h5")
AddPlot("Vector","V")
DrawPlots()
v=GetView3D()
v.imageZoom=3.5
SetView3D(v)
Test("pixie_05")
DeleteAllPlots()

# test a contour plot on double precision data
AddPlot("Contour","Car_variables/Vx")
DrawPlots()
Test("pixie_06")
DeleteAllPlots()

# test a mesh plot of a non-square, 2D, curvilinear mesh 
OpenDatabase("../data/pixie_test_data/pixie3d5.h5")
AddPlot("Mesh","curvemesh_65x33x2")
AddPlot("Pseudocolor","Diagnostics/q factor")
DrawPlots()
v=GetView2D()
v.windowCoords=(-0.241119, 0.241119, -0.162714, 0.162714)
SetView2D(v)
Test("pixie_07")

Exit()
