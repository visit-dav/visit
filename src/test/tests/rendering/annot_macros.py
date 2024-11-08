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

# Set up the annotation colors, etc.
a = GetAnnotationAttributes()
a.backgroundColor = (80, 0, 100, 255)
a.foregroundColor = (255, 255, 255, 255)
a.backgroundMode = a.Solid
SetAnnotationAttributes(a)

#
# Open up the database and do a plot and
# set time step to something in the middle
#
OpenDatabase(silo_data_path("wave.visit"))
AddPlot("Pseudocolor", "pressure")
DrawPlots()
TimeSliderSetState(13)

#
# Loop through through all macros rendering with default format
# both as 2D text and as 3D text
#
macroNames = ("time", "cycle", "index", "numstates", "dbcomment",
    "lod", "vardim", "numvar", "topodim", "spatialdim", "varname",
    "varunits", "meshname", "filename", "fulldbname", "xunits",
    "yunits", "zunits", "xlabel", "ylabel", "zlabel")

TestSection("All macros as Text2D")
text = CreateAnnotationObject("Text2D")
text.height = 0.05
text.position = (0.20, 0.65)
for mtext in macroNames:
    text.text = "%s $%s"%(mtext, mtext)
    Test("text2d_%s"%mtext)
text.Delete()

TestSection("All macros as Text3D")
text = CreateAnnotationObject("Text3D")
text.heightMode = text.Relative
text.relativeHeight = 0.03
text.position=(2.0, 1.0, 7.0)
for mtext in macroNames:
    text.text = "%s $%s"%(mtext, mtext)
    Test("text3d_%s"%mtext)
text.Delete()

TestSection("Multiple macros in same annotation")
text = CreateAnnotationObject("Text2D")
text.height = 0.03
text.position = (0.10, 0.65)
text.text = "State $index of $numstates"
Test("multi_macro_1")
text.text = "Time,Cycle,Index=($time,$cycle,$index)"
Test("multi_macro_2")

TestSection("Printf formatting of $macro%fmt$")
# Print index/state always with 4 digits and leading zeros
text.text = "State $index%04d$ of $numstates%06d$"
Test("printf_fmt_1")
# Print index in hexadecimal format
text.text = "Index=0X$index%X$"
Test("printf_fmt_2")
# Print time with 12 width and 6 after decimal pt.
text.text = "Time=$time%12.6f$"
Test("printf_fmt_3")
# Print time with 20 width, leading sign, 10 after decimal pt.
text.text = "Time=$time%+20.10f$"
Test("printf_fmt_4")
# Print time with 10 char width, 6 precision, exponential format.
text.text = "Time=$time%+10.6e$"
Test("printf_fmt_5")
# Print time with %A (hex) to see full precision of actual value
text.text = "Time=$time ($time%A$)"
Test("printf_fmt_6")
# Print first 3 chars of meshname
text.text = "First 3 chars meshname='$meshname%.3s$'"
Test("printf_fmt_7")
# Print first 3 chars of meshname
text.text = "Left justified in 6 chars meshname='$meshname%-6.6s$'"
Test("printf_fmt_8")
# Handling % and $ chars in string
text.text = "State $index%02d$ of $numstates%02d$ = " + "%2d%%"%(100*float(13)/float(TimeSliderGetNStates()))
Test("printf_fmt_9")
text.text = "State $index is 18.3% into $numstates"
Test("printf_fmt_10")
text.text = "$cycle $US, $cycle$US"
Test("printf_fmt_11")
#text.Delete() just use the same annot object for below section

