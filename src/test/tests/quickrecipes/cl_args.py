# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Mark C. Miller, Fri Aug 26 14:31:48 PDT 2022
# ----------------------------------------------------------------------------

#
# Get some utils shared across many .py files used for quick recipes.
#
Source(tests_path('quickrecipes','vqr_utils.py.inc'))

def handling_command_line_arguments():
 
  try:
    # handling command line arguments {
    import sys
    print(Argv)
    print(sys.argv)
    # handling command line arguments }
    TestValueEQ('handling command line arguments errors',GetLastError(),'')
    TestPOA('handling command line arguments no exceptions')
  except Exception as inst:
    TestFOA('handling command line argumentts exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

handling_command_line_arguments()

Exit()
