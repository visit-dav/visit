# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Mark C. Miller, Fri Aug 26 14:31:48 PDT 2022
# ----------------------------------------------------------------------------

#
# Get some utils shared across many .py files used for quick recipes.
#
Source(tests_path('quickrecipes','vqr_utils.py'))

def opening_a_database():
 
  try:
    # opening a database {
    # Open a database (no time specified defaults to time state 0)
    OpenDatabase("/Users/amina/data/pdb_test_data/allinone00.pdb")
    # opening a database }
    TestValueEQ('opening a database error message',GetLastError(),'')
    TestPOA('opening a database no exceptions')
  except Exception as inst:
    TestFOA('opening a database exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

def opening_a_database_at_specific_time():
 
  try:
    # opening a database at specific time {
    # Open a database at a specific time state to pick up transient variables
    OpenDatabase("/Users/amina/data/pdb_test_data/allinone00.pdb", 17)
    # opening a database at specific time }
    TestValueEQ('opening a database at specific time error message',GetLastError(),'')
    TestPOA('opening a database at specific time no exceptions')
  except Exception as inst:
    TestFOA('opening a database at specific time exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

def opening_a_virtual_database():
 
  try:
    # opening a virtual database {
    # Opening just one file, the first, in series wave0000.silo, wave0010.silo, ...
    OpenDatabase("~juanita/silo/stuff/wave0000.silo")

    # Opening a virtual database representing all available states.
    OpenDatabase("~juanita/silo/stuff/wave*.silo database")
    # opening a virtual database }
    TestValueEQ('opening a virtual database error message',GetLastError(),'')
    TestPOA('opening a virtual database no exceptions')
  except Exception as inst:
    TestFOA('opening a virtual database exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

def opening_a_remote_database():
 
  try:
    # opening a remote database {
    # Opening a file on a remote computer by giving a host name
    # Also, open the database to a later time slice (17)
    OpenDatabase("thunder:~juanita/silo/stuff/wave.visit", 17)
    # opening a remote database }
    TestValueEQ('opening a remote database error message',GetLastError(),'')
    TestPOA('opening a remote database no exceptions')
  except Exception as inst:
    TestFOA('opening a remote database exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

opening_a_database()
opening_a_database_at_specific_time()
opening_a_virtual_database()
opening_a_remote_database()

Exit()
