# ----------------------------------------------------------------------------
#  CLASSES:    nightly
#
#  LIMIT:      1200
#
#  Test Case:  operatorsVsInstall.py 
#
#  Tests:      Building operator plugins against an installed version of VisIt
#              
#
#  Programmer: Kathleen Biagas 
#  Date:       November 8, 2018 
#
#  Modifications:
#
# ----------------------------------------------------------------------------

Source(tests_path("plugins", "pluginVsInstallHelpers"))

do_plugin_type("operators", ["Remap"])

Exit()