TestSection("Text annotation file (tafile) macros")
#
# 71 float values to write to a tafile, value @ index 13 is 0.83457325
#
fvals = [1.00000000, 0.99899307, 0.99597429, 0.99094976, 0.98392959, 0.97492791, 0.96396286,
    0.95105652, 0.93623487, 0.91952777, 0.90096887, 0.88059553, 0.85844879, 0.83457325, 0.80901699,
    0.78183148, 0.75307147, 0.72279486, 0.69106265, 0.65793873, 0.62348980, 0.58778525, 0.55089698,
    0.51289928, 0.47386866, 0.43388374, 0.39302503, 0.35137482, 0.30901699, 0.26603685, 0.22252093,
    0.17855689, 0.13423327, 0.08963931, 0.04486483, 0.00000000, -0.04486483, -0.08963931, -0.13423327,
   -0.17855689, -0.22252093, -0.26603685, -0.30901699, -0.35137482, -0.39302503, -0.43388374,
   -0.47386866, -0.51289928, -0.55089698, -0.58778525, -0.62348980, -0.65793873, -0.69106265,
   -0.72279486, -0.75307147, -0.78183148, -0.80901699, -0.83457325, -0.85844879, -0.88059553,
   -0.90096887, -0.91952777, -0.93623487, -0.95105652, -0.96396286, -0.97492791, -0.98392959,
   -0.99094976, -0.99597429, -0.99899307, -1.00000000]

#
# 71 string values to write to a tafile, value @ index 13 is "Monument"
#
svals = ["Ability", "Zealous", "Question", "Yarn", "Wisdom", "Venture", "Umbrella",
    "Triumph", "Sincere", "Revelation", "Porcelain", "Optimism", "Novelty", "Monument",
    "Latitude", "Kinship", "Journey", "Illumination", "Harvest", "Guidance", "Fidelity",
    "Elixir", "Duration", "Curiosity", "Blueprint", "Aspire", "Artifact", "Validation",
    "Unfold", "Theory", "Sanctuary", "Retreat", "Perspective", "Narrative", "Mastery",
    "Longevity", "Keystone", "Intrigue", "Heritage", "Generation", "Fluid", "Enigma",
    "Delight", "Craft", "Beacon", "Anonymous", "Achieve", "Whisper", "Vigilant", "Unity",
    "Tranquil", "Symbolic", "Resilience", "Quest", "Purity", "Overture", "Mirth",
    "Luminous", "Kindred", "Junction", "Insight", "Horizon", "Gradient", "Flourish",
    "Essence", "Destiny", "Cycle", "Brevity", "Awakening", "Altruism", "Zenith"]

# Get the TMP directory and the USER from the environment variables
tmp_dir = os.getenv('TMPDIR', '/tmp')  # Using TMPDIR as a fallback to TMP
user = os.getenv('USER')

# Construct the path to the directory where the file will be saved
directory_path = os.path.join(tmp_dir, user)

# Ensure the directory exists
os.makedirs(directory_path, exist_ok=True)

# Write the list of strings to the file
fname = os.path.join(directory_path, "stafile1.txt")
with open(fname, 'w') as file:
    for val in svals:
        file.write(val + "\n")
fname = os.path.join(directory_path, "ftafile1.txt")
with open(fname, 'w') as file:
    for val in fvals:
        file.write(f"{val:.8f}" + "\n")

text.text = "stafile=$stafile1, ftafile=$ftafile1"
Test("tafile_1")
TimeSliderNextState()
Test("tafile_2")
TimeSliderNextState()
Test("tafile_3")
TimeSliderSetState(5)
Test("tafile_4")
TimeSliderSetState(13)
# tafiles combined with printf formats
text.text = "stafile=$stafile1%.3s$, ftafile=$ftafile1%8.3f$"
Test("tafile_5")
TimeSliderNextState()
Test("tafile_6")
TimeSliderNextState()
Test("tafile_7")
TimeSliderSetState(5)
Test("tafile_8")
text.Delete()
TimeSliderSetState(13)

TestSection("Timeslider printf format handling")
text = CreateAnnotationObject("TimeSlider")
text.height = 0.15
text.position = (0.05, 0.1)
text.timeFormatString = "%0.4f"
Test("ts_printf_fmt_1")
text.timeFormatString = ""
text.text="Time=$time%-10.6E$"
Test("ts_printf_fmt_2")
text.text="Time=$time"
text.timeFormatString = "%A"
Test("ts_printf_fmt_3")

Exit()
