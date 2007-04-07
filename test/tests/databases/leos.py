# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  leos.py 
#
#  Tests:      Auto Full Frame Mode
#              LEOSFileReader 
#
#  Programmer: Mark C. Miller 
#  Date:       July 20, 2005 
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

OpenDatabase("../data/sesame.pdb")

#
# Test default full frame (became 'auto' 07Mar06)
#
AddPlot("Pseudocolor","Calcium_Ca/Pt")
DrawPlots()
Test("leos_01")

v=GetView2D()
#
# Test auto full frame. This test was obsoleted when
# auto ff became the default
#
#v.fullFrameActivationMode = v.Auto
#SetView2D(v)
#Test("leos_02")

#
# Test turning off full frame
#
v.fullFrameActivationMode = v.Off
SetView2D(v)
Test("leos_03")

#
# Test turning on full frame 
#
v.fullFrameActivationMode = v.On
SetView2D(v)
Test("leos_04")

v.fullFrameActivationMode = v.Auto
SetView2D(v)
DeleteAllPlots()

#
# Test Auto full frame when changing plots
#
AddPlot("Pseudocolor","Water_H2O/Et")
DrawPlots()
ResetView()
v=GetView2D()
v.fullFrameActivationMode = v.Auto
SetView2D(v)
Test("leos_05")

#
# Test the "catch all" mesh
#
AddPlot("Mesh","for active plot")
DrawPlots()
Test("leos_06")

Exit()
