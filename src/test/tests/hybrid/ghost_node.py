# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  ghost_node.py 
#
#  Tests:      ghost node removal of multi_curv3d.silo
#
#  Programmer: Eric Brugger
#  Date:       July 6, 2020 
#
#  Modifications:
#    Kathleen Biagas, Fri Mar 12, 2021
#    Added DeleteAllPlots before CloseDatabase.
#
# ----------------------------------------------------------------------------
TurnOffAllAnnotations() # defines global object 'a'

OpenDatabase(silo_data_path("multi_curv3d.silo"))

AddPlot("Pseudocolor", "d")
pc = PseudocolorAttributes()
pc.opacityType = pc.Constant
pc.opacityVariable = ""
pc.opacity = 0.25
SetPlotOptions(pc)
DrawPlots()

v = View3DAttributes()
v.viewNormal = (-0.491824, 0.420414, 0.76247)
v.focus = (0, 2.5, 15)
v.viewUp = (0.214182, 0.907212, -0.362066)
v.viewAngle = 30
v.parallelScale = 16.0078
v.nearPlane = -32.0156
v.farPlane = 32.0156
v.imagePan = (0, 0)
v.imageZoom = 1
SetView3D(v)

Test("ghost_node_01")
DeleteAllPlots()
CloseDatabase(silo_data_path("multi_curv3d.silo"))

Exit()
