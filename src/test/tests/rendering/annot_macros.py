# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  annot_macros.py
#
#  Tests:      mesh        - 2D curvilinear, single domain
#              plots       - Pseudocolor
#              annotations - Text2D
#
#  Defect ID:  None
#
#  Mark C. Miller, Thu Nov  7 14:17:54 PST 2024
# ----------------------------------------------------------------------------
import os

def create_text2d_annot():
    text = CreateAnnotationObject("Text2D")
    text.height = 0.03
    text.position = (0.10, 0.75)
    return text

def allmacros():
    """All non-empty macros in all text modes"""

    #
    # Loop through through all macros rendering with default format
    # both as 2D text and as 3D text. We skip "lod", "xunits", "yunits",
    # "zunits" and "varunits" because the input database has no values
    # specified for these.
    #
    macroNames = ("time", "cycle", "index", "numstates", "dbcomment",
        "vardim", "numvar", "topodim", "spatialdim", "varname",
        "meshname", "filename", "fulldbname", "xlabel", "ylabel", "zlabel")

    text2d = create_text2d_annot()
    text3d = CreateAnnotationObject("Text3D")
    text3d.heightMode = text3d.Relative
    text3d.relativeHeight = 0.03
    text3d.position=(2.0, 1.0, 7.0)
    textts = CreateAnnotationObject("TimeSlider")
    textts.height = 0.15
    textts.position = (0.05, 0.1)
    for mtext in macroNames:
        text2d.text = "%s $%s"%(mtext, mtext)
        text3d.text = "%s $%s"%(mtext, mtext)
        textts.text = "%s $%s"%(mtext, mtext)
        TestAutoName()
    text2d.Delete()
    text3d.Delete()
    textts.Delete()

def multimacro():
    """Multiple macros in same annotation"""

    text = create_text2d_annot()
    text.text = "State $index of $numstates"
    TestAutoName()
    text.text = "Time,Cycle,Index=($time,$cycle,$index)"
    TestAutoName()
    text.text = "Cycle,Cycle,Cycle=($cycle,$cycle,$cycle)"
    TestAutoName()
    text.Delete()

def printf():
    """Printf formatting of $macro%fmt$"""

    text = create_text2d_annot()
    # Print index/state always with 4 digits and leading zeros
    text.text = "State $index%04d$ of $numstates%06d$"
    TestAutoName()
    # Print index in hexadecimal format
    text.text = "Index=0X$index%X$"
    TestAutoName()
    # Print time with 12 width and 6 after decimal pt.
    text.text = "Time=$time%12.6f$"
    TestAutoName()
    # Print time with 20 width, leading sign, 10 after decimal pt.
    text.text = "Time=$time%+20.10f$"
    TestAutoName()
    # Print time with 10 char width, 6 precision, exponential format.
    text.text = "Time=$time%+10.6e$"
    TestAutoName()
    # Print time with %A (hex) to see full precision of actual value
    text.text = "Time=$time ($time%A$)"
    TestAutoName()
    # Print first 3 chars of meshname
    text.text = "First 3 chars meshname='$meshname%.3s$'"
    TestAutoName()
    # Print first 3 chars of meshname
    text.text = "Left justified in 6 chars meshname='$meshname%-6.6s$'"
    TestAutoName()
    # Handling % and $ chars in string
    text.text = "State $index%02d$ of $numstates%02d$ = " + "%2d%%"%(100*float(13)/float(TimeSliderGetNStates()))
    TestAutoName()
    text.text = "State $index is 18.3% into $numstates"
    TestAutoName()
    text.text = "$cycle $US, $cycle$US"
    TestAutoName()
    text.Delete()

def tafile():
    """Text annotation file (tafile) macros"""

    #
    # Create lists of 71 float values for "ftafile1.txt" and string values 
    # for "stafile1.txt" starting at 0.
    #
    fvals = [] # float values
    svals = [] # string values
    for i in range(71):
        fvals.append(i if i%2==0 else -i)
        svals.append("VisIt_%03d"%i)
    fvals[13] = -13.333678

    try:
        #
        # Write the two lists to their repsective tafile files.
        # Note this is relying up on the tafile being peer to database file
        # method to locate these files. The other options are via ~/.visit
        # and via TMP both of which involve different paths for py code
        # on Windows and so are less attractive. This is inside a try...finally
        # block to ensure the created files are then later removed from
        # the silo data dir where they are created.
        #
        with open(silo_data_path("stafile1.txt"), 'w') as file1, \
             open(silo_data_path("ftafile1.txt"), 'w') as file2:
             for val1, val2 in zip(svals, fvals):
                 file1.write(val1 + "\n")
                 file2.write(f"{val2:.8f}" + "\n")

        #
        # Ok, now produce annotation text using $stafile1 and $ftafile1.
        # Move around in time steps a bit also.
        #
        text = create_text2d_annot()
        text.text = "stafile=$stafile1, ftafile=$ftafile1"
        TestAutoName()
        TimeSliderNextState()
        TestAutoName()
        TimeSliderNextState()
        TestAutoName()
        TimeSliderSetState(5)
        TestAutoName()
        TimeSliderSetState(13)

        #
        # tafiles macros combined with printf formats
        #
        text.text = "stafile=$stafile1%.3s$, ftafile=$ftafile1%8.3f$"
        TestAutoName()
        TimeSliderNextState()
        TestAutoName()
        TimeSliderNextState()
        TestAutoName()
        TimeSliderSetState(5)
        TestAutoName()
        text.Delete()
        TimeSliderSetState(13)

    finally:
        os.remove(silo_data_path("stafile1.txt"))
        os.remove(silo_data_path("ftafile1.txt"))

def tsprintf():
    """Timeslider printf format handling"""

    text = CreateAnnotationObject("TimeSlider")
    text.height = 0.15
    text.position = (0.05, 0.1)
    text.timeFormatString = "%0.4f"
    TestAutoName()
    text.timeFormatString = ""
    text.text="Time=$time%-10.6E$"
    TestAutoName()
    text.text="Time=$time"
    text.timeFormatString = "%A"
    TestAutoName()

def init():
    """Initialization for all tests"""

    # Set up the annotation colors, etc.
    a = GetAnnotationAttributes()
    a.backgroundColor = (80, 0, 100, 255)
    a.foregroundColor = (255, 255, 255, 255)
    a.backgroundMode = a.Solid
    SetAnnotationAttributes(a)

    #
    # Open up the one and only database to be used for all these
    # tests, add a plot set time step to something in the middle,
    # index=13
    #
    OpenDatabase(silo_data_path("wave.visit"))
    AddPlot("Pseudocolor", "pressure")
    DrawPlots()
    TimeSliderSetState(13)

def finalize():
    DeleteAllPlots()
    CloseDatabase(silo_data_path("wave.visit"))

def main():

    init()
    allmacros()
    multimacro()
    printf()
    tafile()
    tsprintf()
    finalize()

main()

Exit()
