# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  l2norm.py
#  Tests:      queries     - l2norms of various curves
#
#  Defect ID:  VisIt00006077
#
#  Programmer: Hank Childs
#  Date:       March 15, 2005
#
# ----------------------------------------------------------------------------

# Test that we can do an l2norm of a degenerate ultra file.
TurnOnAllAnnotations()
OpenDatabase("../data/degenerate.ultra")
AddPlot("Curve", "This is a degenerate ultra file.")
DrawPlots()

Query("L2Norm")
text = GetQueryOutputString()
TestText("l2norm_01", text)

DeleteAllPlots()
OpenDatabase("../data/rect2d.silo")
AddPlot("Pseudocolor", "d")
DrawPlots()

DefineScalarExpression("d_mod", "d+0.4*coord(quadmesh2d)[1]")
AddWindow()
SetActiveWindow(2)
OpenDatabase("../data/rect2d.silo")
DeleteAllPlots()
AddPlot("Pseudocolor", "d_mod")
DrawPlots()

start_angle = 0
stop_angle = 90

import math
nsteps = 10
center=(0,0)
radius = 1
step = (stop_angle-start_angle) / (nsteps-1)
rel_diff = []
for i in range(nsteps):
   angle = start_angle + i*step
   rad_angle = angle*2*math.pi/360.
   P2=(radius*math.cos(rad_angle), radius*math.sin(rad_angle))
   SetActiveWindow(1)
   os.system("sleep 1")
   Lineout(center, P2)
   SetActiveWindow(2)
   os.system("sleep 1")
   Lineout(center, P2)
   SetActiveWindow(3)
   SetActivePlots(0)
   Query("L2Norm")
   norm = GetQueryOutputValue()
   SetActivePlots((0,1))
   os.system("sleep 1")
   Query("L2Norm Between Curves")
   diff = GetQueryOutputValue()
   rel_diff = rel_diff + [1.-diff/norm]
   DeleteAllPlots()

ave=0
for i in range(nsteps):
   ave += rel_diff[i]
ave /= nsteps

text = "The average difference is %f" %(ave)
TestText("l2norm_02", text)

min=1
max=0
for i in range(nsteps):
   if (rel_diff[i]<min):
       min=rel_diff[i]
   if (rel_diff[i]>max):
       max=rel_diff[i]


text = "The minimum relative difference is %f" %(min)
TestText("l2norm_03", text)
text = "The maximum relative difference is %f" %(max)
TestText("l2norm_04", text)


variance = 0
for i in range(nsteps):
   variance += (rel_diff[i]-ave)*(rel_diff[i]-ave)
variance /= (nsteps-1)
stddev = math.sqrt(variance)

text = "The standard deviation of the relative difference is %f" %(stddev)
TestText("l2norm_05", text)

Exit()
