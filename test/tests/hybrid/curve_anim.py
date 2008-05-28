# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  curve_anim.py
#
#  Tests:      mesh      - polygonal line from .curve file.
#              plots     - curve
#              operators - none
#
#  Defect ID:  '2973
#
#  Programmer: Hank Childs
#  Date:       January 15, 2003
#
#  Modifications:
#
#    Hank Childs, Mon Aug  4 09:51:40 PDT 2003
#    Curves are no longer valid subsets, but they are now curve plots.
#
#    Brad Whitlock, Wed Mar 9 09:15:30 PDT 2005
#    Removed deprecated functions.
#
# ----------------------------------------------------------------------------


OpenDatabase("../data/curve.visit")
curv_atts = CurveAttributes()
AddPlot("Curve", "flat") 
curv_atts.color = (0, 255, 0, 255)
SetPlotOptions(curv_atts)
AddPlot("Curve", "going_up")
curv_atts.color = (255, 0, 0, 255)
SetPlotOptions(curv_atts)
AddPlot("Curve", "going_down")
curv_atts.color = (0, 0, 255, 255)
SetPlotOptions(curv_atts)
AddPlot("Curve", "parabolic")
curv_atts.color = (0, 255, 255, 255)
SetPlotOptions(curv_atts)

SetTimeSliderState(40)
DrawPlots()
# This additional set frame is so that the saved
# image is correct.
SetTimeSliderState(40)
Test("curve_anim1")

SetTimeSliderState(99)
Test("curve_anim2")

SetActivePlots(3)
DeleteActivePlots()

Test("curve_anim3")

SetTimeSliderState(40)
Test("curve_anim4")

SetActivePlots(0)
DeleteActivePlots()
Test("curve_anim5")

SetTimeSliderState(5)
AddPlot("Curve", "flat") 
curv_atts.color = (0, 255, 0, 255)
SetPlotOptions(curv_atts)
DrawPlots()
Test("curve_anim6")

Exit()
