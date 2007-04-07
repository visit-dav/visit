# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Tests: weather or not SaveWindow can result in a python exception
#
#  Programmer: Mark C. Miller
#  Date:       March 7, 2006 
#
# ----------------------------------------------------------------------------

import os

swa = SaveWindowAttributes()
swa.outputToCurrentDirectory = 0
swa.outputDirectory = "current"
SetSaveWindowAttributes(swa)

OpenDatabase("../data/wave.visit")
AddPlot("Pseudocolor","pressure")
DrawPlots()

pa = GetProcessAttributes("engine")
enginePid = int(pa.pids[0])

s = ""
for i in range(6):
    TimeSliderSetState(i)
    if i == 3:
        os.system("kill -9 %d"%enginePid)
    try:
        SaveWindow()
    except Exception, inst:
	s = s + "save %d had exception \"%s\"\n"%(i,inst);
    else:
	s = s + "save %d succeeded\n"%i;

TestText("SaveWindowEngineCrash", s)

Exit()
