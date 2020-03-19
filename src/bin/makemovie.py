# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.


import os
import select
import signal
import socket
import string
import sys

if (sys.version_info > (3, 0)):
    import _thread
else:
    import thread as _thread

import distutils.spawn
from xmllib import *

from visit_utils import encoding

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
    else:
        if distutils.spawn.find_executable(command) != None:
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
                _thread.start_new(applyFunctionToNFramesWMutex, (threadID, exitmutex, retval, func, start, end, conversionargs))
                start = start + filesPerThread
                threadID = threadID + 1
            # Do the first part of the work on the main thread.
            applyFunctionToNFramesWMutex(0, exitmutex, retval, func, 0, filesPerThread, conversionargs)

            # Wait for all of the threads to complete
            while 0 in exitmutex: pass
        except ImportError:
            print("Could not import threads module.")
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
#   Kathleen Bonnell, Tue Jul  3 20:02:45 PDT 2007 
#   Put quotes around srcName and destName on NT, in case there are spaces. 
#   In call to symlink, changed imgname to srcName and linkname to destName.
#
###############################################################################

def CopyFile(srcName, destName, allowLinks):
    fileCopied = 0

    # If we want to create a symlink instead of copying, try it now.
    if allowLinks:
        try:
            os.symlink(srcName, destName)
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
                os.system("copy " + '"' + srcName + '" "' + destName + '"')
            else:
                os.system("cp %s %s" % (srcName, destName))
        except VisItInterrupt:
            raise
        except:
            print("Could not copy %s to %s" % (srcName, destName))


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
# Function: visit_pipe
#
# Purpose:    This function is acts like os.popen except that instead of
#             returning a file object, this function returns the exit code
#             of the command and you pass a callback function to process
#             text output from the child process.
#
# Programmer: Brad Whitlock
# Date:       Wed Sep 20 10:19:07 PDT 2006
#
# Modifications:
#   Brad Whitlock, Thu Dec 21 19:42:19 PST 2006
#   Fixed for win32.
#
###############################################################################

def visit_pipe(command, line_callback, line_callback_data):
    child_exit = 0

    iterate = 1
    # Install a signal handler.
    def stop_iterating(a,b):
        iterate = 0

    do_fork = 1
    try:
        signal.signal(signal.SIGCHLD, stop_iterating)
    except ValueError:
        # We're not running from the master thread so we should not fork
        # and read the input. We should just call os.system. Unless we can
        # figure out some other way of determining when the child exits
        # instead of using signals.
        do_fork = 0
    except AttributeError:
        # We have a lame signal module that does not have SIGCHLD so
        # we can't do a fork.
        do_fork = 0

    # We're not going to do a fork so just call os.system
    if not do_fork:
        return os.system(command)

    # We're allowed to do fork.
    fd = os.pipe()
    id = os.fork()
    if id == 0:
        # Child
        os.dup2(fd[1], 1)
        os.dup2(fd[1], 2)
        #os.close(2)
        ret = os.system(command)
        sys.exit(ret)
    else:
        # Parent
        s = ""
        while iterate:
            # Wait for input from the child.
            try:
                fd_list = select.select([fd[0]], [], [])
            except select.error:
                iterate = 0
                break
            # If we have a file descriptor to read then read and process data.
            if len(fd_list[0]) > 0:
                # Read child output
                s = s + os.read(fd_list[0][0], 100)

                # Process the child output into lines.
                if s[0] == '\n':
                    if len(s) > 1:
                        s = s[1:]
                    else:
                        s = ""
                        continue
                line = ""
                len_s = len(s)
                index = 0
                for i in range(len_s):
                    if s[index] == '\n':
                        s = s[index:]
                        line_callback(line, line_callback_data)
                        index = 0
                        line = ""
                    else:
                        line = line + s[index]
                    index = index + 1
        try:
            child_result = os.waitpid(id, 0)
        except IOError:
            child_result=(id,0)
        child_exit = child_result[1] / 256
    return child_exit

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
# Purpose:    This class parses session files for MachineProfiles in the
#             "RunningEngines" node so we can extract information about the
#             compute engines that were running when the session file was
#             saved.
#
# Programmer: Brad Whitlock
# Date:       Tue Aug 3 16:11:01 PST 2004
#
# Modifications:
#    Jeremy Meredith, Tue Jul 13 13:22:47 EDT 2010
#    I changed host profiles to be a machine profile + nested launch profile.
#    During this change I noticed the endtag parsing was making use of the
#    latest "open tag", which is incorrect since self.dataName wasn't a stack.
#    I changed it to instead use the explocit booleans we set during starttag,
#    though a viable alternative would be to make dataName a stack of strings.
#
#    Brad Whitlock, Thu Oct 28 11:49:48 PDT 2010
#    I added code to eliminate leading/trailing quotes from string data to
#    work around a problem introduced in version 2.1.
#
###############################################################################

class EngineAttributesParser(XMLParser):
    def __init__(self):
        XMLParser.__init__(self)
        self.elements = {"Object" : ("<Object>", "</Object>"), "Field" : ("<Field>", "</Field>")}
        self.attributes = {"name" : "", "type" : None, "length" : 0}

        self.readingEngineProperties = 0
        self.readingMachineProfile = 0
        self.readingLaunchProfile = 0
        self.readingField = 0
        self.engineProperties = {}
        self.allEngineProperties = {}
        self.dataName = None
        self.dataAtts = None


    def handle_starttag(self, tag, method, attributes):
        if tag == "Object":
            if "name" in list(attributes.keys()):
                self.dataName = attributes["name"]
                if self.dataName == "RunningEngines":
                    self.readingEngineProperties = 1
                elif self.dataName == "MachineProfile":
                    if self.readingEngineProperties == 1:
                        self.readingMachineProfile = 1
                elif self.dataName == "LaunchProfile":
                    if self.readingMachineProfile == 1:
                        self.readingLaunchProfile = 1
        else:
            self.readingField = 1
            self.dataAtts = attributes


    def handle_endtag(self, tag, method):
        if tag == "Object":
            if self.readingLaunchProfile == 1:
                self.readingLaunchProfile = 0
            elif self.readingMachineProfile == 1:
                self.readingMachineProfile = 0
                if "host" in list(self.engineProperties.keys()):
                    host = self.engineProperties["host"]
                    self.allEngineProperties[host] = self.engineProperties
                    #self.engineProperties = {}
            elif self.readingEngineProperties == 1:
                self.readingEngineProperties = 0
        elif tag == "Field":
            self.readingField = 0


    def handle_data(self, data):
        def not_all_spaces(s):
            space = ""
            for i in range(len(s)):
                space = space + " "
            return s != space
        def StripLeadingTrailingQuotes(s):
            retval = s
            if len(retval) > 0:
                if retval[0] == '"':
                    retval = retval[1:]
            if len(retval) > 0:
                if retval[-1] == '"':
                    retval = retval[:-1]
            return retval
        if (self.readingEngineProperties == 1 or self.readingMachineProfile)\
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
                value = StripLeadingTrailingQuotes(data)
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
                print("Unknown type: ", type)
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
#   Brad Whitlock, Tue Mar 7 15:55:40 PST 2006
#   I made it use a new windowImageSize field that contains the size of the
#   OpenGL part of the window instead of the windowSize, which is the size of
#   the whole window including the decorations and toolbar.
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
            if "name" in list(attributes.keys()):
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
            elif name == "windowImageSize" and type == "intArray":
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
#   Brad Whitlock, Fri Feb 10 14:18:54 PST 2012
#   I added support for -source command line arguments.
#
#   Brad Whitlock, Wed Apr  3 16:44:08 PDT 2013
#   Use visit_utils for movie encoding.
#
###############################################################################

