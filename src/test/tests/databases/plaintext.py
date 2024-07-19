# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  plaintext.py
#
#  Tests:      multiple curves from a CSV file without and with implicit X
#              2D and 3D points with variables in a CSV file
#              An array of data as a 2D regular grid
#
#  Notes:
#    - Code snippets from this file are literalinclude'd in our docs.
#    - Test image results produced by this test are also literalinclude'd.
#    - Input .csv files used here are also literalinclude'd.
#    - If you ever need to change the .csv files, you can regen them using
#      this script with 'genonly' CL arg and vanilla python (it won't make any
#      VisIt CLI calls).
#
#  Mark C. Miller, Fri Mar 11 14:33:30 PST 2022
#  Based on original code by Eric Brugger in our documentation
# ----------------------------------------------------------------------------
import copy, math, os, sys

#
# Example of PlainText open options dict...
#
# {'Data layout': '1D Columns # Options are: 1D Columns, 2D Array',
#  'Lines to skip at beginning of file': 0,
#  'First row has variable names': 0,
#  'Column for X coordinate (or -1 for none)': -1,
#  'Column for Y coordinate (or -1 for none)': -1,
#  'Column for Z coordinate (or -1 for none)': -1}
#

#
# Use this function with deepcopy to ensure we always start from a
# prestine default state for open options.
#
def GetDefaultOpenOptions():
    if not hasattr(GetDefaultOpenOptions, 'defaultOpenOptions'):
        GetDefaultOpenOptions.defaultOpenOptions = copy.deepcopy(GetDefaultFileOpenOptions("PlainText"))
    return copy.deepcopy(GetDefaultOpenOptions.defaultOpenOptions)

def CreateCurvesDataFile(filename):

    # Curve gen {
    with open(filename, "wt") as f:
        # create header
        f.write("angle,sine,cosine\n")
        npts = 73
        for i in range(npts):
            angle_deg = float(i) * (360. / float(npts-1))
            angle_rad = angle_deg * (3.1415926535 / 180.)
            sine = math.sin(angle_rad)
            cosine = math.cos(angle_rad)
            # write abscissa (x value) and ordinates (y-value(s))
            f.write("%g,%g,%g\n" % (angle_deg, sine, cosine))
    # Curve gen }

def CreateCurvesDataFileWithNoXCoordinates(filename):

    # Curve noX gen {
    with open(filename, "wt") as f:
        # create header
        f.write("inverse,sqrt,quadratic\n")
        npts = 100 
        for i in range(npts):
            inv = float(100) / (float(i)+1)
            sqr = 10 * math.sqrt(i)
            quad = float(i*i) / float(100)
            f.write("%g,%g,%g\n" % (inv, sqr, quad))
    # Curve noX gen }

def Create3DPointsWithVariablesDataFile(filename):

    # Points gen {
    with open(filename, "wt") as f:
        # write header
        f.write("x y z velx vely velz temp\n")
        n = 100
        for i in range(n):
            t = float(i) / float(n-1)
            angle = t * (math.pi * 2.) * 5.
            r = t * 10.
            x = r * math.cos(angle)
            y = r * math.sin(angle)
            z = t * 10.
            vx = math.sqrt(x*x + y*y)
            vy = math.sqrt(y*y + z*z)
            vz = math.sqrt(x*x + z*z)
            temp = math.sqrt((t-0.5)*(t-0.5))
            # write point and value(s)
            f.write("%g %g %g %g %g %g %g\n" % (x,y,z,vx,vy,vz,temp))
    # Points gen }

def Create2DArrayDataFile(filename):

    # Array gen {
    with open(filename, "wt") as f:
        # Only the first column name matters.
        # The others are required but otherwise ignored.
        f.write("density c2 c3 c4 c5 c6 c7 c8\n")
        nx = 8
        ny = 10
        for iy in range(ny):
            y = float(iy)
            for ix in range(nx):
                x = float(ix)
                dist = math.sqrt(x*x + y*y)
                if (ix < nx - 1):
                   f.write("%g " % dist)
                else:
                   f.write("%g\n" % dist)
    # Array gen }

