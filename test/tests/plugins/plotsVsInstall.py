# ----------------------------------------------------------------------------
#  CLASSES:    nightly
#
#  Test Case:  plotsVsInstall.py 
#
#  Tests:      Building plot plugins against an installed version of VisIt
#              
#
#  Programmer: Kathleen Biagas 
#  Date:       November 8, 2018 
#
#  Modifications:
#
# ----------------------------------------------------------------------------

Source(tests_path("plugins", "pluginVsInstallHelpers"))

do_plugin_type("plots", [])

Exit()
