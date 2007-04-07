# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  expr_cmfe.py
#
#  Defect ID:  None
#
#  Programmer: Hank Childs
#  Date:       September 9, 2005
#
#  Modifications:
#
#    Hank Childs, Thu Dec 29 11:21:26 PST 2005
#    Expand the color range for a plot of an expression that should result in 
#    uniformly "1", but actually has some small variation.  This causes 
#    issues with coloring between optimized and non-optimized modes.
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


OpenDatabase("../data/wave.visit")


# Test that database expressions can still be generated.
DefineVectorExpression("cmfe", "conn_cmfe(<../data/wave.visit[30]i:direction>, quadmesh)")
AddPlot("Vector", "cmfe")
DrawPlots()
Test("expr_cmfe_01")

DeleteAllPlots()
DefineScalarExpression("cmfe2", "conn_cmfe(coord(<../data/wave.visit[40]i:pressure>)[1], quadmesh)")
AddPlot("Pseudocolor", "cmfe2")
DrawPlots()
Test("expr_cmfe_02")

DeleteAllPlots()
DefineScalarExpression("cmfe3", "coord(quadmesh)[1] - conn_cmfe(coord(<../data/wave.visit[40]i:pressure>)[1], quadmesh)")
AddPlot("Pseudocolor", "cmfe3")
DrawPlots()
Test("expr_cmfe_03")

DeleteAllPlots()
DefineScalarExpression("cmfe4", "coord(quadmesh)[1] - cmfe2")
AddPlot("Pseudocolor", "cmfe4")
DrawPlots()
Test("expr_cmfe_04")

DeleteAllPlots()
DefineScalarExpression("cmfe5", "volume(quadmesh) / conn_cmfe(volume(<../data/wave.visit[40]i:quadmesh>), quadmesh)")
AddPlot("Pseudocolor", "cmfe5")
pc = PseudocolorAttributes()
pc.min = 0.5
pc.minFlag = 1
pc.max = 1.5
pc.maxFlag = 1
SetPlotOptions(pc)
DrawPlots()
Test("expr_cmfe_05")


Exit()
