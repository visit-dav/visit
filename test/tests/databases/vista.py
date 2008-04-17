# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  vista.py 
#
#  Tests:      multi-part vista files 
#              Diablo vista files
#              Ale3d vista files w/materials
#
#  Programmer: Mark C. Miller 
#  Date:       October 28, 2004 
#
#  Modifications:
#    Mark C. Miller, Thu Apr 17 12:05:19 PDT 2008
#    Changed '-default_format' to '-assume_format'
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

#
# We need to force VisIt to use Vista
#
OpenMDServer("localhost",("-assume_format","Vista"))

# open a Vista/Diablo multi-part file
OpenDatabase("../data/Vista_test_data/AAAA_00004_00000.SILO")

TestSection("Vista-Diablo")

#
# Test simple read and display of a variable 
#
AddPlot("Pseudocolor","bc_code_disp")
DrawPlots()

v=GetView3D()
v.viewNormal=(-0.5, 0.296198, 0.813798)
SetView3D(v)
Test("vista_diablo_01")

DeleteAllPlots()

AddPlot("Mesh","AAAA")
AddPlot("Vector","coord_n")
DrawPlots()
Test("vista_diablo_02")

DeleteAllPlots()

# open a Vista/Ale3d file
TestSection("Vista-Ale3d")

OpenDatabase("../data/Vista_test_data/nnq_001_00423.vista")

AddPlot("Pseudocolor","p")
DrawPlots()

v=GetView3D()
v.viewNormal=(-0.0503248, 0.882919, 0.466821)
v.viewUp=(0.784957, 0.323962, -0.528101)
v.focus=(5.86, 0.365, 2.915)
v.parallelScale=9.6
v.imageZoom=2.1
SetView3D(v)

Test("vista_ale3d_01")

DeleteAllPlots()

AddPlot("Mesh","nnq")
AddPlot("FilledBoundary","materials")
DrawPlots()
Test("vista_ale3d_02")

Exit()
