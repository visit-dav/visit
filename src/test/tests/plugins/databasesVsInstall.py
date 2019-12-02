# ----------------------------------------------------------------------------
#  CLASSES:    nightly
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
#    Eric Brugger, Wed Jun 26 09:32:06 PDT 2019
#    Removed the ccm readler.
#
#    Kathleen Biagas, Tue Nov 26 10:23:31 PST 2019
#    Modified to pass a 'test-this' list instead of a 'skip-this' list.
#    Plugins chosen based on third-party dependence, ability to auto-regen
#    their CMakeLists.txt and tests currently available to run against the
#    version built (to be added at a future time).
#
# ----------------------------------------------------------------------------

Source(tests_path("plugins", "pluginVsInstallHelpers"))

do_plugin_type("databases", ["BOV", "Blueprint", "CGNS", "Cale", "Claw", "Exodus", "FITS", "H5Part", "MFEM", "Mili", "SAMRAI", "VTK"])
# for future use
#do_tests("database", ["bov.py", "blueprint.py", "CGNS.py", "Cale.py", "claw.py", "exodus.py", "FITS.py", "h5part.py", "mfem.py", "mili.py", "samrai.py", "vtk.py"])

Exit()

