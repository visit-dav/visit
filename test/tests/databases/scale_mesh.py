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
TurnOffAllAnnotations() # defines global object 'a'

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
