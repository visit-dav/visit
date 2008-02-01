# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Special args to launch VisIt for this test...
#
#  CLARGS: -cli -s ../bin/visitdiff.py -vdiff ../data/visitdiff_test_data/a/ubl_005.visit ../data/visitdiff_test_data/b/ubl_005.visit
#
#  Test Case:  diff.py 
#
#  Tests:      'visit -diff' functionality 
#
#  Programmer: Mark C. Miller 
#  Date:       August 26, 2007 
#
# ----------------------------------------------------------------------------

winToStrMap = {1 : "L-R", 2 : "R-L", 3 : "Left", 4 : "Right"}

def TestWindows(base, num):
    for win in (1,3,4):
        SetActiveWindow(win)
	DrawPlots()
	if num < 0:
            Test("diff_%s_%s"%(base,winToStrMap[win]))
        else:
            Test("diff_%s_%d_%s"%(base,num,winToStrMap[win]))

# Turn off the user/database annotations
a = AnnotationAttributes()
for win in (1,3,4):
    SetActiveWindow(win)
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    SetAnnotationAttributes(a)

# Set up a good view
v=GetView3D()
v.viewNormal=(0.373726, 0.774443, 0.510456)
v.viewUp=(-0.662237, 0.608118, -0.437761)
SetView3D(v)
SetViewExtentsType("original")

# Ok, test the initial state after it comes up
TestSection("Initial state")
TestWindows("init",-1)

TestSection("Toggling the mesh on/off")
ToggleMesh()
TestWindows("toggleMesh",0)
ToggleMesh()
TestWindows("toggleMesh",1)

# Test changing variables
TestSection("Changing a variable")
ChangeVar("p")
TestWindows("changeVar", 0)

# Test using a matvf expression variable
TestSection("Using a matvf expression variable")
ChangeVar("matvf_comps/3_bufferplate_3")
TestWindows("changeVar", 1)

# Ok, change to a variable with some actual diffs
TestSection("A variable with large diffs")
ChangeVar("tkelv")
TestWindows("changeVar", 2)

# Now, do a bunch of stuff in L-R window and
# test syncing the windows.
TestSection("Syncing windows")
SetActiveWindow(1)
SetActivePlots((0,))
pcAtts = PseudocolorAttributes()
pcAtts.minFlag = 1 
pcAtts.min = 5 
pcAtts.colorTableName = "hot_and_cold"
pcAtts.smoothingLevel = 2
SetPlotOptions(pcAtts)
AddOperator("Clip")
clipAtts=ClipAttributes()
clipAtts.plane1Origin = (3, 0, 0)
clipAtts.plane2Origin = (1, 0, 0)
SetOperatorOptions(clipAtts)
DrawPlots()
v=GetView3D()
v.viewNormal=(-0.275981, -0.33486, -0.900946)
v.viewUp=(0.374575, 0.82577, -0.42166)
SetView3D(v)
TestWindows("beforeSync", -1)
SyncWinsL_R()
TestWindows("afterSync", -1)

# Test Diff summary output
TestSection("Difference summary")
s=DiffSummary()
TestText("diffsummary",s)

# Test changing time states
TestSection("Changing time states")
TimeSliderNextState()
v.imageZoom=0.5
SetView3D(v)
TestWindows("changeTime", -1)
TimeSliderPreviousState()

Exit()
