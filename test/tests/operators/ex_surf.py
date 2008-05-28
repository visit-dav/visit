# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  external_surface.py
#
#  Programmer: Hank Childs
#  Date:       August 24, 2005
#
# ----------------------------------------------------------------------------

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'

OpenDatabase("../data/multi_ucd3d.silo")
AddPlot("Pseudocolor", "d")
AddOperator("ExternalSurface")
DrawPlots()

v = GetView3D()
v.viewNormal = (-0.707, 0, 0.707)
v.nearPlane = 0
SetView3D(v)

Test("ops_ex_surf_01")

Query("NumZones", "actual")
t = GetQueryOutputString()
TestText("ops_ex_surf_02", t)

es = ExternalSurfaceAttributes()
es.removeGhosts = 1
SetOperatorOptions(es)

Query("NumZones", "actual")
t = GetQueryOutputString()
TestText("ops_ex_surf_03", t)


DeleteAllPlots()
OpenDatabase("../data/ucd2d.silo")
AddPlot("Pseudocolor", "d")
AddOperator("ExternalSurface")
DrawPlots()
Test("ops_ex_surf_04")

Exit()
