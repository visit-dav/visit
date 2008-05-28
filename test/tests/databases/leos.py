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
TurnOffAllAnnotations() # defines global object 'a'

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
