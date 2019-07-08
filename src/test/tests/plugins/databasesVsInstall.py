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
#     Eric Brugger, Wed Jun 26 09:32:06 PDT 2019
#     Removed the ccm readler.
#
# ----------------------------------------------------------------------------

Source(tests_path("plugins", "pluginVsInstallHelpers"))

do_plugin_type("databases", ["ADIOS", "ADIOS2", "FieldViewXDB", "GMV", "IDX", "MDSplus", "MOAB", "Nektar++", "SimV2", "Uintah"])


Exit()
