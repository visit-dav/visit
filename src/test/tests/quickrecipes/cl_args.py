# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Mark C. Miller, Fri Aug 26 14:31:48 PDT 2022
# ----------------------------------------------------------------------------

#
# Get some utils shared across many .py files. Exec'ing in this way avoid
# an import hell caused by the interdependency of functions used in
# vqr_utils.py but nonetheless defined in other parts of VisIt's test harness.
# This small block of code needs to be copied into any .py file being used
# to demonstrate a quick recipe.
#
with open(tests_path('quickrecipes','vqr_utils.py'), 'r') as f: 
    prog = compile(''.join(f.readlines()),'load_vqr_utils.err','exec')
    exec(prog)

def handling_command_line_arguments():
 
  try:
    # handling command line arguments {
    import sys
    print(Argv)
    print(sys.argv)
    # handling command line arguments }
    TestValueEQ('handling command line arguments errors',GetLastError(),'')
    TestPOA('handling command line arguments no exceptions')
  except:
    TestFOA('handling command line arguments exception', LINE())
    pass
  vqr_cleanup()

handling_command_line_arguments()

Exit()