def TestCSVCurves():
    TestSection("CSV data as Curves")
    CreateCurvesDataFile("curves.csv")

    # Curve plot {
    # GetDefaultOpenOptions is a wrapper to the CLI method GetDefaultFileOpenOptions.
    # GetDefaultOpenOptions function deepcopy to ensure we always start from a prestine default state for open options.
    plainTextOpenOptions = GetDefaultOpenOptions()
    plainTextOpenOptions['First row has variable names'] = 1
    plainTextOpenOptions['Column for X coordinate (or -1 for none)'] = 0
    SetDefaultFileOpenOptions("PlainText", plainTextOpenOptions)

    OpenDatabase("curves.csv")
    AddPlot("Curve","sine")
    AddPlot("Curve","cosine")
    DrawPlots()
    # Curve plot }
    ResetView()
    Test("PlainText_Curves")
    DeleteAllPlots()
    CloseDatabase("curves.csv")
    os.unlink("curves.csv")

def TestCSVCurvesNoX():
    TestSection("CSV data as Curves inferred X coordinate")
    CreateCurvesDataFileWithNoXCoordinates("curves_nox.csv")

    # Curve noX plot {
    # GetDefaultOpenOptions is a wrapper to the CLI method GetDefaultFileOpenOptions.
    # GetDefaultOpenOptions function deepcopy to ensure we always start from a prestine default state for open options.
    plainTextOpenOptions = GetDefaultOpenOptions()
    plainTextOpenOptions['First row has variable names'] = 1
    SetDefaultFileOpenOptions("PlainText", plainTextOpenOptions)

    OpenDatabase("curves_nox.csv")
    AddPlot("Curve","inverse")
    AddPlot("Curve","sqrt")
    AddPlot("Curve","quadratic")
    DrawPlots()
    # Curve noX plot }
    ResetView()
    Test("PlainText_Curves_noX")
    DeleteAllPlots()
    CloseDatabase("curves_nox.csv")
    os.unlink("curves_nox.csv")

def TestCSV3DPointsAndVariables():
    TestSection("CSV data as 3D points with variable(s)")
    Create3DPointsWithVariablesDataFile("points.txt")

    # Points plot {
    # GetDefaultOpenOptions is a wrapper to the CLI method GetDefaultFileOpenOptions.
    # GetDefaultOpenOptions function deepcopy to ensure we always start from a prestine default state for open options.
    plainTextOpenOptions = GetDefaultOpenOptions()
    plainTextOpenOptions['First row has variable names'] = 1
    plainTextOpenOptions['Column for X coordinate (or -1 for none)'] = 0
    plainTextOpenOptions['Column for Y coordinate (or -1 for none)'] = 1
    plainTextOpenOptions['Column for Z coordinate (or -1 for none)'] = 2
    SetDefaultFileOpenOptions("PlainText", plainTextOpenOptions)
    OpenDatabase("points.txt")

    DefineVectorExpression("vel", "{velx,vely,velz}")
    AddPlot("Pseudocolor", "temp")
    AddPlot("Vector","vel")
    DrawPlots()
    # Points plot }
    ResetView()
    Test("PlainText_Points")
    DeleteAllPlots()
    CloseDatabase("points.txt")
    os.unlink("points.txt")

def TestCSV2DArray():
    TestSection("CSV data as 2D array on uniform grid")
    Create2DArrayDataFile("array.txt")

    # Array plot {
    # GetDefaultOpenOptions is a wrapper to the CLI method GetDefaultFileOpenOptions.
    # GetDefaultOpenOptions function deepcopy to ensure we always start from a prestine default state for open options.
    plainTextOpenOptions = GetDefaultOpenOptions()
    plainTextOpenOptions['First row has variable names'] = 1
    plainTextOpenOptions['Data layout'] = '2D Array'
    SetDefaultFileOpenOptions("PlainText", plainTextOpenOptions)

    OpenDatabase("array.txt")
    AddPlot("Pseudocolor", "density")
    DrawPlots()
    ResetView()
    # Array plot }
    Test("PlainText_2DArray")
    DeleteAllPlots()
    CloseDatabase("array.txt")
    os.unlink("array.txt")

def main():

    if 'genonly' in sys.argv:
        CreateCurvesDataFile('curves.csv')
        CreateCurvesDataFileWithNoXCoordinates('curves_nox.csv')
        Create3DPointsWithVariablesDataFile('points.txt')
        Create2DArrayDataFile('array.txt')
        sys.exit(0)

    #
    # Idiom here...we can't manipulate open options until *after*
    # mdserver is running
    #
    OpenMDServer("localhost")

    #
    # Make sure curve plots are without labels
    #
    ca = CurveAttributes()
    ca.showLabels = 0
    SetDefaultPlotOptions(ca)

    TestCSVCurves()

    TestCSVCurvesNoX()

    TestCSV3DPointsAndVariables()

    TestCSV2DArray()

    Exit()

main()
