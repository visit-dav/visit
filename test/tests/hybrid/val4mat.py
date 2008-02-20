# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  val4mat.py
#
#  Tests:      mesh      - 3D structured, multi domain
#              plots     - pc
#
#  Notes:      Migrated value_for_material tests from expressions.py
#              and added post ghost test cases. 
#
#  Programmer: Cyrus Harrison
#  Date:       Tuesday 12, 2008
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

OpenDatabase("../data/thinplane.silo")
atts = PseudocolorAttributes()
atts.minFlag = 1
atts.min = 0.0
atts.maxFlag = 1
atts.max = 10.0
SetDefaultPlotOptions(atts)

# view the per material values for each of the 3 materials

DefineScalarExpression("vfm_1", "value_for_material(den,1)")
AddPlot("Pseudocolor", "vfm_1")
DrawPlots()
Test("value_for_mat_1")

DeleteAllPlots()
DefineScalarExpression("vfm_2", "value_for_material(den,2)")
AddPlot("Pseudocolor", "vfm_2")
DrawPlots()
Test("value_for_mat_2")

DeleteAllPlots()
DefineScalarExpression("vfm_3", "value_for_material(den,\"3\")")
AddPlot("Pseudocolor", "vfm_3")
DrawPlots()
Test("value_for_mat_3")


ChangeActivePlotsVar("vfm_2")

# test w/ ghost zones (switch vfm_2 to nodal)
atts = PseudocolorAttributes(1)
atts.minFlag = 1
atts.min = 0.0
atts.maxFlag = 1
atts.max = 10.0
atts.centering = atts.Nodal
SetPlotOptions(atts)
DrawPlots()
Test("value_for_mat_post_ghost_1")

# make sure post ghost caching doesn't tank non ghost results
atts = PseudocolorAttributes(1)
atts.minFlag = 1
atts.min = 0.0
atts.maxFlag = 1
atts.max = 10.0
atts.centering = atts.Zonal
SetPlotOptions(atts)
DrawPlots()
Test("value_for_mat_post_ghost_2")

Exit()
