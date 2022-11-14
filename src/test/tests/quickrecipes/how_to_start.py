# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Mark C. Miller, Fri Aug 26 14:31:48 PDT 2022
# ----------------------------------------------------------------------------

#
# Get some utils shared across many .py files used for quick recipes.
#
Source(tests_path('quickrecipes','vqr_utils.py'))

def using_session_files():
 
  try:
    # using session files {
    # Import a session file from the current working directory. 
    RestoreSession('/home/juan/visit/visit.session', 0) 
    # Now that VisIt has restored the session, animate through time.
    for state in range(TimeSliderGetNStates()): 
      TimeSliderSetState(state) 
      SaveWindow() 
    # using session files }
    TestValueEQ('using session files error message',GetLastError(),'')
    TestPOA('using session files no exceptions')
  except:
    TestFOA('using session files exception', LINE())
    pass
  vqr_cleanup()

def getting_something_on_the_screen():

  try:
    # getting something on the screen {
    # Step 1: Open a database 
    OpenDatabase('~juanita/silo/stuff/wave.visit') 

    # Step 2: Add plots with default properties
    AddPlot('Pseudocolor', 'pressure') 
    AddPlot('Mesh', 'quadmesh') 

    # Step 3: Draw the plots with default view
    DrawPlots() 

    # Step 4: Iterate through time and save images 
    for state in range(0,TimeSliderGetNStates(),10): 
      TimeSliderSetState(state) 
      SaveWindow() 
    # getting something on the screen }
    TestValueEQ('getting something on the screen error message',GetLastError(),'')
    TestPOA('getting something on the screen no exceptions')
  except:
    TestFOA('getting something on the screen exception', LINE())
    pass
  vqr_cleanup()

using_session_files()
getting_something_on_the_screen()

Exit()
