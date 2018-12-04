# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  Chombo.py
#
#  Tests:      mesh      - 2D AMR
#              plots     - Pseudocolor
#
#  Programmer: Brad Whitlock
#  Date:       Mon Sep 25 14:17:58 PST 2006
#
#  Modifications:
#    Mark C. Miller, Wed Jan 21 10:00:10 PST 2009
#    Removed silly comments regarding global annotation object 'a'
#
# ----------------------------------------------------------------------------

def test0(datapath):
    TestSection("Changing time states")

    # Set up the plots.
    db = pjoin(datapath,"chombo.visit")
    OpenDatabase(db)
    AddPlot("Pseudocolor", "Density")
    AddOperator("Transform")
    t = TransformAttributes()
    t.doScale = 1
    t.scaleY = 8.
    SetOperatorOptions(t)
    DrawPlots()

    Test("Chombo_0_00")

    # Make sure we can change time states. This demonstrates the reader's
    # ability to change metadata/SIL over time.
    SetTimeSliderState(1)
    Test("Chombo_0_01")

    # Get the time steps.
    s = "Number of states: %d\n" %  TimeSliderGetNStates()
    for i in range(TimeSliderGetNStates()):
        SetTimeSliderState(i)
        Query("Cycle")
        cycle = GetQueryOutputValue()
        Query("Time")
        time = GetQueryOutputValue()
        values = "Cycle=%d, Time=%g" % (cycle, time)
        s = s + values + "\n"
    TestText("Chombo_0_02", s)

    DeleteAllPlots()
    CloseDatabase(db)

def main():
    TurnOffAllAnnotations()

    InvertBackgroundColor()

    datapath = data_path("Chombo_test_data")
    test0(datapath)

    InvertBackgroundColor()

main()
Exit()
