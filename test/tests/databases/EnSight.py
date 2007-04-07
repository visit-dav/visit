# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  EnSight.py
#
#  Tests:      mesh      - 2D structured
#              plots     - Pseudocolor
#
#  Programmer: Brad Whitlock
#  Date:       Tue Jun 27 10:27:50 PDT 2006
#
#  Modifications:
#    Brad Whitlock, Wed Sep 27 10:19:38 PDT 2006
#    Test another dataset to make sure that the cycle/time values and the
#    dataset values are right.
#
# ----------------------------------------------------------------------------

def test0(datapath):
    TestSection("MT, ASCII, EnSight Gold files")

    # Set up the plots.
    OpenDatabase(datapath + "CHR.case")
    AddPlot("Pseudocolor", "Pression")
    DrawPlots()

    # Save images for all 8 time steps
    s = "Time values:\n"
    for i in range(8):
        SetTimeSliderState(i)
        DrawPlots()
        Test("EnSight_0_%02d" % i)

        Query("Time")
        s = s + GetQueryOutputString() + "\n"

    TestText("EnSight_0_08", s)
    DeleteAllPlots()

def test1(datapath):
    TestSection("Make sure that we get correct values")

    # Set up the plots.
    OpenDatabase(datapath + "dataset2/CHR.case")
    AddPlot("Pseudocolor", "nb")
    DrawPlots()

    # Query the values at all time steps to make sure that we get the
    # right cycles, times and min dataset value.
    s = "Time values:\n"
    v = "Dataset values\n"
    for i in range(TimeSliderGetNStates()):
        SetTimeSliderState(i)
        DrawPlots()

        # Get cycle and time.
        Query("Time")
        t = GetQueryOutputValue()

        # Get min value
        Query("Min")
        m = GetQueryOutputValue()

        line = "Time=%g\n" % t
        s = s + line
        line = "Min Value=%g\n" % m
        v = v + line

    TestText("EnSight_1_00", s)
    TestText("EnSight_1_01", v)
    DeleteAllPlots()

def main():
    a = GetAnnotationAttributes()
    a.databaseInfoFlag = 0
    a. userInfoFlag = 0
    a.axesFlag = 0
    a.bboxFlag = 0
    a.legendInfoFlag = 0
    a.triadFlag = 0
    SetAnnotationAttributes(a)

    datapath = "../data/EnSight_test_data/"
    test0(datapath)
    test1(datapath)

main()
Exit()
