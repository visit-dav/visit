# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Mark C. Miller, Fri Aug 26 14:31:48 PDT 2022
# ----------------------------------------------------------------------------

#
# Get some utils shared across many .py files used for quick recipes.
#
Source(tests_path('quickrecipes','vqr_utils.py.inc'))

def using_gradient_background_colors():
 
  try:
    # using gradient background colors {
    # Set a blue/black, radial, gradient background.
    a = AnnotationAttributes()
    a.backgroundMode = a.Gradient
    a.gradientBackgroundStyle = a.Radial
    a.gradientColor1 = (0,0,255,255) # Blue
    a.gradientColor2 = (0,0,0,255) # Black
    SetAnnotationAttributes(a)
    # using gradient background colors }
    TestValueEQ('using gradient background colors error message',GetLastError(),'')
    TestPOA('using gradient background colors exceptions')
  except Exception as inst:
    TestFOA('using gradient background colors exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

def adding_a_banner():
 
  try:
    # adding a banner {
    # Create a Text2D object to indicate the visualization is unclassified.

    # Note the annoation object is added to the viewer window the moment it is created.
    banner = CreateAnnotationObject("Text2D")

    # Note text is updated in the viewer window the moment it is changed.
    banner.text = "Unclassified"

    banner.position = (0.37, 0.95)
    banner.fontBold = 1

    # print the attributes to see what you can set in the Text2D object.
    print(banner)
    # print(banner) will print something like...
    #    visible = 1
    #    position = (0.5, 0.5)
    #    height = 0.03
    #    textColor = (0, 0, 0, 255)
    #    useForegroundForTextColor = 1
    #    text = "2D text annotation"
    #    fontFamily = Arial  # Arial, Courier, Times
    #    fontBold = 0
    #    fontItalic = 0
    #    fontShadow = 0
    # adding a banner }
    TestValueEQ('adding a banner error message',GetLastError(),'')
    TestPOA('adding a banner exceptions')
  except Exception as inst:
    TestFOA('adding a banner exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

def adding_a_time_slider():
 
  try:
    # adding a time slider {
    # Add a time slider in the lower left corner
    slider = CreateAnnotationObject("TimeSlider")

    # Adjust the height. Takes effect immediately as the value is assigned.
    slider.height = 0.07

    # Print members that are available in the time slider object
    print(slider)
    # will produce something like...
    #    visible = 1
    #    active = 1
    #    position = (0.01, 0.01)
    #    width = 0.4
    #    height = 0.05
    #    textColor = (0, 0, 0, 255)
    #    useForegroundForTextColor = 1
    #    startColor = (0, 255, 255, 255)
    #    endColor = (255, 255, 255, 153)
    #    text = "Time=$time"
    #    timeFormatString = "%g"
    #    timeDisplay = AllFrames  # AllFrames, FramesForPlot, StatesForPlot, UserSpecified
    #    percentComplete = 0
    #    rounded = 1
    #    shaded = 1
    # adding a time slider }
    TestValueEQ('adding a time slider error message',GetLastError(),'')
    TestPOA('adding a time slider exceptions')
  except Exception as inst:
    TestFOA('adding a time slider exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

def adding_a_logo():
 
  try:
    # adding a logo {
    # Incorporate LLNL logo image (llnl.jpeg) as an annotation
    image = CreateAnnotationObject("Image")
    image.image = "llnl.jpeg"
    image.position = (0.02, 0.02)

    # Print the other image annotation options
    print(image)
    # Will print something like...
    #    visible = 1
    #    active = 1
    #    position = (0, 0)
    #    transparencyColor = (0, 0, 0, 255)
    #    useTransparencyColor = 0
    #    width = 100.000000
    #    height = 100.000000
    #    maintainAspectRatio = 1
    #    image = ("")
    # adding a logo }
    TestValueEQ('adding a logo error message',GetLastError(),'')
    TestPOA('adding a logo exceptions')
  except Exception as inst:
    TestFOA('adding a logo exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

def modifying_a_legend():
 
  try:
    # modifying a legend {
    # Open a file and make a plot
    OpenDatabase("~juanita/silo/stuff/noise.silo")
    AddPlot("Mesh", "Mesh")
    AddPlot("Pseudocolor", "hardyglobal")
    DrawPlots()
    # Get the legend annotation object for the Pseudocolor plot, the second
    # plot in the list (0-indexed).
    plotName = GetPlotList().GetPlots(1).plotName
    legend = GetAnnotationObject(plotName)
    # See if we can scale the legend.
    legend.xScale = 3.
    legend.yScale = 3.
    # the bounding box.
    legend.drawBoundingBox = 1
    legend.boundingBoxColor = (180,180,180,230)
    # Make it horizontal
    legend.orientation = legend.HorizontalBottom
    # moving the legend
    legend.managePosition = 0
    legend.position = (0.7,0.15)
    # text color
    InvertBackgroundColor()
    legend.useForegroundForTextColor = 0
    legend.textColor = (255, 0, 0, 255)
    # number format
    legend.numberFormat = "%1.4e"
    # the font.
    legend.fontFamily = legend.Arial
    legend.fontBold = 1
    legend.fontItalic = 1
    # turning off the labels.
    legend.fontItalic = 0
    legend.drawLabels = 0
    legend.drawMinMax = 0
    # turning off the title.
    legend.drawTitle = 0
    # Use user-supplied labels, rather than numeric values.
    legend.controlTicks=0
    legend.drawLabels = legend.Labels
    # suppliedLabels must be strings, only valid when controlTicks is 0
    legend.suppliedLabels=("A", "B", "C", "D", "E")
    # Give the legend a custom title
    legend.useCustomTitle=1
    legend.customTitle="my custom title"
    # modifying a legend }
    TestValueEQ('modifying a legend error message',GetLastError(),'')
    TestPOA('modifying a legend exceptions')
  except Exception as inst:
    TestFOA('modifying a legend exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

using_gradient_background_colors()
adding_a_banner()
adding_a_time_slider()
adding_a_logo()
modifying_a_legend()

Exit()
