# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case: pseudocolor.py 
#
#  Tests:      meshes    - 2D rectilinear
#              plots     - pseudocolor 
#
#  Defect ID:  '1016, '987
#
#  Programmer: Kevin Griffin
#  Date:       March 19, 2015
#
#  Modifications:
#
#    Kevin Griffin, Thu Mar 19 12:00:23 PDT 2015
#    Add test for drawing pseudocolor plot using a log scaling
#    and very small min value.
#
# ----------------------------------------------------------------------------


OpenDatabase(silo_data_path("rect2d.silo"))

AddPlot("Pseudocolor", "d")

pc = PseudocolorAttributes()
pc.minFlag = 1
pc.min = 1e-5
pc.maxFlag = 1
pc.max = 1
pc.centering = pc.Nodal
pc.scaling = pc.Log
SetPlotOptions(pc)
DrawPlots()

Test("pseudocolor_01")

Exit()
