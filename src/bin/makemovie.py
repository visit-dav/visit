#
# This script makes a movie given a state XML file.
#
import os, string, sys, thread
from xmllib import *

###############################################################################
# Function: CommandInPath
#
# Purpose:    This function determines if a command is in the user's path.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
# Modifications:
# 
#   Hank Childs, Thu Apr  1 09:33:16 PST 2004
#   Simplified a bit (previous implementation was not portable).
#
###############################################################################

def CommandInPath(command):
    retval = 0
    if(sys.platform != "win32"):
        # Look for the command using the which command
        cmd = "which %s > /dev/null" % command
        rv = os.system(cmd)
        if (rv == 0):
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
# Modifications:
#   Brad Whitlock, Mon Jul 25 16:17:39 PST 2005
#   Made it possible to set the return value into a list of return values.
#
###############################################################################

def applyFunctionToNFramesWMutex(threadID, exitmutex, retval, func, start, end, conversionargs):
    try:
        retval[threadID] = func(threadID, start, end, conversionargs)
        exitmutex[threadID] = 1
    except IndexError:
        return
    
def applyFunctionToNFrames(func, filesPerThread, nframes, conversionargs):
    retval = 0
    if(nframes < filesPerThread):
        retval = func(0, 0, nframes, conversionargs)
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
            retval = [0] * numThreads
            while keepSpawning == 1:
                end = end + filesPerThread
                if(end >= nframes):
                    end = nframes
                    keepSpawning = 0
                thread.start_new(applyFunctionToNFramesWMutex, (threadID, exitmutex, retval, func, start, end, conversionargs))
                start = start + filesPerThread
                threadID = threadID + 1
            # Do the first part of the work on the main thread.
            applyFunctionToNFramesWMutex(0, exitmutex, retval, func, 0, filesPerThread, conversionargs)

            # Wait for all of the threads to complete
            while 0 in exitmutex: pass
        except ImportError:
            print "Could not import threads module."
            retval = func(0, 0, nframes, conversionargs)
    return retval

###############################################################################
# Function: applyFunctionToFrames
#
# Purpose:    This function applies a function to all of the frames and uses
#             multiple threads to do so.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
# Modifications:
#   Brad Whitlock, Mon Jul 25 16:17:09 PST 2005
#   I made it return a value.
#
###############################################################################

def applyFunctionToFrames(func, nframes, conversionargs):
    maxThreads = 10
    filesPerThread = nframes / maxThreads
    if(filesPerThread < 1):
        filesPerThread = 1
    return applyFunctionToNFrames(func, filesPerThread, nframes, conversionargs)

###############################################################################
# Function: CopyFile
#
# Purpose:    This function copies a file to another file on disk.
#
# Programmer: Brad Whitlock
# Date:       Mon Apr 26 16:20:14 PST 2004
#
# Modifications:
#   Brad Whitlock, Tue Jun 21 11:10:47 PDT 2005
#   I made it re-raise VisItInterrupt if it gets one.
#
###############################################################################

def CopyFile(srcName, destName, allowLinks):
    fileCopied = 0

    # If we want to create a symlink instead of copying, try it now.
    if allowLinks:
        try:
            os.symlink(imgname, linkname)
            fileCopied = 1
        except VisItInterrupt:
            raise
        except:
            # The OS module does not have symlink
            fileCopied = 0

    # Try copying the file if a symlink could not be made.
    if fileCopied == 0:
        try:
            if os.name == "nt":
                os.system("copy %s %s" % (srcName, destName))
            else:
                os.system("cp %s %s" % (srcName, destName))
        except VisItInterrupt:
            raise
        except:
            print "Could not copy %s to %s" % (srcName, destName)


###############################################################################
# Function: removeFilesHelper
#
# Purpose:    This function removes files from the disk.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
# Modifications:
#   Brad Whitlock, Fri Oct 3 13:12:54 PST 2003
#   Changed so it uses unlink to remove files.
#
#   Brad Whitlock, Mon Jul 25 16:16:28 PST 2005
#   I made it fail gracefully.
#
###############################################################################

def RemoveFile(fileName):
    try:
        os.unlink(fileName)
    except OSError:
        pass

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
# Modifications:
#   Brad Whitlock, Mon Jul 25 16:16:01 PST 2005
#   Made it return a value.
#
###############################################################################

def removeFiles(format, nframes):
    conversionargs = format
    return applyFunctionToFrames(removeFilesHelper, nframes, conversionargs)

###############################################################################
# Function: createQuickTimeFile
#
# Purpose:    This function creates a quicktime movie.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
# Modifications:
#   Hank Childs, Thu Apr  1 07:48:41 PST 2004
#   Added frames per second.
#
#   Brad Whitlock, Mon Jul 25 16:15:35 PST 2005
#   Made it return a value.
#
###############################################################################

def createQuickTimeFile(moviename, baseFormat, start, end, xres, yres, fps):
    names = ""
    for i in range(start, end):
        filename = baseFormat % i
        names = names + " " + filename
    # Create the movie file.
    command = "dmconvert -f qt -p video,comp=qt_mjpega,inrate=%d,rate=%d %s %s" % (fps, fps, names, moviename)
    #print command
    return os.system(command)

###############################################################################
# Function: EncodeQuickTimeMovieHelper
#
# Purpose:    This function is a thread callback function that creates a
#             QuickTime movie for a small set of frames.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
# Modifications:
#   Hank Childs, Thu Apr  1 07:52:00 PST 2004
#   Added frames per second.
#
#   Brad Whitlock, Mon Jul 25 16:15:24 PST 2005
#   Made it return a value.
#
###############################################################################

def EncodeQuickTimeMovieHelper(threadID, start, end, conversionargs):
    # Extract the arguments.
    moviename = conversionargs[0]
    baseFormat = conversionargs[1]
    xres = conversionargs[2]
    yres = conversionargs[3]
    fps = conversionargs[4]
    # Create the name of the part of the movie and the names of the files to use.
    subMovieName = "%s.%d" % (moviename, threadID)
    return createQuickTimeFile(subMovieName, baseFormat, start, end, xres, yres, fps)

###############################################################################
# Function: MovieClassSaveWindow
#
# Purpose:    This function is called from the mangled Python script when it
#             wants to save an image.
#
# Programmer: Brad Whitlock
# Date:       Fri Oct 3 12:47:32 PDT 2003
#
###############################################################################
classSaveWindowObj = 0
def MovieClassSaveWindow():
    return classSaveWindowObj.SaveImage2()

###############################################################################
# Class: EngineAttributesParser
#
# Purpose:    This class parses session files for HostProfiles in the
#             "RunningEngines" node so we can extract information about the
#             compute engines that were running when the session file was
#             saved.
#
# Programmer: Brad Whitlock
# Date:       Tue Aug 3 16:11:01 PST 2004
#
# Modifications:
#
###############################################################################

class EngineAttributesParser(XMLParser):
    def __init__(self):
        XMLParser.__init__(self)
        self.elements = {"Object" : ("<Object>", "</Object>"), "Field" : ("<Field>", "</Field>")}
        self.attributes = {"name" : "", "type" : None, "length" : 0}

        self.readingEngineProperties = 0
        self.readingHostProfile = 0
        self.readingField = 0
        self.engineProperties = {}
        self.allEngineProperties = {}
        self.dataName = None
        self.dataAtts = None


    def handle_starttag(self, tag, method, attributes):
        if tag == "Object":
            if "name" in attributes.keys():
                self.dataName = attributes["name"]
                if self.dataName == "RunningEngines":
                    self.readingEngineProperties = 1
                elif self.dataName == "HostProfile":
                    if self.readingEngineProperties == 1:
                        self.readingHostProfile = 1
        else:
            self.readingField = 1
            self.dataAtts = attributes


    def handle_endtag(self, tag, method):
        if tag == "Object":
            if self.dataName == "RunningEngines":
                self.readingEngineProperties = 0
                self.dataName = None
            elif self.dataName == "HostProfile":
                if self.readingEngineProperties == 1:
                    self.readingHostProfile = 0
                    self.dataName = None
                    if "host" in self.engineProperties.keys():
                        host = self.engineProperties["host"]
                        self.allEngineProperties[host] = self.engineProperties
                        #self.engineProperties = {}
        elif tag == "Field":
            self.readingField = 0


    def handle_data(self, data):
        def not_all_spaces(s):
            space = ""
            for i in range(len(s)):
                space = space + " "
            return s != space
        if (self.readingEngineProperties == 1 or self.readingHostProfile)\
            and self.readingField == 1 and len(data) > 0:
            name = self.dataAtts["name"]
            type = self.dataAtts["type"]
            value = None
            if type == "bool":
                if data == "true":
                    value = 1
                else:
                    value = 0
            elif type == "string":
                value = data
            elif type == "stringVector":
                fragments = string.split(data, "\"")
                value = []
                for s in fragments:
                   if len(s) > 0:
                       if not_all_spaces(s):
                           value = value + [s]
            elif type == "int":
                value = int(data)
            else:
                print "Unknown type: ", type
                return
            self.engineProperties[name] = value

