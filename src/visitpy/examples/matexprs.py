###############################################################################
#
# Purpose: Use VisIt CLI to iterate over Curves in a material database and
#     compute and plot some common difference curves and output the results
#     to either a curve or image file format.
#
# Programmer: Mark C. Miller
# Date:       Wed May 27 13:15:07 PDT 2009
###############################################################################
import sys, re, os, glob
from optparse import *

#
# Convert '#FFCC13" strings to color tuple
#
def ColorTupleFromHexString(s):
    if s[0] != '#':
        return (0, 0, 0, 255)
    return (int("0x%s"%s[1:3],16), \
            int("0x%s"%s[3:5],16), \
            int("0x%s"%s[5:7],16), \
            255)

#
# We have to override the _process_args method of OptionParser
# to get desired behavior in the presence of unrecognized options.
# Ordinarily, OptionParser errors out if it encounters an option
# it does not recognize, stopping processing of any further options
# making it rather difficult for options to visit to co-exist on
# same command line as options to this script. 
#
class MyOptionParser(OptionParser):
    def _process_args(self, largs, rargs, values):
        while rargs:
            arg = rargs[0]
            if arg == "--":
                del rargs[0]
                return
            elif arg[0:2] == "--":
                try:
                    self._process_long_opt(rargs, values)
                except BadOptionError: # These two lines result in allowing
                    pass               # an unrecognized option to be overlooked
            elif arg[:1] == "-" and len(arg) > 1:
                try:
                    self._process_short_opts(rargs, values)
                except BadOptionError: # These two lines result in allowing
                    pass               # an unrecognized option to be overlooked
            elif self.allow_interspersed_args:
                largs.append(arg)
                del rargs[0]
            else:
                return

#
# Command-line options
#
def BuildCommandLineOptions():
    parser = MyOptionParser()

    parser.add_option("--image-width",
        help="Set width of images [%default].",
        type="int", dest="image_width", default="500", metavar="INT")

    parser.add_option("--image-height",
        help="Set height of images [%default].",
        type="int", dest="image_height", default="500", metavar="INT")

    parser.add_option("--data-min",
        type="float", dest="data_min", metavar="FLOAT",
        help="Mininum data value to be applied to all plots. If no "
             "value is specified, the minimum will be allowed to vary "
             "as needed from plot to plot.")

    parser.add_option("--data-max",
        type="float", dest="data_max", metavar="FLOAT",
        help="Mininum data value to be applied to all plots. If no "
             "value is specified, the minimum will be allowed to vary "
             "as needed from plot to plot.")

    parser.add_option("--log-data",
        help="Display data (y) axis in log scaling.",
        action="store_true", dest="log_data", default=False)

    parser.add_option("--x-min",
        type="float", dest="x_min", metavar="FLOAT",
        help="Mininum positional (x) value to be applied to all plots. If no "
             "value is specified, the minimum will be allowed to vary "
             "as needed from plot to plot.")

    parser.add_option("--x-max",
        type="float", dest="x_max", metavar="FLOAT",
        help="Maximum positional (x) value to be applied to all plots. If no "
             "value is specified, the minimum will be allowed to vary "
             "as needed from plot to plot.")

    parser.add_option("--log-x",
        help="Display positional (x) axis in log scaling.",
        action="store_true", dest="log_x", default=False)

    parser.add_option("--image-format",
        help="Set output format for images (e.g. 'tiff', 'png', 'jpeg'). "
             "If none specified, no images will be saved.",
        dest="image_format", metavar="STRING")

    parser.add_option("--curve-format",
        help="Set output format for curves (e.g. 'ultra', 'curve'). "
             "If none specified, no curve files will be saved.",
        dest="curve_format", metavar="STRING")

    parser.add_option("--color0",
        help="Set color to be used for first curve plot.",
        dest="color0", metavar="#RRGGBB")

    parser.add_option("--color1",
        help="Set color to be used for second curve plot.",
        dest="color1", metavar="#RRGGBB")

    parser.add_option("--line-width",
        help="Set line width for curves.",
        type="int", default=0, dest="line_width", metavar="INT")

    parser.add_option("--point-density",
        help="Plot symbols representing individual points in curves every Nth point. "
             "A value of zero turns the display of points off [%default].",
        type="int", default=0, dest="point_density", metavar="N")

    parser.add_option("--point-size",
        help="Size of symbols representing individual points in curve plots.",
        type="int", default=5, dest="point_size", metavar="INT")

    parser.add_option("--show-legend",
        help="Display curve plot legends.",
        action="store_true", dest="show_legend", default=False)

    parser.add_option("--show-labels",
        help="Display curve plot labels.",
        action="store_true", dest="show_labels", default=False)

    parser.set_usage("matexprs.py [options] dbname")

    return parser

