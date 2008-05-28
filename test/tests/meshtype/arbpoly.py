# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  arbpoly.py
#
#  Tests:      mesh      - a ucd mesh with arbitrary polyhedral zones 
#              plots     - pc, contour, mat subset, mesh
#              operators - none
#              selection - material
#
#  Defect ID:  none
#
#  Programmer: Mark C. Miller 
#  Date:       October 24, 2004
#
# ----------------------------------------------------------------------------


OpenDatabase("../data/poly3d.silo")

AddPlot("Mesh", "ucdmesh3d")
DrawPlots()

v = GetView3D()
v.SetViewNormal(-0.0356529, 0.010858, 0.999305)
v.SetViewUp(0.0100239, 0.999895, -0.0105068)
v.SetImageZoom(2.75)
SetView3D(v)

Test("arbpoly_01")

#
# See if we can handle a zone centered variable correctly
#
AddPlot("Pseudocolor", "d")
DrawPlots()
Test("arbpoly_02")

DeleteActivePlots()

#
# See if we can handle a material correctly
# Turn off alternate materials.
AddPlot("FilledBoundary", "mat1")
silr=SILRestriction()
silr.TurnOffSet(1)
SetPlotSILRestriction(silr)
DrawPlots()

Test("arbpoly_03")

Exit()
