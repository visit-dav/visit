# ----------------------------------------------------------------------------
#  CLASSES:    nightly
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
#    Kathleen Biagas, Tue Nov 26 10:23:31 PST 2019
#    Modified to pass a 'test-this' list instead of a 'skip-this' list.
#    Plugins chosen based on external dependence, expression creation, or a
#    a simple operator, the ability to auto-regen their CMakeLists.txt and
#    tests currently available to run against the version built (to be added
#    at a future time).
#
# ----------------------------------------------------------------------------

Source(tests_path("plugins", "pluginVsInstallHelpers"))

do_plugin_type("operators", ["CartographicProjection", "CracksClipper", "RadialResample", "Slice", "Threshold"])
# for future use
#do_tests("operators", ["cart_proj.py", "cracksclipper.py", "radial_resample.py", "slice.py", "threshold.py"])

Exit()
