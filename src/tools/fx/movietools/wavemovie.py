import imageops, moviegenerator, movietools

###############################################################################
# Class: WaveMovieGenerator
#
# Purpose:    This is a derived class of MovieGenerator that creates a movie
#             of our wave dataset and tries to save the movie out in various
#             movie formats of varying resolutions. 
#
# Programmer: Brad Whitlock
# Date:       Tue Jan 14 16:28:16 PST 2003
#
# Modifications:
#   Brad Whitlock, Wed Jan 29 07:59:50 PDT 2003
#   I updated the argument list for SetupVisualization.
#
#   Kathleen Bonnell, Wed Nov 12 16:59:24 PST 2003 
#   I updated the attributes used to set 3d View.
#
##############################################################################

class WaveMovieGenerator(moviegenerator.MovieGenerator):
    def __init__(self):
        # Call the superclass's constructor.
        moviegenerator.MovieGenerator.__init__(self, imageops.ImageOps())

        #
        # Override some of the superclass's properties. Look at
        # the superclass's constructor to see the properties that can be
        # set.
        #
        self.movieBase = "wave"
        self.timestepFile = "/usr/gapps/visit/data/wave.visit"
        self.visitDir = "/data_vobs/VisIt"
        self.engineArgs = ("-debug", "2")
        self.nstep = 1
        # Initialize the views
        self.InitViews()

    #
    # This method overrides the superclass's ReadTimeSteps method. I'm doing this
    # because I want to use a public .visit file but it doesn't have the path
    # to the files in it so I'm prepending them here.
    #
    def ReadTimeSteps(self):
        # Read the name of each timestep in the timestep file.
        lines = movietools.ReadDataFile(self.timestepFile)
        # Stride through the timesteps to create a list of timesteps.
        L = []
        for i in range(0, len(lines), self.nstep):
            line = lines[i]
            L = L + ["%s:/usr/gapps/visit/data/%s" % (self.dataHost, line)]
        return L

    #
    # This method initializes the views that this movie will use.
    #
    def InitViews(self):
        v0 = View3DAttributes()
        v0.viewNormal = (-0.497104, 0.575734, 0.649168)
        v0.focus = (5, 0.353448, 2.5)
        v0.viewUp = (0.382047, 0.81696, -0.431991)
        v0.viewAngle = 30
        v0.parallelScale = 6.23027
        v0.nearPlane = -11.2018
        v0.farPlane = 11.2018
        v0.perspective = 1

        v1 = View3DAttributes()
        v1.viewNormal = (0.384151, 0.536474, 0.751414)
        v1.focus = (5, 0.353448, 2.5)
        v1.viewUp = (0.028325, 0.80663, -0.590377)
        v1.viewAngle = 30
        v1.parallelScale = 6.23027
        v1.nearPlane = -11.2018
        v1.farPlane = 11.2018
        v1.perspective = 1

        v2 = View3DAttributes()
        v2.viewNormal = (0.784207, 0.514617, 0.346682)
        v2.focus = (5.47361, -0.0795047, 2.07135)
        v2.viewUp = (-0.356176, 0.830834, -0.427613)
        v2.viewAngle = 30
        v2.parallelScale = 4.98178
        v2.nearPlane = -11.2018
        v2.farPlane = 11.2018
        v2.perspective = 1

        v3 = View3DAttributes()
        v3.viewNormal = (0.919312, 0.388049, 0.0654506)
        v3.focus = (5.47361, -0.0795047, 2.07135)
        v3.viewUp = (-0.381256, 0.919449, -0.0962209)
        v3.viewAngle = 30
        v3.parallelScale = 3.58871
        v3.nearPlane = -11.2018
        v3.farPlane = 11.2018
        v3.perspective = 1

        v4 = View3DAttributes()
        v4.viewNormal = (0.75288, 0.461699, -0.469047)
        v4.focus = (5.47361, -0.0795047, 2.07135)
        v4.viewUp = (-0.284761, 0.87102, 0.400295)
        v4.viewAngle = 30
        v4.parallelScale = 3.58871
        v4.nearPlane = -11.2018
        v4.farPlane = 11.2018
        v4.perspective = 1

        v5 = View3DAttributes()
        v5.viewNormal = (0.491718, 0.545418, -0.678773)
        v5.focus = (5.44464, 0.329047, 2.37865)
        v5.viewUp = (-0.322383, 0.83816, 0.439951)
        v5.viewAngle = 30
        v5.parallelScale = 5.6009
        v5.nearPlane = -11.2018
        v5.farPlane = 11.2018
        v5.perspective = 1

        # Save the view control points and weights as part of this object.
        self.cpts = (v0, v1, v2, v3, v4, v5)
        self.x=[]
        for j in range(len(self.cpts)):
            self.x = self.x + [float(j) / float(len(self.cpts) - 1)]

    #
    # This method returns the view for the time t through the movie.
    #
    def GetViewForTime(self, t):
        v = EvalCubicSpline(t, self.x, self.cpts)
        v.nearPlane = 1.5 * v.nearPlane
        v.farPlane = 1.5 * v.farPlane
        return v

    #
    # This method overrides the superclass's SetupVisualization method.
    # This is where VisIt scripting to set up the plots goes. You can choose
    # to do some static plots for the given database (dbName - which is opened
    # by the time this method is called) or you can provide dynamic behavior
    # for the movie by changing the visualization's behavior based on the
    # t value.
    #
    def SetupVisualization(self, dbName, t, index, times):
        print "Opening %s" % dbName

        # Add a mesh plot.
        AddPlot("Mesh", "quadmesh")
        if(t < 0.5):
            AddPlot("Pseudocolor", "pressure")
            pc = PseudocolorAttributes()
            pc.opacity = 1. - 2.* t
            SetPlotOptions(pc)

            # Set the view
            SetView3D(self.GetViewForTime(t))

            # Draw the plots
            self.ProtectedDrawPlots()

            frameName = self.SaveSourceFrame()
            # Add the filename to the top of every frame in red text.
            self.ops.loadFile(frameName)
            self.ops.drawText(0.02, 0.96, dbName, 0.03, (255,0,0))
            self.ops.saveFile()
        elif(self.GetFlag("contourFaded") == 0):
            # Set the flag to 1 indicating that we've been here.
            self.SetFlag("contourFaded", 1)

            # Draw the plots
            self.ProtectedDrawPlots()
            # Set the view
            SetView3D(self.GetViewForTime(t))

            self.SaveImage("meshonly.tif")
            self.ops.loadFile("meshonly.tif")
            self.ops.drawText(0.02, 0.96, dbName, 0.03, (255,0,0))
            self.ops.saveFileAs("meshonly2.tif")

            AddPlot("Contour", "pressure")
            self.ProtectedDrawPlots()
            self.SaveImage("meshandcontour.tif")
            self.ops.loadFile("meshandcontour.tif")
            self.ops.drawText(0.02, 0.96, dbName, 0.03, (255,0,0))
            self.ops.saveFileAs("meshandcontour2.tif")

            # Fade in the contour plot.
            self.ops.interpolateFrames("meshonly2.tif", "meshandcontour2.tif", 15)
        else:
            AddPlot("Contour", "pressure")
            # Set the view
            SetView3D(self.GetViewForTime(t))

            # Draw the plots
            self.ProtectedDrawPlots()

            frameName = self.SaveSourceFrame()
            # Add the filename to the top of every frame in red text.
            self.ops.loadFile(frameName)
            self.ops.drawText(0.02, 0.96, dbName, 0.03, (255,0,0))
            self.ops.saveFile()
#
# This is a simple main function that shows how to create the movie generator
# object, request movies for it to generate, and create the movie.
#
def main():
    generator = WaveMovieGenerator()

    # Request the movie formats and resolutions that we want.
    generator.requestQuickTimeMovie(generator.xres, generator.yres, 1)
    generator.requestQuickTimeMovie(500, 500, 0)
    generator.requestMPEGMovie(500, 500, 0)
    generator.requestStreamingMovie(500, 500, 1)

    # Create the movie.
    generator.CreateMovie()
 
main()
