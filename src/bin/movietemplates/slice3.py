###############################################################################
# Function: Sequence1Frames_advance_time_and_slice
#
# Purpose:
#   This is a callback function for sequence 1's IterateCallbackAndSaveFrames
#   function. This function sets the time and updates the time slider so
#   it has the right time value. This function also updates a slice plane.
#
# Programmer: Brad Whitlock
# Creation:   Thu Nov 16 11:46:31 PDT 2006
#
# Modifications:
#
###############################################################################

def Sequence1Frames_advance_time_and_slice(i, cbdata):
    obj = cbdata[0]
    slicePercent = cbdata[1]
    sliceAxis = cbdata[2]
    timeState = cbdata[3]
    rotatedView = cbdata[4]   

    # Set the time state
    ret = obj.SetTimeState(timeState[i])

    # Do something with the slice.
    ClearWindow()
    RemoveAllOperators()
    if sliceAxis[i] == 0:
        # Slice along x.
        AddOperator("Slice")
        s = SliceAttributes()
        s.originType = s.Percent
        s.axisType = s.XAxis
        s.originPercent = slicePercent[i]
        s.project2d = 0
        SetOperatorOptions(s)
    elif sliceAxis[i] == 1:
        # Slice along y.
        AddOperator("Slice")
        s = SliceAttributes()
        s.originType = s.Percent
        s.axisType = s.YAxis
        s.originPercent = slicePercent[i]
        s.project2d = 0
        SetOperatorOptions(s)
    elif sliceAxis[i] == 2:
        # Slice along z.
        AddOperator("Slice")
        s = SliceAttributes()
        s.originType = s.Percent
        s.axisType = s.ZAxis
        s.originPercent = slicePercent[i]
        s.project2d = 0
        SetOperatorOptions(s)
    DrawPlots()
    SetView3D(rotatedView)

    return ret

###############################################################################
# Class: Slice3MovieTemplate
#
# Purpose:
#   This is movie template class creates a movie of a FilledBoundary plot
#   and a Curve plot that animates over time.
#
# Programmer: Brad Whitlock
# Creation:   Thu Nov 16 11:46:31 PDT 2006
#
# Modifications:
#
###############################################################################

