#
# This script makes a movie given a state XML file.
#
import os, sys, thread

###############################################################################
# Function: CommandInPath
#
# Purpose:    This function determines if a command is in the user's path.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def CommandInPath(command):
    retval = 0
    if(sys.platform != "win32"):
        # Look for the command using the which command
        cmd = "which %s" % command
        f = os.popen(cmd)
        lines = f.readlines()
        f.close()
        # Glue the lines together.
        str = ""
        for line in lines:
            str = str + line
        # See if the command could not be found
        noFail1 = (str.find("no %s in" % command) == -1)
        noFail2 = (str.find("Command not found") == -1)
        if(noFail1 and noFail2):
            retval = 1
    return retval

###############################################################################
# Function: applyFunctionToNFrames
#
# Purpose:    This function applies a function to all of the frames and uses
#             multiple threads to do so. The number of threads is determined
#             by the filesPerThread and nframes arguments.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def applyFunctionToNFramesWMutex(threadID, exitmutex, func, start, end, conversionargs):
    try:
        func(threadID, start, end, conversionargs)
        exitmutex[threadID] = 1
    except IndexError:
        return
    
def applyFunctionToNFrames(func, filesPerThread, nframes, conversionargs):
    if(nframes < filesPerThread):
        func(0, 0, nframes, conversionargs)
    else:
        try:
            import thread
            keepSpawning = 1
            start = filesPerThread
            end = filesPerThread
            threadID = 1
            # Create a mutex array
            numThreads = nframes / filesPerThread
            if(numThreads * filesPerThread < nframes):
                numThreads = numThreads + 1
            exitmutex = [0] * numThreads
            while keepSpawning == 1:
                end = end + filesPerThread
                if(end >= nframes):
                    end = nframes
                    keepSpawning = 0
                thread.start_new(applyFunctionToNFramesWMutex, (threadID, exitmutex, func, start, end, conversionargs))
                start = start + filesPerThread
                threadID = threadID + 1
            # Do the first part of the work on the main thread.
            applyFunctionToNFramesWMutex(0, exitmutex, func, 0, filesPerThread, conversionargs)

            # Wait for all of the threads to complete
            while 0 in exitmutex: pass
        except ImportError:
            print "Could not import threads module."
            func(0, 0, nframes, conversionargs)

###############################################################################
# Function: applyFunctionToFrames
#
# Purpose:    This function applies a function to all of the frames and uses
#             multiple threads to do so.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def applyFunctionToFrames(func, nframes, conversionargs):
    maxThreads = 10
    filesPerThread = nframes / maxThreads
    if(filesPerThread < 1):
        filesPerThread = 1
    applyFunctionToNFrames(func, filesPerThread, nframes, conversionargs)

###############################################################################
# Function: removeFilesHelper
#
# Purpose:    This function removes files from the disk.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def RemoveFile(fileName):
    command = "rm "
    if(sys.platform == "win32"):
        command = "del "
    command = command + " " + fileName
    # Remove the files.
    os.system(command)

def removeFilesHelper(threadID, start, end, conversionargs):
    # Get the arguments out of the tuple.
    #print "removeFilesHelper(%d, %d," % (start, end), conversionargs,")"
    # Create the list of files to remove.
    format = conversionargs
    for i in range(start, end):
        fileName = format % i
        RemoveFile(fileName)

###############################################################################
# Function: removeFiles    
#
# Purpose:    This function removes files from the disk.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def removeFiles(format, nframes):
    conversionargs = format
    applyFunctionToFrames(removeFilesHelper, nframes, conversionargs)

###############################################################################
# Function: createQuickTimeFile
#
# Purpose:    This function creates a quicktime movie.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def createQuickTimeFile(moviename, baseFormat, start, end, xres, yres):
    names = ""
    for i in range(start, end):
        filename = baseFormat % i
        names = names + " " + filename
    # Create the movie file.
    command = "dmconvert -f qt -p video,comp=qt_mjpega,inrate=15,rate=15 %s %s" % (names, moviename)
    #print command
    os.system(command)

###############################################################################
# Function: EncodeQuickTimeMovieHelper
#
# Purpose:    This function is a thread callback function that creates a
#             QuickTime movie for a small set of frames.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def EncodeQuickTimeMovieHelper(threadID, start, end, conversionargs):
    # Extract the arguments.
    moviename = conversionargs[0]
    baseFormat = conversionargs[1]
    xres = conversionargs[2]
    yres = conversionargs[3]
    # Create the name of the part of the movie and the names of the files to use.
    subMovieName = "%s.%d" % (moviename, threadID)
    createQuickTimeFile(subMovieName, baseFormat, start, end, xres, yres)

