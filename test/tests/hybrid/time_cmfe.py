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
# ----------------------------------------------------------------------------



OpenDatabase("../data/silo_%s_test_data/wave.visit"%SILO_MODE)


# Test general capability.
DefineScalarExpression("cmfe", "conn_cmfe(<../data/silo_%s_test_data/wave.visit[10]i:pressure>, quadmesh)"%SILO_MODE)
AddPlot("Pseudocolor", "cmfe")
DrawPlots()
Test("time_cmfe_01")

# Should be same pressure, since we aren't doing deltas
SetTimeSliderState(20)
Test("time_cmfe_02")

DefineScalarExpression("cmfe2", "conn_cmfe(<../data/silo_%s_test_data/wave.visit[-10]id:pressure>, quadmesh)"%SILO_MODE)
ChangeActivePlotsVar("cmfe2")
Test("time_cmfe_03")

SetTimeSliderState(40)
Test("time_cmfe_04")

DefineScalarExpression("cmfe3", "conn_cmfe(<../data/silo_%s_test_data/wave.visit[200]c:pressure>, quadmesh)"%SILO_MODE)
ChangeActivePlotsVar("cmfe3")
Test("time_cmfe_05")

SetTimeSliderState(20)
Test("time_cmfe_06")

DefineScalarExpression("cmfe4", "conn_cmfe(<../data/silo_%s_test_data/wave.visit[200]cd:pressure>, quadmesh)"%SILO_MODE)
ChangeActivePlotsVar("cmfe4")
Test("time_cmfe_07")

SetTimeSliderState(40)
Test("time_cmfe_08")

DefineScalarExpression("cmfe5", "conn_cmfe(<../data/silo_%s_test_data/wave.visit[-0.9]t:pressure>, quadmesh)"%SILO_MODE)
ChangeActivePlotsVar("cmfe5")
Test("time_cmfe_09")

SetTimeSliderState(20)
Test("time_cmfe_10")

DefineScalarExpression("cmfe6", "conn_cmfe(<../data/silo_%s_test_data/wave.visit[-0.9]td:pressure>, quadmesh)"%SILO_MODE)
ChangeActivePlotsVar("cmfe6")
Test("time_cmfe_11")

SetTimeSliderState(40)
Test("time_cmfe_12")

Exit()
