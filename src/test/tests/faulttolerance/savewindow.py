# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Tests: weather or not SaveWindow can result in a python exception
#
#  Programmer: Mark C. Miller
#  Date:       March 7, 2006 
#
#  Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------

import os

TurnOnAllAnnotations()

swa = SaveWindowAttributes()
swa.outputToCurrentDirectory = 0
swa.outputDirectory = "current"
SetSaveWindowAttributes(swa)

OpenDatabase(silo_data_path("wave.visit"))

AddPlot("Pseudocolor","pressure")
DrawPlots()

pa = GetProcessAttributes("engine")
enginePid = int(pa.pids[0])

s = ""
# TODO_WINDOWS THIS WONT WORK ON WINDOWS
for i in range(6):
    TimeSliderSetState(i)
    if i == 3:
        os.system("kill -9 %d"%enginePid)
    try:
        SaveWindow()
    except Exception as inst:
	s = s + "save %d had exception \"%s\"\n"%(i,inst);
    else:
	s = s + "save %d succeeded\n"%i;

TestText("SaveWindowEngineCrash", s)

Exit()