class Slice3MovieTemplate(VisItMovieTemplate):
    def __init__(self, mm, tr):
        VisItMovieTemplate.__init__(self, mm, tr)
        self.ts = 0

    def __del__(self):
        VisItMovieTemplate.__del__(self)

    ###########################################################################
    # Function: SetTimeState
    #
    # Purpose:
    #   This method provides the sequence 1 callback function safe access
    #   to the object's time slider annotation.
    #
    # Programmer: Brad Whitlock
    # Creation:   Thu Nov 16 11:46:31 PDT 2006
    #
    # Modifications:
    #
    ###########################################################################

    def SetTimeState(self, timeState):
        ret = SetTimeSliderState(timeState)
        Query("Time")
        time = GetQueryOutputValue()
        print "time=", time
        self.ts.text = "Time = %1.5f" % time
        return ret

    ###########################################################################
    # Function: Sequence1Frames
    #
    # Purpose:
    #   This method creates the frames for sequence 1.
    #
    # Programmer: Brad Whitlock
    # Creation:   Thu Nov 16 11:46:31 PDT 2006
    #
    # Modifications:
    #
    ###########################################################################

    def Sequence1Frames(self, formats, percents):
        self.Debug(1, "Slice3MovieTemplate.Sequence1Frames: begin")
        options = self.sequence_data["SEQUENCE_1"]

        # Set up the plots.
        DeleteAllPlots()
        OpenDatabase(options["DATABASE"])
        if AddPlot("Pseudocolor", options["PLOT_VAR"]) == 0:
            raise self.error("The Pseudocolor plot could not be created for "
                             "sequence 1.")
        DrawPlots()

        # Set the background color.
        annot = GetAnnotationAttributes()
        annot.foregroundColor = (255, 255, 255, 255)
        annot.gradientColor1 = options["GRADIENT_BGCOLOR1"]
        annot.gradientColor2 = options["GRADIENT_BGCOLOR2"]
        annot.gradientBackgroundStyle = annot.TopToBottom
        annot.backgroundMode = annot.Gradient
        # Turn off certain annotations.
        annot.userInfoFlag = 0
        annot.databaseInfoFlag = 0
        annot.legendInfoFlag = 0
        # Set the axis names
        annot.xAxisUserTitle = options["XAXIS_TEXT"]
        annot.yAxisUserTitle = options["YAXIS_TEXT"]
        annot.zAxisUserTitle = options["ZAXIS_TEXT"]
        annot.xAxisUserTitleFlag = 1
        annot.yAxisUserTitleFlag = 1
        annot.zAxisUserTitleFlag = 1
        # Turn on axes
        annot.bboxFlag = 1
        annot.triadFlag = 1
        annot.axesType = annot.ClosestTriad
        annot.axesFlag = 1
        SetAnnotationAttributes(annot)

        # Rotate the view a little.
        ResetView()
        view = GetView3D()
        rv1 = self.RotateAxis(1, 30., 1, view)
        rotatedView = self.RotateAxis(0, 20., 1, rv1)
        SetView3D(rotatedView)

        self.ts = CreateAnnotationObject("TimeSlider")

        classification = CreateAnnotationObject("Text2D")
        classification.text = options["CLASSIFICATION_TEXT"]
        classification.useForegroundForTextColor = 0
        classification.textColor = options["CLASSIFICATION_TEXTCOLOR"]
        classification.position = (0.83, 0.97)
        classification.width = 0.15
        classification.fontBold = 1

        title = CreateAnnotationObject("Text2D")
        title.text = options["TITLE"]
        title.position = (0.01, 0.97)
        title.width = 0.01 * len(title.text) # for now...
        title.fontBold = 1

        # Determine the time states at which we'll begin slice sequences.
        tsToStartSlices = []
        if options["NSLICE_SEQUENCES"] == 1:
            tsToStartSlices = [0]
        elif options["NSLICE_SEQUENCES"] == 2:
            tsToStartSlices = [0, TimeSliderGetNStates()-1]
        else:
            for i in range(options["NSLICE_SEQUENCES"]):
                t = float(i) / float(options["NSLICE_SEQUENCES"] - 1)
                index = int(t * (TimeSliderGetNStates()-1))
                tsToStartSlices = tsToStartSlices + [index]

        # Construct the lists of values that we'll use for advancing
        # through time and moving the slice planes.
        slicePercent = [0.,0.,0.,0.,0.]
        sliceAxis = [-1,-1,-1,-1,-1]
        timeState = [0,0,0,0,0]
        for ts in range(TimeSliderGetNStates()):
            if len(tsToStartSlices) > 0 and tsToStartSlices[0] == ts:
                # Populate the lists with slice instructions.
                for axis in (0,1,2):
                    for step in range(options["NFRAMES_PER_SLICE"]):
                        t = float(step) / float(options["NFRAMES_PER_SLICE"]-1)
                        slicePercent = slicePercent + [t * 100.]
                        sliceAxis = sliceAxis + [axis]
                        timeState = timeState + [ts]
                # Remove the first element.
                tsToStartSlices = tsToStartSlices[1:]
            else:
                # Just advance time. No slicing.
                slicePercent = slicePercent + [0.]
                sliceAxis = sliceAxis + [-1]
                timeState = timeState + [ts]
        slicePercent = slicePercent + [0.,0.,0.,0.,0.]
        sliceAxis = sliceAxis + [-1,-1,-1,-1,-1]
        timeState = timeState + [0,0,0,0,0]

        # Save the frames.
        real_cb_data = (self, slicePercent, sliceAxis, timeState, rotatedView)
        cb_data = (len(timeState) , Sequence1Frames_advance_time_and_slice, real_cb_data)
        ret = self.IterateCallbackAndSaveFrames(cb_data, "seq1", formats, percents, "Generating sequence 1 frames")

        DeleteAllPlots()
        self.ts.Delete()
        self.ts = 0
        classification.Delete()
        title.Delete()

        self.Debug(1, "Slice3MovieTemplate.Sequence1Frames: end")
        return (ret, "seq1", GetAnnotationAttributes().backgroundColor)

    ###########################################################################
    # Function: HandleScriptingSequence
    #
    # Purpose:
    #   This method invokes the appropriate routine for creating sequence
    #   frames.
    #
    # Programmer: Brad Whitlock
    # Creation:   Thu Nov 16 11:46:31 PDT 2006
    #
    # Modifications:
    #
    ###########################################################################

    def HandleScriptingSequence(self, seqName, formats, percents):
        ret = 0
        if seqName == "SEQUENCE_1":
            ret = self.Sequence1Frames(formats, percents)

        return ret

# Public
def InstantiateMovieTemplate(moviemaker, templateReader):
    return Slice3MovieTemplate(moviemaker, templateReader)
