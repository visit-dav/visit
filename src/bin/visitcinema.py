##############################################################################
#
# Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# All rights reserved.
#
# This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or materials provided with the distribution.
#  - Neither the name of the UC/LLNL nor  the names of its contributors may be
#    used to  endorse or  promote products derived from  this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
# CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
# ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
##############################################################################

import math, os, string, sys

###############################################################################
# Class: VisItCinema
#
# Purpose:    This class makes Cinema databases.
#
# Programmer: Brad Whitlock
# Date:       Tue Sep 12 13:51:46 PDT 2017
#
# Modifications:
#
###############################################################################

class VisItCinema(object):
    def __init__(self):
        super(VisItCinema,self).__init__()
        self.formatInfo = {
            "bmp"  : ("bmp",),\
            "exr"  : ("exr",),\
            "jpeg" : ("jpeg", "jpg"),\
            "png"  : ("png",), \
            "tiff" : ("tif", "tiff")
        }
        self.cameraModes = (
            "static",
            "phi-theta"
        )
        # disable 'C' specification until it is fixed
        #self.specifications = ("A", "C", "D")
        self.specifications = ("A", "D")

        self.specification = "A"
        self.cameraMode = "static"
        self.format = "png";
        self.theta = 7
        self.phi = 12
        self.width = 800
        self.height = 800
        self.screenCaptureImages = 0
        self.fileName = "visit.cdb"
        self.frameStart = 0
        self.frameEnd = -1
        self.frameStride = 1
        self.sendClientFeedback = 0
        self.log = 0
        self.debug_real = [0,0,0,0,0]
        self.sessionFile = None
        self.scriptFile = None
        # disable composite, which can only be used with C specification until it is fixed
        #self.composite = False
        self.vars = []

        # Try and open debug logs
        debug_level = GetDebugLevel()
        if debug_level > 0:
            for i in range(debug_level):
                debug_name = "visit-cinema.%d.vlog" % (i + 1)
                try:
                    self.debug_real[i] = open(debug_name, "wt")
                except IOError: 
                    print "Could not open debug log %s" % debug_name
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
    # Date:       Tue Sep 19 10:13:13 PDT 2017
    #
    # Modifications:
    #
    ###########################################################################

    def PrintUsage(self):
        print "Usage: visit -cinema [-output dbname] [-format fmt] [-geometry size]"
        print "                     [-start index] [-end index] [-stride step]"
        print "                     [-theta n] [-phi n] [-camera mode]"
        print "                     [-sessionfile name | -scriptfile name] "
        # disable C specification and composite until C-spec is fixed.
        #print "                     [-specification A|C|D] [-docomposite] [-var varname]"
        print "                     [-specification A|D]"
        print ""
        print "OPTIONS"
        print "    The following options are recognized by visit -cinema"
        print ""
        print "-output dbname       The name of the output Cinema database. If the"
        print "                     name does not end in .cdb then that suffix will"
        print "                     be added. A directory with this name will be"
        print "                     created and files will be saved in it."
        print ""
        print "-format fmt          The file format to use when saving images."
        s = ""
        for fmt in self.formatInfo.keys():
            s = s + " " + fmt
        print "                      (" + s + ")"
        print "-geometry wxh        Set the size of the output images."
        print ""
        print "-start index         The starting index in the input database."
        print ""
        print "-end   index         The ending index in the input database."
        print ""
        print "-stride  incr        The number of frame indices to increment to "
        print "                     go to the next time step."
        print ""      
        print "-camera mode         Set the mode for the Cinema database. The mode"
        print "                     value must be one of: ",str(self.cameraModes)[1:-1]
        print ""
        print "-theta n             The number of theta divisions for a phi-theta camera."
        print ""
        print "-phi n               The number of phi divisions for a phi-theta camera."
        print ""
        print "-sessionfile name    The sessionfile option lets you pick the name"
        print "                     of the VisIt session to use as input for setting"
        print "                     up plots. The VisIt session is a file that describes"
        print "                     the movie that you want to make and it is created"
        print "                     when you save your session from within VisIt's "
        print "                     GUI after you set up your plots how you want them."
        print ""
        print "-scriptfile name     The scriptfile option lets you pick the name"
        print "                     of a VisIt Python script to use as input for your"
        print "                     movie."
        print ""
        # disable C specification and composite until C-spec is fixed.
        #print "-specification spec  Set the database specification to A, C, D"
        print "-specification spec  Set the database specification to A, D"
        print ""
        #print "-docomposite         Tell the script to produce composite images."
        #print "                     In composite mode, images that store luminance, Z, "
        #print "                     and value images are produced."
        #print ""
        #print "-var varname         Add a variable to the list of variables (composite only)."

    ###########################################################################
    # Method: Log
    #
    # Purpose:    Outputs log messages.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    # Modifications:
    #
    ###########################################################################

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
            ClientMethod("CinemaProgress", args)

    ###########################################################################
    # Method: ClientMessageBox
    #
    # Purpose:    Sends the client MessageBoxOk client method.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Sep 12 12:05:28 PDT 2017
    #
    ###########################################################################

    def ClientMessageBox(self, s):
        if self.sendClientFeedback:
            ClientMethod("MessageBoxOk", str(s))
        else:
            print str(s)

    ###########################################################################
    # Method: ProcessArguments
    #
    # Purpose:    This method processes arguments for the program.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Sep 12 12:05:28 PDT 2017
    #
    # Modifications:
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
        #print commandLine
        i = 0
        processingLA = 0
        while(i < len(commandLine)):
            # We're processing arguments as usual.
            if(commandLine[i] == "-format"):
                if((i+1) < len(commandLine)):
                    # Look up the format.
                    fmt = commandLine[i+1]
                    for f in self.formatInfo.keys():
                        if fmt in self.formatInfo[f]:
                            self.format = f
                            break
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-output"):
                if((i+1) < len(commandLine)):
                    self.fileName = commandLine[i+1]
                    if self.fileName[-4:] != ".cdb":
                        self.fileName = self.fileName + ".cdb"
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-geometry"):
                if((i+1) < len(commandLine)):
                    try:
                        geom = [int(x) for x in string.split(commandLine[i+1], "x")]
                        self.width,self.height = geom
                    except:
                        pass
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
                        print "A bad value was provided for frame start. Using a frame start of 0."
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
            elif(commandLine[i] == "-stride"):
                if((i+1) < len(commandLine)):
                    try:
                        self.frameStride = int(commandLine[i+1])
                        if(self.frameStride < 1):
                            self.frameStride = 1
                    except ValueError:
                        self.frameStride = 1
                        print "A bad value was provided for frame step. Using a frame stride of 1."
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-theta"):
                if((i+1) < len(commandLine)):
                    try:
                        self.theta = int(commandLine[i+1])
                        if(self.theta < 1):
                            self.theta = 1
                    except ValueError:
                        print "A bad value was provided for theta. ", commandLine[i+1]
                    i = i + 1
            elif(commandLine[i] == "-phi"):
                if((i+1) < len(commandLine)):
                    try:
                        self.phi = int(commandLine[i+1])
                        if(self.phi < 1):
                            self.phi = 1
                    except ValueError:
                        print "A bad value was provided for phi. ", commandLine[i+1]
                    i = i + 1
            elif(commandLine[i] == "-camera"):
                if((i+1) < len(commandLine)):
                    if commandLine[i+1] in self.cameraModes:
                        self.cameraMode = commandLine[i+1]
                    else:
                        print "A bad value was provided for camera. Using camera ", self.cameraMode
                    i = i + 1
            elif(commandLine[i] == "-sessionfile"):
                if((i+1) < len(commandLine)):
                    self.sessionFile = commandLine[i+1]
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-scriptfile"):
                if((i+1) < len(commandLine)):
                    self.scriptFile = commandLine[i+1]
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            elif(commandLine[i] == "-specification"):
                if((i+1) < len(commandLine)):
                    if commandLine[i+1] in self.specifications:
                        self.specification = commandLine[i+1]
                    i = i + 1
                else:
                    self.PrintUsage()
                    sys.exit(-1)
            # disable composite , which can only be used with C specification, until it is fixed
            #elif(commandLine[i] == "-composite" or commandLine[i] == "-docomposite"):
            #    self.composite = True
            #elif(commandLine[i] == "-var"):
            #    if((i+1) < len(commandLine)):
            #        self.vars = self.vars + [commandLine[i+1]]
            #        i = i + 1
            #    else:
            #        self.PrintUsage()
            #        sys.exit(-1)

            # On to the next argument.
            i = i + 1

        # Write out a log file.
        try:
            logName = os.path.join(os.path.abspath(os.curdir), "%s.log" % self.fileName)
            self.log = open(logName, "wt")
        except:
            self.log = 0

        # Do a little checking.
        #if self.composite and self.specification != "C":
        #    print "Composite images can only be selected with specification C."
        #    self.composite = False
        #if self.screenCaptureImages and self.specification == "C":
        #    print "Screen capture cannot be used with specification C."
        #    self.screenCaptureImages = 0

    ###########################################################################
    # Method: SaveImage
    #
    # Purpose:    This method tells VisIt to save an RGB image.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Jul 28 13:58:06 PST 2003
    #
    # Modifications:
    #
    ###########################################################################

    def SaveImage(self, filename, width, height, frameFormat):
        s = SaveWindowAttributes()
        old_sw = GetSaveWindowAttributes()
        s.saveTiled = old_sw.saveTiled
        s.family = 0
        s.screenCapture = self.screenCaptureImages
        s.resConstraint = s.NoConstraint
        s.width = width
        s.height = height
        s.fileName = os.path.basename(filename)
        s.outputToCurrentDirectory = 0
        s.outputDirectory = os.path.dirname(filename)
        if 0: #self.composite:
            s.pixelData = 4|8|16 #FIXME: s.Luminance | s.Value | s.Depth
        else:
            s.pixelData = 1 #s.ColorRGB

        # Determine the format of the frame that VisIt needs to save.
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
        elif(frameFormat == "exr"):
            s.format =  s.EXR
        SetSaveWindowAttributes(s)

        self.Debug(5, "SaveImage: %s" % filename)
        retval = 0
        try:
            retval = SaveWindow()
        except VisItInterrupt:
            raise
        except:
            retval = 0

        # Restore the old save window atts.
        SetSaveWindowAttributes(old_sw)

        return retval

    def SaveValueImage(self, filename, width, height):
        s = SaveWindowAttributes()
        old_sw = GetSaveWindowAttributes()
        s.saveTiled = old_sw.saveTiled
        s.family = 0
        s.screenCapture = 0
        s.resConstraint = s.NoConstraint
        s.width = width
        s.height = height
        s.fileName = os.path.basename(filename)
        s.outputToCurrentDirectory = 0
        s.outputDirectory = os.path.dirname(filename)
        s.pixelData = 8 #FIXME: s.Value
        s.format =  s.PNG
        SetSaveWindowAttributes(s)

        self.Debug(5, "SaveValueImage: %s" % filename)
        retval = 0
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
    # Method: CreatePlots
    #
    # Purpose:    Creates plots
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Sep 12 12:05:28 PDT 2017
    #
    # Modifications:
    #
    ###########################################################################

    def CreatePlots(self):
        if self.sessionFile != None:
            self.Debug(1, "Setting up plots from session file:"+self.sessionFile)
            RestoreSession(self.sessionFile, 0)
        elif self.scriptFile != None:
            Source(self.scriptFile)
        else:
            OpenDatabase("~/Development/data/wave/wave*.silo database")
            AddPlot("Pseudocolor", "pressure")
            DrawPlots()

    ###########################################################################
    # Method: DirectoryExists
    #
    # Purpose:    This method determines whether a directory exist.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Sep 11 17:33:38 PDT 2017
    #
    ###########################################################################
    def DirectoryExists(self, dirName):
        try:
           s = os.stat(dirName)
           retval = 1
        except:
           retval = 0
        return retval

    ###########################################################################
    # Method: JSONList
    #
    # Purpose:    This method turns a tuple into a string containing a JSON list.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Sep 11 17:33:38 PDT 2017
    #
    ###########################################################################
    def JSONList(self, slist):
        s = "["
        n = len(slist)
        for i in xrange(n):
            if type(slist[i]) == type(""):
                s = s + "\"%s\"" % slist[i]
            else:
                s = s + str(slist[i])
            if i < n-1:
               s = s +", "
        s = s + "]"
        return s

    ###########################################################################
    # Method: CreateContainerDirectory
    #
    # Purpose:    This method creates the containing directory for the Cinema database.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Sep 11 17:33:38 PDT 2017
    #
    ###########################################################################

    def CreateContainerDirectory(self):
        # Create the directory where we'll make the files.
        cdbDir = os.path.abspath(self.fileName)
        try:
            if not self.DirectoryExists(cdbDir):
                self.Debug(1, "Creating directory "+cdbDir)
                os.mkdir(cdbDir)
        except:
            self.Debug(1, "Error creating directory "+cdbDir)
            return ""
        try:
            imageDir = os.path.join(cdbDir, "image")
            if not self.DirectoryExists(imageDir):
                self.Debug(1, "Creating directory "+imageDir)
                os.mkdir(imageDir)
        except:
            self.Debug(1, "Error creating directory "+imageDir)
            return ""
        return cdbDir,imageDir

    ###########################################################################
    # Method: CreateDirectory
    #
    # Purpose:    This method creates a path of directories if they need to be
    #             created.
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 28 10:51:02 PDT 2017
    #
    ###########################################################################

    def CreateDirectory(self, baseDir, subdirs):
        # We assume that baseDir exists.
        dirName = baseDir
        for d in subdirs:
            thisDirName = os.path.join(dirName, d)
            try:
                if not self.DirectoryExists(thisDirName):
                    self.Debug(1, "Creating directory "+thisDirName)
                    os.mkdir(thisDirName)
            except:
                self.Debug(1, "Error creating directory "+thisDirName)
                return dirName
            dirName = thisDirName
        return dirName

    ###########################################################################
    # Method: RenameFile
    #
    # Purpose:    This method renames a file.
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 28 10:51:02 PDT 2017
    #
    ###########################################################################
    def RenameFile(self, old, new):
        try:
            os.rename(old, new)
        except:
            pass
        self.Debug(5, "Renaming %s -> %s" % (old, new))

    ###########################################################################
    # Method: PrintState
    #
    # Purpose:    This method prints the state.
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 28 10:51:02 PDT 2017
    #
    ###########################################################################

    def PrintState(self):
        print "====================================="
        print "VisIt Cinema Script"
        print "====================================="
        print "Specification:  ", self.specification
        print "Camera Mode:    ", self.cameraMode
        print "Format:         ", self.format
        print "Theta:          ", self.theta
        print "Phi:            ", self.phi
        print "Width:          ", self.width
        print "Height:         ", self.height
        # disable Composite, which can only be used with C specification,
        # until C spec is fixed.
        #print "Composite:      ", self.composite
        print "Screen Capture: ", self.screenCaptureImages
        print "Filename:       ", self.fileName
        print "Frame Start:    ", self.frameStart
        print "Frame End:      ", self.frameEnd
        print "Frame Stride:   ", self.frameStride
        print "Variables:      ", self.vars
        print "====================================="

    ###########################################################################
    # Method: Execute
    #
    # Purpose:    This is the main method that starts making the Cinema db.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Sep 12 12:05:28 PDT 2017
    #
    # Modifications:
    #
    ###########################################################################

    def Execute(self):
        # Get the current time slider's state
        tsState = -1
        windowInfo = GetWindowInformation()
        if windowInfo.activeTimeSlider >= 0:
            tsState = windowInfo.timeSliderCurrentStates[windowInfo.activeTimeSlider]

        # Turn off some annotations.
        backup_annot = GetAnnotationAttributes()
        annot = GetAnnotationAttributes()
        annot.databaseInfoFlag = 0
        annot.userInfoFlag = 0

        # Suppress query output
        SuppressQueryOutputOn()

        # Make sure plots are drawn.
        DrawPlots()

        # Use the plots that are currently set up. Iterate over all of
        # the frames and save them out.
        if 0: #self.composite:
            # We turn off more things if composite images.
            annot.axes3D.visible = 0
            annot.axes3D.bboxFlag = 0
            annot.legendInfoFlag = 0
            annot.axes3D.triadFlag = 0
            SetAnnotationAttributes(annot)

            if self.cameraMode == "static":
                self.CreateStaticDatabaseComposite()
            elif self.cameraMode == "phi-theta":
                self.CreatePhiThetaDatabaseComposite()
        else:
            # We turn off more things if EXR (composite) images.
            if self.format == "exr":
                annot.axes3D.visible = 0
                annot.axes3D.bboxFlag = 0
                annot.legendInfoFlag = 0
                annot.axes3D.triadFlag = 0
            SetAnnotationAttributes(annot)

            if self.cameraMode == "static":
                self.CreateStaticDatabase()
            elif self.cameraMode == "phi-theta":
                self.CreatePhiThetaDatabase()

        # Restore some settings.
        if tsState != -1:
            SetTimeSliderState(tsState)
        SetAnnotationAttributes(backup_annot)
        SuppressQueryOutputOff()

    ###########################################################################
    # Method: IterateAndSaveFrames
    #
    # Purpose:    Iterate over time and save frames using a save function.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Sep 12 12:05:28 PDT 2017
    #
    # Modifications:
    #
    ###########################################################################

    def IterateAndSaveFrames(self, outputDir, saveFunc):
        self.Debug(1, "IterateAndSaveFrames")
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
        self.PrintState()

        # Save an image for each frame in the animation.
        i = self.frameStart
        drawThePlots = 0
        nTotalFrames = self.frameEnd - self.frameStart + 1
        lastProgress = -1
        numFrames = 0
        timeAtts = []
        time0 = ""
        while(i <= self.frameEnd):
            # Send progress to the GUI.
            t = float(i - self.frameStart) / float(nTotalFrames)
            progress = int(t * 100.)
            if progress != lastProgress:
                self.SendClientProgress("Generating frames", progress, 100)
                lastProgress = progress

            # Make sure we draw the plots.
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

            # Query the simulation time.
            Query("Time")
            currentTime = "%e" % GetQueryOutputValue()
            if numFrames == 0:
                time0 = currentTime
            else:
                # File metadata with times is not reliable for time-series
                # or for certain file formats. We want to be able to catch
                # 2 same time values in a row to we can use different values
                # in order to name directories such that files do not overwrite.
                if time0 == currentTime:
                    currentTime = "%e" % numFrames

            # Save the output image.
            if saveFunc(outputDir, numFrames, currentTime) == 0:
                print "There was an error when trying to save the window "\
