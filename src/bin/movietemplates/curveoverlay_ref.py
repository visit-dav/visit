###############################################################################
# Function: Sequence1Frames_set_timeslider
#
# Purpose:
#   This is a callback function for sequence 1's IterateCallbackAndSaveFrames
#   function. This function sets the time and updates the time slider so
#   it has the right time value.
#
# Programmer: Brad Whitlock
# Creation:   Thu Nov 16 11:46:31 PDT 2006
#
# Modifications:
#
###############################################################################

def Sequence1Frames_set_timeslider(i, cbdata):
    ts = cbdata
    ret = SetTimeSliderState(i)
    Query("Time")
    time = GetQueryOutputValue()
    ts.text = "Time = %1.5f" % time
    return ret

###############################################################################
# Function: Sequence1Frames_set_timeslider
#
# Purpose:
#   This is a callback function for sequence 2's IterateCallbackAndSaveFrames
#   function. This function lets us adjust the clip plane as a function of 
#   the number of time states and save out an image each time.
#
# Programmer: Brad Whitlock
# Creation:   Thu Nov 16 11:46:31 PDT 2006
#
# Modifications:
#
###############################################################################

def Sequence2Frames_clip_cb(i, cbdata):
    nts = cbdata[0]
    clip = cbdata[1]
    xmin = cbdata[2]
    xmax = cbdata[3]
    vc = cbdata[4]
    t = float(i) / float(nts-1)
    newX = t * (xmax - xmin) + xmin
    clip.plane1Origin = (newX, 0, 0)
    ret = SetOperatorOptions(clip)
    SetViewCurve(vc)
    return ret

###############################################################################
# Class: OverlayCurveOnReflectedPlotsMovieTemplate
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

