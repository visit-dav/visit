import movietools, os, sys, visit

###############################################################################
# Class: MovieGenerator
#
# Purpose:    This base class contains the machinery to create a movie and
#             it provides restart capability so that if the script stops
#             running, it can be restarted and it will pick up where it left
#             off.
#
# Programmer: Brad Whitlock
# Date:       Tue Jan 14 16:28:16 PST 2003
#
##############################################################################

class MovieGenerator:
    def __init__(self, ops):
        # Movie properties
        self.movieBase = "movie"
        self.basename = "source"
        self.xres = 1024
        self.yres = 1024
        self.processedBaseName = "frame"
        self.processedBaseFormat = "%s-%dx%d-%%04d.png" % (self.processedBaseName, self.xres, self.yres)
        self.fileIndex = 0
        self.movieFormats = []
        # Script operation properties
        self.stage = "GenerateFrames"
        self.ops = ops
        self.flags = {"invalid":0}
        # Database properties
        self.dataHost = "localhost"
        self.timestepFile = "movie.visit"
        self.nstep = 1
        # Engine properties
        self.engineNP = 1
        self.launchMethod = "psub"
        self.poolName = "pbatch"
        self.visitDir = "/usr/gapps/visit"
        self.timeLimit = 60
        self.engineArgs = []
        self.preventativeClose = 0
        # MDServer properties
        self.mdserverArgs = []
        # Set up some callbacks to visit module functions.
        self.CloseComputeEngine      = visit.CloseComputeEngine
        self.DeleteAllPlots          = visit.DeleteAllPlots
        self.DrawPlots               = visit.DrawPlots
        self.OpenComputeEngine       = visit.OpenComputeEngine
        self.OpenDatabase            = visit.OpenDatabase
        self.OpenMDServer            = visit.OpenMDServer
        self.SaveWindow              = visit.SaveWindow
        self.SaveWindowAttributes    = visit.SaveWindowAttributes
        self.SetSaveWindowAttributes = visit.SetSaveWindowAttributes

        # If the -clear command line argument was given then remove the
        # restart files.
        if("-clear" in sys.argv):
            self.RemoveRestartFiles()

        # Look for the -geometry argument in the cli's arguments and pick
        # out the image resolution if it was given.
        for i in range(1, len(sys.argv)):
            if(sys.argv[i] == "-geometry" and ((i+1) < len(sys.argv))):
                geometry = sys.argv[i+1]
                xloc = geometry.find("x")
                if(xloc != -1):
                    self.xres = int(geometry[:xloc])
                    self.yres = int(geometry[xloc+1:])
                    self.processedBaseFormat = "%s-%dx%d-%%04d.png" % (self.processedBaseName, self.xres, self.yres)
                    break

    ##########################################################################
    # Method: requestStreamingMovie
    #
    # Purpose:    This method tells the movie generator to create a streaming
    #             movie of the specified size.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################

    def requestStreamingMovie(self, xres, yres, tar):
        self.movieFormats = self.movieFormats + [("sm", xres, yres, tar)]

    ##########################################################################
    # Method: requestQuickTimeMovie
    #
    # Purpose:    This method tells the movie generator to create a
    #             QuickTime movie of the specified size.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################

    def requestQuickTimeMovie(self, xres, yres, tar):
        self.movieFormats = self.movieFormats + [("qt", xres, yres, tar)]

    ##########################################################################
    # Method: requestMPEGMovie
    #
    # Purpose:    This method tells the movie generator to create an MPEG
    #             movie of the specified size.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################

    def requestMPEGMovie(self, xres, yres, tar):
        self.movieFormats = self.movieFormats + [("mpeg", xres, yres, tar)]

    ##########################################################################
    # Method: CreateMovie
    #
    # Purpose:    This the main method for creating the movie.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    # Modifications:
    #   Brad Whitlock, Wed Jan 29 07:53:48 PDT 2003
    #   I fixed a bug that prevented the stage from the restart file from
    #   being used.
    #
    ##########################################################################

    def CreateMovie(self):
        # Read the list of files to use.
        times = self.ReadTimeSteps()

        # Read the restart file and figure out where we should pick up.
        restart = self.ProcessRestartFile(times)
        self.stage = restart[0]
        t = restart[1]
        startFile = restart[2]

        # If the restart indicated that we're at the GenerateFrames stage,
        # then generate the remaining movie frames.
        if(self.stage == "GenerateFrames"):
            self.GenerateFrames(times, t, startFile)
            self.stage = "ImageProcessing"

        # If the restart indicated that we're at the ImageProcessing stage,
        # then process the images into new frames.
        nframes = 0
        nframesUnknown = 1
        if(self.stage == "ImageProcessing"):
            self.WriteScriptRestart(self.stage, "done", self.fileIndex - 1)
            nframes = self.ops.process(self.processedBaseFormat, 0)
            self.stage = "Packaging"
            self.WriteScriptRestart(self.stage, "done", nframes)
            nframesUnknown = 0

        # If the restart indicated that we're at the Packaging stage,
        # then package the frames into movies.
        if(self.stage == "Packaging"):
            # If we don't know the number of frames, read the restart file
            # again to get that information
            if(nframesUnknown):
                restart = self.ProcessRestartFile(times)
                nframes = restart[2]
            self.PackageMovies(nframes)
            self.stage = "Done"
            self.WriteScriptRestart(self.stage, "done", nframes)

        if(self.stage == "Done"):
            print "The movies are done."

    ##########################################################################
    # Method: SetupVisualization
    #
    # Purpose:    This method is called to perform visualization at t==0. If
    #             any special coding is desired, override in subclasses.
    #
    # Notes: You have to call SaveSource frame for this method to save images.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    # Modifications:
    #   Brad Whitlock, Wed Jan 29 07:35:29 PDT 2003
    #   I added the index and times arguments. index is the index of the
    #   current dbName within the times database name tuple.
    #
    ##########################################################################

    def SetupVisualization(self, dbName, t, index, times): pass

    ##########################################################################
    # Method: ProcessRestartFile
    #
    # Purpose:    This method reads the restart file and interprets the stored
    #             values so that we know the stage that we're at in the movie
    #             generation pipeline and the file that we should be working
    #             on.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    # Modifications:
    #   Brad Whitlock, Tue Jan 28 14:40:34 PST 2003
    #   I fixed an error reading flags back from the restart file. I also
    #   prevented the t value from being anything but one if the stage is
    #   not "GenerateFrames".
    #
    ##########################################################################

    def ProcessRestartFile(self, times):
        # Initialize some local variables.
        t = 0.
        stage = "GenerateFrames"
        startFile = 0
        startImageIndex = 0
      
        # Try reading the script's restart file.
        restart = self.ReadScriptRestart()

        # If we have restart information, use it to determine the last file
        # that was processed successfully and compute the initial "t" value
        # as well as the index that should be used to save the next image file.
        if(len(restart) >= 3):
            # Get the raw values from the restart file.
            stage = restart[0]
            lastFile = restart[1]
            lastImageIndex = restart[2]

            # Determine the initial time that we're trying to visualize.
            findex = 0
            for i in range(0, len(times)):
                if(times[i] == lastFile):
                    findex = i
                    break;
            startFile = findex + 1
            if(stage == "GenerateFrames"):
                t = float(startFile) / float(len(times) - 1)
            else:
                t = 1.
            print "Initial t value=%g" % t

            # Determine the initial image frame that we should save.
            startImageIndex = int(lastImageIndex) + 1

            # Try and read the image ops so we don't overwrite them.
            self.ReadImageOps()

            # Set the internal flags from the information in the restart file.
            if(len(restart) > 3):
                for line in restart[3:]:
                    eloc = line.find("=")
                    if(eloc != -1):
                        key = line[:eloc]
                        val = int(line[eloc+1:])
                        self.flags[key] = val
                        print "Read flag \"%s\" from restart. Its value is %d" % (key,val)
        elif(len(restart) > 0):
            print "The restart file has an error. We're not going to use it."

        # Set the file index, which is used to create names for saved images,
        # from the value stored in the restart file.
        self.fileIndex = startImageIndex

        return (stage, t, startFile)

    ##########################################################################
    # Method: SetFlag
    #
    # Purpose:    This method sets the value of a flag that we use for
    #             program execution. The flags are saved to the restart file.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################

    def SetFlag(self, name, boolean):
        self.flags[name] = (boolean != 0)

    ##########################################################################
    # Method: GetFlag
    #
    # Purpose:    This method returns the value of the named flag.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################

    def GetFlag(self, name):
        retval = 0
        try:
            retval = self.flags[name]
        except KeyError:
            retval = 0
        return retval

    ##########################################################################
    # Method: GenerateFrames
    #
    # Purpose:    This method generates the frames for the movie. Its main
    #             job is to handle restart and iterating through the list of
    #             timesteps while calling other helper method to actually
    #             set up the plots.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    # Modifications:
    #   Brad Whitlock, Wed Jan 22 14:27:47 PST 2003
    #   I added code to launch an mdserver.
    #
    #   Brad Whitlock, Tue Jan 28 14:39:25 PST 2003
    #   I moved the code to delete the plots so it happens before we open
    #   the new database to avoid a bug in the viewer that causes the plots
    #   to be reexecuted needlessly on the engine. I also changed the
    #   interface of the SetupVisualization method so it accepts new args.
    #
    ##########################################################################

    def GenerateFrames(self, times, t, startFile):
        timeStepsPerEngine = 15
        timeStepsProcessed = 1

        # Launch an mdserver
        self.LaunchMDServer()

        # Launch a compute engine.
        self.LaunchComputeEngine()

        # Iterate through the timesteps.
        for i in range(startFile, len(times)):
            # t is the parametric time through the animation.
            t = float(i) / float(len(times) - 1)

            # Delete any plots that may be in the window.
            self.DeleteAllPlots()

            # Open the first timestep and delete all plots.
            if(self.OpenDatabase(times[i]) != 1):
                # This may have failed because the engine was dead. Try to
                # start another engine.
                self.LaunchComputeEngine()

            # Set up the visualization for time t.
            self.SetupVisualization(times[i], t, i, times)

            # Save the script's restart file.
            self.WriteScriptRestart(self.stage, times[i], self.fileIndex - 1)

            # Save the image ops
            self.WriteImageOps()

            # If we've processed a certain number of timesteps with this compute
            # engine then close it down and restart it so hopefully, it will be
            # less likely to crash and stall the script.
            timeStepsProcessed = timeStepsProcessed + 1
            if(self.preventativeClose and (timeStepsProcessed == timeStepsPerEngine)):
                print "**\n**\n**\n** We're shutting down the compute engine!**\n**\n**\n"
                self.CloseComputeEngine()
                print "**\n**\n**\n** We're restarting the compute engine!**\n**\n**\n"
                self.LaunchComputeEngine()
                timeStepsProcessed = 0
    
        # Close the compute engine
        self.CloseComputeEngine()

    ##########################################################################
    # Method: LaunchMDServer
    #
    # Purpose:    This method tells VisIt how to launch an mdserver.
    #
    # Programmer: Brad Whitlock
    # Date:       Wed Jan 22 14:27:10 PST 2003
    #
    ##########################################################################

    def LaunchMDServer(self):
        args = tuple(["-dir", self.visitDir] + list(self.mdserverArgs))
        self.OpenMDServer(self.dataHost, args)

    ##########################################################################
    # Method: LaunchComputeEngine
    #
    # Purpose:    This method tells VisIt how to launch a compute engine.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################

    def LaunchComputeEngine(self):
        retval = 0
        if(self.engineNP > 1):
            if(self.dataHost[:5] == "frost" or self.dataHost[:5] == "white"):
                nn = self.engineNP / 16
                nnstr = "%d" % nn
                npstr = "%d" % self.engineNP
                limitstr = "%d" % self.timeLimit
                args = ["-np", npstr, "-nn", nnstr, "-l", self.launchMethod, "-p", self.poolName, "-b", "bdivp", "-forcestatic", "-t", limitstr]
            elif(self.dataHost[:4] == "blue"):
                nn = self.engineNP / 4
                nnstr = "%d" % nn
                npstr = "%d" % self.engineNP
                limitstr = "%d" % self.timeLimit
                args = ["-np", npstr, "-nn", nnstr, "-l", self.launchMethod, "-p", self.poolName, "-b", "bdivp", "-forcestatic", "-t", limitstr]
            else:
                npstr = "%d" % self.engineNP
                args = ["-np", npstr, "-forcestatic"]

            # Add more arguments.
            args = args + list(self.engineArgs)
            if(self.visitDir != "/usr/gapps/visit"):
                args = args + ["-dir", self.visitDir]
            retval = self.OpenComputeEngine(self.dataHost, tuple(args))
        elif(len(self.engineArgs) > 0):
            retval = self.OpenComputeEngine(self.dataHost, tuple(self.engineArgs))
        else:
            retval = self.OpenComputeEngine(self.dataHost)
        return retval

    ##########################################################################
    # Method: ReadScriptRestart
    #
    # Purpose:    Reads the script's restart file and return the contents.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################

    def ReadScriptRestart(self):
        return movietools.ReadDataFile("%s-restart" % self.movieBase)

    ##########################################################################
    # Method: WriteScriptRestart
    #
    # Purpose:    This method writes the script's restart file which it uses
    #             to pick up where it left off if there were errors.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################
        
    def WriteScriptRestart(self, stage, lastDataFile, lastImageFile):
        try:
            # Write the restart file.
            f = open("%s-restart" % self.movieBase, "w")
            f.write("%s\n" % stage)
            f.write("%s\n" % lastDataFile)
            f.write("%d\n" % lastImageFile)
            # Write the flags so we can keep a little program state
            for key in self.flags.keys():
                if(key != "invalid"):
                    f.write("%s=%d\n" % (key, self.flags[key]))
            f.close()
        except IOError:
            print "Could not write the script restart file"

    ##########################################################################
    # Method: WriteImageOps
    #
    # Purpose:    This method writes the script's image ops to a file.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################

    def WriteImageOps(self):
        self.ops.backup("%s-imageops" % self.movieBase)

    ##########################################################################
    # Method: ReadImageOps
    #
    # Purpose:    This method reads the script's image ops from a file.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################

    def ReadImageOps(self):
        self.ops.restore("%s-imageops" % self.movieBase)

    ##########################################################################
    # Method: RemoveRestartFiles
    #
    # Purpose:    This method removes the script's restart files.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################

    def RemoveRestartFiles(self):
        print "Removing the restart file"
        restartName = "%s-restart" % self.movieBase
        imageOpsName = "%s-imageops" % self.movieBase
        command = "rm -f %s %s" % (restartName, imageOpsName)
        os.system(command)
       
    ##########################################################################
    # Method: ReadTimeSteps
    #
    # Purpose:    This method reads the timesteps from the .visit file and
    #             strides through them to produce the list of timesteps that
    #             we'll use to make the movie.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################

    def ReadTimeSteps(self):
        # Read the name of each timestep in the timestep file.
        lines = movietools.ReadDataFile(self.timestepFile)
        # Stride through the timesteps to create a list of timesteps.
        L = []
        for i in range(0, len(lines), self.nstep):
            line = lines[i]
            if(line[0] == '/'):
                # Add the hostname of where the data's stored.
                L = L + ["%s:%s" % (self.dataHost, line)]
            else:
                L = L + [line]
        return L

    ##########################################################################
    # Method: SaveImage
    #
    # Purpose:    This method saves the active window to a tiff image.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################

    def SaveImage(self, filename):
        s = self.SaveWindowAttributes()
        s.fileName = filename
        s.family = 0
        s.format = self.SaveWindowAttributes().TIFF
        self.SetSaveWindowAttributes(s)
        return self.SaveWindow()

    ##########################################################################
    # Method: SaveSourceFrame
    #
    # Purpose:    This method saves the active window to an image file as
    #             part of the running image sequence.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################

    def SaveSourceFrame(self):
        name = "%s%04d.tif" % (self.basename, self.fileIndex)
        self.fileIndex = self.fileIndex + 1
        self.SaveImage(name)
        return name

    ##########################################################################
    # Method: ProtectedDrawPlots
    #
    # Purpose:    This method tells the viewer to draw the plots. If it
    #             encounters errors while drawing plots, it tries to restart
    #             the engine and redraw the plots.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################

    def ProtectedDrawPlots(self):
        if not self.DrawPlots():
            self.LaunchComputeEngine()
            if not self.DrawPlots():
                 self.LaunchComputeEngine()
                 if not self.DrawPlots():
                     print "DrawPlots failed"

    ##########################################################################
    # Method: PackageMovies
    #
    # Purpose:    This method packages the movie frames into various playable
    #             movie formats.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Jan 14 16:28:16 PST 2003
    #
    ##########################################################################

    def PackageMovies(self, nframes):
        #
        # Loop over the requested movies to determine the list of sizes and
        # formats that we need to create from the source images.
        #
        generatedFrameInfo = (self.processedBaseFormat, self.xres, self.yres)
        formatsWeNeed = [generatedFrameInfo]
        for format in self.movieFormats:
            movieFormat = format[0]
            xres = format[1]
            yres = format[2]
            ppm = "%s-%dx%d-%%04d.ppm" % (self.movieBase, xres, yres)
            png = "%s-%dx%d-%%04d.png" % (self.movieBase, xres, yres)
            tif = "%s-%dx%d-%%04d.tif" % (self.movieBase, xres, yres)
            if(movieFormat == "sm"):
                if((png, xres, yres) in formatsWeNeed):
                    continue
                elif((tif, xres, yres) in formatsWeNeed):
                    continue
                else:
                    formatsWeNeed = formatsWeNeed + [(tif, xres, yres)]
            elif(movieFormat == "qt"):
                if((tif, xres, yres) in formatsWeNeed):
                    continue
                else:
                    formatsWeNeed = formatsWeNeed + [(tif, xres, yres)]
            elif(movieFormat == "mpeg"):
                if((ppm, xres, yres) in formatsWeNeed):
                    continue
                else:
                    formatsWeNeed = formatsWeNeed + [(ppm, xres, yres)]

        #
        # Create the list of resize/conversions that we need to perform.
        #
        conversions = []
        for format in formatsWeNeed:
            if(format == generatedFrameInfo):
                continue
            else:
                conversions = conversions + [format]

        #
        # Perform the resizes and conversions.
        #
        if(len(conversions) > 0):
            movietools.resizeMultipleFrames(self.processedBaseFormat, nframes, conversions)

        #
        # Generate movies
        #
        for format in self.movieFormats:
            movieFormat = format[0]
            xres = format[1]
            yres = format[2]
            tar = format[3]
            movieName = "%s-%dx%d.%s" % (self.movieBase, xres, yres, movieFormat)
            # Create the frame baseformats
            ppm = "%s-%dx%d-%%04d.ppm" % (self.movieBase, xres, yres)
            png = "%s-%dx%d-%%04d.png" % (self.movieBase, xres, yres)
            tif = "%s-%dx%d-%%04d.tif" % (self.movieBase, xres, yres)
            # Create the tar file names
            ppmTar = "%s-ppm-frames-%dx%d.tar" % (self.movieBase, xres, yres)
            pngTar = "%s-png-frames-%dx%d.tar" % (self.movieBase, xres, yres)
            tifTar = "%s-tif-frames-%dx%d.tar" % (self.movieBase, xres, yres)
            # Create the tar file contents
            ppmTarFiles = "%s-%dx%d-????.ppm" % (self.movieBase, xres, yres)
            pngTarFiles = "%s-%dx%d-????.png" % (self.movieBase, xres, yres)
            tifTarFiles = "%s-%dx%d-????.tif" % (self.movieBase, xres, yres)
          
            if(movieFormat == "sm"):
                if((png, xres, yres) in formatsWeNeed):
                    movietools.createStreamingMovie(movieName, png, nframes)
                    if(tar):
                        command = "tar -cf %s %s" % (pngTar, pngTarFiles)
                        os.system(command)
                else:
                    movietools.createStreamingMovie(movieName, tif, nframes)
                    if(tar):
                        command = "tar -cf %s %s" % (tifTar, tifTarFiles)
                        os.system(command)
            elif(movieFormat == "qt"):
                movietools.createQuickTimeMovie(movieName, tif, nframes, xres, yres)
                if(tar):
                    command = "tar -cf %s %s" % (tifTar, tifTarFiles)
                    os.system(command)
            elif(movieFormat == "mpeg"):
                movietools.createMPEG(movieName, ppm, nframes, xres, yres)
                if(tar):
                    command = "tar -cf %s %s" % (ppmTar, ppmTarFiles)
                    os.system(command)
