# ----------------------------------------------------------------------------
#  CLASSES:    nightly
#
#  LIMIT:      1800
#
#  Test Case:  databasesVsInstall.py 
#
#  Tests:      Building database plugins against an installed version of VisIt
#              
#
#  Programmer: Kathleen Biagas 
#  Date:       November 8, 2018 
#
#  Modifications:
#
# ----------------------------------------------------------------------------

Source(tests_path("plugins", "pluginVsInstallHelpers"))

do_plugin_type("databases", ["ADIOS", "ADIOS2", "CCM", "FieldViewXDB", "GMV", "IDX", "MDSplus", "MOAB", "Nektar++", "SimV2", "Uintah"])


Exit()