###############################################################################
# Class: WindowSizeParser
#
# Purpose:    This class parses session files for the windowSize field in
#             the ViewerWindow and builds a list of [width,height] values.
#
# Programmer: Brad Whitlock
# Date:       Fri Jun 24 10:03:23 PDT 2005
#
# Modifications:
#
###############################################################################

class WindowSizeParser(XMLParser):
    def __init__(self):
        XMLParser.__init__(self)
        self.elements = {"Object" : ("<Object>", "</Object>"), "Field" : ("<Field>", "</Field>")}
        self.attributes = {"name" : "", "type" : None, "length" : 0}

        self.readAtts = 0
        self.readingField = 0
        self.windowSizes = []
        self.activeWindow = 0
        self.objectNames = []
        self.dataAtts = None

    def handle_starttag(self, tag, method, attributes):
        if tag == "Object":
            if "name" in attributes.keys():
                name = attributes["name"]
                self.objectNames = self.objectNames + [name]
                if name == "ViewerWindowManager":
                    self.readAtts = 1
        else:
            self.readingField = 1
            self.dataAtts = attributes

    def handle_endtag(self, tag, method):
        if tag == "Object":
            name = self.objectNames[-1]
            if name == "ViewerWindowManager":
                self.readAtts = 0
            self.objectNames = self.objectNames[:-1]
        elif tag == "Field":
            self.readingField = 0

    def handle_data(self, data):
        def not_all_spaces(s):
            space = ""
            for i in range(len(s)):
                space = space + " "
            return s != space
        if self.readAtts and self.readingField and len(data) > 0:
            name = self.dataAtts["name"]
            type = self.dataAtts["type"]
            value = []
            if name == "activeWindow" and type == "int":
                try:
                    self.activeWindow = int(data)
                    if self.activeWindow < 0:
                        self.activeWindow = 0
                except ValueError:
                    return
            elif name == "windowSize" and type == "intArray":
                length = self.dataAtts["length"]
                if length == "2":
                    fragments = string.split(data, " ")
                    value = []
                    for s in fragments:
                       if len(s) > 0:
                           if not_all_spaces(s):
                               try:
                                   val = int(s)
                                   value = value + [val]
                               except ValueError:
                                   continue
                    self.windowSizes = self.windowSizes + [value[:2]]

