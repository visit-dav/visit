# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  threshold.py
#
#  Tests:      mesh      - 3D unstructured, single domain,
#                          3D rectilinear, multiple domain,
#                          2D curvilinear, multiple domain
#              plots     - pc, mesh, subset, contour
#              operators - threshold
#              selection - none
#
#  Defect ID:  '6590, '7631
#
#  Programmer: Hank Childs
#  Date:       July 24, 2002
#
#  Modifications:
#    Kathleen Bonnell, Thu Sep  5 10:55:47 PDT 2002
#    Changed the variable format for SubsetPlots, to reflect new interface
#    which allows subsets of groups.
#
#    Eric Brugger, Thu May  8 12:50:50 PDT 2003
#    Remove some code for setting the view when changing to a new data set
#    since visit should automatically adjust the view by default.
#
#    Kathleen Bonnell, Thu Aug 28 14:34:57 PDT 2003
#    Remove compound var name from subset plots.
#
#    Hank Childs, Thu Sep 15 16:31:31 PDT 2005
#    Added testing for points only mode ('6590).
#
#    Mark Blair, Fri May 19 17:36:00 PDT 2006
#    Modified all tests, added new tests, for multi-variable mode.
#
#    Hank Childs, Sat Jan 27 13:03:55 PST 2007
#    Test points only with material selection on ('7631).
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

OpenDatabase("../data/globe.silo")

AddPlot("Pseudocolor", "u")
DrawPlots()

v = GetView3D()
v.SetViewNormal(-0.528889, 0.367702, 0.7649)
v.SetViewUp(0.176641, 0.929226, -0.324558)
v.SetParallelScale(17.3205)
v.SetPerspective(1)
SetView3D(v)

#
# Normal PC threshold for globe.
#
thresh = ThresholdAttributes()

thresh.outputMeshType = 0
thresh.zonePortions = (1)
thresh.lowerBounds = (-4)
thresh.upperBounds = (4)
SetDefaultOperatorOptions(thresh)
AddOperator("Threshold")

Test("ops_thresh01")

#
# Normal PC threshold for globe, but now change the zone inclusion criteria.
#
RemoveAllOperators()
thresh.outputMeshType = 0
thresh.zonePortions = (0)
thresh.lowerBounds = (-4)
thresh.upperBounds = (4)
SetDefaultOperatorOptions(thresh)
AddOperator("Threshold")

Test("ops_thresh02")

#
# Normal PC plot of globe, thresholding by a variable different than the
# PC coloring variable.
#
RemoveAllOperators()
thresh.outputMeshType = 0
thresh.zonePortions = (1)
thresh.lowerBounds = (140)
thresh.upperBounds = (340)
thresh.listedVarNames = ("t")
SetDefaultOperatorOptions(thresh)
AddOperator("Threshold")

Test("ops_thresh03")

DeleteAllPlots()

#
# Contour lines by one variable, thresholding by another.  Multi-block,
# curvilinear, 2D.
#
OpenDatabase("../data/multi_curv2d.silo")
AddPlot("Contour", "u")
DrawPlots()

thresh.outputMeshType = 0
thresh.zonePortions = (1)
thresh.lowerBounds = (0.7)
thresh.upperBounds = (0.9)
thresh.listedVarNames = ("v")
SetDefaultOperatorOptions(thresh)
AddOperator("Threshold")

Test("ops_thresh04")

DeleteAllPlots()

#
# Material plot, thresholded by a scalar variable.  Multi-block,
# curvilinear, 2D.
#
AddPlot("Subset", "mat1")
DrawPlots()

thresh.outputMeshType = 0
thresh.zonePortions = (1)
thresh.lowerBounds = (-0.4)
thresh.upperBounds = (1.0)
thresh.listedVarNames = ("u")
SetDefaultOperatorOptions(thresh)
AddOperator("Threshold")

Test("ops_thresh05")

DeleteAllPlots()

#
# Mesh plot and PC plot, both thresholded by the same variable and zone
# inclusion criteria.  Then change the output mesh type to a point mesh.
# Multi-block, rectilinear, 3D.
#
OpenDatabase("../data/multi_rect3d.silo")

thresh.outputMeshType = 0
thresh.zonePortions = (1)
thresh.lowerBounds = (0.4)
thresh.upperBounds = (1.0)
thresh.listedVarNames = ("u")
SetDefaultOperatorOptions(thresh)

AddPlot("Pseudocolor", "u")
AddOperator("Threshold")
AddPlot("Mesh", "mesh1")
AddOperator("Threshold")
DrawPlots()

Test("ops_thresh06")

DeleteActivePlots()

thresh.outputMeshType = 1
SetDefaultOperatorOptions(thresh)
AddOperator("Threshold")

Test("ops_thresh07")

DeleteAllPlots()

OpenDatabase("../data/globe.silo")
AddPlot("Pseudocolor", "u")
DrawPlots()

#
# Normal PC plot of globe, thresholding by two variables different than the
# PC coloring variable.
#
thresh.outputMeshType = 0
thresh.zonePortions = (1, 1)
thresh.lowerBounds = (140, 0)
thresh.upperBounds = (340, 1)
thresh.listedVarNames = ("t", "dx")
SetDefaultOperatorOptions(thresh)
AddOperator("Threshold")

Test("ops_thresh08")

DeleteAllPlots()

#
# Contour lines by one variable, thresholding by that variable and another,
# using two different zone inclusion criteria.  Multi-block, curvilinear, 2D.
#
OpenDatabase("../data/multi_curv2d.silo")
AddPlot("Contour", "u")
DrawPlots()

thresh.zonePortions = (0, 1)
thresh.lowerBounds = (-0.7, 0.7)
thresh.upperBounds = (0.7, 0.9)
thresh.listedVarNames = ("u", "v")
SetDefaultOperatorOptions(thresh)
AddOperator("Threshold")

Test("ops_thresh09")

DeleteAllPlots()

#
# Material plot, thresholded by two scalar variables.  Multi-block,
# curvilinear, 2D.
#
AddPlot("Subset", "mat1")
DrawPlots()

thresh.zonePortions = (0, 0)
thresh.lowerBounds = (-0.4, 0.25)
thresh.upperBounds = (1.0, 0.45)
thresh.listedVarNames = ("u", "p")
SetDefaultOperatorOptions(thresh)
AddOperator("Threshold")

Test("ops_thresh10")

DeleteAllPlots()
OpenDatabase("../data/globe.silo")
AddPlot("Pseudocolor", "u")
AddOperator("Threshold")
ThresholdAtts = ThresholdAttributes()
ThresholdAtts.outputMeshType = 1
ThresholdAtts.listedVarNames = ("u")
ThresholdAtts.zonePortions = (1)
ThresholdAtts.lowerBounds = (0)
ThresholdAtts.upperBounds = (1e+37)
ThresholdAtts.defaultVarName = "u"
ThresholdAtts.defaultVarIsScalar = 1
SetOperatorOptions(ThresholdAtts)
DrawPlots()
ResetView()
Test("ops_thresh11")

# See that the points go away when you turn off the materials.
sil = SILRestriction()
m = sil.SetsInCategory("mat1")
sil.TurnOffSet(m[2])
sil.TurnOffSet(m[3])
SetPlotSILRestriction(sil)
Test("ops_thresh12")

Exit()
