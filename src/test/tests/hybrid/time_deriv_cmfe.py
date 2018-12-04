# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  time_deriv_cmfe.py
#
#  Defect ID:  '2715, '6897
#
#  Programmer: Hank Childs
#  Date:       January 12, 2006
#
#  Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------



OpenDatabase(silo_data_path("wave.visit"))



# Test general capability.
AddPlot("Pseudocolor", "time_derivative/conn_based/pressure")
DrawPlots()
Test("time_deriv_cmfe_01")

TimeSliderSetState(50)
Test("time_deriv_cmfe_02")

ChangeActivePlotsVar("time_derivative/pos_based/pressure")
Test("time_deriv_cmfe_03")

Exit()