###############################################################################
# Class: MakeMovie
#
# Purpose:    This class makes movies.
#
# Programmer: Brad Whitlock
# Date:       Mon Jul 28 12:45:45 PDT 2003
#
# Modifications:
#   Brad Whitlock, Fri Oct 3 11:14:59 PDT 2003
#   Added support for processing Python files.
#
#   Brad Whitlock, Fri Dec 5 12:23:19 PDT 2003
#   Added support for automatically determining the movie file name.
#
#   Brad Whitlock, Wed Aug 4 10:46:48 PDT 2004
#   Added support for reading compute engine information needed to restart
#   parallel compute engines from the session file.
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
    # Modifications:
    #   Brad Whitlock, Fri Oct 3 11:14:50 PDT 2003
    #   Added support for processing Python files.
    #
    #   Hank Childs, Wed Mar 31 08:44:34 PST 2004
    #   Added frames per second.
    #
    #   Brad Whitlock, Thu Apr 22 09:18:20 PDT 2004
    #   Added frameStart and frameEnd.
    #
    #   Brad Whitlock, Tue Aug 3 16:58:47 PST 2004
    #   Added engineCommandLineProperties.
    #
    #   Brad Whitlock, Wed Dec 15 11:22:42 PDT 2004
    #   Made it possible to save PNG files.
    #
    #   Brad Whitlock, Tue Apr 12 16:53:22 PST 2005
    #   Added support for stereo, multiple output sizes and formats.
    #
    #   Brad Whitlock, Mon Jul 25 15:21:25 PST 2005
    #   I made img2sm and dmconvert use PNG files.
    #
    ###########################################################################

    def __init__(self):
        # Output file types.
        self.FORMAT_PPM  = 0
        self.FORMAT_TIFF = 1
        self.FORMAT_JPEG = 2
        self.FORMAT_BMP  = 3
        self.FORMAT_RGB  = 4
        self.FORMAT_PNG  = 5
        self.FORMAT_MPEG = 6
        self.FORMAT_QT   = 7
        self.FORMAT_SM   = 8

        # This map helps us determine what kind of image VisIt needs to save
        # in order to create the specified type of movie
        self.outputNeedsInput = { \
           self.FORMAT_PPM  : self.FORMAT_PPM, \
           self.FORMAT_TIFF : self.FORMAT_TIFF, \
           self.FORMAT_JPEG : self.FORMAT_JPEG, \
           self.FORMAT_BMP  : self.FORMAT_BMP, \
           self.FORMAT_RGB  : self.FORMAT_RGB, \
           self.FORMAT_PNG  : self.FORMAT_PNG, \
           self.FORMAT_MPEG : self.FORMAT_PPM, \
           self.FORMAT_QT   : self.FORMAT_PNG, \
           self.FORMAT_SM   : self.FORMAT_PNG \
        }

        # This map gives us the file extension to use for a specific file format.
        self.formatExtension = { \
           self.FORMAT_PPM  : ".ppm", \
           self.FORMAT_TIFF : ".tif", \
           self.FORMAT_JPEG : ".jpeg", \
           self.FORMAT_BMP  : ".bmp", \
           self.FORMAT_RGB  : ".rgb", \
           self.FORMAT_PNG  : ".png", \
           self.FORMAT_MPEG : ".mpeg", \
           self.FORMAT_QT   : ".qt", \
           self.FORMAT_SM   : ".sm" \
        }

        # This map gives us the type associated with a format name.
        self.formatNameToType = { \
           "ppm"  : self.FORMAT_PPM, \
           "tiff" : self.FORMAT_TIFF, \
           "jpeg" : self.FORMAT_JPEG, \
           "bmp"  : self.FORMAT_BMP , \
           "rgb"  : self.FORMAT_RGB , \
           "png"  : self.FORMAT_PNG , \
           "mpeg" : self.FORMAT_MPEG, \
           "qt"   : self.FORMAT_QT  , \
           "sm"   : self.FORMAT_SM \
        }

        # Movie properties.
        self.stateFile = ""
        self.scriptFile = ""
        self.usesCurrentPlots = 0
        self.usesStateFile = 0
        self.movieBase = "movie"
        self.movieFormats = []
        self.numFrames = 0
        self.frameStep = 1
        self.frameStart = 0
        self.frameEnd = -1
        self.tmpDir = os.curdir
        self.fps = 10
        self.stereo = 0
        self.templateFile = "ActiveVisWindowMovieTemplate.py"
        self.usesTemplateFile = 0
        self.screenCaptureImages = 0
        self.sendClientFeedback = 0
        self.percentAllocationFrameGen = 1.
        self.percentAllocationEncode = 0.

        # Compute engine properties.
        self.useSessionEngineInformation = 1
        self.engineCommandLineProperties = {}

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
    #   Brad Whitlock, Fri Oct 3 11:16:12 PDT 2003
    #   Added -scriptfile.
    #
    #   Hank Childs, Wed Mar 31 08:44:34 PST 2004
    #   Added -fps.
    #
    #   Brad Whitlock, Thu Apr 22 09:18:20 PDT 2004
    #   Added frameStart and frameEnd.
    #
    #   Brad Whitlock, Tue Aug 3 17:13:25 PST 2004
    #   Added parallel options.
    #
    #   Brad Whitlock, Wed Dec 15 11:23:53 PDT 2004
    #   Added PNG files.
    #
    #   Brad Whitlock, Wed Jun 1 10:13:56 PDT 2005
    #   Added -stereo and support for multiple formats and geometries.
    #
    #   Brad Whitlock, Fri Jun 24 10:25:36 PDT 2005
    #   Added a note about omitting the -geometry flag.
    #
    ###########################################################################

    def PrintUsage(self):
        print "Usage: visit -movie [-format fmt] [-geometry size]"
        print "                    -sessionfile name | -scriptfile name "
        print "                    [-output moviename] [-framestep step]"
        print "                    [PARALLEL OPTIONS]"
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
        print "                       png  : Save raw movie frames as individual"
        print "                              PNG files."
        print ""
        print "                       You can specify multiple formats by concatenating"
        print "                       more than one allowed format using commas. Example:"
        print "                       -format mpeg,qt,jpeg."
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
        print "                       You can specify multiple geometries by concatenating"
        print "                       more than one WxH using commas. Example:"
        print "                       -geometry 320x320,1024x768."
        print ""
        print "                       If you omit the -geometry argument then the"
        print "                       window sizes stored in your session file will be"
        print "                       used. If you are not using a session file then"
        print "                       the default size will be 512x512."
        print ""
        print "    -sessionfile name  The sessionfile option lets you pick the name"
        print "                       of the VisIt session to use as input for your"
        print "                       movie. The VisIt session is a file that describes"
        print "                       the movie that you want to make and it is created"
        print "                       when you save your session from within VisIt's "
        print "                       GUI after you set up your plots how you want them."
        print ""
        print "    -scriptfile name   The scriptfile option lets you pick the name"
        print "                       of a VisIt Python script to use as input for your"
        print "                       movie."
        print ""
        print "    -framestep step    The number of frames to advance when going to "
        print "                       the next frame."
        print ""
        print "    -start frame       The frame that we want to start at."
        print ""
        print "    -end frame         The frame that we want to end at."
        print ""
        print "    -output moviename  The output option lets you set the name of "
        print "                       your movie."
        print ""
        print "    -fps number        Sets the frames per second the movie should "
        print "                       play at."
        print ""
        print "    -stereo            Makes a stereo movie. Note that stereo movies are"
        print "                       only created for Streaming movie format, though "
        print "                       right and left channel images are saved if you"
        print "                       save a movie in an image format."
        print ""
        print "    -ignoresessionengines Prevents compute engine information in the"
        print "                          session file from being used to restart"
        print "                          the compute engine(s) during movie generation."
        print ""
        print "Parallel arguments:"
        print "    -np   <# procs>    The number of processors to use."
        print "    -nn   <# nodes>    The number of nodes to allocate."
        print "    -l    <method>     Launch in parallel using the given method."
        print "                       Method is one of the following: mpirun, poe,"
        print "                       psub, srun."
        print "    -la   <args>       Additional arguments for the parallel launcher."
        print "    -p    <part>       Partition to run in."
        print "    -b    <bank>       Bank from which to draw resources."
        print "    -t    <time>       Maximum job run time."
        print "    -expedite          Makes batch system give priority scheduling."
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
        return #print str

    ###########################################################################
    # Method: SendClientProgress
    #
    # Purpose:    Sends the client progress.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue May 10 11:13:29 PDT 2005
    #
    ###########################################################################

    def SendClientProgress(self, str, current, total):
        if self.sendClientFeedback:
            ClientMethod("MovieProgress", (str, current, total))

    ###########################################################################
    # Method: ClientMessageBox
    #
    # Purpose:    Sends the client MessageBoxOk client method.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue May 10 11:13:29 PDT 2005
    #
    ###########################################################################

    def ClientMessageBox(self, str):
        if self.sendClientFeedback:
            ClientMethod("MessageBoxOk", str)
        else:
            print str

    ###########################################################################
    # Method: DetermineMovieBase
    #
    # Purpose:    This method tries to detect the movie base that should be
    #             used to generate the movie.
    #
    # Programmer: Brad Whitlock
    # Date:       Fri Dec 5 12:22:40 PDT 2003
    #
    # Modifications:
    #
    ###########################################################################

    def DetermineMovieBase(self):
        # Get the file name that we should use
        if(self.usesStateFile == 0):
            fileName = LongFileName(self.scriptFile)
        else:
            fileName = LongFileName(self.stateFile)

        # Try and detect the extension
        extensions = (".session", ".vses", ".VSES", ".VSE", ".py", ".PY")
        extensionLocated = 0
        for ext in extensions:
            pos = string.rfind(fileName, ext)
            if(pos != -1):
                fileName = fileName[:pos]
                extensionLocated = 1
                break

        # If we located an extension then try and look for a path separator.
        if(extensionLocated == 1):
            for separator in ("/", "\\"):
                pos = string.rfind(fileName, separator)
                if(pos != -1):
                    fileName = fileName[pos+1:]
                    break
            # Set the movie base to fileName
            self.movieBase = fileName

    ###########################################################################
    # Method: SplitString
    #
    # Purpose:    This method splits a string using a delimeter.
    #
    # Programmer: Brad Whitlock
    # Date:       Wed Jun 22 14:41:25 PST 2005
    #
    # Modifications:
    #
    ###########################################################################

    def SplitString(self, s, delim):
        retval = []
        tokens = string.split(s, delim)
        for t in tokens:
            if len(t) > 0:
                retval = retval + [t]
        return retval

    ###########################################################################
    # Method: RequestFormat
    #
    # Purpose:    This method tells the MakeMovie class to make another movie
    #             using the specified format and resolution.
    #
    # Programmer: Brad Whitlock
    # Date:       Wed Jun 22 14:41:25 PST 2005
    #
    # Modifications:
    #
    ###########################################################################

    def RequestFormat(self, fmtName, w, h):
        self.Debug("Requesting movie %s at %dx%d" % (fmtName, w, h))
        fmt = self.formatNameToType[fmtName]
        formatString = ""
        self.movieFormats = self.movieFormats + [[formatString, fmt, w, h]]

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
    #   Brad Whitlock, Fri Oct 3 11:25:33 PDT 2003
    #   Added support for -scriptfile.
    #
    #   Brad Whitlock, Fri Dec 5 14:04:09 PST 2003
    #   I made it set the movieBase from the session or script filename
    #   if -output was not given.
    #
    #   Hank Childs, Wed Mar 31 08:44:34 PST 2004
    #   Parse -fps.
    #
    #   Brad Whitlock, Thu Apr 22 09:18:20 PDT 2004
    #   Added frameStart and frameEnd.
    #
    #   Brad Whitlock, Tue Aug 3 16:59:30 PST 2004
    #   Added VisIt's parallel arguments so they can feed into calls to
    #   open a compute engine if they are given.
    #
    #   Brad Whitlock, Wed Dec 15 11:24:51 PDT 2004
    #   Added support for PNG files.
    #
    #   Brad Whitlock, Wed Jun 1 13:51:27 PST 2005
    #   Added support for multiple formats and geometries and for stereo.
    #
    #   Brad Whitlock, Fri Jun 24 10:11:31 PDT 2005
    #   Added support for reading the window sizes out of the session file.
    #
    #   Brad Whitlock, Thu Jan 5 16:41:17 PST 2006
    #   Added code to treat tiff format specially because of the preference
    #   for giving "-format tiff" on the command line but saving files with
    #   ".tif" extension. This lets the option work again.
    #
    ###########################################################################

    def ProcessArguments(self):
        #
        # Scan the command line arguments for -engineargs. If we find it, split
        # it and insert the items into the commandLine list.
        #
        splitEngineArgs = 0
        commandLine = []
        for arg in sys.argv:
            if splitEngineArgs == 1:
                splitEngineArgs = 0
                eargs = string.split(arg, ";")
                for earg in eargs:
                    if len(earg) > 0:
                        commandLine = commandLine + [earg]
            elif arg == "-engineargs":
                splitEngineArgs = 1
            else:
                commandLine = commandLine + [arg]

        # Initially the formats and sizes are set to empty.
        formatList = []
        sizeList = []

        i = 0
        outputSpecified = 0
        processingLA = 0
        while(i < len(commandLine)):
            # If we're processing launch arguments for the parallel launcher
            # then append the argument to the the list of launch args.
            if processingLA == 1:
                self.engineCommandLineProperties["launchArgs"] = \
                self.engineCommandLineProperties["launchArgs"] + [commandLine[i]]
                continue

            # We're processing arguments as usual.
            if(commandLine[i] == "-format"):
                if((i+1) < len(commandLine)):
                    formats = self.SplitString(commandLine[i+1], ",")
                    for format in formats:
                        if(format == "mpeg"):
                            if(sys.platform != "win32"):
                                formatList = formatList + [format]
                            else:
                                self.PrintUsage();
                                sys.exit(-1)
                        elif(format == "qt"):
                            if(sys.platform != "win32"):
                                formatList = formatList + [format]
                            else:
                                self.PrintUsage();
                                sys.exit(-1)
                        elif(format == "sm"):
                            if(sys.platform != "win32"):
                                formatList = formatList + [format]
                            else:
                                self.PrintUsage();
                                sys.exit(-1)
                        elif(format == "tiff" or format == "tif"):
                            formatList = formatList + ["tiff"]
                        else:
                            found = 0
                            format2 = "." + format
                            # Look for the format in the list of supported formats.
                            for k in self.formatExtension.keys():
                                if self.formatExtension[k] == format2:
                                    formatList = formatList + [format]
                                    found = 1
                                    break;
                            if(found == 0):
                                self.PrintUsage()
                                sys.exit(-1)
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-sessionfile"):
                if((i+1) < len(commandLine)):
                    self.stateFile = commandLine[i+1]
                    self.usesStateFile = 1
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-scriptfile"):
                if((i+1) < len(commandLine)):
                    self.scriptFile = commandLine[i+1]
                    self.usesStateFile = 0
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-framestep"):
                if((i+1) < len(commandLine)):
                    try:
                        self.frameStep = int(commandLine[i+1])
                        if(self.frameStep < 1):
                            self.frameStep = 1
                    except ValueError:
                        self.frameStep = 1
                        print "A bad value was provided for frame step. Using a frame step of 1."
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-start"):
                if((i+1) < len(commandLine)):
                    try:
                        self.frameStart = int(commandLine[i+1])
                        if(self.frameStart < 0):
                            self.frameStart = 0
                    except ValueError:
                        self.frameStart = 0
                        print "A bad value was provided for frame start. Using a frame step of 0."
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-end"):
                if((i+1) < len(commandLine)):
                    try:
                        self.frameEnd = int(commandLine[i+1])
                        if(self.frameEnd < 0):
                            self.frameEnd = 0
                    except ValueError:
                        self.frameEnd = 0
                        print "A bad value was provided for frame end. Using a frame end of 0."
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-output"):
                if((i+1) < len(commandLine)):
                    self.movieBase = commandLine[i+1]
                    outputSpecified = 1
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-fps"):
                if((i+1) < len(commandLine)):
                    try:
                        self.fps = int(commandLine[i+1])
                        if(self.fps < 1):
                            self.frameStep = 10
                    except ValueError:
                        self.fps = 10
                        print "A bad value was provided for frames per second.  Using 10."
                    i = i + 1
                else:
                    self.PrintUsage()
            elif(commandLine[i] == "-geometry"):
                if((i+1) < len(commandLine)):
                    geometries = self.SplitString(commandLine[i+1], ",")
                    for geometry in geometries:
                        xloc = geometry.find("x")
                        if(xloc != -1):
                            w = int(geometry[:xloc])
                            h = int(geometry[xloc+1:])
                            sizeList = sizeList + [(w, h)]
                        else:
                            print "Malformed geometry string: %s" % geometry
                            sys.exit(-1)
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-stereo"):
                self.stereo = 1
            elif(commandLine[i] == "-ignoresessionengines"):
                self.useSessionEngineInformation = 0

            #
            # Parallel engine options.
            #
            elif(commandLine[i] == "-np"):
                if((i+1) < len(commandLine)):
                    try:
                        np = int(commandLine[i+1])
                        if(np < 1):
                            print "A bad value was provided for number of processors."
                            self.PrintUsage()
                            sys.exit(-1)
                        self.engineCommandLineProperties["numProcessors"] = np
                    except ValueError:
                        print "A bad value was provided for number of processors."
                        self.PrintUsage()
                        sys.exit(-1)
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-nn"):
                if((i+1) < len(commandLine)):
                    try:
                        nn = int(commandLine[i+1])
                        if(nn < 1):
                            print "A bad value was provided for number of nodes."
                            self.PrintUsage()
                            sys.exit(-1)
                        self.engineCommandLineProperties["numNodes"] = nn
                        self.engineCommandLineProperties["numNodesSet"] = 1
                    except ValueError:
                        print "A bad value was provided for number of nodes."
                        self.PrintUsage()
                        sys.exit(-1)
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-b"):
                if((i+1) < len(commandLine)):
                    self.engineCommandLineProperties["bank"] = commandLine[i+1]
                    self.engineCommandLineProperties["bankSet"] = 1
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-p"):
                if((i+1) < len(commandLine)):
                    self.engineCommandLineProperties["partition"] = commandLine[i+1]
                    self.engineCommandLineProperties["partitionSet"] = 1
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-t"):
                if((i+1) < len(commandLine)):
                    self.engineCommandLineProperties["timeLimit"] = commandLine[i+1]
                    self.engineCommandLineProperties["timeLimitSet"] = 1
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-l"):
                if((i+1) < len(commandLine)):
                    self.engineCommandLineProperties["launchMethod"] = commandLine[i+1]
                    self.engineCommandLineProperties["launchMethodSet"] = 1
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-la"):
                self.engineCommandLineProperties["launchArgs"] = []
                self.engineCommandLineProperties["launchArgsSet"] = 1
                processingLA = 1
            elif(commandLine[i] == "-expedite"):
                if "arguments" in self.engineCommandLineProperties.keys():
                    self.engineCommandLineProperties["arguments"] = self.engineCommandLineProperties["arguments"] + ["-expedite"]
                else:
                    self.engineCommandLineProperties["arguments"] = ["-expedite"]

            # On to the next argument.
            i = i + 1

        # If the formatList or sizeList were not specified, give them 
        # default values.
        if len(formatList) == 0:
            formatList = ["mpeg"]
        if len(sizeList) == 0:
            if self.stateFile != "":
                # No -geometry flag was specified but we're using a session file
                # so let's parse the sizes out of the session file.
                windowInfo = self.ReadWindowSizes(self.stateFile)
                activeWindow = windowInfo[0]
                windowSizes  = windowInfo[1]
                if len(windowSizes) > 0:
                    if activeWindow >= len(windowSizes):
                        activeWindow = 0
                    sizeList = [tuple(windowSizes[activeWindow])]
                else:
                    sizeList = [(512, 512)]
            else:
                sizeList = [(512, 512)]

        # If the formatList and sizeList are not the same size, make them 
        # be the same size by repeating the last value in the shorter of the
        # lists until the lists are the same length.
        if len(formatList) != len(sizeList):
            if len(formatList) > len(sizeList):
                lastValue = sizeList[-1]
                for i in range(len(formatList) - len(sizeList)):
                    sizeList = sizeList + [lastValue]
            else:
                lastValue = formatList[-1]
                for i in range(len(sizeList) - len(formatList)):
                    formatList = formatList + [lastValue]
        # Now that the formatList and sizeList have been populated by the command
        # line arguments, use the information stored therein to request movie
        # formats.
        for i in range(len(formatList)):
            self.RequestFormat(formatList[i], sizeList[i][0], sizeList[i][1])

        # Make sure that the user provided the -sessionfile option or we can't
        # make a movie!
        if(self.stateFile == "" and self.scriptFile == ""):
            self.PrintUsage()
            print "You must provide the -sessionfile or the -scriptfile option!"
            sys.exit(-1)

        # If no movie output was specified, try and determine the output
        # base name from the session file or the script file.
        if(outputSpecified == 0):
            self.DetermineMovieBase()


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
    # Modifications:
    #   Brad Whitlock, Tue May 31 14:44:48 PST 2005
    #   Determines whether a temporary directory needs to be created. Changed
    #   the name of the temp directory.
    #
    ###########################################################################

    def CreateTemporaryDirectory(self):
        retval = os.curdir

        # Determine if any movie formats get encoded.
        any_formats_get_encoded = 0
        for format in self.movieFormats:
            fmt = format[1]
            if self.outputNeedsInput[fmt] != fmt:
                any_formats_get_encoded = 1
                break

        if any_formats_get_encoded == 1:
            # Determine the name of the directory to use.
            fileName = self.movieBase
            for separator in ("/", "\\"):
                pos = string.rfind(self.movieBase, separator)
                if(pos != -1):
                    fileName = self.movieBase[pos+1:]
                    break
            index = 0
            d = "%s-%d" % (fileName, index)
            files = os.listdir(os.curdir)
            self.Debug("CreateTemporaryDirectory: d=" + str(d))
            self.Debug("CreateTemporaryDirectory: files=" + str(files))
            while(d in files):
                self.Debug("The directory %s already exists, we'll try another name." % d)
                index = index + 1
                d = "%s-%d" % (fileName, index)
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
    # Modifications:
    #   Brad Whitlock, Tue Aug 24 09:32:39 PDT 2004
    #   I set the outputToCurrentDirectory flag to true because on Windows,
    #   the default is to output files to a specified directory, which is not
    #   what we want when saving movie frames.
    #
    #   Mark C. Miller, Mon Dec 13 16:35:42 PST 2004
    #   Added try/catch protection for SaveWindow
    #
    #   Brad Whitlock, Wed Dec 15 11:25:27 PDT 2004
    #   Added support for PNG files.
    #
    #   Hank Childs, Fri Mar  4 15:04:29 PST 2005
    #   Save a tiled window if the session has tiled windows.
    #
    #   Brad Whitlock, Tue May 10 09:05:35 PDT 2005
    #   Made it a little more general.
    #
    ###########################################################################

    def SaveImage(self, index):
        s = SaveWindowAttributes()
        old_sw = GetSaveWindowAttributes()
        s.saveTiled = old_sw.saveTiled
        s.family = 0
        s.screenCapture = self.screenCaptureImages
        s.stereo = old_sw.stereo
        if self.stereo == 1:
            s.stereo = 1

        # Save the frame in the required formats.
        retval = 1
        for format in self.movieFormats:
            # Create a filename
            frameFormatString = format[0]
            s.fileName = frameFormatString % index

            # Determine the format of the frame that VisIt needs to save.
            frameFormat = self.outputNeedsInput[format[1]]
            if(frameFormat == self.FORMAT_PPM):
                s.format =  s.PPM
            elif(frameFormat == self.FORMAT_TIFF):
                s.format =  s.TIFF
            elif(frameFormat == self.FORMAT_JPEG):
                s.format =  s.JPEG
            elif(frameFormat == self.FORMAT_BMP):
                s.format =  s.BMP
            elif(frameFormat == self.FORMAT_RGB):
                s.format =  s.RGB
            elif(frameFormat == self.FORMAT_PNG):
                s.format =  s.PNG

            s.width = format[2]
            s.height = format[3]
            s.outputToCurrentDirectory = 1
            SetSaveWindowAttributes(s)
            try:
                retval = SaveWindow()
            except VisItInterrupt:
                raise
            except:
                retval = 0

        # Restore the old save window atts.
        SetSaveWindowAttributes(old_sw)

        return retval 

    ###########################################################################
    # Method: SaveImage2
    #
    # Purpose:    This method tells VisIt to save an image.
    #
    # Notes:      This method is called when we save an image from a Python
    #             script.
    #
    # Programmer: Brad Whitlock
    # Date:       Fri Oct 3 12:45:30 PDT 2003
    #
    # Modifications:
    #   Brad Whitlock, Tue May 31 14:16:11 PST 2005
    #   I removed the ext argument from SaveImage.
    #
    ###########################################################################

    def SaveImage2(self):
        retval = self.SaveImage(self.numFrames)
        self.numFrames = self.numFrames + 1
        return retval

    ###########################################################################
    # Method: LookupImageExtension
    #
    # Purpose:    This method returns the file extension associated with the
    #             specified output image type.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    # Modifications:
    #   Brad Whitlock, Wed Dec 15 11:26:47 PDT 2004
    #   Added support for PNG files.
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
        elif(fmt == self.OUTPUT_PNG):
            retval = "png"
        return retval

    ###########################################################################
    # Method: CreateMangledSource
    #
    # Purpose:    This method reads in the Python file that we want to execute
    #             and creates an alternate version in which all of the
    #             SaveWindow method calls have been replaced with
    #             MovieClassSaveWindow.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    ###########################################################################
    
    def CreateMangledSource(self, name):
        import string

        # Read in the Python file that we want to source. 
        f = open(name, "r")
        lines = f.readlines()
        f.close()

        #
        # Create a new Python file and save modified source where every call
        # to VisIt's SaveWindow() method gets replaced by MovieClassSaveWindow()
        # which causes the script to save the image using the name and format
        # prescribed by this movie framework.
        #
        newname = "%s.mangled" % name
        f = open(newname, "w")
        for line in lines:
            index = string.find(line, "SaveWindow()")
            if index == -1:
                f.write(line)
            else:
                before = line[:index]
                after = line[index+len("SaveWindow()")]
                f.write(before + "MovieClassSaveWindow()" + after)
        f.close()

        return newname

    ###########################################################################
    # Method: ReadEngineProperties
    #
    # Purpose:    This method reads the specified session file and extracts
    #             host profile information about the engines that were running
    #             when the session file was saved. That information is then
    #             partially overridden by any parallel options that were passed
    #             on the command line and then the entire dictionary of
    #             properties for each host is returned.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Aug 3 17:27:53 PST 2004
    #
    # Modifications:
    #   Brad Whitlock, Tue Jun 21 11:10:47 PDT 2005
    #   I made it re-raise VisItInterrupt if it gets one.
    #
    ###########################################################################

    def ReadEngineProperties(self, sessionfile):   
        try:
            # Read the file.
            f = open(sessionfile, "r")
            lines = f.readlines()
            f.close()

            # Parse the file
            p = EngineAttributesParser()
            for line in lines:
                p.feed(line)
            p.close()

            if len(self.engineCommandLineProperties.keys()) == 0:
                # There were no hosts in the engine attributes so add the command
                # line options for localhost.
                p.allEngineProperties["localhost"] = self.engineCommandLineProperties
            else:
                # Override the EngineAttributesParser's engine attributes
                # with options that were provided on the command line.
                for host in p.allEngineProperties.keys():
                    dest = p.allEngineProperties[host]
                    for key in self.engineCommandLineProperties.keys():
                        dest[key] = self.engineCommandLineProperties[key]
            
            return p.allEngineProperties
        except VisItInterrupt:
            raise
        except:
            return {}

    ###########################################################################
    # Method: ReadWindowSizes
    #
    # Purpose:    This method reads the ViewerWindow sizes out of a session
    #             file and returns the list of sizes.
    #
    # Programmer: Brad Whitlock
    # Date:       Fri Jun 24 10:06:24 PDT 2005
    #
    ###########################################################################

    def ReadWindowSizes(self, sessionFile):
        try:
            # Read the file.
            f = open(sessionFile, "r")
            lines = f.readlines()
            f.close()

            # Parse the file
            p = WindowSizeParser()
            for line in lines:
                p.feed(line)
            p.close()
            
            return (p.activeWindow, p.windowSizes)
        except VisItInterrupt:
            raise
        except:
            return (0, [])

    ###########################################################################
    # Method: CreateEngineArguments
    #
    # Purpose:    This method converts the compute engine properties in a 
    #             dictionary into a tuple of command line arguments that can
    #             be passed to OpenComputeEngine.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Aug 3 17:27:45 PST 2004
    #
    ###########################################################################

    def CreateEngineArguments(self, engineProperties):
        arguments = []
        if "numProcessors" in engineProperties.keys():
            np = engineProperties["numProcessors"]
            if np > 1:
                arguments = arguments + ["-par", "-np", "%d" % np]

                if "numNodesSet" in engineProperties.keys() and\
                   "numNodes" in engineProperties.keys():
                    nn = engineProperties["numNodes"]
                    if engineProperties["numNodesSet"] == 1 and nn > 0:
                        arguments = arguments + ["-nn", "%d" % nn]

                if "partitionSet" in engineProperties.keys() and\
                   "partition" in engineProperties.keys():
                    if engineProperties["partitionSet"] == 1:
                        arguments = arguments + ["-p", engineProperties["partition"]]
    
                if "bankSet" in engineProperties.keys() and\
                   "bank" in engineProperties.keys():
                    if engineProperties["bankSet"] == 1:
                        arguments = arguments + ["-b", engineProperties["bank"]]

                if "timeLimitSet" in engineProperties.keys() and\
                   "timeLimit" in engineProperties.keys():
                    if engineProperties["timeLimitSet"] == 1:
                        arguments = arguments + ["-t", engineProperties["timeLimit"]]

                if "launchMethodSet" in engineProperties.keys() and\
                   "launchMethod" in engineProperties.keys():
                    if engineProperties["launchMethodSet"] == 1:
                        arguments = arguments + ["-l", engineProperties["launchMethod"]]

                if "launchArgsSet" in engineProperties.keys() and\
                   "launchArgs" in engineProperties.keys():
                    if engineProperties["launchArgsSet"] == 1:
                        args = engineProperties["launchArgs"]
                        s = ""
                        for arg in args:
                            s = s + arg + " "
                        arguments = arguments + ["-la", s]

        arguments = arguments + ["-forcestatic"]

        if "timeout" in engineProperties.keys():
            arguments = arguments + ["-timeout", "%d" % engineProperties["timeout"]]

        if "arguments" in engineProperties.keys():
            arguments = arguments + engineProperties["arguments"]

        return tuple(arguments)

    ###########################################################################
    # Method: IterateAndSaveFrames
    #
    # Purpose:    This method tells VisIt to iterate over each frame and save
    #             the necessary images.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon May 9 17:52:22 PST 2005
    #
    # Modifications:
    #
    ###########################################################################
    def IterateAndSaveFrames(self):
        self.Debug("IterateAndSaveFrames:")

        # Make sure that camera view mode is on so that if we have a
        # saved keyframe animation, we really get the keyframed view.
        if(GetWindowInformation().cameraViewMode == 0):
            ToggleCameraViewMode()

        # Figure out a good value for the frameEnd.
        if self.frameEnd == -1:
            self.frameEnd = TimeSliderGetNStates() - 1
        # If the start and end are reversed, put them in the right order.
        if self.frameEnd < self.frameStart:
            tmp = self.frameEnd
            self.frameEnd = self.frameStart
            self.frameStart = tmp

        self.Debug("*** frameStart=%d, frameEnd=%d" % (self.frameStart, self.frameEnd))

        # Save an image for each frame in the animation.
        self.numFrames = 0
        i = self.frameStart
        drawThePlots = 0
        nTotalFrames = self.frameEnd - self.frameStart + 1
        lastProgress = -1
        while(i <= self.frameEnd):
            t = float(i - self.frameStart) / float(nTotalFrames);
            progress = int(t * self.percentAllocationFrameGen * 100.)
            if progress != lastProgress:
                self.SendClientProgress("Generating frames", progress, 100)
                lastProgress = progress

            if(SetTimeSliderState(i) == 0):
                drawThePlots = 1
                print "There was an error when trying to set the "\
