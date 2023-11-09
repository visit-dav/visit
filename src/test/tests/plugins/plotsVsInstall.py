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
#    Kathleen Biagas, Wed Nov  8 09:41:41 PST 2023
#    Temporarily remove Volume from being tested. We need to use the VTK 9
#    version, but it's CMakeLists.txt file currently cannot be
#    auto-regenerated. See issue #19044.
#    Add Molecule in it's place.
#
# ----------------------------------------------------------------------------

Source(tests_path("plugins", "pluginVsInstallHelpers"))

do_plugin_type("plots", ["Contour", "Label", "Molecule", "Tensor"])

# for future use
#do_tests("plots", ["contour.py", "label.py", "tensor.py", "ray_trace.py or volumePlot.py"])

Exit()
