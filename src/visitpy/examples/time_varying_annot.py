###############################################################################
# Function: UpdateMaxAnnotation
#
# Purpose:  Example of a utility method to be called from various callbacks to
#           update a specifically named text annotation object, 'maxAnnot' with
#           the result of a Query("Max")
#
# Programmer: Mark C. Miller, Tue Jan  6 16:07:05 PST 2009
#
###############################################################################
def UpdateMaxAnnotation(arg):
    global lastState
    if arg.timeSliderCurrentStates[0] != lastState:
        try:
            annot_obj = GetAnnotationObject("maxAnnot")
            Query("Max")
            val = GetQueryOutputValue()
            annot_obj.text = "%f"%val
            lastState = arg.timeSliderCurrentStates[0]
        except:
            return

def onSetTimeSliderState0():
    UpdateMaxAnnotation(GetWindowInformation())

def onSetTimeSliderState1(timeState):
    UpdateMaxAnnotation(GetWindowInformation())

def onWindowInformation(arg):
    UpdateMaxAnnotation(arg)

###############################################################################
# Purpose:  Register functions with various callbacks so that the annoation
#           will update as time state is changed. We use WindowInformation
#           callback slot to deal with the VCR 'play' button as that is one of
#           the few state objects that gets updated between intervening state
#           changes during VCR 'play' mode. However, unfortunately, VisIt can
#           wind up queueing up all the requests for callbacks until after
#           VCR 'play' mode is stopped. So, this code won't necessarily result
#           in the annotation being updated correctly during VCR 'play' mode.
#
# Programmer: Mark C. Miller, Tue Jan  6 16:07:05 PST 2009
#
###############################################################################
lastState = -1
RegisterCallback("SetTimeSliderStateRPC", onSetTimeSliderState1)
RegisterCallback("TimeSliderNextStateRPC", onSetTimeSliderState0)
RegisterCallback("TimeSliderPreviousStateRPC", onSetTimeSliderState0)
RegisterCallback("WindowInformation", onWindowInformation)
