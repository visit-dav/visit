# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  scale_mesh.py
#
#  Purpose:    Tests whether or not we try to scale a mesh when only one
#              of its dimensions are out of whack (we shouldn't).
#
#  Programmer: Hank Childs
#  Date:       April 22, 2004
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

# Create a Pseudocolor plot of wave by opening it up at a late time state.
OpenDatabase("../data/near_planar.vtk")
AddPlot("Pseudocolor", "hardyglobal")
DrawPlots()

# Set the view.
v0 = GetView3D()
v0.viewNormal = (-0.661743, 0.517608, 0.542382)
SetView3D(v0)
Test("scale_mesh01")

Exit()
