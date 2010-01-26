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

if SILO_MODE == "pdb":
    SILO_MODE = "hdf5"
else:
    SILO_MODE = "pdb"
Source("tests/databases/silo.py")