class OverlayCurveOnReflectedPlotsMovieTemplate(VisItMovieTemplate):
    def __init__(self, mm, tr):
        VisItMovieTemplate.__init__(self, mm, tr)
        self.timeSlider = ""

    def __del__(self):
        VisItMovieTemplate.__del__(self)

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
        self.Debug(1, "OverlayCurveOnReflectedPlotsMovieTemplate.Sequence1Frames: begin")
        options = self.sequence_data["SEQUENCE_1"]

        # Set up the plots.
        DeleteAllPlots()
        OpenDatabase(options["DATABASE1"])
        if options["PLOT_TYPE1"] == 0:
            if AddPlot("FilledBoundary", options["PLOT_VAR1"]) == 0:
                raise self.error("The FilledBoundary plot could not be created for "
                            "sequence 1.")
        elif options["PLOT_TYPE1"] == 1:
            if AddPlot("Boundary", options["PLOT_VAR1"]) == 0:
                raise self.error("The Boundary plot could not be created for "
                            "sequence 1.")
        else:
            if AddPlot("Pseudocolor", options["PLOT_VAR1"]) == 0:
                raise self.error("The Pseudocolor plot could not be created for "
                            "sequence 1.")
        # Create plot 2
        OpenDatabase(options["DATABASE2"])
        if options["PLOT_TYPE2"] == 0:
            if AddPlot("FilledBoundary", options["PLOT_VAR2"]) == 0:
                raise self.error("The FilledBoundary plot could not be created for "
                            "sequence 1.")
        elif options["PLOT_TYPE2"] == 1:
            if AddPlot("Boundary", options["PLOT_VAR2"]) == 0:
                raise self.error("The Boundary plot could not be created for "
                            "sequence 1.")
        else:
            if AddPlot("Pseudocolor", options["PLOT_VAR2"]) == 0:
                raise self.error("The Pseudocolor plot could not be created for "
                            "sequence 1.")
        AddOperator("Reflect", 0)
        refl = ReflectAttributes()
        refl.reflections = (0,0,1,0,0,0,0,0)
        SetOperatorOptions(refl)
        DrawPlots()

        # If the databases are not the same then create a database correlation
        # so we can get a new time slider to use. In any case, keep track
        # of the time slider that we'll be using.
        self.timeSlider = options["DATABASE1"]
        if options["DATABASE1"] != options["DATABASE2"]:
            dbs = (options["DATABASE1"], options["DATABASE2"])
            if CreateDatabaseCorrelation("DB1DB2", dbs, 1) == 1:
                self.timeSlider = "DB1DB2"
                SetActiveTimeSlider(self.timeSlider)

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
        annot.xAxisUserTitle2D = options["XAXIS_TEXT"]
        annot.yAxisUserTitle2D = options["YAXIS_TEXT"]
        annot.xAxisUserTitleFlag2D = 1
        annot.yAxisUserTitleFlag2D = 1

        SetAnnotationAttributes(annot)

        # Change the viewport
        v = GetView2D()
        v.viewportCoords = (0.1, 0.95, 0.35, 0.95)
        SetView2D(v)

        ts = CreateAnnotationObject("TimeSlider")

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

        # Save the frames.
        cb_data = (TimeSliderGetNStates(), Sequence1Frames_set_timeslider, ts)
        ret = self.IterateCallbackAndSaveFrames(cb_data, "seq1", formats, percents, "Generating sequence 1 frames")

        DeleteAllPlots()
        ts.Delete()
        classification.Delete()
        title.Delete()

        self.Debug(1, "OverlayCurveOnReflectedPlotsMovieTemplate.Sequence1Frames: end")
        return (ret, "seq1", GetAnnotationAttributes().backgroundColor)

    ###########################################################################
    # Function: Sequence2Frames
    #
    # Purpose:
    #   This method creates the frames for sequence 2.
    #
    # Programmer: Brad Whitlock
    # Creation:   Thu Nov 16 11:46:31 PDT 2006
    #
    # Modifications:
    #
    ###########################################################################

    def Sequence2Frames(self, formats, percents):
        self.Debug(1, "OverlayCurveOnReflectedPlotsMovieTemplate.Sequence2Frames: begin")
        options = self.sequence_data["SEQUENCE_2"]

        # Determine the number of time steps in the first sequence's time 
        # slider so we can know how to advance the Clip operator.
        dbc = GetDatabaseCorrelation(self.timeSlider)
        nts = dbc.numStates
        DeleteAllPlots()

        # Set up the Curve plot.
        OpenDatabase(options["CURVE_DATABASE"])
        AddPlot("Curve", options["CURVE_VARIABLE"])
        DrawPlots()
        ResetView()
        vc = GetViewCurve()
        vc.viewportCoords = (0.1, 0.95, 0.15, 1.)

        # Get the Curve plot extents
        Query("SpatialExtents")
        extents = GetQueryOutputValue()
        AddOperator("Clip")
        clip = ClipAttributes()
        clip.funcType = clip.Plane
        clip.plane1Status = 1
        clip.plane2Status = 0
        clip.plane3Status = 0
        clip.plane1Origin = (extents[0], 0, 0)
        clip.plane1Normal = (1, 0, 0)
        clip.planeInverse = 0
        SetOperatorOptions(clip)
       
        # Set the background color.
        annot = GetAnnotationAttributes()
        annot.backgroundMode = annot.Solid
        annot.foregroundColor = (255, 255, 255, 255)
        annot.backgroundColor = (0, 0, 0, 255)
        # Turn off most annotations.
        annot.userInfoFlag = 0
        annot.databaseInfoFlag = 0
        annot.legendInfoFlag = 0
        annot.xAxisLabels2D = 0
        annot.yAxisLabels2D = 0
        annot.xAxisTitle2D = 0
        annot.yAxisTitle2D = 0
        SetAnnotationAttributes(annot)

        title = CreateAnnotationObject("Text2D")
        title.text = options["CURVE_TITLE"]
        title.position = (0.11, 0.90)
        title.width = 0.01 * len(title.text) # for now...
        title.fontBold = 1

        # Save the frames. This will be done by some other thing so the 
        # will have the viewport names worked in.
        cb_data = (nts, Sequence2Frames_clip_cb, (nts, clip, extents[0], extents[1], vc))
        ret = self.IterateCallbackAndSaveFrames(cb_data, "seq2", formats, percents, "Generating sequence 2 frames")

        title.Delete()
        DeleteAllPlots()

        self.Debug(1, "OverlayCurveOnReflectedPlotsMovieTemplate.Sequence2Frames: end")
        return (ret, "seq2", GetAnnotationAttributes().backgroundColor)

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
        elif seqName == "SEQUENCE_2":
            ret = self.Sequence2Frames(formats, percents)
        return ret

# Public
def InstantiateMovieTemplate(moviemaker, templateReader):
    return OverlayCurveOnReflectedPlotsMovieTemplate(moviemaker, templateReader)
