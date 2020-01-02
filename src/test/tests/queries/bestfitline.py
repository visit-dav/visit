# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  bestfit.py
#
#  Tests:      queries     - "Best Fit Line"
#              expressions - distance_to_best_fit_line
#
#  Defect ID:  none
#
#  Programmer: Brad Whitlock
#  Date:       Fri Nov 18 16:43:42 PST 2005
#
#  Modifications:
#
#    Mark C. Miller, Tue Mar 14 07:54:20 PST 2006
#    Changed how full-frame is turned on
#    
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Matt Larsen Wed May 09 08:31:00 PST 2018
#    Adding view reset so image actual shows a line 
# ----------------------------------------------------------------------------

import math

def writeDataSet():
    nts = 100
    nsamples = 100
    x0 = -5.
    y0 = -5.
    x1 = 5.
    y1 = 5.

    names = []
    coefficients = []
    for i in range(nts):
        t = float(i) / float(nts - 1)
        y_left = t * y1 + (1. - t) * y0
        y_right = t * y0 + (1. - t) * y1
        filename = data_path("curve_test_data/line%04d.curve") % i
        names = names + [filename]
        try:
            f = open(filename, "w")
        
            f.write("#TIME %g\n" % t)
            f.write("#line\n")

            m = (y_right - y_left) / (x1 - x0)
            angle = t * 3.14159 * 2.
            b  = math.sin(angle)

            #print "Y = %gX + %g" % (m, b)
            coefficients = coefficients + [(m, b)]

            for j in range(nsamples):
                t2 = float(j) / float(nsamples - 1)
                x = t2 * x1 + (1. - t2) * x0
                y = t2 * y_right + (1. - t2) * y_left + b
                f.write("%g %g\n" % (x, y))

            f.close()
        except:
            break

    return (names, coefficients)

def removeFiles(ds):
    for d in ds:
        try:
            os.unlink(d)
        except:
            print("Could not remove ", d)

#
# Test best fit line of known lines from Curve plots
#
def test1():
    data = writeDataSet()
    filenames = data[0]
    coefficients = data[1]

    OpenDatabase(data_path("curve_test_data/line*.curve database"))

    AddPlot("Curve", "line")
    DrawPlots()

    s = ""
    try:
        for state in range(0, len(coefficients), 2):
            SetTimeSliderState(state)
            Query("Best Fit Line")
            values = GetQueryOutputValue()
            s = s + "Original (m=%g, b=%g)\t\t\tCalculated (m=%g, b=%g, r=%g)\n" %\
            (coefficients[state][0], coefficients[state][1], values[0], \
            values[1], values[2])
    except:
        pass

    TestText("bestline_1_00", s)
    removeFiles(data[0])
    DeleteAllPlots()

#
# Test best fit line of 2D Scatter plots
#
def test2():
    # Do Scatter plot of d vs d since we know that it will make Y=X
    OpenDatabase(silo_data_path("multi_curv2d.silo"))

    AddPlot("Scatter", "d")
    s = ScatterAttributes()
    s.var2 = "d"
    s.var2Role = s.Coordinate1
    s.var3Role = s.None
    s.var4Role = s.None
    s.scaleCube = 0
    s.pointSizePixels = 5
    SetPlotOptions(s)
    DrawPlots()

    # Turn on Fullframe
    v = GetView2D()
    v.fullFrameActivationMode = v.On
    SetView2D(v)
    Test("bestline_2_00")
    Query("Best Fit Line")
    TestText("bestline_2_01", GetQueryOutputString())

    # Check the best fit with another known line eq.
    DeleteAllPlots()
    OpenDatabase(silo_data_path("curv2d.silo"))

    DefineScalarExpression("t", "zoneid(curvmesh2d)")
    DefineScalarExpression("line", "t * 0.33333 + 10.")
    AddPlot("Scatter", "t")
    s.var2 = "line"
    SetPlotOptions(s)
    DrawPlots()
    ResetView()
    Test("bestline_2_02")
    Query("Best Fit Line")
    TestText("bestline_2_03", GetQueryOutputString())

    # Make the scatter plot be d vs. dpu
    DeleteAllPlots()
    OpenDatabase(silo_data_path("multi_curv2d.silo"))

    AddPlot("Scatter", "d")
    DefineScalarExpression("dpu", "d + u / 3.")
    s.var2 = "dpu"
    SetPlotOptions(s)
    DrawPlots()
    Test("bestline_2_04")
    Query("Best Fit Line")
    TestText("bestline_2_05", GetQueryOutputString())

    return s

#
# Test distance to best fit line expression. The colors should appear
# to be banded about the best fit line.
#
def test3(s):
    # We still have the Scatter plot from the previous test. Color it
    # by the distance from the best fit line.
    DefineScalarExpression("DBFL", "distance_to_best_fit_line(d, dpu)")
    DefineScalarExpression("DBFL2", "distance_to_best_fit_line2(d, dpu)")
    s.var3 = "DBFL"
    s.var3Role = s.Color
    s.colorTableName = "difference"
    s.pointSizePixels = 10
    SetPlotOptions(s)
    Test("bestline_3_00")

    s.var3 = "DBFL2"
    SetPlotOptions(s)
    Test("bestline_3_01")

    DefineScalarExpression("dpus", "d + u / 2. + sin(d * 5.) * 2.")
    DefineScalarExpression("DBFL3", "distance_to_best_fit_line(d, dpus)")
    DefineScalarExpression("DBFL4", "distance_to_best_fit_line2(d, dpus)")
    s.var2 = "dpus"
    s.var3 = "DBFL3"
    SetPlotOptions(s)
    Test("bestline_3_02")

    s.var3 = "DBFL4"
    SetPlotOptions(s)
    Test("bestline_3_03")

    return


def main():
    test1()
    s = test2()
    test3(s)


main()
Exit()