"time slider state to %d. VisIt will now try to redraw the plots." % i
            if(drawThePlots):
                if(DrawPlots() == 0):
                    print "VisIt could not draw plots for time slider "\
"state %d. You should investigate the files used for that state." % i
                else:
                    drawThePlots = 0
            if self.SaveImage(self.numFrames) == 0:
                print "There was an error when trying to save the window "\
"for time slider state %d. VisIt will now try to redraw the plots and re-save "\
"the window." % i
                if(DrawPlots() == 0):
                    print "VisIt could not draw plots for time slider "\
"state %d. You should investigate the files used for that state." % i
                else:
                    if self.SaveImage(self.numFrames) == 0:
                        print "VisIt could not re-save the window for "\
"time slider state %d. You should investigate the files used for that state." % i

            self.numFrames = self.numFrames + 1
            i = i + self.frameStep

    ###########################################################################
    # Method: GenerateFrames
    #
    # Purpose:    This method tells VisIt to generate the frames for the movie.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    # Modifications:
    #   Brad Whitlock, Fri Oct 3 11:27:50 PDT 2003
    #   Changed to allow scripts to save images. I also forced camera view
    #   mode to be enabled so keyframe animations that change the view actually
    #   save images that have a keyframed view.
    #
    #   Brad Whitlock, Tue Feb 24 11:57:39 PDT 2004
    #   If there was a problem when trying to set the animation frame,
    #   try and draw the plots again.
    #
    #   Brad Whitlock, Tue Mar 2 10:55:33 PDT 2004
    #   I changed the animation methods so they animate using the active
    #   time slider.
    #
    #   Brad Whitlock, Thu Apr 22 09:25:06 PDT 2004
    #   I added support for using a user-specified start, end frame.
    #
    #   Brad Whitlock, Tue Aug 3 16:51:36 PST 2004
    #   I added support for getting parallel options from a session file and
    #   using those options to launch compute engines.
    #
    #   Mark C. Miller, Mon Dec 13 16:35:42 PST 2004
    #   Added fault tolerance for engine failures in SR mode during SaveWindow
    #
    #   Brad Whitlock, Mon May 9 17:07:21 PST 2005
    #   Moved some code to the IterateAndSaveFrames method. Added code to make
    #   sure that each movie format has a valid format string.
    #
    #   Brad Whitlock, Mon Oct 31 12:05:14 PDT 2005
    #   Moved the code around for the stateFile case so that we launch compute
    #   engines before restoring the session file. The viewer does not try to
    #   use the list of compute engines stored in the session file so it is
    #   up to this script to launch the right parallel compute engines before
    #   restoring the session file.
    #
    ###########################################################################

    def GenerateFrames(self):
        self.Debug("GenerateFrames")

        # Make sure that the movieFormats list is not empty
        if len(self.movieFormats) < 1:
            self.ClientMessageBox("No formats were requested so no movie will be made.")
            return

        # Create a temporary directory in which to store the frames if we're
        # generating a movie that ends up as a single file.
        self.tmpDir = self.CreateTemporaryDirectory()

        # If we have any movies that need to get encoded, make sure that we allot
        # some time in the progress to encoding.
        for format in self.movieFormats:
            fmt = format[1]
            if(fmt == self.FORMAT_MPEG or fmt == self.FORMAT_QT or fmt == self.FORMAT_SM):
                self.percentAllocationFrameGen = 0.9
                self.percentAllocationEncode = 0.1

        # Create file format strings for the frames.
        for format in self.movieFormats:
            filedir = ""
            filebase = self.movieBase
            fmt = format[1]

            # If the string begins with a slash then append the tmpDir to it.
            if self.movieBase[0] == self.slash[0] or \
               string.find(self.movieBase, ":\\") != -1:
                slashPos = string.rfind(self.movieBase, self.slash)
                filedir = self.movieBase[:slashPos+1]
                filebase = self.movieBase[slashPos+1:]
                if fmt != self.outputNeedsInput[fmt]:
                    filedir = filedir + self.tmpDir + self.slash

            if len(self.movieFormats) > 1:
                w = format[2]
                h = format[3]
                filebase = "%s_%dx%d_%%04d" % (filebase, w, h)
            else:
                filebase = "%s%%04d" % filebase
            # Store the file format string in the movieFormats list.
            format[0] = filedir + filebase

        if(self.usesTemplateFile):
            self.Debug("GenerateFrames: using template file %s" % self.templateFile)
            # Source the template file to get the template's class definition
            # and the InstantiateMovieTemplate function.
            Source(self.templateFile)

            #template = InstantiateMovieTemplate()
            #template
        elif(self.usesCurrentPlots):
            self.Debug("GenerateFrames: using current plots")

            # Get the current time slider's state
            tsState = -1
            windowInfo = GetWindowInformation()
            if windowInfo.activeTimeSlider >= 0:
                tsState = windowInfo.timeSliderCurrentStates[windowInfo.activeTimeSlider]

            # Use the plots that are currently set up. Iterate over all of
            # the frames and save them out.
            self.IterateAndSaveFrames()

            # Restore the time slider's state.
            if tsState != -1:
                SetTimeSliderState(tsState)
        elif(self.usesStateFile):
            self.Debug("GenerateFrames: using session file %s" % self.stateFile)

            # If we're using a session file, try and read it to determine which
            # compute engines need to be launched.
            properties = {}
            if self.useSessionEngineInformation == 1:
                properties = self.ReadEngineProperties(self.stateFile)
                self.Debug(properties)

            # Launch any compute engines that were either specified on the
            # command line or in the session file.
            for host in properties.keys():
                if properties[host] != {}:
                    args = self.CreateEngineArguments(properties[host])
                    self.Debug("Starting compute engine: %s" % host)
                    OpenComputeEngine(host, args)

            # Make the viewer try and restore its session using the file that
            # was passed in.
            RestoreSession(self.stateFile, 0)

            # Make sure that plots are all drawn.
            DrawPlots()

            # Iterate over all of the frames and save them out.
            self.IterateAndSaveFrames()
        else:
            self.Debug("GenerateFrames: using Python file %s" % self.scriptFile)
            # Save the callback data so we can call methods on this object.
            globals()['classSaveWindowObj'] = self
            # Create a modified version of the user's script.
            name = self.CreateMangledSource(self.scriptFile)
            # Try executing the modified version of the user's script.
            Source(name)
            # Remove the modified script.
            RemoveFile(name);

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
    #   Hank Childs, Sat Mar 27 11:51:10 PST 2004
    #   Don't do such aggressive compression.
    #
    #   Hank Childs, Wed Mar 31 08:50:48 PST 2004
    #   Backed off compression even more.  Allow for fps to be set as well.
    #
    #   Brad Whitlock, Mon Apr 26 16:42:39 PST 2004
    #   I moved the code to symlink files into a method that copies files or
    #   symlinks them depending on what the operating system can do.
    #
    #   Brad Whitlock, Tue May 10 11:36:53 PDT 2005
    #   I changed the return values so more information gets returned. Passed
    #   in imageFormatString, xres, yres.
    #
    ###########################################################################

    def EncodeMPEGMovie(self, moviename, imageFormatString, xres, yres):
        self.Debug("EncodeMPEGMovie")

        # Tell the user about MPEGs on Windows.
        if os.name == "nt":
            s =     "The Windows version of VisIt does not come with an MPEG\n"
            s = s + "encoding program. You can use your own MPEG software to\n"
            s = s + "make a movie from the frames in:\n %s." % self.tmpDir
            return (0, moviename, s)

        retval = 0
        paramFile = "%s-mpeg_encode-params" % self.movieBase
        try:
            # We can only drive the movie at 30 fps.  So if the user wants
            # a movie at 10 fps, we have to pad frames.  Determine what that
            # pad rate is.
            pad_rate=-1
            for i in range(1,31):
               if (pad_rate < 0):
                  if ((30./i) <= self.fps):
                     pad_rate = i
                     if ((30./i) != self.fps):
                         print "Because of limitations in MPEG encoding, the "
                         print "movie will be encoded at %f frames per second" %(30./i)

            # Now create symbolic links to the images at that pad rate.
            linkbase = "mpeg_link"
            linkindex = 0
            doSymlink = "symlink" in dir(os)
            for i in range(self.numFrames):
                imgname = imageFormatString % i
                imgname = imgname + ".ppm"
                for j in range(pad_rate):
                    linkname = self.tmpDir+self.slash+linkbase+"%04d.ppm"%(linkindex)
                    linkindex = linkindex + 1
                    CopyFile(imgname, linkname, doSymlink)

            nframes=pad_rate*self.numFrames

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
            f.write("IQSCALE             1\n")
            f.write("PQSCALE             1\n")
            f.write("BQSCALE             1\n")
            f.write("REFERENCE_FRAME     DECODED\n")
            f.write("FORCE_ENCODE_LAST_FRAME\n")
            f.write("YUV_SIZE            %dx%d\n" % (xres, yres))
            f.write("INPUT_CONVERT       *\n")
            f.write("INPUT_DIR           %s\n" % self.tmpDir)
            f.write("INPUT\n")
            f.write("%s*.ppm             [0000-%04d]\n" % (linkbase, nframes-1))
            f.write("END_INPUT\n")
            f.close();

            # Create the movie
            command = "visit -v %s -mpeg_encode %s" % (Version(), paramFile)
            r = os.system(command)

            # Remove the param file.
            RemoveFile(paramFile);

            # Remove the symbolic links.
            linkindex = 0
            for i in range(self.numFrames):
                for j in range(pad_rate):
                   linkname=self.tmpDir+self.slash+linkbase+"%04d.ppm"%(linkindex)
                   RemoveFile(linkname)
                   linkindex = linkindex + 1

            if(r == 0):
                retval = (1, moviename, "")
            else:
                s =     "VisIt's mpeg_encode program was not successful. No MPEG \n"
                s = s + "movie was created. You can access the raw source frames\n"
                s = s + "in: %s." % self.tmpDir
                retval = (0, moviename, s)
        except IOError:
            s =     "VisIt could not create an MPEG parameter file! No MPEG \n"
            s = s + "movie will be created. You can access the raw source \n"
            s = s + "frames in: %s." % self.tmpDir
            retval = (0, moviename, s)

        return retval

    ###########################################################################
    # Method: GetDMConvertSuccess
    #
    # Purpose:    This method determines whether dmconvert ran successfully.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 25 16:14:42 PST 2005
    #
    # Modifications:
    #
    ###########################################################################

    def GetDMConvertSuccess(self, retval):
        success = 1
        if type(retval) == type([]):
            for v in retval:
                if v == 256:
                    success = 0
        elif type(retval) == type(0):
            if retval == 256:
                success = 0

        return success

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
    # Modifications:
    #   Hank Childs, Thu Apr  1 07:48:41 PST 2004
    #   Set frames per second.
    #
    #   Brad Whitlock, Tue May 10 11:22:46 PDT 2005
    #   Made it return more status about the movie encoding process. Passed in
    #   imageFormatString, xres, yres.
    #
    #   Brad Whitlock, Mon Jul 25 16:09:01 PST 2005
    #   Made it more tolerant to failures of the dmconvert command.
    #
    ###########################################################################

    def EncodeQuickTimeMovie(self, moviename, imageFormatString, xres, yres):
        self.Debug("EncodeQuickTimeMovie")
        retval = 0

        if(CommandInPath("dmconvert")):
            # Determine the image extension
            ext = self.formatExtension[self.outputNeedsInput[self.FORMAT_QT]]
            fps = self.fps
            baseFormat = imageFormatString + ext

            # Create small QuickTime movies
            conversionargs = (moviename, baseFormat, xres, yres, fps)
            framesPerMovie = 50
            nframes = self.numFrames
            retval = applyFunctionToNFrames(EncodeQuickTimeMovieHelper, framesPerMovie, nframes, conversionargs)
            msg = ""
            success = self.GetDMConvertSuccess(retval)

            # Glue the submovies together.
            nSubMovies = nframes / framesPerMovie
            if(nSubMovies * framesPerMovie < nframes):
                nSubMovies = nSubMovies + 1
            subMovieFormat = "%s.%%d" % moviename
            if success == 1:
                s = createQuickTimeFile(moviename, subMovieFormat, 0, nSubMovies, xres, yres, fps)
                success = self.GetDMConvertSuccess(s)

            if success == 0:
                msg =        "The command \"dmconvert\" could not create a QuickTime \n"
                msg = msg + "movie. You can still access the frames of your \n"
                msg = msg + "movie in: %s.\n" % self.tmpDir

            # Delete the submovies.
            removeFiles(subMovieFormat, nSubMovies)

            retval = (success, moviename, msg)
        else:
            s =     "The command \"dmconvert\", which is required to make \n"
            s = s + "QuickTime movies is not in your path so your source \n"
            s = s + "frames cannot be automatically converted into a \n"
            s = s + "QuickTime movie. You can, however, still access the \n"
            s = s + "frames of your movie in: %s.\n" % self.tmpDir
            retval = (0, moviename, s)

        return retval

    ###########################################################################
    # Method: EncodeStreamingMovie
    #
    # Purpose:    This method creates a streaming movie.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    # Modifications:
    #   Hank Childs, Thu Apr  1 07:45:06 PST 2004
    #   Added frames per second.
    #
    #   Brad Whitlock, Tue May 10 11:41:31 PDT 2005
    #   I made it pass back more information.
    #
    #   Brad Whitlock, Mon Jul 25 15:19:15 PST 2005
    #   I made it use the extension for the filename since it did not have
    #   that and was not working.
    #
    ###########################################################################

    def EncodeStreamingMovie(self, moviename, imageFormatString):
        self.Debug("EncodeStreamingMovie")
        retval = 0

        if(CommandInPath("img2sm")):
            # Determine the image extension
            ext = self.formatExtension[self.outputNeedsInput[self.FORMAT_SM]]

            # Execute the img2sm command
            command = "img2sm -rle -FPS %d -first 0 -last %d -form png %s%s %s" % \
                      (self.fps, self.numFrames-1, imageFormatString, ext, moviename)
            self.Debug(command)
            r = os.system(command)

            if(r == 0):
                retval = (1, moviename, "")
            else:
                s =     "The img2sm program was not successful. No streaming movie\n"
                s = s + "was created. You can access the raw source frames in:\n"
                s = s + "%s." % self.tmpDir
                retval = (0, moviename, s)
        else:
            s =     "The command \"img2sm\", which is required to make \n"
            s = s + "streaming movies, is not in your path so your source \n"
            s = s + "frames cannot be automatically converted into a streaming\n"
            s = s + "movie. You can, however, still access the frames of your \n"
            s = s + "movie in: %s.\n" % self.tmpDir
            retval = (0, moviename, s)

        return retval

    ###########################################################################
    # Method: MakeMovieName
    #
    # Purpose:    This method returns a movie filename.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue May 31 17:19:20 PST 2005
    #
    # Modifications:
    #
    ###########################################################################

    def MakeMovieName(self, fmt, xres, yres, useGeom):
        # Make sure that the movie
        filebase = self.movieBase
        moviename = filebase
        if useGeom == 1:
            filebase = "%s_%dx%d" % (self.movieBase, xres, yres)
        formatExt = self.formatExtension[fmt]
        ext = filebase[-len(formatExt):]
        if(ext != formatExt):
            moviename = filebase + self.formatExtension[fmt]
        return moviename

    ###########################################################################
    # Method: CreateMovieName
    #
    # Purpose:    This method calls returns a suitable movie filename.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue May 31 17:19:20 PST 2005
    #
    # Modifications:
    #
    ###########################################################################

    def CreateMovieName(self, fmt, xres, yres):
        moviename = self.MakeMovieName(fmt, xres, yres, 0)
        movienames = {}
        for format in self.movieFormats:
            f = format[1]
            xr = format[2]
            yr = format[3]
            name = self.MakeMovieName(f, xr, yr, 0)
            if name in movienames.keys():
                movienames[name] = movienames[name] + 1
            else:
                movienames[name] = 1
        if movienames[moviename] > 1:
            moviename = self.MakeMovieName(fmt, xres, yres, 1)

        return moviename

    ###########################################################################
    # Method: EncodeFrames
    #
    # Purpose:    This method calls the appropriate method to encode frames
    #             into a movie file.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    # Modifications:
    #   Brad Whitlock, Tue May 10 11:50:14 PDT 2005
    #   I made it give some feedback to the user about how the encoding went.
    #   I also made it iterate over the movies that have to be encoded.
    #
    ###########################################################################

    def EncodeFrames(self):
        self.Debug("EncodeFrames")

        # Count the number of movies that have to get encoded.
        nEncodedMovies = 0
        for format in self.movieFormats:
            fmt = format[1]
            if(fmt == self.FORMAT_MPEG or fmt == self.FORMAT_QT or fmt == self.FORMAT_SM):
               nEncodedMovies = nEncodedMovies + 1

        # Send a "stage transition" by changing the text.
        if nEncodedMovies > 0:
            progress = int(self.percentAllocationFrameGen * 100.)
            self.SendClientProgress("Encoding movie", progress, 100)

        safeToRemoveFrames = 1
        count = 0
        framesGenerated = 0
        framesGeneratedMessage = ""
        for format in self.movieFormats:
            formatString = format[0]
            fmt = format[1]
            xres = format[2]
            yres = format[3]
            val = 0

            # Come up with a movie name.
            movieName = self.CreateMovieName(fmt, xres, yres)

            # Encode the movie if needed.
            encodeMovie = 0
            if(fmt == self.FORMAT_MPEG):
                val = self.EncodeMPEGMovie(movieName, formatString, xres, yres)
                encodeMovie = 1
            elif(fmt == self.FORMAT_QT):
                val = self.EncodeQuickTimeMovie(movieName, formatString, xres, yres)
                encodeMovie = 1
            elif(fmt == self.FORMAT_SM):
                val = self.EncodeStreamingMovie(movieName, formatString)
                encodeMovie = 1

            # If we encoded a movie, send the client some progress.
            if encodeMovie:
                count = count + 1
                t = float(count) / float(nEncodedMovies)
                t2 = self.percentAllocationFrameGen + t * self.percentAllocationEncode
                progress = int(t2 * 100.)
                self.SendClientProgress("Encoding movie", progress, 100)

            # Tell the user about how the movie generation went.
            if val == 0:
                s = "VisIt completed generating frames."
                filedir = format[0]
                slashPos = string.rfind(filedir, self.slash)
                if slashPos != -1:
                    filedir = filedir[:slashPos]
                    s = s + " They are located at: \n%s." % filedir
                framesGenerated = 1
                framesGeneratedMessage = s
            elif val[0] == 0:
                s =     "VisIt was not able to complete your movie:\n"
                s = s + "%s because of the following problem:\n\n" % val[1]
                s = s + val[2]
                self.ClientMessageBox(s)
                safeToRemoveFrames = 0
            else:
                s =     "VisIt completed your movie. It is located at: \n"
                s = s + "%s." % val[1]
                self.ClientMessageBox(s)

        # Tell the user that frames were generated.
        if framesGenerated:
            self.ClientMessageBox(framesGeneratedMessage)

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


