# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  time_cmfe.py
#
#  Defect ID:  None
#
#  Programmer: Hank Childs
#  Date:       August 30, 2005
#
#  Modifications:
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Brad Whitlock, Thu Apr 19 14:35:20 PDT 2012
#    I changed the last couple of tests so we can directly compare the
#    data at a time step with what time deltas should give us.
#
# ----------------------------------------------------------------------------

OpenDatabase(silo_data_path("wave.visit"))



# Test general capability.
DefineScalarExpression("cmfe", "conn_cmfe(<%s[10]i:pressure>, quadmesh)" % cmfe_silo_data_path("wave.visit"))
AddPlot("Pseudocolor", "cmfe")
DrawPlots()
Test("time_cmfe_01")

# Should be same pressure, since we aren't doing deltas
SetTimeSliderState(20)
Test("time_cmfe_02")

DefineScalarExpression("cmfe2", "conn_cmfe(<%s[-10]id:pressure>, quadmesh)" % cmfe_silo_data_path("wave.visit"))
ChangeActivePlotsVar("cmfe2")
Test("time_cmfe_03")

SetTimeSliderState(40)
Test("time_cmfe_04")

DefineScalarExpression("cmfe3", "conn_cmfe(<%s[200]c:pressure>, quadmesh)" % cmfe_silo_data_path("wave.visit"))
ChangeActivePlotsVar("cmfe3")
Test("time_cmfe_05")

SetTimeSliderState(20)
Test("time_cmfe_06")

DefineScalarExpression("cmfe4", "conn_cmfe(<%s[200]cd:pressure>, quadmesh)" % cmfe_silo_data_path("wave.visit"))
ChangeActivePlotsVar("cmfe4")
Test("time_cmfe_07")

SetTimeSliderState(40)
Test("time_cmfe_08")

DefineScalarExpression("cmfe5", "conn_cmfe(<%s[-0.9]t:pressure>, quadmesh)" % cmfe_silo_data_path("wave.visit"))
ChangeActivePlotsVar("cmfe5")
Test("time_cmfe_09")

SetTimeSliderState(20)
Test("time_cmfe_10")

# Get rid of the wave in the mesh and slice it. I'm slicing it because I was 
# originally doing lineouts but those didn't line up for some reason.
DefineScalarExpression("i", "mod(nodeid(quadmesh), 101.)")
DefineScalarExpression("j", "floor((nodeid(quadmesh) - (k*101.*11.) - i) / 101.)")
DefineScalarExpression("k", "floor(nodeid(quadmesh) / (101.*11.))")
DefineVectorExpression("offset", "{coords(quadmesh)[0], j * 0.06, coords(quadmesh)[2]} - coords(quadmesh)")
AddOperator("Displace")
disp = DisplaceAttributes()
disp.variable = "offset"
disp.factor = 1
SetOperatorOptions(disp)
AddOperator("Slice")
DrawPlots()

ChangeActivePlotsVar("pressure")
# Do a plot of pressure at time state 25 since the time there is 3. 
SetTimeSliderState(25)
Test("time_cmfe_11")

# Now create a time delta expression where we want to get the data that trails
# the current time step by 0.8. We'll set the time slider to state 29 where the 
# time is 3.8 and we'll use the cmfe time delta to obtain data from 0.8 time
# units before, or in other words, time state 25. Then we'll do a plot to see
# if the data matches the previous test.
DefineScalarExpression("cmfe6", "conn_cmfe(<%s[-0.8]td:pressure>, quadmesh)" % cmfe_silo_data_path("wave.visit"))
SetTimeSliderState(29)
ChangeActivePlotsVar("cmfe6")
Test("time_cmfe_12")

Exit()