"for time slider state %d. VisIt will now try to redraw the plots and re-save "\
"the window." % i
                if(DrawPlots() == 0):
                    print "VisIt could not draw plots for time slider "\
"state %d. You should investigate the files used for that state." % i
                else:
                    if saveFunc(outputDir, numFrames, currentTime) == 0:
                        print "VisIt could not re-save the window for "\
"time slider state %d. You should investigate the files used for that state." % i

            numFrames = numFrames + 1
            i = i + self.frameStride
            timeAtts = timeAtts + [currentTime]
        return timeAtts

    ###########################################################################
    # Method: GetActivePlotVar
    #
    # Purpose:    This method returns the variable name for the first active plot.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Sep 11 17:33:38 PDT 2017
    #
    ###########################################################################

    def GetActivePlotVar(self):
        pL = GetPlotList()
        usablePlots = self.GetUsablePlots()
        np = len(usablePlots)
        if len(usablePlots) > 0:
            plotVar = pL.GetPlots(usablePlots[0]).plotVar
        else:
            if pL.GetNumPlots() > 0:
                plotVar = pL.GetPlots(0).plotVar
            else:
                plotVar = "scalar"
        return plotVar

    ###########################################################################
    # Method: CreateStaticDatabase
    #
    # Purpose:    This method creates the images for a static Cinema database.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Sep 11 17:33:38 PDT 2017
    #
    ###########################################################################

    def CreateStaticDatabase(self):
        def static_save(outputDir, timestep, currentTime):
            filename = "%s/time_%s.%s" % (outputDir, currentTime, self.format)
            self.SaveImage(filename,  self.width, self.height, self.format)

        self.Debug(1, "CreateStaticDatabase")

        # Create the directory where we'll make the files.
        cdbDir, imageDir = self.CreateContainerDirectory()
        if cdbDir == "":
            return False

        # Save the frames.
        timeAtts = self.IterateAndSaveFrames(imageDir, static_save)

        if self.specification == "D":
            # Make the CSV file
            filename = os.path.join(cdbDir, "data.csv")
            f = open(filename, "wt")
            f.write("time,FILE\n")
            for i in xrange(len(timeAtts)):
                f.write("%s,image/time_%s.%s\n" % (timeAtts[i], timeAtts[i], self.format))
            f.close()
        else:
            # Make the JSON file
            filename = os.path.join(imageDir, "info.json")  #TODO: see if we can have the info.json above imageDir
            f = open(filename, "wt")
            f.write('{\n')
            f.write('  "type" : "simple",\n')
            f.write('  "version": "1.1",\n')
            f.write('  "metadata":{"type":"parametric-image-stack"},\n')
            f.write('  "name_pattern":"time_{time}.%s",\n' % self.format)
            f.write('  "arguments":{\n')
            f.write('    "time": {\n')
            f.write('       "default":\"%s\",\n' % timeAtts[0])
            f.write('       "label":"Time",\n')
            f.write('       "type":"range",\n')
            f.write('       "values":%s\n' % self.JSONList(timeAtts))
            f.write('    }\n')
            f.write('  }\n')
            f.write('}\n')
            f.close()

    ###########################################################################
    # Method: GetUsablePlots
    #
    # Purpose:    This method gets the list of "usable" plots (realized plots)
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 28 10:51:02 PDT 2017
    #
    ###########################################################################

    def GetUsablePlots(self):
        pL = GetPlotList()
        usablePlots = []
        for i in xrange(pL.GetNumPlots()):
            plot = pL.GetPlots(i)
            if plot.stateType == plot.Completed:
                usablePlots = usablePlots + [i]
        return tuple(usablePlots)

    ###########################################################################
    # Method: WriteCompositeJSONFile
    #
    # Purpose:    This method writes the JSON file needed for a composite database.
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 28 10:51:02 PDT 2017
    #
    ###########################################################################

    def WriteCompositeJSONFile(self, outputDir, params, np, plotVarRanges):
        filename = os.path.join(outputDir, "info.json")
        f = open(filename, "wt")
        plot1toN = ["plot%d"%p for p in xrange(1,np+1)]
        json_plot1toN = str(self.JSONList(plot1toN))
        f.write('{\n')
        f.write('  "constraints": {\n')
        f.write('     "image": {\n')
        f.write('        "vis": %s\n' % json_plot1toN)
        f.write('     },\n')
        for i in xrange(len(plot1toN)):
            f.write('     "%s": {\n' % plot1toN[i])
            f.write('        "vis": "%s"\n' % plot1toN[i])
            f.write('     }')
            if i < len(plot1toN)-1:
                f.write(',')
            f.write('\n')
        f.write('  },\n')
        f.write('  "metadata": {\n')
        f.write('     "type":"composite-image-stack",\n')
        f.write('     "version": "0.2",\n')
        f.write('     "store_type": "FS",\n')
        f.write('     "camera_model" : "%s",\n' % self.cameraMode)
        f.write('     "endian": "%s",\n' % sys.byteorder)
        f.write('     "image_size": [%d, %d],\n' % (self.width, self.height))
        f.write('     "value_mode" : 2\n')
        # Camera data optional
