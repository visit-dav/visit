# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  rect.py 
#
#  Tests:      rect files 
#
#  Programmer: Mark C. Miller 
#  Date:       May 15, 2008 
#
# ----------------------------------------------------------------------------

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'

# test the main mesh
OpenDatabase("../src/databases/Rect/data/datafile.rect")
AddPlot("Mesh","Mesh")
DrawPlots()
v=GetView3D()
v.RotateAxis(1, 30.0)
SetView3D(v)
Test("rect_01")

# test a variable
AddPlot("Pseudocolor","pressure")
DrawPlots()
Test("rect_02")

# test going forward in time
TimeSliderNextState()
Test("rect_03")

# test restriction
silr = SILRestriction()
for i in range(silr.NumSets()):
    if silr.SetName(i) == "domain2":
        silr.TurnOffSet(i)
#SetPlotSILRestriction(silr) ghost zone comm. bug 8612 
DrawPlots()
Test("rect_04")

Exit()
