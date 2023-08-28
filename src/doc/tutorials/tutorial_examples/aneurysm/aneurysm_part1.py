#
# file: aneurysm_part1.py
# info: Corresponds to actions taken in
# https://visit-sphinx-github-user-manual.readthedocs.io/en/develop/tutorials/Aneurysm.html
#

OpenDatabase("aneurysm.visit")
AddPlot("Mesh", "Mesh")
DrawPlots()

View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (-1, 0, 0)
View3DAtts.focus = (3.89585, 3.99132, 4.90529)
View3DAtts.viewUp = (0, 1, 0)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 2.05937
View3DAtts.nearPlane = -4.11875
View3DAtts.farPlane = 4.11875
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 1
View3DAtts.perspective = 1
View3DAtts.eyeAngle = 2
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (3.89585, 3.99132, 4.90529)
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
View3DAtts.shear = (0, 0, 1)
View3DAtts.windowValid = 1
SetView3D(View3DAtts)

MeshAtts = MeshAttributes()
MeshAtts.meshColor = (51, 204, 255, 255)
SetPlotOptions(MeshAtts)

SetQueryFloatFormat("%g")
Query("NumNodes")

Query("NumZones")

AddPlot("Pseudocolor", "pressure")
PseudocolorAtts = PseudocolorAttributes()
PseudocolorAtts.colorTableName = "Spectral"
PseudocolorAtts.invertColorTable = 1
SetPlotOptions(PseudocolorAtts)
DrawPlots()

QueryOverTimeAtts = GetQueryOverTimeAttributes()
QueryOverTimeAtts.timeType = QueryOverTimeAtts.DTime  # Cycle, DTime, Timestep
SetQueryOverTimeAttributes(QueryOverTimeAtts)

QueryOverTime("Max", end_time=199, start_time=0, stride=1)

DeleteAllPlots()


AddPlot("Mesh", "Mesh")
MeshAtts = MeshAttributes()
MeshAtts.opaqueColor = (204, 255, 255, 255)
MeshAtts.opacity = 0.25
SetPlotOptions(MeshAtts)

DrawPlots()
AddPlot("Pseudocolor", "velocity_magnitude")
DrawPlots()
AddOperator("Isosurface")
IsosurfaceAtts = IsosurfaceAttributes()
IsosurfaceAtts.contourValue = (10, 15, 20)
IsosurfaceAtts.contourMethod = IsosurfaceAtts.Value  # Level, Value, Percent
SetOperatorOptions(IsosurfaceAtts, 1)

DrawPlots()

SetActivePlots(1)
RemoveOperator(0, 0)

AddOperator("Isovolume", 0)
IsovolumeAtts = IsovolumeAttributes()
IsovolumeAtts.lbound = 10
IsovolumeAtts.ubound = 20
IsovolumeAtts.variable = "default"
SetOperatorOptions(IsovolumeAtts, 0)
DrawPlots()
SetTimeSliderState(94)