#
# Iterate through curves, finding all unique 'dirs' containing curves.
#
def GetVarMap(metadata):
    dirMap = {}
    for i in range(metadata.GetNumCurves()):
       dirinfo = re.search("(.*)/([^/]*)", metadata.GetCurves(i).name)
       if dirinfo != None:
           dirname = dirinfo.group(1)
           varname = dirinfo.group(2)
           varMap = {}
           if dirname in dirMap:
               varMap = dirMap[dirname]
           varMap[varname] = 1
           dirMap[dirname] = varMap
    return dirMap

#
# Begin main program
#
parser = BuildCommandLineOptions()

#
# This bit of logic allows users to get usage/help from
# the command 'python matexpers.py --help'. Without it
# using VisIt's cli the '--help' will get interpreted
# in internallauncher and never make it into this script.
#
if "-h" in sys.argv or \
   "--help" in sys.argv or \
   "-help" in sys.argv or \
   "help" in sys.argv:
    parser.print_help()
    sys.exit(1)

(clOpts, clArgs) = parser.parse_args(list(Argv()))

#
# Set the name of the database. It is the only 'positional'
# argument on the command line.
#
dbname = ""
if len(clArgs) > 0:
    dbname = clArgs[0]
if not glob.glob(dbname):
    if dbname == "":
        sys.stderr.write("No database specified.\n")
    else:
        sys.stderr.write("Invalid database, \"%s\", specified.\n"%dbname)
    parser.print_usage()
    sys.exit(1)

#
# Open the database, get metadata, get info on curve 'dirs'
#
OpenDatabase(dbname)
metadata = GetMetaData(dbname)
dirMap = GetVarMap(metadata)

#
# Build up base save window attributes
#
swa = SaveWindowAttributes()
swa.family = 0
swa.width = clOpts.image_width
swa.height = clOpts.image_height

#
# Build up base curve attributes
#
ca = CurveAttributes()
ca.lineWidth = clOpts.line_width
if clOpts.color0 != None:
    ca.color = ColorTupleFromHexString(clOpts.color0)
    ca.cycleColors = 0
ca.showLabels = clOpts.show_labels
#if clOpts.point_density > 0:
#    ca.showPoints = 1
#ca.pointSize = clOpts.point_size 
ca.showLegend = clOpts.show_legend
#ca.symbolDensity = clOpts.point_density
SetDefaultPlotOptions(ca)

#
# Iterate through all curve 'dirs', finding instances where
# all essential variables exist. Create expressions and plot 'em
#
for k in dirMap.keys():
    if not ("Ec" in dirMap[k] and \
            "cEc" in dirMap[k] and \
            "cEc_fit" in dirMap[k]):
        print "Ignoring %s because not all required vars are present."%k
        #del dirMap[k]
        continue

    DefineCurveExpression("%s/c0"%k, "<%s/Ec>-<%s/cEc_fit>"%(k,k))
    DefineCurveExpression("%s/c1"%k, "<%s/cEc>-<%s/cEc_fit>"%(k,k))
    AddPlot("Curve","%s/c0"%k)
    AddPlot("Curve","%s/c1"%k)
    DrawPlots()
    v = GetViewCurve()
    if clOpts.x_min != None:
        v.domainCoords = (clOpts.x_min, v.domainCoords[1])
    if clOpts.x_max != None:
        v.domainCoords = (v.domainCoords[0], clOpts.x_max)
    if clOpts.log_x:
        v.domainScale = v.LOG
    if clOpts.data_min != None:
        v.rangeCoords = (clOpts.data_min, v.rangeCoords[1])
    if clOpts.data_max != None:
        v.rangeCoords = (v.rangeCoords[0], clOpts.data_max)
    if clOpts.log_data:
        v.rangeScale = v.LOG
    SetViewCurve(v)
    if clOpts.color1 != None:
        ca2 = CurveAttributes()
        ca2.color = ColorTupleFromHexString(clOpts.color1)
        ca2.cycleColors = 0
        SetActivePlots((1,))
        SetPlotOptions(ca2)
    DrawPlots()
    if clOpts.curve_format != None:
        swa.format = getattr(swa,clOpts.curve_format.upper())
        swa.fileName = k # .curve is added automatically
        SetSaveWindowAttributes(swa)
        SaveWindow()
    if clOpts.image_format != None:
        swa.format = getattr(swa,clOpts.image_format.upper())
        #swa.fileName = "%s.%s"%(k,clOpts.image_format.lower())
        swa.fileName = k
        SetSaveWindowAttributes(swa)
        SaveWindow()
    DeleteAllPlots()