#        f.write('     "pipeline": [\n')
#        f.write('        {\n')
#        f.write('           "children": [],\n')
#        f.write('           "id": "100",\n')
#        f.write('           "name": "plot1",\n')
#        f.write('           "parents": [\n')
#        f.write('              "0"\n')
#        f.write('           ],\n')
#        f.write('           "visibility": 1\n')
#        f.write('        }\n')
#        f.write('     ]\n')
        f.write('  },\n')
        # Write user-supplied params like time, etc.
        keys = sorted(params.keys())
        kpath = string.join(["{"+str(k)+"}" for k in keys], "/")
        f.write('  "name_pattern":"%s/{vis}/{image}.png",\n' % kpath)
        f.write('  "parameter_list":{\n')
        for k in keys:
            f.write('    "%s": {\n' % k)
            f.write('       "default":%s,\n' % params[k]["default"])
            f.write('       "label":"%s",\n' % params[k]["label"])
            f.write('       "type":"%s",\n' % params[k]["type"])
            f.write('       "values":%s\n' % params[k]["values"])
            f.write('    },\n')
        # Write fixed params.
        keys = sorted(plotVarRanges.keys())
        f.write('    "image": {\n')
        f.write('       "default": "%s",\n' % keys[0])
        f.write('       "label": "image",\n')
        f.write('       "role": "field",\n')
        f.write('       "type": "hidden",\n')
        f.write('       "types": [\n')
        f.write('          "luminance",\n')
        f.write('          "depth",\n')
        for i in xrange(len(keys)):
            f.write('          "value"')
            if i < len(keys)-1:
                f.write(',')
            f.write('\n')
        f.write('       ],\n')
        f.write('       "valueRanges": {\n')
        for i in xrange(len(keys)):
            f.write('          "%s": [%g, %g]' % (keys[i], plotVarRanges[keys[i]][0], plotVarRanges[keys[i]][1]))
            if i < len(keys)-1:
                f.write(',')
            f.write('\n')
        f.write('       },\n')
        f.write('       "values": [\n')
        f.write('          "luminance",\n')
        f.write('          "depth",\n')
        for i in xrange(len(keys)):
            f.write('          "%s"' % keys[i])
            if i < len(keys)-1:
                f.write(',')
            f.write('\n')
        f.write('       ]\n')
        f.write('    },\n')
        f.write('    "vis": {\n')
        f.write('       "default": "plot1",\n')
        f.write('       "label": "vis",\n')
        f.write('       "role": "layer",\n')
        f.write('       "type": "option",\n')
        f.write('       "values": %s\n' % json_plot1toN)
        f.write('    }\n')
        f.write('  }\n')
        f.write('}\n')
        f.close()

    ###########################################################################
    # Method: IteratePlotsAndSaveFrames
    #
    # Purpose:    This method iterates over the plots, showing them one at a
    #             time and saving a "vis" image for each.
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 28 10:51:02 PDT 2017
    #
    ###########################################################################

    def IteratePlotsAndSaveFrames(self, outputDir, timestep, currentTime, saveFunc):
        # Figure out which plots are active and visible.
        pL = GetPlotList()
        activePlots = []
        visible = []
        for i in xrange(pL.GetNumPlots()):
            plot = pL.GetPlots(i)
            visible = visible + [plot.hiddenFlag == 0]
            if plot.activeFlag:
                activePlots = activePlots + [i]
        self.Debug(5, "activePlots = " + str(activePlots))
        self.Debug(5, "visible = " + str(visible))

        # Figure out the "usable" plots.
        usablePlots = self.GetUsablePlots()
        self.Debug(5, "usablePlots = " + str(usablePlots))

        # Iterate over the usable plots and save images for each independently.
        currentVisible = list(visible)
        vis = 0
        for plotId in usablePlots:
            # Figure out which plots must be turned off so only plotId is visible.
            turnoff = []
            for i in xrange(len(currentVisible)):
                if currentVisible[i] and i != plotId:
                    turnoff = turnoff + [i]
                    currentVisible[i] = 0
            if len(turnoff) > 0:
                SetActivePlots(tuple(turnoff))
                HideActivePlots()
                self.Debug(5, "Turning off plots " + str(turnoff))
            # Turn on the one plot we want.
            SetActivePlots(plotId)
            if currentVisible[plotId] == 0:
                HideActivePlots()
                self.Debug(5, "Made sure plot %d is visible." % plotId)
                currentVisible[plotId] = 1
                self.Debug(5, "currentVisible = " + str(currentVisible))

            # Save the file we need for this vis.
            saveFunc(outputDir, timestep, currentTime, vis)
            vis = vis + 1

        # Now, only usablePlots[-1] will be visible. It had to start out that way.
        # Turn on the rest of the plots that need to be visible.
        turnon = []
        for i in xrange(len(visible)):
            if visible[i] == 1 and i != usablePlots[-1]:
                turnon = turnon + [i]
        if len(turnon) > 0:
            SetActivePlots(tuple(turnon))
            HideActivePlots()
            self.Debug(5, "Turning on plots " + str(turnon))

        # Restore the active plots
        SetActivePlots(tuple(activePlots))
        self.Debug(5, "Restoring active plots " + str(activePlots))
        return

    ###########################################################################
    # Method: CreateStaticDatabaseComposite
    #
    # Purpose:    This method creates the images for a static composited
    #             Cinema database.
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 28 10:51:02 PDT 2017
    #
    ###########################################################################

    def CreateStaticDatabaseComposite(self):
        # We get here from IteratePlotsAndSaveFrames
        def static_save_plot(outputDir, timestep, currentTime, vis):
            # Messing with plot visibility sometimes affects the view.
            # We get this view from CreateStaticDatabaseComposite
            SetView3D(initialView)

            # Create the right vis directory if needed.
            imgDir = self.CreateDirectory(outputDir, ("time=%d" % timestep, "vis=%d" % vis))

            # Iterate over variables.
            ioffset = 0
            for i in xrange(len(self.vars)):
                # Change to the right var if needed.
                ChangeActivePlotsVar(self.vars[i])

                # Get the minmax. We get plotVarRanges from CreateStaticDatabaseComposite
                Query("MinMax")
                plotRange = GetQueryOutputValue()
                if plotRange[0] < plotVarRanges[self.vars[i]][0]:
                    plotVarRanges[self.vars[i]][0] = plotRange[0]
                if plotRange[1] > plotVarRanges[self.vars[i]][1]:
                    plotVarRanges[self.vars[i]][1] = plotRange[1]

                # Save.
                if i == 0:
                    filename = "%s/image.%s" % (imgDir, self.format)
                    self.SaveImage(filename,  self.width, self.height, self.format)

                    # We need to rename some files.
                    if self.format == "exr":
                        # exr stores all data into the same file.
                        self.RenameFile(os.path.join(imgDir, "image."+self.format), os.path.join(imgDir,"image=0."+self.format))
                        ioffset = 0
                    else:
                        # We will have gotten image.png, image.depth.Z, image.value.Z
                        self.RenameFile(os.path.join(imgDir, "image."+self.format), os.path.join(imgDir,"image=0."+self.format))
                        self.RenameFile(os.path.join(imgDir, "image.depth.Z"), os.path.join(imgDir,"image=1.Z"))
                        self.RenameFile(os.path.join(imgDir, "image.value.Z"), os.path.join(imgDir,"image=2.Z"))
                        ioffset = 2
                else:
                    filename = "%s/image.%s" % (imgDir, self.format)
                    self.SaveValueImage(filename, self.width, self.height)
                    self.RenameFile(os.path.join(imgDir, "image.value.Z"), os.path.join(imgDir,"image=%d.Z" % (ioffset+i)))

        # We get here from IterateAndSaveFrames
        def static_save(outputDir, timestep, currentTime):
            self.IteratePlotsAndSaveFrames(outputDir, timestep, currentTime, static_save_plot)

        self.Debug(1, "CreateStaticDatabase")

        # Create the directory where we'll make the files.
        cdbDir, imageDir = self.CreateContainerDirectory()
        if cdbDir == "":
            return False

        # We have a couple problems here. First, different plots have different
        # variables and might come from various databases. We have "vis" as a 
        # layer with different plots but this means using the same variable 
        # in each layer, which may not make sense. We need a way to indicate
        # that we have multiple plots, each with its own images and variable
        # and extents. Would I then create another "image" parameter and 
        # directory level?

        # Get the plot var name
        pL = GetPlotList()
        usablePlots = self.GetUsablePlots()
        np = len(usablePlots)
        if len(usablePlots) > 0:
            plotVar = pL.GetPlots(usablePlots[0]).plotVar
        else:
            plotVar = pL.GetPlots(0).plotVar
            np = 1

        # Make sure that the current plot var is in the list of vars that we 
        # want to plot.
        if plotVar not in self.vars:
            self.vars = sorted([plotVar] + self.vars)
        else:
            self.vars = sorted(self.vars)
        plotVarRanges = {}
        for v in self.vars:
            plotVarRanges[v] = [0.,0.]

        # Get the initial view. This is used in static_save_plot
        initialView = GetView3D()

        # Save the frames.
        timeAtts = self.IterateAndSaveFrames(imageDir, static_save)

        # Make the JSON file
        params = {"time":{"default":'"'+timeAtts[0]+'"', "label":"Time","type":"range","values":self.JSONList(timeAtts)}}
        self.WriteCompositeJSONFile(imageDir, params, np, plotVarRanges)

    ###########################################################################
    # Method: compute_theta_phi
    #
    # Purpose:    Compute the theta and phi values we'll use.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Sep 11 17:33:38 PDT 2017
    #
    ###########################################################################

    def compute_theta_phi(self):
        ntheta = self.theta
        nphi = self.phi

        # Compute the theta/phi values we use.
        theta = [0]*ntheta
        phi   = [0]*nphi
        for itheta in xrange(ntheta):
            if ntheta > 1:
                tt = float(itheta) / float(ntheta-1)
            else:
                tt = 0.
            theta[itheta] = ((1.-tt) * -math.pi/2.) + (tt*math.pi/2.)
        for iphi in xrange(nphi):
            if nphi > 1:
                tp = float(iphi) / float(nphi-1)
            else:
                tp = 0.
            phi[iphi] = ((1.-tp) * -math.pi) + (tp*math.pi)
        return theta,phi

    ###########################################################################
    # Method: CreatePhiThetaDatabase
    #
    # Purpose:    This method creates the images for a static Cinema database.
    #
    # Programmer: Brad Whitlock
    # Date:       Mon Sep 11 17:33:38 PDT 2017
    #
    ###########################################################################

    def CreatePhiThetaDatabase(self):
        def spherical_to_cartesian(theta, phi):
            return ( math.cos(theta)*math.cos(phi), math.sin(theta), math.cos(theta)*math.sin(phi) )

        def ideg(rad):
            return int((360. * rad) / (2. * math.pi))

        def phi_theta_save(outputDir, timestep, currentTime):
            view = GetView3D()

            for iphi in xrange(len(phi)):
                phiDir="phi_%d"%ideg(phi[iphi])
                for itheta in xrange(len(theta)):
                    thetaDir="theta_%d"%ideg(theta[itheta])

                    # Create the directory we need.
                    imgDir = self.CreateDirectory(outputDir, (phiDir, thetaDir))

                    # Compensate to match PV.
                    pOffset = -math.pi/2.

                    # Change the view.
                    view.viewNormal = spherical_to_cartesian(theta[itheta], phi[iphi] + pOffset)
                    view.viewUp = spherical_to_cartesian(theta[itheta]+math.pi/4., phi[iphi] + pOffset)
                    SetView3D(view)

                    # Make filename and save the image.
                    filename = "%s/time_%s.%s" % (imgDir, currentTime, self.format)
                    self.SaveImage(filename,  self.width, self.height, self.format)

        self.Debug(1, "CreateThetaPhiDatabase")

        # Create the directory where we'll make the files.
        cdbDir,imageDir = self.CreateContainerDirectory()
        if cdbDir == "":
            return False

        theta,phi = self.compute_theta_phi()

        # We save a bunch of images per time step.
        # Save a back up view.
        backup_view = GetView3D()

        # Save the frames.
        timeAtts = self.IterateAndSaveFrames(imageDir, phi_theta_save)

        # Restore view.
        SetView3D(backup_view)

        if self.specification == "D":
            # Make the CSV file
            filename = os.path.join(cdbDir, "data.csv")
            f = open(filename, "wt")
            f.write("time,phi,theta,FILE\n")
            for i in xrange(len(timeAtts)):
                for iphi in xrange(self.phi):
                    for itheta in xrange(self.theta):
                        fn = "image/phi_%d/theta_%d/time_%s.%s" % (ideg(phi[iphi]), ideg(theta[itheta]), timeAtts[i], self.format)
                        f.write("%s,%g,%g,%s\n" % (timeAtts[i], ideg(phi[iphi]), ideg(theta[itheta]), fn))
            f.close()
        else:
            # Make the JSON file
            filename = os.path.join(imageDir, "info.json")
            f = open(filename, "wt")
            f.write('{\n')
            f.write('  "type" : "simple",\n')
            f.write('  "version": "1.1",\n')
            f.write('  "metadata":{"type":"parametric-image-stack"},\n')
            f.write('  "name_pattern":"phi_{phi}/theta_{theta}/time_{time}.%s",\n' % self.format)
            f.write('  "arguments":{\n')
            f.write('    "theta": {\n')
            f.write('       "default":%d,\n' % ideg(theta[self.theta/2]))
            f.write('       "label":"Theta",\n')
            f.write('       "type":"range",\n')
            f.write('       "values":[')
            for i in xrange(len(theta)):
                f.write("%d" % ideg(theta[i]))
                if i < len(theta)-1:
                    f.write(", ")
            f.write(']\n')
            f.write('    },\n')

            f.write('    "phi": {\n')
            f.write('       "default":%d,\n' % ideg(phi[0]))  # Giving self.phi/2 for the index gives PV default view.
            f.write('       "label":"Phi",\n')
            f.write('       "type":"range",\n')
            f.write('       "values":[')
            for i in xrange(len(phi)):
                f.write("%d" % ideg(phi[i]))
                if i < len(phi)-1:
                    f.write(", ")
            f.write(']\n')
            f.write('    },\n')

            f.write('    "time": {\n')
            f.write('       "default":\"%s\",\n' % timeAtts[0])
            f.write('       "label":"Time",\n')
            f.write('       "type":"range",\n')
            f.write('       "values":%s\n' % self.JSONList(timeAtts))
            f.write('    }\n')
            f.write('  }\n')
            f.write('}\n')
            f.close()

    ###########################################################################
    # Method: CreatePhiThetaDatabaseComposite
    #
    # Purpose:    This method creates the images for a phi-theta, composite
    #             Cinema database.
    #
    # Programmer: Brad Whitlock
    # Date:       Fri Sep 29 00:35:32 PDT 2017
    #
    ###########################################################################

    def CreatePhiThetaDatabaseComposite(self):
        def spherical_to_cartesian(theta, phi):
            return ( math.cos(theta)*math.cos(phi), math.sin(theta), math.cos(theta)*math.sin(phi) )

        def ideg(rad):
            return int((360. * rad) / (2. * math.pi))

        # We get here from IteratePlotsAndSaveFrames
        def theta_phi_save_plot(outputDir, timestep, currentTime, vis):
            view = initialView
            timeDir="time=%d"%timestep
            visDir="vis=%d"%vis

            # Iterate over variables.
            ioffset = 0
            for i in xrange(len(self.vars)):
                # Change to the right var if needed.
                ChangeActivePlotsVar(self.vars[i])

                # Get the minmax. We get plotVarRanges from CreatePhiThetaDatabaseComposite
                Query("MinMax")
                plotRange = GetQueryOutputValue()
                if plotRange[0] < plotVarRanges[self.vars[i]][0]:
                    plotVarRanges[self.vars[i]][0] = plotRange[0]
                if plotRange[1] > plotVarRanges[self.vars[i]][1]:
                    plotVarRanges[self.vars[i]][1] = plotRange[1]

                # Iterate over theta,phi
                for itheta in xrange(len(theta)):
                    thetaDir="theta=%d"%itheta
                    for iphi in xrange(len(phi)):
                        phiDir="phi=%d"%iphi
                        # Create the directory we need.
                        imgDir = self.CreateDirectory(outputDir, (phiDir, thetaDir, timeDir, visDir))

                        # Compensate to match PV.
                        pOffset = -math.pi/2.

                        # Change the view.
                        view.viewNormal = spherical_to_cartesian(theta[itheta], phi[iphi] + pOffset)
                        view.viewUp = spherical_to_cartesian(theta[itheta]+math.pi/4., phi[iphi] + pOffset)
                        SetView3D(view)

                        # Save.
                        if i == 0:
                            # Make filename and save the image.
                            filename = "%s/image.%s" % (imgDir, self.format)
                            self.SaveImage(filename,  self.width, self.height, self.format)
                            # We need to rename some files.
                            if self.format == "exr":
                                # exr stores all data into the same file.
                                self.RenameFile(os.path.join(imgDir, "image."+self.format), os.path.join(imgDir,"image=0."+self.format))
                                ioffset = 0
                            else:
                                # We will have gotten image.png, image.depth.Z, image.value.Z
                                self.RenameFile(os.path.join(imgDir, "image."+self.format), os.path.join(imgDir,"image=0."+self.format))
                                self.RenameFile(os.path.join(imgDir, "image.depth.Z"), os.path.join(imgDir,"image=1.Z"))
                                self.RenameFile(os.path.join(imgDir, "image.value.Z"), os.path.join(imgDir,"image=2.Z"))
                                ioffset = 2
                        else:
                            filename = "%s/image.%s" % (imgDir, self.format)
                            self.SaveValueImage(filename, self.width, self.height)
                            self.RenameFile(os.path.join(imgDir, "image.value.Z"), os.path.join(imgDir,"image=%d.Z" % (ioffset+i)))


        # We get here from IterateAndSaveFrames
        def theta_phi_save(outputDir, timestep, currentTime):
            DisableRedraw() # Cut down on redraws.
            self.IteratePlotsAndSaveFrames(outputDir, timestep, currentTime, theta_phi_save_plot)
            SetView3D(backup_view)
            RedrawWindow()

        self.Debug(1, "CreateThetaPhiDatabase")

        # Create the directory where we'll make the files.
        cdbDir,imageDir = self.CreateContainerDirectory()
        if cdbDir == "":
            return False

        theta,phi = self.compute_theta_phi()

        # We save a bunch of images per time step.
        # Save a back up view.
        backup_view = GetView3D()

        # Get the plot var name and minmax
        pL = GetPlotList()
        usablePlots = self.GetUsablePlots()
        np = len(usablePlots)
        if len(usablePlots) > 0:
            plotVar = pL.GetPlots(usablePlots[0]).plotVar
        else:
            plotVar = pL.GetPlots(0).plotVar
            np = 1

        # Make sure that the current plot var is in the list of vars that we 
        # want to plot.
        if plotVar not in self.vars:
            self.vars = sorted([plotVar] + self.vars)
        else:
            self.vars = sorted(self.vars)
        plotVarRanges = {}
        for v in self.vars:
            plotVarRanges[v] = [0.,0.]

        # Get the current view. Used in theta_phi_save_plot.
        initialView = GetView3D()

        # Save the frames.
        timeAtts = self.IterateAndSaveFrames(imageDir, theta_phi_save)

        # Restore view.
        SetView3D(backup_view)

        # Make the JSON file
        timeP = {"default":'"'+timeAtts[0]+'"', "label":"Time","type":"range","values":self.JSONList(timeAtts)}
        phiDegrees = [ideg(x) for x in phi]
        phiP  = {"default":ideg(phi[0]), "label":"Phi","type":"range","values":self.JSONList(phiDegrees)}
        thetaDegrees = [ideg(x) for x in theta]
        thetaP  = {"default":ideg(theta[self.theta/2]), "label":"Theta","type":"range","values":self.JSONList(thetaDegrees)}
        params = {"time": timeP, "phi":phiP, "theta":thetaP}
        self.WriteCompositeJSONFile(imageDir, params, np, plotVarRanges)



