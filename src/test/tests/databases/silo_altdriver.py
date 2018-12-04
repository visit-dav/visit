# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  silo_altdriver
#
#  Tests:      Runs the silo.py test using the 'other' driver. If silo.py
#              was run on PDB driver, this will run them on HDF5 driver and
#              vice versa.
#
#  Programmer: Mark C. Miller
#  Date:       25Jan10
#
# ----------------------------------------------------------------------------

if TestEnv.params["silo_mode"] == "pdb":
    TestEnv.params["silo_mode"] = "hdf5"
else:
    TestEnv.params["silo_mode"] = "pdb"

Source(tests_path("databases","silo.py"))
