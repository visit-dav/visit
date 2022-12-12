# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Mark C. Miller, Fri Aug 26 14:31:48 PDT 2022
# ----------------------------------------------------------------------------

#
# Get some utils shared across many .py files used for quick recipes.
#
Source(tests_path('quickrecipes','vqr_utils.py'))

def setting_output_image_characteristics():
 
  try:
    # setting output image characteristics {
    # Prepare to save a BMP file at 1024x768 resolution 
    s = SaveWindowAttributes() 
    s.format = s.BMP 
    s.fileName = 'mybmpfile' 
    s.width, s.height = 1024,768 
    s.screenCapture = 0 
    SetSaveWindowAttributes(s) 
    # Subsequent calls to SaveWindow() will use these settings
    # setting output image characteristics }
    TestValueEQ('setting output image characteristics error message',GetLastError(),'')
    TestPOA('setting output image characteristics no exceptions')
  except Exception as inst:
    TestFOA('setting output image characteristics exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

def saving_an_image():
 
  try:
    # saving an image {
    # Save images of all timesteps and add each image filename to a list.
    names = []
    for state in range(TimeSliderGetNStates()):
      SetTimeSliderState(state)
      # Save the image
      n = SaveWindow()
      names = names + [n]
    print(names)
    # saving an image }
    TestValueEQ('saving an image error message',GetLastError(),'')
    TestPOA('saving an image no exceptions')
  except Exception as inst:
    TestFOA('saving an image exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

setting_output_image_characteristics()
saving_an_image()

Exit()