class MakeMovie(object):
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
    #   Brad Whitlock, Wed Sep 20 11:50:38 PDT 2006
    #   Added debugging logs.
    #
    #   Brad Whitlock, Mon Apr  7 13:22:49 PDT 2008
    #   Added a flag for whether we should use ffmpeg for mpeg encoding.
    #
    #   Brad Whitlock, Fri Feb 10 14:19:17 PST 2012
    #   Set sources to empty list.
    #
    #   Brad Whitlock, Wed Apr  3 16:44:08 PDT 2013
    #   Change various lists to formatInfo dictionary.
    #
    #   Eric Brugger, Tue Oct 22 13:22:35 PDT 2013
    #   Change the movie formats to all use PNG as the input format when
    #   ffmpeg is present and PPM otherwise (for use with mpeg2encode).
    #
    ###########################################################################

    def __init__(self):
        super(MakeMovie,self).__init__()

        # format, extensions, input file type, needs encoding)
        self.FMT_EXTENSIONS = 0
        self.FMT_INPUTFORMAT = 1
        self.FMT_ENCODER = 2
        self.FMT_WINDOWSCOMPATIBLE = 3
        self.FMT_SUPPORTS_STEREO = 4
        self.formatInfo = {
            "ppm"  : [("ppm", ),       "ppm",  None, 1, 0],\
            "tiff" : [("tif", "tiff"), "tiff", None, 1, 0],\
            "jpeg" : [("jpeg", "jpg"), "jpeg", None, 1, 0],\
            "bmp"  : [("bmp",),        "bmp",  None, 1, 0],\
            "rgb"  : [("rgb",),        "rgb",  None, 1, 0],\
            "png"  : [("png",),        "png",  None, 1, 0],\
            "mpeg" : [("mpg", "mpeg"), "png", "mpg", 1, 0],\
            "qt"   : [("mov", "qt"),   "png", "mov", 0, 0],\
            "sm"   : [("sm",),         "png",  "sm", 0, 1]\
        }

        # See if there are any other encoders that we can use.
        for fmt in encoding.encoders():
            if fmt not in ("mpg", "mov", "sm"):
                self.formatInfo[fmt] = [(fmt,), "jpeg", fmt, 0, 0]

        self.STEREO_NONE = 0
        self.STEREO_LEFTRIGHT = 1
        self.STEREO_REDBLUE = 2
        self.STEREO_REDGREEN = 3
        self.stereoNameToType = {\
           "off"       : self.STEREO_NONE, \
           "leftright" : self.STEREO_LEFTRIGHT, \
           "redblue"   : self.STEREO_REDBLUE, \
           "redgreen"  : self.STEREO_REDGREEN, \
        }

        # Set the slash used in filenames based on the platform.
        self.slash = "/"
        if(sys.platform == "win32"):
            self.slash = "\\"

        # Set allow_ffmpeg based on whether it's in the path.
        self.allow_ffmpeg = 0
        if CommandInPath("ffmpeg"):
            self.allow_ffmpeg = 1

        if(self.allow_ffmpeg == 0):
            self.formatInfo["mpeg"][self.FMT_INPUTFORMAT] = "ppm"

        # Movie properties.
        self.stateFile = ""
        self.scriptFile = ""
        self.usesCurrentPlots = 0
        self.usesStateFile = 0
        self.outputDir = os.path.abspath(os.curdir)
        self.movieBase = "movie"
        self.movieFormats = []
        self.numFrames = 0
        self.frameStep = 1
        self.frameStart = 0
        self.frameEnd = -1
        self.tmpDir = os.path.abspath(os.curdir)
        self.fps = 10
        self.initialFrameValue = 0
        self.templateFile = ""
        self.usesTemplateFile = 0
        self.screenCaptureImages = 0
        self.sendClientFeedback = 0
        self.percentAllocationFrameGen = 1.
        self.percentAllocationEncode = 0.
        self.log = 0
        self.debug_real = [0,0,0,0,0]
        self.emailAddresses = ""
        self.engineRestartInterval = 1000000
        self.sources = []
        self.adjustview = 0

        # Compute engine properties.
        self.useSessionEngineInformation = 1
        self.engineCommandLineProperties = {}

        # Try and open debug logs
        debug_level = GetDebugLevel()
        if debug_level > 0:
            for i in range(debug_level):
                debug_name = "visit-movie.%d.log" % (i + 1)
                try:
                    self.debug_real[i] = open(debug_name, "wt")
                except IOError: 
                    print("Could not open debug log %s" % debug_name)
                    pass

    
    ###########################################################################
    # Method: __del__
    #
    # Purpose:    Destructor
    #
    # Programmer: Brad Whitlock
    # Date:       Wed Sep 20 11:28:55 PDT 2006
    #
    # Modifications:
    # 
    ###########################################################################

    def __del__(self):
        # Try and close the log.
        try:
            if self.log != 0:
                self.log.close()
        except:
            pass

        # Try and close the debug logs.
        for file in self.debug_real:
            if file != 0:
               try:
                   file.close()
               except:
                   pass

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
    #   Brad Whitlock, Fri Oct 20 15:59:14 PST 2006
    #   Added templates and changed how stereo works.
    #
    #   Brad Whitlock, Mon Apr  7 13:25:22 PDT 2008
    #   Added a -noffmpeg flag.
    #
    #   Eric Brugger, Tue Jun 16 15:12:46 PDT 2009
    #   Added -enginerestartinterval flag.
    #
    #   Brad Whitlock, Fri Feb 10 14:20:02 PST 2012
    #   Add -source argument.
    #
    #   Brad Whitlock, Wed Apr  3 16:49:40 PDT 2013
    #   Print formats based on the available encoders.
    #
    ###########################################################################

    def PrintUsage(self):
        print("Usage: visit -movie [-format fmt] [-geometry size]")
        print("                    -sessionfile name | -scriptfile name ")
        print("                    [-output moviename] [-framestep step]")
        print("                    [PARALLEL OPTIONS]")
        print("")
        print("OPTIONS")
        print("    The following options are recognized by visit -movie")
        print("")
        print("    -format fmt        The format option allows you to set the output")
        print("                       format for your movie. The supported values ")
        print("                       for fmt are:")
        print("")
        print("                       Movie formats")
        print("                       -------------")
        keys = list(self.formatInfo.keys())
        keys.sort()
        for k in keys:
            if self.formatInfo[k][self.FMT_ENCODER] != None:
                s = ""
                for ext in self.formatInfo[k][self.FMT_EXTENSIONS]:
                    if s != "":
                        s = s + ", " + ext
                    else:
                        s = ext
                print("                       %s" % s)
        print("")
        print("                       Image formats")
        print("                       -------------")
        for k in keys:
            if self.formatInfo[k][self.FMT_ENCODER] == None:
                s = ""
                for ext in self.formatInfo[k][self.FMT_EXTENSIONS]:
                    if s != "":
                        s = s + ", " + ext
                    else:
                        s = ext
                print("                       %s" % s)
        print("")
        print("                       You can specify multiple formats by concatenating")
        print("                       more than one allowed format using commas. Example:")
        print("                       -format mpeg,qt,jpeg.")
        print("")
        print("                       *** Important Note ***")
        print("                       The qt and sm formats are not supported")
        print("                       on the Windows platform.")
        print("")
        print("    -geometry size     The geometry option allows you to set the movie")
        print("                       resolution. The size argument is of the form")
        print("                       WxH where W is the width of the image and H is")
        print("                       the height of the image. For example, if you ")
        print("                       want an image that is 1024 pixels wide and 768")
        print("                       pixels tall, you would provide:")
        print("                       -geometry 1024x768.")
        print("")
        print("                       You can specify multiple geometries by concatenating")
        print("                       more than one WxH using commas. Example:")
        print("                       -geometry 320x320,1024x768.")
        print("")
        print("                       If you omit the -geometry argument then the")
        print("                       window sizes stored in your session file will be")
        print("                       used. If you are not using a session file then")
        print("                       the default size will be 512x512.")
        print("")
        print("    -sessionfile name  The sessionfile option lets you pick the name")
        print("                       of the VisIt session to use as input for your")
        print("                       movie. The VisIt session is a file that describes")
        print("                       the movie that you want to make and it is created")
        print("                       when you save your session from within VisIt's ")
        print("                       GUI after you set up your plots how you want them.")
        print("")
        print("    -source filename   The source option appends a filename to a list of ")
        print("                       filenames to be used as the new sources when ")
        print("                       restoring the session file selected via the ")
        print("                       -sessionfile command line argument. If you want to")
        print("                       override multiple sources in the session file then ")
        print("                       you must pass multiple instances of the -source ")
        print("                       command line argument.")
        print("")
        print("                       Example:")
        print("                          visit -movie -geometry 1000x1000 -format mpeg \ ")
        print("                                -sessionfile A.session \ ")
        print("                                -source new.silo -source other.silo")
        print("")
        print("                       If you are replacing a time series of files, remember")
        print("                       that VisIt often renames them slightly. A time series")
        print("                       composed of files with names A0000.silo, A0001.silo, ")
        print('                       A0002.silo, ... will have the name: "A*.silo database"')
        print("                       When you pass the -source argument to visit -movie, ")
        print('                       be sure to pass -source "A*.silo database". ')
        print("")
        print("    -adjustview        This command line argument is useful when -sessionfile")
        print("                       and -source arguments are given since it will cause the")
        print("                       view to be adjusted for the bounds of the new dataset.")
        print("                       When replacing a dataset using -source, the new dataset")
        print("                       may have different extents, making the view stored in")
        print("                       the session file less then optimal. The -adjustview")
        print("                       option resets the view so the new dataset will fill the")
        print("                       screen while still preserving the view normal & up ")
        print("                       vectors, 2 important components of the view.")
        print("")
        print("    -scriptfile name   The scriptfile option lets you pick the name")
        print("                       of a VisIt Python script to use as input for your")
        print("                       movie.")
        print("")
        print("    -templatefile name The templatefile option lets you pick the name of a")
        print("                       VisIt movie template file to use as input for your")
        print("                       movie. A movie template file is an XML file that ")
        print("                       describes how to put together a movie.")
        print("")
        print("    -framestep step    The number of frames to advance when going to ")
        print("                       the next frame.")
        print("")
        print("    -start frame       The frame that we want to start at.")
        print("")
        print("    -end frame         The frame that we want to end at.")
        print("")
        print("    -frame value       The initial value used in the name of the first frame.")
        print("")
        print("    -output moviename  The output option lets you set the name of ")
        print("                       your movie.")
        print("")
        print("    -fps number        Sets the frames per second the movie should ")
        print("                       play at.")
        print("")
        print("    -stereo fmt        Makes a stereo movie. Note that stereo movies with")
        print("                       left and right channels are only created for ")
        print("                       Streaming movie format, though left and right channel")
        print("                       images are saved if you save a movie in an image ")
        print("                       format. The redblue and redgreen formats are available")
        print("                       in any image format. The supported values for fmt are:")
        print("")
        print("                       off       : No stereo")
        print("                       leftright : Left/Right channel stereo. Use this")
        print("                                   format with streaming movie to create ")
        print("                                   movies viewable with stereo projection ")
        print("                                   and glasses.")
        print("                       redblue   : Red/Blue stereo.")
        print("                       redgreen  : Red/Green stereo.")
        print("")
        print("                       You can specify multiple stereo formats by ")
        print("                       concatenating more than one allowed format using ")
        print("                       commas. Example: -stereo off,leftright,redgreen")
        print("")
        print("    -ignoresessionengines Prevents compute engine information in the")
        print("                          session file from being used to restart")
        print("                          the compute engine(s) during movie generation.")
        print("")
        print("    -email addresses   If specified, this script will provide e-mail ")
        print("                       progress reports for movie generation. Multiple ")
        print("                       e-mail addresses can be provided if they are ")
        print("                       separated by commas.")
        print("")
        print("    -noffmpeg          Don't use ffmpeg for the mpeg encoder even if it")
        print("                       is available.")
        print("")
        print("    -enginerestartinterval number Restarts the compute engine after")
        print("                                  the specified number of images are")
        print("                                  generated.")
        print("")
        print("Parallel arguments:")
        print("    -np   <# procs>    The number of processors to use.")
        print("    -nn   <# nodes>    The number of nodes to allocate.")
        print("    -l    <method>     Launch in parallel using the given method.")
        print("                       Method is one of the following: mpirun, poe,")
        print("                       psub, srun.")
        print("    -la   <args>       Additional arguments for the parallel launcher.")
        print("    -p    <part>       Partition to run in.")
        print("    -b    <bank>       Bank from which to draw resources.")
        print("    -t    <time>       Maximum job run time.")
        print("    -expedite          Makes batch system give priority scheduling.")
        print("")

    def Log(self, msg):
        if self.log != 0:
            self.log.write("%s\n" % msg)
            self.log.flush()

    ###########################################################################
    # Method: Debug
    #
    # Purpose:    Outputs debug messages.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    # Modifications:
    #   Brad Whitlock, Wed Sep 20 11:50:17 PDT 2006
    #   Changed so it writes to debug logs.
    #
    ###########################################################################

    def Debug(self, level, msg):
        if level not in (1,2,3,4,5):
            return
        debug_level = GetDebugLevel()
        if debug_level > 0:
            for i in range(level, 6):
                 if self.debug_real[i-1] != 0:
                     self.debug_real[i-1].write("%s\n" % msg)
                     self.debug_real[i-1].flush()
        # Write debug1 to the log also.
        if level == 1:
            self.Log(msg)
          
    ###########################################################################
    # Method: SendClientProgress
    #
    # Purpose:    Sends the client progress.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue May 10 11:13:29 PDT 2005
    #
    ###########################################################################

    def SendClientProgress(self, s, current, total):
        if self.sendClientFeedback:
            args = (str(s), int(current), int(total))
            ClientMethod("MovieProgress", args)

    ###########################################################################
    # Method: ClientMessageBox
    #
    # Purpose:    Sends the client MessageBoxOk client method.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue May 10 11:13:29 PDT 2005
    #
    ###########################################################################

    def ClientMessageBox(self, s):
        if self.sendClientFeedback:
            ClientMethod("MessageBoxOk", str(s))
        else:
            print(str(s))

    ###########################################################################
    # Method: SendEmail
    #
    # Purpose:    Sends e-mail to the user regarding the status of the movie.
    #
    # Programmer: Brad Whitlock
    # Date:       Wed Sep 20 13:19:09 PST 2006
    #
    ###########################################################################

    def SendEmail(self, subject, msg):
        if self.emailAddresses != "":
            try:
                import sys, smtplib
                server = smtplib.SMTP('localhost')

                domain = "llnl.gov"
                d = string.split(os.uname()[1], ".")
                if len(d) > 2:
                    name = ""
                    for i in range(1, len(d)):
                        name = name + d[i]
                        if i < len(d)-1:
                            name = name + "."
                fromaddr = "visit@" + domain

                msg2 = "To: %s\n" % self.emailAddresses
                msg2 = msg2 + "Subject: %s\n" % subject
                msg2 = msg2 + msg
                self.Debug(2, "E-mail sent: \n" + msg)
                server.sendmail(fromaddr, self.emailAddresses, msg2)
                server.quit()
            except:
                self.Debug(2, "E-mail not sent due to an exception.\n" + msg)
        else:
            self.Debug(2, "E-mail not sent.\n" + msg)

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
    #   Brad Whitlock, Wed Sep 27 16:59:47 PST 2006
    #   Added support for template files.
    #
    ###########################################################################

    def DetermineMovieBase(self):
        # Get the file name that we should use
        if(self.usesStateFile == 1):
            fileName = LongFileName(self.stateFile)
        elif(self.usesTemplateFile == 1):
            return
        else:
            fileName = LongFileName(self.scriptFile)

        self.Debug(5, "DetermineMovieBase: fileName=%s" % fileName)

        # Try and detect the extension
        extensions = (".session", ".vses", ".VSES", ".VSE", ".py", ".PY", ".xml", ".XML")
        extensionLocated = 0
        for ext in extensions:
            pos = string.rfind(fileName, ext)
            if(pos != -1):
                fileName = fileName[:pos]
                extensionLocated = 1
                break
        self.Debug(5, "DetermineMovieBase: extensionLocated=%d" % extensionLocated)

        # If we located an extension then try and look for a path separator.
        if(extensionLocated == 1):
            for separator in ("/", "\\"):
                pos = string.rfind(fileName, separator)
                if(pos != -1):
                    self.outputDir = fileName[:pos]
                    self.movieBase = fileName[pos+1:]
                    break

        self.Debug(5, "DetermineMovieBase: outputDir=%s" % self.outputDir)
        self.Debug(5, "DetermineMovieBase: movieBase=%s" % self.movieBase)

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
    #   Brad Whitlock, Fri Oct 20 16:29:55 PST 2006
    #   Added stereoName argument.
    #
    #   Brad Whitlock, Thu Apr  4 10:46:58 PDT 2013
    #   Change how the format is determined.
    #
    ###########################################################################

    def RequestFormat(self, fmtName, w, h, stereoName):
        self.Debug(1, "Requesting movie %s at %dx%d with stereo %s" % (fmtName, w, h, stereoName))

        # The string used as fmtName will be one of the extensions for the format.
        fmt = ""
        for k in list(self.formatInfo.keys()):
            for ext in self.formatInfo[k][self.FMT_EXTENSIONS]:
                if fmtName == ext:
                    fmt = k

        sfmt = self.stereoNameToType[stereoName]
        formatString = ""
        # If we're making MPEG then restrict the resolution to multiples of 16.
        if fmt == "mpeg":
            w = int(w / 16) * 16
            h = int(h / 16) * 16

        self.movieFormats = self.movieFormats + [[formatString, fmt, w, h, sfmt]]

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
    #   Brad Whitlock, Tue Sep 19 17:13:09 PST 2006
    #   Changed to allow mpeg on Windows platform.
    #
    #   Brad Whitlock, Wed Sep 20 13:22:30 PST 2006
    #   Added support for sending e-mails and for movie template files. I also
    #   changed how stereo works.
    #
    #   Brad Whitlock, Mon Apr  7 13:26:40 PDT 2008
    #   Added -noffmpeg.
    #
    #   Eric Brugger, Tue Jun 16 15:12:46 PDT 2009
    #   Added -enginerestartinterval flag.
    #
    #   Brad Whitlock, Fri Feb 10 14:20:32 PST 2012
    #   Add -source argument.
    #
    #   Eric Brugger, Tue Oct 22 13:22:35 PDT 2013
    #   Change the movie formats to all use PNG as the input format when
    #   ffmpeg is present and PPM otherwise (for use with mpeg2encode).
    #
    #   Kathleen Biagas, Tue Sep 29 15:52:44 MST 2015
    #   On Windows, only fail if format NOT compatible.
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
        stereoList = []

        i = 0
        outputName = "movie"
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
                        # See if the format is valid.
                        validFormat = ""
                        for k in list(self.formatInfo.keys()):
                            for ext in self.formatInfo[k][self.FMT_EXTENSIONS]:
                                if ext == format:
                                    validFormat = k

                        if validFormat != "":
                            if sys.platform == "win32" and not self.formatInfo[validFormat][self.FMT_WINDOWSCOMPATIBLE]:
                                print("Error! ", format, " is not supported on Windows.")
                                sys.exit(-1)
                            else:
                                formatList = formatList + [validFormat]
                        else:
                            print("Error! ",format," is not a valid format.")
                            self.PrintUsage();
                            sys.exit(-1)

                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-stereo"):
                if((i+1) < len(commandLine)):
                    formats = self.SplitString(commandLine[i+1], ",")
                    for format in formats:
                        if format not in list(self.stereoNameToType.keys()):
                            self.PrintUsage();
                            sys.exit(-1)
                        else:
                            stereoList = stereoList + [format]
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-sessionfile"):
                if((i+1) < len(commandLine)):
                    self.stateFile = commandLine[i+1]
                    self.usesStateFile = 1
                    self.usesTemplateFile = 0
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-source"):
                if((i+1) < len(commandLine)):
                    filename = commandLine[i+1]
                    if string.find(filename, ":") == -1:
                        filename = "localhost:" + filename
                    self.sources = self.sources + [filename]
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-adjustview"):
                self.adjustview = 1
            elif(commandLine[i] == "-scriptfile"):
                if((i+1) < len(commandLine)):
                    self.scriptFile = commandLine[i+1]
                    self.usesStateFile = 0
                    self.usesTemplateFile = 0
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-templatefile"):
                if((i+1) < len(commandLine)):
                    self.templateFile = commandLine[i+1]
                    self.usesTemplateFile = 1
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
                        print("A bad value was provided for frame step. Using a frame step of 1.")
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
                        print("A bad value was provided for frame start. Using a frame step of 0.")
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
                        print("A bad value was provided for frame end. Using a frame end of 0.")
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-frame"):
                if((i+1) < len(commandLine)):
                    try:
                        self.initialFrameValue = int(commandLine[i+1])
                        if(self.initialFrameValue < 0):
                            self.initialFrameValue = 0
                    except ValueError:
                        self.initialFrameValue = 0
                        print("A bad value was provided for frame initial value. Using a value of 0.")
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-output"):
                if((i+1) < len(commandLine)):
                    outputName = commandLine[i+1]
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
                            self.fps = 10
                    except ValueError:
                        self.fps = 10
                        print("A bad value was provided for frames per second.  Using 10.")
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
                            print("Malformed geometry string: %s" % geometry)
                            sys.exit(-1)
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-ignoresessionengines"):
                self.useSessionEngineInformation = 0
            elif(commandLine[i] == "-email"):
                if((i+1) < len(commandLine)):
                    self.emailAddresses = commandLine[i+1]
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-noffmpeg"):
                self.allow_ffmpeg = 0
            elif(commandLine[i] == "-enginerestartinterval"):
                if((i+1) < len(commandLine)):
                    try:
                        self.engineRestartInterval = int(commandLine[i+1])
                        if(self.engineRestartInterval < 0):
                            self.engineRestartInterval = 0
                    except ValueError:
                        self.engineRestartInterval = 1000000
                        print("A bad value was provided for engine restart interval. Using an engine restart interval of 1000000.")
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)

            #
            # Parallel engine options.
            #
            elif(commandLine[i] == "-np"):
                if((i+1) < len(commandLine)):
                    try:
                        np = int(commandLine[i+1])
                        if(np < 1):
                            print("A bad value was provided for number of processors.")
                            self.PrintUsage()
                            sys.exit(-1)
                        self.engineCommandLineProperties["numProcessors"] = np
                    except ValueError:
                        print("A bad value was provided for number of processors.")
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
                            print("A bad value was provided for number of nodes.")
                            self.PrintUsage()
                            sys.exit(-1)
                        self.engineCommandLineProperties["numNodes"] = nn
                        self.engineCommandLineProperties["numNodesSet"] = 1
                    except ValueError:
                        print("A bad value was provided for number of nodes.")
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
                if "arguments" in list(self.engineCommandLineProperties.keys()):
                    self.engineCommandLineProperties["arguments"] = self.engineCommandLineProperties["arguments"] + ["-expedite"]
                else:
                    self.engineCommandLineProperties["arguments"] = ["-expedite"]
            elif(commandLine[i] == "-help" or commandLine[i] == "-h" or commandLine[i] == "--help"):
                self.PrintUsage()
                sys.exit(0)
                

            # On to the next argument.
            i = i + 1

        # If we are using ffmpeg for the mpeg encoder then let's make sure that 
        # we make JPEG images to start with.
        if self.allow_ffmpeg:
            self.formatInfo["mpeg"][self.FMT_INPUTFORMAT] = "png"
        else:
            self.formatInfo["mpeg"][self.FMT_INPUTFORMAT] = "ppm"

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
        # If the formatList and stereoList are not the same size then make
        # them be the same size.
        if len(formatList) != len(stereoList):
            if len(formatList) > len(stereoList):
                lastValue = "off"
                if len(stereoList) > 0:
                    lastValue = stereoList[-1]
                for i in range(len(formatList) - len(stereoList)):
                    stereoList = stereoList + [lastValue]
            else:
                lastFormatValue = formatList[-1]
                lastSizeValue = sizeList[-1]
                for i in range(len(stereoList) - len(formatList)):
                    formatList = formatList + [lastFormatValue]
                    sizeList = sizeList + [lastSizeValue]

        # Now that the formatList and sizeList have been populated by the command
        # line arguments, use the information stored therein to request movie
        # formats.
        for i in range(len(formatList)):
            self.RequestFormat(formatList[i], sizeList[i][0], sizeList[i][1], stereoList[i])

        # Make sure that the user provided the -sessionfile option or we can't
        # make a movie!
        if(self.stateFile == "" and self.scriptFile == "" and self.templateFile == ""):
            self.PrintUsage()
            print("You must provide one of the -sessionfile,-scriptfile,-templatefile options!")
            sys.exit(-1)

        # If no movie output was specified, try and determine the output
        # base name from the session file or the script file.
        if(outputSpecified == 0):
            self.DetermineMovieBase()
        else:
            # Output was specified. Let's break it up into outputDir and movieBase.
            if outputName[0] == '/':
                # Absolute path
                entirePath = outputName
            elif string.find(outputName, self.slash) != -1:
                # Relative path
                entirePath = os.path.abspath(os.curdir) + self.slash + outputName
            else:
                # No path given.
                entirePath = os.path.abspath(os.curdir) + self.slash + outputName

            # Separate into outputDir, movieBase.
            pos = string.rfind(entirePath, self.slash)
            if pos != -1:
                self.outputDir = entirePath[:pos]
                self.movieBase = entirePath[pos+1:]
            else:
                self.Debug(1, "Error determining movie base from -output %s" % outputName)
        self.Debug(1, "ProcessArguments: self.outputDir = %s" % self.outputDir)
        self.Debug(1, "ProcessArguments: self.movieBase = %s" % self.movieBase)

        # Write out a log file.
        try:
            logName = os.path.abspath(os.curdir) + self.slash + "%s.log" % self.movieBase
            self.log = open(logName, "wt")
        except:
            self.log = 0


    ###########################################################################
    # Method: CreateTemporaryDirectory
    #
    # Purpose:    This method creates a temporary directory to which the frames
    #             for the movie will be written if we are encoding the movie
    #             in an animation format such as MPEG, QuickTime, or Streaming
    #             movie.
    #
    # Notes:      The output directory will be outputDir + movieBase plus some
    #             number.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    # Modifications:
    #   Brad Whitlock, Tue May 31 14:44:48 PST 2005
    #   Determines whether a temporary directory needs to be created. Changed
    #   the name of the temp directory.
    #
    #   Brad Whitlock, Wed Sep 27 17:57:59 PST 2006
    #   Made it use new outputDir member.
    #
    #   Brad Whitlock, Thu Apr  4 13:12:51 PDT 2013
    #   Use formatInfo.
    #
    ###########################################################################

    def CreateTemporaryDirectory(self):
        retval = self.outputDir

        # Determine if any movie formats get encoded.
        any_formats_get_encoded = 0
        for format in self.movieFormats:
            fmt = format[1]
            if self.formatInfo[fmt][self.FMT_ENCODER] != None:
                any_formats_get_encoded = 1
                break

        if any_formats_get_encoded == 1:
            # Determine the name of the directory to use.
            index = 0
            d = "%s-%d" % (self.movieBase, index)
            files = os.listdir(self.outputDir)
            self.Debug(1, "CreateTemporaryDirectory: d=" + str(d))
            self.Debug(1, "CreateTemporaryDirectory: files=" + str(files))
            while(d in files):
                self.Debug(1, "The directory %s already exists, we'll try another name." % d)
                index = index + 1
                d = "%s-%d" % (self.movieBase, index)
            # Create a temporary directory in which to store frames.
            newdir = self.outputDir + self.slash + d
            self.Debug(1, "Creating %s directory to store temporary frames" % newdir)
            os.mkdir(newdir)
            self.Debug(1, "Created %s directory to store temporary frames" % newdir)
            retval = newdir
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
    #   Brad Whitlock, Wed Sep 27 18:09:32 PST 2006
    #   I made frames always get saved to tmpDir. I changed how stereo works.
    #
    #   Brad Whitlock, Wed Apr  3 17:32:26 PDT 2013
    #   Change how format is determined.
    #
    ###########################################################################

    def SaveImage(self, index):
        s = SaveWindowAttributes()
        old_sw = GetSaveWindowAttributes()
        s.saveTiled = old_sw.saveTiled
        s.family = 0
        s.screenCapture = self.screenCaptureImages
        s.outputToCurrentDirectory = 0
        s.resConstraint = s.NoConstraint

        currentRA = GetRenderingAttributes()

        # Save the frame in the required formats.
        retval = 1
        for format in self.movieFormats:

            # Determine if any movie formats get encoded if so ignore the
            # initialFrameValue
            initialFrameValue = self.initialFrameValue

            fmt = format[1]
            if self.formatInfo[fmt][self.FMT_ENCODER] != None:
                initialFrameValue = 0

            # Stereo format
            stereo = format[4]
            ra = GetRenderingAttributes()
            if stereo == self.STEREO_NONE:
                s.stereo = 0
                ra.stereoRendering = 0
            elif stereo == self.STEREO_LEFTRIGHT:
                s.stereo = 1
                ra.stereoRendering = 0
            elif stereo == self.STEREO_REDBLUE:
                s.stereo = 0
                ra.stereoRendering = 1
                ra.stereoType = ra.RedBlue
            elif stereo == self.STEREO_REDGREEN:
                s.stereo = 0
                ra.stereoRendering = 1
                ra.stereoType = ra.RedGreen
            SetRenderingAttributes(ra)
            currentRA = ra

            # Create a filename
            frameFormatString = format[0]
            s.fileName = frameFormatString % (initialFrameValue+index)
            self.Debug(5, "SaveImage: %s" % s.fileName)

            # Determine the format of the frame that VisIt needs to save.
            frameFormat = self.formatInfo[format[1]][self.FMT_INPUTFORMAT]
            if(frameFormat == "ppm"):
                s.format =  s.PPM
            elif(frameFormat == "tiff"):
                s.format =  s.TIFF
            elif(frameFormat == "jpeg"):
                s.format =  s.JPEG
            elif(frameFormat == "bmp"):
                s.format =  s.BMP
            elif(frameFormat == "rgb"):
                s.format =  s.RGB
            elif(frameFormat == "png"):
                s.format =  s.PNG

            # Images that will be used in a movie go to tmpDir while other
            # images go directly to outputDir.
            if frameFormat != format[1]:
                s.outputDirectory = self.tmpDir
            else:
                s.outputDirectory = self.outputDir

            s.width = format[2]
            s.height = format[3]
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
        old_ra = GetRenderingAttributes()
        retval = self.SaveImage(self.numFrames)
        self.numFrames = self.numFrames + 1
        # Restore the old rendering attributes.
        SetRenderingAttributes(old_ra)
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
    #   Brad Whitlock, Mon May 19 13:38:42 PDT 2008
    #   Check the number of keys in allEngineProperties not in
    #   self.engineCommandLineProperties. This makes the script once again
    #   able to read engine settings used to create a session file.
    #
    #   Brad Whitlock, Fri Feb 10 16:05:14 PST 2012
    #   Something higher up in the session file was choking the parsing so
    #   let's just focus on the RunningEngines section if we can.
    #
    ###########################################################################

    def ReadEngineProperties(self, sessionfile):   
        try:
            # Read the file.
            f = open(sessionfile, "rt")
            lines = f.readlines()
            f.close()

            # Try and just get the RunningEngines part of it. This code here 
            # relies on the white spacing, which is a hack but session files
            # are machine-generated so it should not matter much.
            runningEngines = []
            reading = 0
            indent = 0
            for line in lines:
                level = string.find(line, "<")
                if reading == 0:
                    if string.find(line, "<Object name=\"RunningEngines\">") != -1:
                        reading = 1
                        runningEngines = runningEngines + [line]
                        indent = level
                else:
                    runningEngines = runningEngines + [line]
                    if level <= indent:
                        break

            # Parse the running engines section, if present.
            p = EngineAttributesParser()
            try:
                for line in runningEngines:
                    p.feed(line)
                p.close()
            except:
                print()
                print("ERROR: We could not parse the session file for engine ")
                print("launch parameters.  This will cause problems getting ")
                print("the right type of engine launched.  Please report this.")
                print()

            if len(list(p.allEngineProperties.keys())) == 0:
                # There were no hosts in the engine attributes so add the command
                # line options for localhost.
                p.allEngineProperties["localhost"] = self.engineCommandLineProperties
            else:
                # Override the EngineAttributesParser's engine attributes
                # with options that were provided on the command line.
                for host in list(p.allEngineProperties.keys()):
                    dest = p.allEngineProperties[host]
                    for key in list(self.engineCommandLineProperties.keys()):
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
    # Modifications:
    #   Brad Whitlock, Thu Oct  9 13:17:04 PDT 2008
    #   Applied a user's fix for setting the launch arguments.
    #
    #   Brad Whitlock, Fri Feb 10 16:09:24 PST 2012
    #   Change -timeout to -idle-timeout.
    #
    ###########################################################################

    def CreateEngineArguments(self, engineProperties):
        arguments = []
        if "numProcessors" in list(engineProperties.keys()):
            np = engineProperties["numProcessors"]
            if np > 1:
                arguments = arguments + ["-par", "-np", "%d" % np]

                if "numNodesSet" in list(engineProperties.keys()) and\
                   "numNodes" in list(engineProperties.keys()):
                    nn = engineProperties["numNodes"]
                    if engineProperties["numNodesSet"] == 1 and nn > 0:
                        arguments = arguments + ["-nn", "%d" % nn]

                if "partitionSet" in list(engineProperties.keys()) and\
                   "partition" in list(engineProperties.keys()):
                    if engineProperties["partitionSet"] == 1:
                        arguments = arguments + ["-p", engineProperties["partition"]]
    
                if "bankSet" in list(engineProperties.keys()) and\
                   "bank" in list(engineProperties.keys()):
                    if engineProperties["bankSet"] == 1:
                        arguments = arguments + ["-b", engineProperties["bank"]]

                if "timeLimitSet" in list(engineProperties.keys()) and\
                   "timeLimit" in list(engineProperties.keys()):
                    if engineProperties["timeLimitSet"] == 1:
                        arguments = arguments + ["-t", engineProperties["timeLimit"]]

                if "launchMethodSet" in list(engineProperties.keys()) and\
                   "launchMethod" in list(engineProperties.keys()):
                    if engineProperties["launchMethodSet"] == 1:
                        arguments = arguments + ["-l", engineProperties["launchMethod"]]

                if "launchArgsSet" in list(engineProperties.keys()) and\
                   "launchArgs" in list(engineProperties.keys()):
                    if engineProperties["launchArgsSet"] == 1:
                        arguments = arguments + ["-la", str(engineProperties["launchArgs"])]

        arguments = arguments + ["-forcestatic"]

        if "timeout" in list(engineProperties.keys()):
            arguments = arguments + ["-idle-timeout", "%d" % engineProperties["timeout"]]

        if "arguments" in list(engineProperties.keys()):
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
    #   Eric Brugger, Tue Jun 16 15:12:46 PDT 2009
    #   Added -enginerestartinterval flag.
    #
    ###########################################################################
    def IterateAndSaveFrames(self):

        self.Debug(1, "IterateAndSaveFrames:")

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

        self.Debug(1, "*** frameStart=%d, frameEnd=%d" % (self.frameStart, self.frameEnd))

        # Save the old rendering mode.
        old_ra = GetRenderingAttributes()

        # Save an image for each frame in the animation.
        self.numFrames = 0
        i = self.frameStart
        drawThePlots = 0
        nTotalFrames = self.frameEnd - self.frameStart + 1
        lastProgress = -1
        framesGenerated = 0
        while(i <= self.frameEnd):
            t = float(i - self.frameStart) / float(nTotalFrames);
            progress = int(t * self.percentAllocationFrameGen * 100.)
            if progress != lastProgress:
                self.SendClientProgress("Generating frames", progress, 100)
                lastProgress = progress

            if(SetTimeSliderState(i) == 0):
                drawThePlots = 1
                print("There was an error when trying to set the "\
"time slider state to %d. VisIt will now try to redraw the plots." % i)
            if(drawThePlots):
                if(DrawPlots() == 0):
                    print("VisIt could not draw plots for time slider "\
"state %d. You should investigate the files used for that state." % i)
                else:
                    drawThePlots = 0
            if self.SaveImage(self.numFrames) == 0:
                print("There was an error when trying to save the window "\
"for time slider state %d. VisIt will now try to redraw the plots and re-save "\
"the window." % i)
                if(DrawPlots() == 0):
                    print("VisIt could not draw plots for time slider "\
"state %d. You should investigate the files used for that state." % i)
                else:
                    if self.SaveImage(self.numFrames) == 0:
                        print("VisIt could not re-save the window for "\
"time slider state %d. You should investigate the files used for that state." % i)

            self.numFrames = self.numFrames + 1
            i = i + self.frameStep

            framesGenerated = framesGenerated + 1
            if (framesGenerated >= self.engineRestartInterval):
                CloseComputeEngine()
                framesGenerated = 0

        # Restore the old rendering attributes.
        SetRenderingAttributes(old_ra)

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
    #   Brad Whitlock, Wed Sep 20 11:53:55 PDT 2006
    #   I made it use the tmp directory in the filename. Add support for
    #   template files and stereo.
    #
    #   Brad Whitlock, Wed Dec 12 16:15:49 PST 2007
    #   Turn off animation caching if it is on so saving movies will work.
    #
    #   Brad Whitlock, Wed Apr 16 16:43:03 PDT 2008
    #   Fixed a problem with finding user-defined movie template work files.
    #
    #   Brad Whitlock, Fri Feb 10 14:25:56 PST 2012
    #   I added support for restoring sessions using different sources.
    #
    #   Brad Whitlock, Tue May  1 10:57:40 PDT 2012
    #   Move the movietemplates directory into resources.
    #
    #   Brad Whitlock, Wed Apr  3 17:36:15 PDT 2013
    #   Check formatInfo.
    #
    #   Kathleen Biagas, Mon Oct 23 17:04:22 MST 2017
    #   Resources dir on Windows is same now whether dev build or not.
    #
    ###########################################################################

    def GenerateFrames(self):
        self.Debug(1, "GenerateFrames")

        # Make sure that animation caching is off
        anim = GetAnimationAttributes()
        if anim.pipelineCachingMode > 0:
            anim.pipelineCachingMode = 0
            SetAnimationAttributes(anim)

        # Make sure that the movieFormats list is not empty
        if len(self.movieFormats) < 1:
            self.ClientMessageBox("No formats were requested so no movie will be made.")
            return 0

        # Create a temporary directory in which to store the frames if we're
        # generating a movie that ends up as a single file.
        self.tmpDir = self.CreateTemporaryDirectory()
        self.Debug(1, "GenerateFrames: tmpDir = %s" % self.tmpDir)      

        # If we have any movies that need to get encoded, make sure that we allot
        # some time in the progress to encoding.
        index = 0
        for format in self.movieFormats:
            fmt = format[1]
            if self.formatInfo[fmt][self.FMT_ENCODER] != None:
                self.percentAllocationFrameGen = 0.9
                self.percentAllocationEncode = 0.1
                if format[4] == self.STEREO_LEFTRIGHT:
                    if not self.formatInfo[fmt][self.FMT_SUPPORTS_STEREO]:
                        msg = "Left/Right stereo is not supported for %s movies. VisIt will instead create stereo PPM files." % fmt
                        self.ClientMessageBox(msg)
                        self.Log(msg)
                        self.movieFormats[index][1] = "ppm"
            index = index + 1

        # Determine if the formats contain different resolutions.
        differentResolutions = 0
        if len(self.movieFormats) > 1:
            w = self.movieFormats[0][2]
            h = self.movieFormats[0][3]
            for i in range(1, len(self.movieFormats)):
                if w != self.movieFormats[i][2] or h != self.movieFormats[i][3]:
                    differentResolutions = 1
                    break

        # Determine if the formats contain different stereo settings.
        differentStereo = 0
        if len(self.movieFormats) > 1:
            s = self.movieFormats[0][4]
            for i in range(1, len(self.movieFormats)):
                if s != self.movieFormats[i][4]:
                    differentStereo = 1
                    break

        # Create file format strings for the frames.
        for index in range(len(self.movieFormats)):
            filebase = self.movieBase
            fmt = self.movieFormats[index][1]
            si = self.movieFormats[index][4]
            s = list(self.stereoNameToType.keys())[si]

            if differentResolutions:
                w = self.movieFormats[index][2]
                h = self.movieFormats[index][3]
                if differentStereo and si > 0:
                    filebase = "%s_%dx%d_%s_%%04d" % (filebase, w, h, s)
                else:
                    filebase = "%s_%dx%d_%%04d" % (filebase, w, h)
            else:
                if differentStereo and si > 0:
                    filebase = "%s_%s_%%04d" % (filebase, s)
                else:
                    filebase = "%s%%04d" % filebase

            # Store the file format string in the movieFormats list.
            self.movieFormats[index][0] = filebase
        self.Debug(1, "GenerateFrames: movieFormats=" + str(self.movieFormats))

        if(self.usesTemplateFile):
            # Determine the name of the movie template base class's file.
            prefix = ""
            if os.name == "nt":
                prefix = sys.executable[:-7] + "resources" + self.slash
            else:
                pos = string.find(sys.argv[0], "exe" + self.slash + "cli")
                if pos != -1:
                    # Development version
                    prefix = sys.argv[0][:pos+4]
                    exe_dir = self.slash + "exe" + self.slash
                    resources_dir = self.slash + "resources" + self.slash
                    prefix = string.replace(prefix, exe_dir, resources_dir)
                else:
                    # Installed version
                    pos = string.find(sys.exec_prefix, "lib" + self.slash + "python")
                    prefix = sys.exec_prefix[:pos] + "resources" + self.slash
            templateBaseFile = prefix + "movietemplates" + self.slash + "visitmovietemplate.py"
            self.Debug(1, "GenerateFrames: sourcing template base class file %s" % templateBaseFile)
            Source(templateBaseFile)


            # Determine the movie template's actual template file. We must read
            # the movie template XML file to determine the template file. We may
            # also need to prepend a path to it.
            try:
                # Read the XML file.
                self.Debug(1, "Opening template file: %s" % self.templateFile)
                f = open(self.templateFile, "rt")
                lines = f.readlines()
                f.close()
                templateReader = MovieTemplateReader()
                for line in lines:
                   templateReader.feed(line)
                templateReader.close()

                # Get the name of the template work file from the XML template.
                templatePY = prefix + "movietemplates" + self.slash + "visitmovietemplate.py"
                self.Debug(1, "GenerateFrames: generic template work file: %s" % templatePY)
                if "TEMPLATEFILE" not in templateReader.generic_data:
                    self.Log("The movie template file did not have a TEMPLATEFILE key. Use visitmovietemplate.py")
                    self.Debug(1,"The movie template file did not have a TEMPLATEFILE key. Use visitmovietemplate.py")
                else:
                    tFile = templateReader.generic_data["TEMPLATEFILE"]
                    self.Debug(1, "TEMPLATEFILE = %s" % tFile)
                    print(tFile)
                    fileFound = 0
                    for name in (tFile, prefix + "movietemplates" + self.slash + tFile):
                        if (sys.platform != "win32") and string.find(name, "~") != -1:
                            name2 = string.replace(name, "~", os.getenv("HOME"))
                        else:
                            name2 = name
                        tmpPY = os.path.abspath(name2)
                        # Try and stat the tmpPY file.
                        try:
                            self.Debug(1, "Looking for template work file: %s" % tmpPY)
                            s = os.stat(tmpPY)
                            templatePY = tmpPY
                            fileFound = 1
                        except OSError:
                            continue
                    if fileFound == 0:
                        self.ClientMessageBox("VisIt could not locate the template work file %s." % tFile)
                        return 0

                # Source the template work file to get the template's class 
                # definition and the InstantiateMovieTemplate function.
                self.Debug(1, "GenerateFrames: sourcing %s" % templatePY)
                try:
                    if templatePY != templateBaseFile:
                        Source(templatePY)
                except VisItException:
                    msg = "The movie template work file %s could not be read." % templatePY
                    self.Debug(1, msg)
                    self.ClientMessageBox(msg)
                    return 0

                try:
                    # Call the template work file's InstantiateMovieTemplate function.
                    template = globals()['InstantiateMovieTemplate'](self, templateReader)

                    # Call the template to make it generate frames.
                    self.Debug(1, "GenerateFrames: using template file %s" % templatePY)
                    ret = template.GenerateFrames(self.templateFile, self.movieFormats)
                    if ret == -1:
                        return 0

                    # The template did not return -1 for error so it must have 
                    # returned the number of frames.
                    self.numFrames = ret
                except KeyError as value:
                    msg = "The movie template work file %s contains errors or does not "\
                          "contain an InstantiateMovieTemplate function." % templatePY
                    self.Log(str(value))
                    self.ClientMessageBox(msg)
                    raise
                    return 0
            except IOError:
                self.ClientMessageBox("VisIt could not open your movie template file %s\n" % self.templateFile)
                return 0

        elif(self.usesCurrentPlots):
            self.Debug(1, "GenerateFrames: using current plots")

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
            self.Debug(1, "GenerateFrames: using session file %s" % self.stateFile)

            # If we're using a session file, try and read it to determine which
            # compute engines need to be launched.
            properties = {}
            if self.useSessionEngineInformation == 1:
                properties = self.ReadEngineProperties(self.stateFile)
                self.Debug(1, str(properties))

            # Launch any compute engines that were either specified on the
            # command line or in the session file.
            for host in list(properties.keys()):
                if properties[host] != {}:
                    args = self.CreateEngineArguments(properties[host])
                    self.Debug(1, "Starting compute engine: %s" % host)
                    OpenComputeEngine(host, args)

            # Make the viewer try and restore its session using the file that
            # was passed in.
            if len(self.sources) == 0:
                RestoreSession(self.stateFile, 0)
            else:
                RestoreSessionWithDifferentSources(self.stateFile, 0, self.sources)
                # Restoring the session is messing up adding the new time sliders 
                # from the new databases to the new window. Reopen the files to
                # make the time sliders get added properly.
                for src in self.sources:
                    ReOpenDatabase(src)

            # Make sure that plots are all drawn.
            DrawPlots()

            # Reset the view but preserve the viewpoint and image zoom.
            if self.adjustview:
                v0 = GetView3D()
                ResetView()
                v1 = GetView3D()
                v1.viewNormal = v0.viewNormal
                v1.viewUp = v0.viewUp
                v1.imageZoom = v0.imageZoom
                SetView3D(v1)

            # Iterate over all of the frames and save them out.
            self.IterateAndSaveFrames()
        else:
            self.Debug(1, "GenerateFrames: using Python file %s" % self.scriptFile)
            # Save the callback data so we can call methods on this object.
            globals()['classSaveWindowObj'] = self
            # Create a modified version of the user's script.
            name = self.CreateMangledSource(self.scriptFile)
            # Try executing the modified version of the user's script.
            Source(name)
            # Remove the modified script.
            RemoveFile(name);

        return 1

    ###########################################################################
    # Method: OutputExtension
    #
    # Purpose:    This method returns the extension to use for a file.
    #
    # Programmer: Brad Whitlock
    # Date:       Wed Apr  3 16:32:42 PDT 2013
    #
    # Modifications:
    #
    ###########################################################################

    def OutputExtension(self, fmt):
        return "." + self.formatInfo[fmt][self.FMT_EXTENSIONS][0]

    ###########################################################################
    # Method: EncodeMPEGMovie
    #
    # Purpose:    This method creates an MPEG movie.
    #
    # Programmer: Brad Whitlock
    # Date:       Wed Apr  3 16:32:42 PDT 2013
    #
    # Modifications:
    #   Cyrus Harrison, Sat Mar 27 11:51:10 PST 2004
    #   Bring back our old fps / frame duplication heuristic for mpeg encoding.
    #
    ###########################################################################

    def EncodeMPEGMovie(self, moviename, imageFormatString, xres, yres):
        if self.allow_ffmpeg:        
            # keep old fps / frame duplication heuristic, feed dup rate into 
            # visit_utils.encoding
            #
            # We can only drive the movie at 30 fps.  So if the user wants
            # a movie at 10 fps, we have to pad frames.  Determine what that
            # pad rate is.
            pad_rate=-1
            for i in range(1,31):
               if (pad_rate < 0):
                  if ((30./i) <= self.fps):
                     pad_rate = i
                     if ((30./i) != self.fps):
                         print("Because of limitations in MPEG encoding, the ")
                         print("movie will be encoded at %f frames per second" %(30./i))
            frameExt = self.OutputExtension(self.formatInfo["mpeg"][self.FMT_INPUTFORMAT])
            framePattern = self.tmpDir + self.slash + imageFormatString + frameExt
            absMovieName = self.outputDir + self.slash + moviename

            success = 1
            msg = ""
            # use fdup to set the duplication frame rate
            if encoding.encode(framePattern, absMovieName,fdup=pad_rate) > 0:
                success = 0
                msg = "The movie encoder used in the visit_utils module did not complete successfully."

            return (success, moviename, msg)
        else:
            return self.EncodeMPEGMovie_old(moviename, imageFormatString, xres, yres)

    ###########################################################################
    # Method: EncodeMPEGMovie_old
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
    #   Brad Whitlock, Tue Sep 19 17:13:51 PST 2006
    #   Rewrote for mpeg2encode.
    #
    #   Kathleen Bonnell, Tue Jul  3 20:09:39 PDT 2007 
    #   Prepended outputDir to paramFile and statFile.  For command, if on 
    #   windows, use mpeg2enc directly, instead of via "visit -mpeg2encode".
    #
    #   Dave Bremer, Tue Oct 23 18:40:09 PDT 2007
    #   Added a flag to write MPEG-1 or 2.  Clamp the bitrate parameter to go
    #   no higher than what mpeg2encode will allow.  Raise the number of 
    #   intermediate frames between key frames from 9 to 15, because I think 
    #   we generally will have lots of frame to frame coherence.  Set the 
    #   Level ID parameter to the highest-quality setting.  Choosing a lower
    #   Level ID caused the encoder to demand a lower bitrate than we sometimes
    #   passed in.
    #
    #   Dave Bremer, Fri Oct 26 14:57:08 PDT 2007
    #   Removed support for MPEG-2, because of licensing problems.
    #
    #   Brad Whitlock, Mon Apr  7 13:30:21 PDT 2008
    #   Added support for the ffmpeg encoder in case the user has it installed.
    #
    #   Hank Childs, Thu Jul  9 08:29:10 PDT 2009
    #   Incorporate fixes for new ffmpeg syntax from "Max"/iprmaster.
    #
    #   Brad Whitlock, Fri Jan 22 15:50:39 PST 2010
    #   Lower bitrate a little for ffmpeg.
    #
    #   Kathleen Bonnell, Tue Nov  2 12:20:11 PDT 2010 
    #   Executable is mpeg2encode.exe on Windows (since CMake overhaul). 
    # 
    #   Brad Whitlock, Wed Apr  3 16:30:39 PDT 2013
    #   Rename to EncodeMPEGMovie_old and remove ffmpeg code.
    #
    #   Kathleen Biagas, Tue Jan 13 11:00:19 PST 2015
    #   Use mpeg2encode directly (instead of visit -mpeg2encode)
    #
    ###########################################################################

    def EncodeMPEGMovie_old(self, moviename, imageFormatString, xres, yres):
        self.Debug(1, "EncodeMPEGMovie")

        retval = 0
        paramFile = self.outputDir + self.slash + "%s-mpeg2encode-params" % self.movieBase
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
                         print("Because of limitations in MPEG encoding, the ")
                         print("movie will be encoded at %f frames per second" %(30./i))

            # Now create symbolic links to the images at that pad rate.
            formatExt = self.OutputExtension(self.formatInfo["mpeg"][self.FMT_INPUTFORMAT])
            linkbase = "mpeg_link"
            linkindex = 0
            doSymlink = "symlink" in dir(os)
            for i in range(self.numFrames):
                imgname = imageFormatString % i
                imgname = self.tmpDir + self.slash + imgname + formatExt
                for j in range(pad_rate):
                    number4 = "%04d" % linkindex
                    linkname = self.tmpDir+self.slash+linkbase+number4+formatExt
                    linkindex = linkindex + 1
                    CopyFile(imgname, linkname, doSymlink)

            nframes=pad_rate*self.numFrames
            statFile = self.outputDir + self.slash + "%s-stat.out" % moviename

            bitrate = xres * yres * 30 * 3;
            if bitrate > 104000000:
                bitrate = 104000000

            absMovieName = self.outputDir + self.slash + moviename

            # Use VisIt's mpeg2encode MPEG encoder program.
            f = open(paramFile, "w")
            f.write('Generated by VisIt (http://www.llnl.gov/visit), MPEG-1 Movie, 30 frames/sec\n')
            f.write(self.tmpDir + self.slash + linkbase + '%04d  /* name of source files */\n')
            f.write('-         /* name of reconstructed images ("-": do not store) */\n')
            f.write('-         /* name of intra quant matrix file     ("-": default matrix) */\n')
            f.write('-         /* name of non intra quant matrix file ("-": default matrix) */\n')
            f.write('%s        /* name of statistics file ("-": stdout ) */\n' % statFile)
            f.write('2         /* input picture file format: 0=*.Y,*.U,*.V, 1=*.yuv, 2=*.ppm */\n')
            f.write('%d        /* number of frames */\n' % nframes)
            f.write('0         /* number of first frame */\n')
            f.write('00:00:00:00 /* timecode of first frame */\n')
            f.write('15        /* N (# of frames in GOP) */\n')  # 15
            f.write('3         /* M (I/P frame distance) */\n')
            f.write('1         /* ISO/IEC 11172-2 stream */\n')
            f.write('0         /* 0:frame pictures, 1:field pictures */\n')
            f.write('%d        /* horizontal_size */\n' % xres)
            f.write('%d        /* vertical_size */\n' % yres)
            f.write('8         /* aspect_ratio_information 8=CCIR601 625 line, 9=CCIR601 525 line */\n')
            f.write('5         /* frame_rate_code 1=23.976, 2=24, 3=25, 4=29.97, 5=30 frames/sec. */\n')
            f.write('%d.0      /* bit_rate (bits/s) */\n' % bitrate)
            f.write('112       /* vbv_buffer_size (in multiples of 16 kbit) */\n')
            f.write('0         /* low_delay  */\n')
            f.write('0         /* constrained_parameters_flag */\n')
            f.write('4         /* Profile ID: Simple = 5, Main = 4, SNR = 3, Spatial = 2, High = 1 */\n')
            f.write('4         /* Level ID:   Low = 10, Main = 8, High 1440 = 6, High = 4          */\n')
            f.write('1         /* progressive_sequence */\n')
            f.write('1         /* chroma_format: 1=4:2:0, 2=4:2:2, 3=4:4:4 */\n')
            f.write('0         /* video_format: 0=comp., 1=PAL, 2=NTSC, 3=SECAM, 4=MAC, 5=unspec. */\n')
            f.write('5         /* color_primaries */\n')
            f.write('5         /* transfer_characteristics */\n')
            f.write('5         /* matrix_coefficients */\n')
            f.write('%d        /* display_horizontal_size */\n' % xres)
            f.write('%d        /* display_vertical_size */\n' % yres)
            f.write('0         /* intra_dc_precision (0: 8 bit, 1: 9 bit, 2: 10 bit, 3: 11 bit */\n')
            f.write('0         /* top_field_first */\n')
            f.write('1 1 1     /* frame_pred_frame_dct (I P B) */\n')
            f.write('0 0 0     /* concealment_motion_vectors (I P B) */\n')
            f.write('0 0 0     /* q_scale_type  (I P B) */\n')
            f.write('0 0 0     /* intra_vlc_format (I P B)*/\n')
            f.write('0 0 0     /* alternate_scan (I P B) */\n')
            f.write('0         /* repeat_first_field */\n')
            f.write('1         /* progressive_frame */\n')
            f.write('0         /* P distance between complete intra slice refresh */\n')
            f.write('0         /* rate control: r (reaction parameter) */\n')
            f.write('0         /* rate control: avg_act (initial average activity) */\n')
            f.write('0         /* rate control: Xi (initial I frame global complexity measure) */\n')
            f.write('0         /* rate control: Xp (initial P frame global complexity measure) */\n')
            f.write('0         /* rate control: Xb (initial B frame global complexity measure) */\n')
            f.write('0         /* rate control: d0i (initial I frame virtual buffer fullness) */\n')
            f.write('0         /* rate control: d0p (initial P frame virtual buffer fullness) */\n')
            f.write('0         /* rate control: d0b (initial B frame virtual buffer fullness) */\n')
            f.write('2 2 11 11 /* P:  forw_hor_f_code forw_vert_f_code search_width/height */\n')
            f.write('1 1 3  3  /* B1: forw_hor_f_code forw_vert_f_code search_width/height */\n')
            f.write('1 1 7  7  /* B1: back_hor_f_code back_vert_f_code search_width/height */\n')
            f.write('1 1 7  7  /* B2: forw_hor_f_code forw_vert_f_code search_width/height */\n')
            f.write('1 1 3  3  /* B2: back_hor_f_code back_vert_f_code search_width/height */\n')
            f.close();
            # Create the movie
            if (sys.platform != "win32"):
                command = "mpeg2encode %s %s" % (paramFile, absMovieName)
            else:
                command = "mpeg2encode.exe "  + '"' + paramFile + '" "' + absMovieName + '"'
            self.Debug(1, command)
            # Function to print the mpeg2encode output
            def print_mpeg_line_cb(line, this):
                if line[:8] == "Encoding":
                    print(line)
                    this.Debug(1, line)
                else:
                    this.Debug(5, line)
            r = visit_pipe(command, print_mpeg_line_cb, self)
            self.Debug(1, "mpeg2encode returned %d" % r)

            # Make sure that the movie exists before we delete files.
            files = os.listdir(self.outputDir)
            if moviename not in files:
                r = -1
                self.Debug(1, "%s was not located in the %s directory. The MPEG encoder must have failed." % (moviename, self.outputDir))
            else:
                # The file exists but if it is too small then tell the user that the
                # encoding failed.
                try:
                    movieSize = os.stat(absMovieName)[6]
                    if movieSize < 20000:
                        r = -1
                    self.Debug(1, "Checking MPEG movie size. Size = %d" % movieSize)
                except:
                    self.Debug(1, "Could not determine MPEG movie size.")

            if(r == 0):
                retval = (1, moviename, "")

                # Remove the param and the statistics file.
                RemoveFile(paramFile);
                RemoveFile(statFile);

                # Remove the symbolic links.
                linkindex = 0
                for i in range(self.numFrames):
                    for j in range(pad_rate):
                        number4 = "%04d" % linkindex
                        linkname=self.tmpDir+self.slash+linkbase+number4+formatExt
                        self.Debug(5, "Removing link %s" % linkname)
                        RemoveFile(linkname)
                        linkindex = linkindex + 1
            else:
                pName = "VisIt's mpeg2encode"
                s = pName + " program was not successful. No MPEG \n"
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
    # Method: EncodeMovie
    #
    # Purpose:    This method creates a movie with visit_utils.
    #
    # Programmer: Brad Whitlock
    # Date:       Wed Apr  3 17:51:17 PDT 2013
    #
    # Modifications:
    #   Cyrus Harrison, Sat Mar 27 11:51:10 PST 2004
    #   Bring back our old fps / frame duplication heuristic for mpeg encoding.
    #
    ###########################################################################

    def EncodeMovie(self, fmt, moviename, imageFormatString, xres, yres, stereo):
        self.Debug(1, "EncodeMovie")

        # keep old fps / frame duplication heuristic, feed dup rate into 
        # visit_utils.encoding
        #
        # We can only drive the movie at 30 fps.  So if the user wants
        # a movie at 10 fps, we have to pad frames.  Determine what that
        # pad rate is.
        pad_rate=-1
        for i in range(1,31):
           if (pad_rate < 0):
              if ((30./i) <= self.fps):
                 pad_rate = i
                 if ((30./i) != self.fps):
                     print("Because of limitations in MPEG encoding, the ")
                     print("movie will be encoded at %f frames per second" %(30./i))
        formatExt = "." + self.formatInfo[fmt][self.FMT_INPUTFORMAT]
        framePattern = self.tmpDir + self.slash + imageFormatString + formatExt
        absMovieName = self.outputDir + self.slash + moviename

        success = 1
        msg = ""
        # use fdup to set the duplication frame rate
        if encoding.encode(framePattern, absMovieName,fdup=pad_rate) > 0:
            success = 0
            msg = "The movie encoder used in the visit_utils module did not complete successfully."

        return (success, moviename, msg)

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
    #   Brad Whitlock, Mon Nov 6 16:44:58 PST 2006
    #   Added support for stereo.
    #
    #   Brad Whitlock, Thu Apr  4 14:59:38 PDT 2013
    #   Use formatInfo. Use current location for img2sm at LC.
    #
    ###########################################################################

    def EncodeStreamingMovie(self, moviename, imageFormatString, xres, yres, stereo):
        self.Debug(1, "EncodeStreamingMovie")
        retval = 0

        # Look for the location of the img2sm command. The code to get the 
        # latest at LC is adapted from visit_utils.
        img2sm = ""
        if CommandInPath("img2sm"):
            img2sm = "img2sm"
        else:
            if 'SYS_TYPE' in os.environ:
                res = os.path.join("/usr/gapps/asciviz/blockbuster/latest",os.environ["SYS_TYPE"],"bin/img2sm")
                if os.path.exists(res):
                    img2sm = res

        if img2sm != "":
            # Determine the image extension
            ext = "." + self.formatInfo["sm"][self.FMT_INPUTFORMAT]

            # Execute the img2sm command
            r = 0
            if stereo == self.STEREO_LEFTRIGHT:
                # All of the frames are named left*, right* so we need to temporarily
                # rename them all to some common file base where left and right alternate.
                lastSlash = string.find(imageFormatString, self.slash)
                if lastSlash != -1:
                    leftFmt = imageFormatString[:lastSlash+1] + "left_" + imageFormatString[lastSlash+1:] + ext
                    rightFmt = imageFormatString[:lastSlash+1] + "right_" + imageFormatString[lastSlash+1:] + ext
                else:
                    leftFmt = "left_" + imageFormatString + ext
                    rightFmt = "right_" + imageFormatString + ext
                imageFormatStringWext = imageFormatString + ext
                index = 0
                for i in range(self.numFrames):
                    leftFile = leftFmt % i
                    newFile = imageFormatStringWext % index
                    CopyFile(leftFile, newFile, 1)
                    self.Debug(5, "cp %s %s" % (leftFile, newFile))
                    index = index + 1

                    rightFile = rightFmt % i
                    newFile = imageFormatStringWext % index
                    CopyFile(rightFile, newFile, 1)
                    self.Debug(5, "cp %s %s" % (leftFile, newFile))
                    index = index + 1

                # Now make the stereo movie.
                format = self.tmpDir + self.slash + imageFormatString
                command = "%s -rle -stereo -fps %d -first 0 -last %d -form pnm %s%s %s" % \
                          (img2sm, self.fps, (self.numFrames-1)*2, format, ext, moviename)
                self.Debug(1, command)
                r = os.system(command)
            else:
                format = self.tmpDir + self.slash + imageFormatString
                command = "%s -rle -fps %d -first 0 -last %d -form png %s%s %s" % \
                          (img2sm, self.fps, self.numFrames-1, format, ext, moviename)
                self.Debug(1, command)
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
            s = s + "streaming movies, could not be located so your source \n"
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
    #   Brad Whitlock, Wed Apr  3 17:44:42 PDT 2013
    #   Use formatInfo.
    #
    ###########################################################################

    def MakeMovieName(self, fmt, xres, yres, useGeom):
        # Make sure that the movie
        filebase = self.movieBase
        moviename = filebase
        if useGeom == 1:
            filebase = "%s_%dx%d" % (self.movieBase, xres, yres)

        formatExt = self.OutputExtension(fmt)

        ext = filebase[-len(formatExt):]
        if(ext != formatExt):
            moviename = filebase + formatExt
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
            if name in list(movienames.keys()):
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
    #   Brad Whitlock, Wed Sep 20 15:02:40 PST 2006
    #   I made it send e-mail at the end.
    #
    #   Brad Whitlock, Thu Dec 21 19:14:02 PST 2006
    #   Use a safer function to get the host.
    #
    #   Kathleen Bonnell, Fri Jul 20 10:36:10 PDT 2007 
    #   Use self.outputDir to report where movie was stored when completed. 
    #
    #   Brad Whitlock, Wed Apr  3 17:45:51 PDT 2013
    #   Change how encoders are invoked.
    #
    ###########################################################################

    def EncodeFrames(self):
        self.Debug(1, "EncodeFrames")

        # Count the number of movies that have to get encoded.
        nEncodedMovies = 0
        for format in self.movieFormats:
            fmt = format[1]
            if self.formatInfo[fmt][self.FMT_ENCODER] != None:
                nEncodedMovies = nEncodedMovies + 1

        # Send a "stage transition" by changing the text.
        if nEncodedMovies > 0:
            progress = int(self.percentAllocationFrameGen * 100.)
            self.SendClientProgress("Encoding movie", progress, 100)

        safeToRemoveFrames = 1
        count = 0
        framesGenerated = 0
        framesGeneratedMessage = ""
        framesGeneratedEmail = ""
        # Compose an e-mail message
        host = "?"
        try:
            host = socket.gethostname()
        except ImportError:
            try:
                host = os.uname()[1]
            except AttributeError:
                host = "?"
        emailMsg = """
DO NOT REPLY TO THIS AUTOMATICALLY GENERATED MESSAGE.
                       
Message from \"visit -movie\" running on %s.\n\n""" % host

        for format in self.movieFormats:
            formatString = format[0]
            fmt = format[1]
            xres = format[2]
            yres = format[3]
            stereo = format[4]
            val = 0

            # Come up with a movie name.
            movieName = self.CreateMovieName(fmt, xres, yres)

            # Encode the movie if needed.
            encodeMovie = 0
            if(fmt == "mpeg"):
                val = self.EncodeMPEGMovie(movieName, formatString, xres, yres)
                encodeMovie = 1
            elif(fmt == "sm"):
                val = self.EncodeStreamingMovie(movieName, formatString, xres, yres, stereo)
                encodeMovie = 1
            elif self.formatInfo[fmt][self.FMT_ENCODER] != None:
                val = self.EncodeMovie(fmt, movieName, formatString, xres, yres, stereo)
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
                s = s + " They are located at: \n%s." % self.outputDir
                s2 = "VisIt completed generating frames."
                s2 = s2 + " They are located at: %s." % self.outputDir
                framesGenerated = 1
                framesGeneratedMessage = s
                framesGeneratedEmail = s2

            elif val[0] == 0:
                s =     "VisIt was not able to complete your movie:\n"
                s = s + "%s because of the following problem:\n\n" % val[1]
                s = s + val[2]
                self.ClientMessageBox(s)
                safeToRemoveFrames = 0

                # Try and read the log so we can include it in the e-mail.
                logContents = ""
                try:
                    if self.log != 0:
                        # Close the log
                        self.log.flush()
                        self.log.close()
                        # Read the log's contents
                        f = open(self.log.name, "rt")
                        lines = f.readlines()
                        f.close()
                        for line in lines:
                            logContents = logContents + line
                        if len(logContents) > 0:
                            s1 = "#########################################################################\n"
                            s2 = "Contents of \"visit -movie\" log file.\n"
                            logContents = "\n\n" + s1 + s2 + s1 + logContents
                        # Set the log to 0 so we don't try to close it later.
                        self.log = 0
                except:
                    logContents = ""

                self.SendEmail("Movie generation error", emailMsg + s + logContents)
            else:
                s =     "VisIt completed your movie. It is located at: \n"
                s = s + "%s." % self.outputDir
                self.ClientMessageBox(s)
                s2 =     "VisIt completed your movie. It is located at: "
                s2 = s2 + "%s." % self.outputDir
                self.SendEmail("Movie completed", emailMsg + s2)

        # Tell the user that frames were generated.
        if framesGenerated:
            self.ClientMessageBox(framesGeneratedMessage)
            self.SendEmail("Frames completed", emailMsg + framesGeneratedEmail)

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
        if(self.tmpDir != self.outputDir):
            self.Debug(1, "Need to delete %s directory!" % self.tmpDir)
            tmpfiles = os.listdir(self.tmpDir)
            for f in tmpfiles:
                os.remove("%s%s%s" % (self.tmpDir, self.slash, f))
            os.rmdir(self.tmpDir)