###############################################################################
# Class: MakeMovie
#
# Purpose:    This class makes movies.
#
# Programmer: Brad Whitlock
# Date:       Mon Jul 28 12:45:45 PDT 2003
#
# Modifications:
#
###############################################################################

class MakeMovie:
    ###########################################################################
    # Method: __init__
    #
    # Purpose:    Constuctor.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    ###########################################################################

    def __init__(self):
        # Movie types.
        self.MPEG_MOVIE = 0
        self.QUICKTIME_MOVIE = 1
        self.STREAMING_MOVIE = 2
        self.JUST_FRAMES_MOVIE = 3

        # Output file types.
        self.OUTPUT_PPM  = 0
        self.OUTPUT_TIFF = 1
        self.OUTPUT_JPEG = 2
        self.OUTPUT_BMP  = 3
        self.OUTPUT_RGB  = 4

        # Movie properties.
        self.stateFile = ""
        self.frameBase = "frame"
        self.movieBase = "movie"
        self.movieFormat = self.MPEG_MOVIE
        self.outputFormat = self.OUTPUT_PPM
        self.numFrames = 0
        self.frameStep = 1
        self.xres = 512
        self.yres = 512
        self.tmpDir = os.curdir

        # Set the slash used in filenames based on the platform.
        self.slash = "/"
        if(sys.platform == "win32"):
            self.slash = "\\"


    ###########################################################################
    # Method: PrintUsage
    #
    # Purpose:    This method prints the usage for the program.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    # Modifications:
    #   Brad Whitlock, Tue Aug 12 09:57:05 PDT 2003
    #   Added a note about Windows.
    #
    ###########################################################################

    def PrintUsage(self):
        print "Usage: visit -movie [-format fmt] [-geometry size] -sessionfile name"
        print "                    [-output moviename] [-framestep step]"
        print ""
        print "OPTIONS"
        print "    The following options are recognized by visit -movie"
        print ""
        print "    -format fmt        The format option allows you to set the output"
        print "                       format for your movie. The supported values "
        print "                       for fmt are:"
        print ""
        print "                       mpeg : MPEG 2 movie."
        print "                       qt   : Quicktime movie."
        print "                       sm   : Streaming movie format"
        print "                              (popular for powerwall demos)."
        print "                       ppm  : Save raw movie frames as individual"
        print "                              PPM files."
        print "                       tiff : Save raw movie frames as individual"
        print "                              TIFF files."
        print "                       jpeg : Save raw movie frames as individual"
        print "                              JPEG files."
        print "                       bmp  : Save raw movie frames as individual"
        print "                              BMP (Windows Bitmap) files."
        print "                       rgb  : Save raw movie frames as individual"
        print "                              RGB (SGI format) files."
        print ""
        print "                       *** Important Note ***"
        print "                       The mpeg, qt, and sm formats are not supported"
        print "                       on the Windows platform."
        print ""
        print "    -geometry size     The geometry option allows you to set the movie"
        print "                       resolution. The size argument is of the form"
        print "                       WxH where W is the width of the image and H is"
        print "                       the height of the image. For example, if you "
        print "                       want an image that is 1024 pixels wide and 768"
        print "                       pixels tall, you would provide:"
        print "                       -geometry 1024x768."
        print ""
        print "    -sessionfile name  The sessionfile option lets you pick the name"
        print "                       of the VisIt session to use as input for your"
        print "                       movie. The VisIt session is a file that describes"
        print "                       the movie that you want to make and it is created"
        print "                       when you save your session from within VisIt's "
        print "                       GUI after you set up your plots how you want them."
        print ""
        print "    -framestep step    The number of frames to advance when going to "
        print "                       the next frame."
        print ""
        print "    -output moviename  The output option lets you set the name of "
        print "                       your movie."
        print ""

    ###########################################################################
    # Method: Debug
    #
    # Purpose:    Outputs debug messages.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    ###########################################################################

    def Debug(self, str):
        pass #print str

    ###########################################################################
    # Method: ProcessArguments
    #
    # Purpose:    This method sets some of the object's attributes based on
    #             command line flags.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    # Modifications:
    #   Brad Whitlock, Tue Aug 12 09:57:31 PDT 2003
    #   Prevented use of mpeg, qt, and sm on Windows.
    #
    ###########################################################################

    def ProcessArguments(self):
        i = 0
        while(i < len(sys.argv)):
            if(sys.argv[i] == "-format"):
                if((i+1) < len(sys.argv)):
                    format = sys.argv[i+1]
                    if(format == "mpeg"):
                        if(sys.platform != "win32"):
                            self.movieFormat = self.MPEG_MOVIE
                            self.outputFormat = self.OUTPUT_PPM
                        else:
                            self.PrintUsage();
                            sys.exit(-1)
                    elif(format == "qt"):
                        if(sys.platform != "win32"):
                            self.movieFormat = self.QUICKTIME_MOVIE
                            self.outputFormat = self.OUTPUT_TIFF
                        else:
                            self.PrintUsage();
                            sys.exit(-1)
                    elif(format == "sm"):
                        if(sys.platform != "win32"):
                            self.movieFormat = self.STREAMING_MOVIE
                            self.outputFormat = self.OUTPUT_TIFF
                        else:
                            self.PrintUsage();
                            sys.exit(-1)
                    elif(format == "ppm"):
                        self.movieFormat = self.JUST_FRAMES_MOVIE
                        self.outputFormat = self.OUTPUT_PPM
                    elif(format == "tiff"):
                        self.movieFormat = self.JUST_FRAMES_MOVIE
                        self.outputFormat = self.OUTPUT_TIFF
                    elif(format == "jpeg"):
                        self.movieFormat = self.JUST_FRAMES_MOVIE
                        self.outputFormat = self.OUTPUT_JPEG
                    elif(format == "bmp"):
                        self.movieFormat = self.JUST_FRAMES_MOVIE
                        self.outputFormat = self.OUTPUT_BMP
                    elif(format == "rgb"):
                        self.movieFormat = self.JUST_FRAMES_MOVIE
                        self.outputFormat = self.OUTPUT_RGB
                    else:
                        self.PrintUsage()
                        sys.exit(-1)
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(sys.argv[i] == "-sessionfile"):
                if((i+1) < len(sys.argv)):
                    self.stateFile = sys.argv[i+1]
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(sys.argv[i] == "-framestep"):
                if((i+1) < len(sys.argv)):
                    try:
                        self.frameStep = int(sys.argv[i+1])
                        if(self.frameStep < 0):
                            self.frameStep = 1
                    except ValueError:
                        self.frameStep = 1
                        print "A bad value was provided for frame step. Using a frame step of 1."
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(sys.argv[i] == "-output"):
                if((i+1) < len(sys.argv)):
                    self.movieBase = sys.argv[i+1]
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(sys.argv[i] == "-geometry"):
                if((i+1) < len(sys.argv)):
                    geometry = sys.argv[i+1]
                    xloc = geometry.find("x")
                    if(xloc != -1):
                        self.xres = int(geometry[:xloc])
                        self.yres = int(geometry[xloc+1:])
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            # On to the next argument.
            i = i + 1

        # Make sure that the user provided the -sessionfile option or we can't
        # make a movie!
        if(self.stateFile == ""):
            self.PrintUsage()
            print "You must provide the -sessionfile option!"
            sys.exit(-1)

        # If the movie is just a set of frames, make sure that we use the
        # movie base name as the frame base.
        if(self.movieFormat == self.JUST_FRAMES_MOVIE):
            self.frameBase = self.movieBase

    ###########################################################################
    # Method: CreateTemporaryDirectory
    #
    # Purpose:    This method creates a temporary directory to which the frames
    #             for the movie will be written if we are encoding the movie
    #             in an animation format such as MPEG, QuickTime, or Streaming
    #             movie.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    ###########################################################################

    def CreateTemporaryDirectory(self):
        retval = os.curdir
        if(self.movieFormat != self.JUST_FRAMES_MOVIE):
            index = 0
            d = "%s-%dx%d-%d" % (self.movieBase, self.xres, self.yres, index)
            while(d in os.listdir(os.curdir)):
                self.Debug("The directory %s already exists, we'll try another name." % d)
                index = index + 1
                d = "%s-%dx%d-%d" % (self.movieBase, self.xres, self.yres, index)
            # Create a temporary directory in which to store frames.
            os.mkdir(d)
            self.Debug("Created %s directory to store temporary frames" % d)
            retval = d
        return retval

    ###########################################################################
    # Method: SaveImage
    #
    # Purpose:    This method tells VisIt to save an image.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    ###########################################################################

    def SaveImage(self, index, ext):
        s = SaveWindowAttributes()
        s.fileName = "%s%s%s%04d.%s" % (self.tmpDir, self.slash, self.frameBase, index, ext)
        s.family = 0
        if(self.outputFormat == self.OUTPUT_PPM):
            s.format =  s.PPM
        elif(self.outputFormat == self.OUTPUT_TIFF):
            s.format =  s.TIFF
        elif(self.outputFormat == self.OUTPUT_JPEG):
            s.format =  s.JPEG
        elif(self.outputFormat == self.OUTPUT_BMP):
            s.format =  s.BMP
        elif(self.outputFormat == self.OUTPUT_RGB):
            s.format =  s.RGB
        s.width = self.xres
        s.height = self.yres
        SetSaveWindowAttributes(s)
        return SaveWindow()

    ###########################################################################
    # Method: LookupImageExtension
    #
    # Purpose:    This method returns the file extension associated with the
    #             specified output image type.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    ###########################################################################

    def LookupImageExtension(self, fmt):
        retval = "ppm"
        if(fmt == self.OUTPUT_TIFF):
            retval = "tif"
        elif(fmt == self.OUTPUT_JPEG):
            retval = "jpeg"
        elif(fmt == self.OUTPUT_BMP):
            retval = "bmp"
        elif(fmt == self.OUTPUT_RGB):
            retval = "rgb"
        return retval

    ###########################################################################
    # Method: GenerateFrames
    #
    # Purpose:    This method tells VisIt to generate the frames for the movie.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    ###########################################################################

    def GenerateFrames(self):
        self.Debug("GenerateFrames")
        # Create a temporary directory in which to store the frames if we're
        # generating a movie that ends up as a single file.
        self.tmpDir = self.CreateTemporaryDirectory()

        # Figure out the extension for the output files.
        ext = self.LookupImageExtension(self.outputFormat)

        # Make the viewer try and restore its session using the file that
        # was passed in.
        RestoreSession(self.stateFile, 0)

        # Make sure that plots are all drawn.
        DrawPlots()

        # Save an image for each frame in the animation.
        self.numFrames = 0
        i = 0
        while(i < AnimationGetNFrames()):
            AnimationSetFrame(i)
            self.SaveImage(self.numFrames, ext)
            self.numFrames = self.numFrames + 1
            i = i + self.frameStep

    ###########################################################################
    # Method: EncodeMPEGMovie
    #
    # Purpose:    This method creates an MPEG movie.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    # Modifications:
    #   Brad Whitlock, Tue Aug 12 09:37:37 PDT 2003
    #   Passed the desired version to mpeg_encode.
    #
    ###########################################################################

    def EncodeMPEGMovie(self):
        retval = 0
        self.Debug("EncodeMPEGMovie")
        paramFile = "%s-mpeg_encode-params" % self.movieBase
        try:
            # Make sure that the movie extension is ".mpeg".
            moviename = self.movieBase
            ext = self.movieBase[-5:]
            if(ext != ".mpeg"):
                moviename = self.movieBase + ".mpeg"

            f = open(paramFile, "w")
            f.write("PATTERN             IBBPBBPBBPBBPBB\n")
            f.write("OUTPUT              %s\n" % moviename)
            f.write("BASE_FILE_FORMAT    PPM\n")
            f.write("GOP_SIZE            30\n")
            f.write("SLICES_PER_FRAME    1\n")
            f.write("PIXEL               HALF\n")
            f.write("RANGE               10\n")
            f.write("PSEARCH_ALG         TWOLEVEL\n")
            f.write("BSEARCH_ALG         SIMPLE\n")
            f.write("IQSCALE             5\n")
            f.write("PQSCALE             8\n")
            f.write("BQSCALE             18\n")
            f.write("REFERENCE_FRAME     DECODED\n")
            f.write("FORCE_ENCODE_LAST_FRAME\n")
            f.write("YUV_SIZE            %dx%d\n" % (self.xres, self.yres))
            f.write("INPUT_CONVERT       *\n")
            f.write("INPUT_DIR           %s\n" % self.tmpDir)
            f.write("INPUT\n")
            f.write("%s*.ppm             [0000-%04d]\n" % (self.frameBase, self.numFrames-1))
            f.write("END_INPUT\n")
            f.close();

            # Create the movie
            command = "visit -v %s -mpeg_encode %s" % (Version(), paramFile)
            r = os.system(command)

            # Remove the param file.
            RemoveFile(paramFile);

            retval = (r == 0)
        except IOError:
            print "VisIt could not create an MPEG parameter file! No MPEG movie"
            print "will be created. You can access the raw source frames in:"
            print self.tmpDir

        return retval

    ###########################################################################
    # Method: EncodeQuickTimeMovie
    #
    # Purpose:    This method creates a QuickTime movie.
    #
    # Notes:      If there are many movies, this routine will create several
    #             smaller QuickTime movies that are concatenated once they are
    #             all generated. This avoids a limit of 1000 or so command
    #             line arguments on the shell.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    ###########################################################################

    def EncodeQuickTimeMovie(self):
        self.Debug("EncodeQuickTimeMovie")
        retval = 0
        if(CommandInPath("dmconvert")):
            # Make sure that the movie extension is ".qt".
            moviename = self.movieBase
            ext = self.movieBase[-3:]
            if(ext != ".qt"):
                moviename = self.movieBase + ".qt"

            ext = self.LookupImageExtension(self.outputFormat)
            baseFormat  = "%s%s%s%%04d.%s" % (self.tmpDir, self.slash, self.frameBase, ext)
            xres = self.xres
            yres = self.yres

            # Create small quicktime movies
            conversionargs = (moviename, baseFormat, xres, yres)
            framesPerMovie = 50
            nframes = self.numFrames
            applyFunctionToNFrames(EncodeQuickTimeMovieHelper, framesPerMovie, nframes, conversionargs)
            # Glue the submovies together.
            nSubMovies = nframes / framesPerMovie
            if(nSubMovies * framesPerMovie < nframes):
                nSubMovies = nSubMovies + 1
            subMovieFormat = "%s.%%d" % moviename
            createQuickTimeFile(moviename, subMovieFormat, 0, nSubMovies, xres, yres)
            # Delete the submovies.
            removeFiles(subMovieFormat, nSubMovies)
            retval = 1
        else:
            print "The command \"dmconvert\", which is required to make QuickTime,"
            print "movies is not in your path so your source frames cannot be converted"
            print "into a QuickTime movie. You can, however, still access the frames of "
            print "your movie in %s." % self.tmpDir

        return retval

    ###########################################################################
    # Method: EncodeStreamingMovie
    #
    # Purpose:    This method creates a streaming movie.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    ###########################################################################

    def EncodeStreamingMovie(self):
        self.Debug("EncodeStreamingMovie")
        retval = 0
        if(CommandInPath("img2sm")):
            # Create a format string that describes the files in the movie.
            ext = self.LookupImageExtension(self.outputFormat)
            format  = "%s%s%s%%04d.%s" % (self.tmpDir, self.slash, self.frameBase, ext)

            # Make sure that the movie extension is ".sm".
            moviename = self.movieBase
            ext = self.movieBase[-3:]
            if(ext != ".sm"):
                moviename = self.movieBase + ".sm"

            # Execute the img2sm command
            command = "img2sm -rle -first 0 -last %d -form tiff %s %s" % (self.numFrames-1, format, moviename)
            self.Debug(command)
            os.system(command)
            retval = 1
        else:
            print "The command \"img2sm\", which is required to make streaming movies,"
            print "is not in your path so your source frames cannot be converted into"
            print "a streaming movie. You can, however, still access the frames of "
            print "your movie in %s." % self.tmpDir

        return retval

    ###########################################################################
    # Method: EncodeFrames
    #
    # Purpose:    This method calls the appropriate method to encode frames
    #             into a movie file.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    ###########################################################################

    def EncodeFrames(self):
        safeToRemoveFrames = 0
        self.Debug("EncodeFrames")
        if(self.movieFormat == self.MPEG_MOVIE):
            safeToRemoveFrames = self.EncodeMPEGMovie()
        elif(self.movieFormat == self.QUICKTIME_MOVIE):
            safeToRemoveFrames = self.EncodeQuickTimeMovie()
        elif(self.movieFormat == self.STREAMING_MOVIE):
            safeToRemoveFrames = self.EncodeStreamingMovie()
        return safeToRemoveFrames

    ###########################################################################
    # Method: Cleanup
    #
    # Purpose:    This method cleans up any temporary files that were created.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    ###########################################################################

    def Cleanup(self):
        if(self.tmpDir != os.curdir):
            self.Debug("Need to delete %s directory!" % self.tmpDir)
            tmpfiles = os.listdir(self.tmpDir)
            for f in tmpfiles:
                os.remove("%s%s%s" % (self.tmpDir, self.slash, f))
            os.rmdir(self.tmpDir)


###############################################################################
# Function: main
#
# Purpose:    This is the main function for the program.
#
# Programmer: Brad Whitlock
# Date:       Mon Jul 28 15:35:54 PST 2003
#
# Modifications:
#
###############################################################################

def main():
    movie = MakeMovie()
    movie.ProcessArguments()
    movie.GenerateFrames()
    if(movie.EncodeFrames()):
        movie.Cleanup()
    sys.exit(0)

#
# Call the main function.
#
main()
