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
#
#    Jeremy Meredith, Tue Jul 15 10:43:58 EDT 2008
#    Changed number of vectors in vector plot to match the old behavior.
#    (We now account for how many domains there are.)
# ----------------------------------------------------------------------------


#
# We need to force VisIt to use Vista
#
OpenMDServer("localhost")

# open a Vista/Diablo multi-part file
OpenDatabase(data_path("Vista_test_data/AAAA_00004_00000.SILO"),0,"Vista_1.0")


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
vec = VectorAttributes()
vec.nVectors = 400*4
SetPlotOptions(vec)
DrawPlots()
Test("vista_diablo_02")

DeleteAllPlots()

# open a Vista/Ale3d file
TestSection("Vista-Ale3d")

OpenDatabase(data_path("Vista_test_data/nnq_001_00423.vista"))


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
