##############################################################################
#
# Copyright (c) 2000 - 2006, The Regents of the University of California
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
from xml.sax import make_parser 
from xml.sax.handler import ContentHandler 
import string

###############################################################################
# Class: MovieTemplateReader
#
# Purpose:    This class knows how to read a movie template specification file
#             and convert it into Python dictionaries that the movie templates
#             can use.
#
# Programmer: Brad Whitlock
# Date:       Tue Nov 14 13:56:07 PST 2006
#
# Modifications:
#
###############################################################################

class MovieTemplateReader(ContentHandler): 
    ###########################################################################
    # Method: __init__
    #
    # Purpose:
    #   Constructor.
    #
    # Arguments:
    #   self : This object reference.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def __init__ (self): 
        self.viewport_data = {}
	self.sequence_data = {}
	self.generic_data = {}
        self.viewport_name = ""
        self.sequence_name = ""

	self.READ_GENERIC = 0
	self.READ_VIEWPORT = 1
	self.READ_SEQUENCE = 2
	self.readMode = self.READ_GENERIC

        self.line = ""
        self.fieldName = ""
        self.fieldType = ""
        self.fieldLength = 0
        self.objectIndent = 0

    ###########################################################################
    # Method: startElement
    #
    # Purpose:
    #   This method gets called when the XML parser begins processing a tag.
    #
    # Arguments:
    #   self  : This object reference.
    #   name  : The XML tag name
    #   attrs : Any attributes that are defined for the tag.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def startElement(self, name, attrs): 
        self.line = ""
        self.fieldName = ""
        self.fieldType = ""
        self.fieldLength = 0

        #print "<%s name=\"%s\">" % (name, attrs.getValue("name"))
	if name == "Object":
            if attrs.getValue("name") == "VIEWPORTS":
                self.readMode = self.READ_VIEWPORT
                self.objectIndent = 1
	    elif attrs.getValue("name") == "SEQUENCEDATA":
                self.readMode = self.READ_SEQUENCE
                self.objectIndent = 1
            else:
                self.objectIndent = self.objectIndent + 1
	    
        if name == "Object":
            objName = attrs.getValue("name")
            if self.readMode == self.READ_VIEWPORT:
                self.viewport_name = objName
            elif self.readMode == self.READ_SEQUENCE:
                self.sequence_name = objName
        elif name == "Field":
            self.fieldName = attrs.getValue("name")
            self.fieldType = attrs.getValue("type")
            try:
                self.fieldLength = eval(attrs.getValue("length"))
	    except KeyError:
                self.fieldLength = 0

    ###########################################################################
    # Method: characters
    #
    # Purpose:
    #   This method gets called when the XML parser processes a character. We
    #   use it to build up the line that will be processed later.
    #
    # Arguments:
    #   self : This object reference.
    #   ch   : The character being handled
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def characters (self, ch): 
        self.line = self.line + ch

    ###########################################################################
    # Method: SpacedListToStringTuple
    #
    # Purpose:
    #   Converts a list of space-separated words into a tuple of strings.
    #
    # Arguments:
    #   self : This object reference.
    #   line : The input line.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def SpacedListToStringTuple(self, line):
        words = []
        in_string = 0
        in_token = 0
        tok = ""
        for c in line:
            if in_string:
                tok = tok + c
                if c == '"':
                    in_string = 0
                    words = words + [tok[1:-1]]
                    tok = ""
            else:
                if c == '"':
                    tok = tok + c
                    in_string = 1
                elif c != ' ':
                    tok = tok + c
                    in_token = 1
                elif in_token:
                    in_token = 0
                    words = words + [tok]
                    tok = ""
        if in_token:
            words = words + [tok]
        return tuple(words)

    ###########################################################################
    # Method: EvalField
    #
    # Purpose:
    #   Turns the XML file's field data into Python objects.
    #
    # Arguments:
    #   self : This object reference.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def EvalField(self):
        ret = 0
        s_line = str(self.line)
        if string.find(self.fieldType, "Array") != -1:
            stuple = self.SpacedListToStringTuple(s_line)
            if self.fieldType in ("intArray", "ucharArray", "longArray"):
                values = []
                for s in stuple:
                    values = values + [int(s)]
                while len(values) < self.fieldLength:
                    values = values + [0]
                ret = tuple(values)
            elif self.fieldType in ("floatArray", "doubleArray"):
                values = []
                for s in stuple:
                    values = values + [float(s)]
                while len(values) < self.fieldLength:
                    values = values + [0.]
                ret = tuple(values)
        elif string.find(self.fieldType, "Vector") != -1:
            stuple = self.SpacedListToStringTuple(s_line)
            if self.fieldType in ("intVector", "ucharVector", "longVector"):
                values = []
                for s in stuple:
                    values = values + [int(s)]
                ret = tuple(values)
            elif self.fieldType in ("floatVector", "doubleVector"):
                values = []
                for s in stuple:
                    values = values + [float(s)]
                ret = tuple(values)
            elif self.fieldType == "stringVector":
                ret = stuple
        elif self.fieldType in ("int", "uchar", "char", "long"):
            stuple = self.SpacedListToStringTuple(s_line)
            if len(stuple) > 0:
                ret = int(stuple[0])
            else:
                ret = 0
        elif self.fieldType in ("float", "double"):
            stuple = self.SpacedListToStringTuple(s_line)
            if len(stuple) > 0:
                ret = float(stuple[0])
            else:
                ret = 0
        elif self.fieldType == "bool":
            stuple = self.SpacedListToStringTuple(s_line)
            if len(stuple) > 0:
                if stuple[0] == "true":
                    ret = 1
                else:
                    ret = 0
            else:
                ret = 0
        elif self.fieldType == "string":
            ret = s_line

        return ret

    ###########################################################################
    # Method: endElement
    #
    # Purpose:
    #   This method gets called when the XML parser needs to end a tag.
    #
    # Arguments:
    #   self : This object reference.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def endElement(self, name): 
        #print self.line
        #print "</%s name=%s, type=%s, length=%d>" % (name, self.fieldName, self.fieldType, self.fieldLength)
        if name == "Field":
            if self.readMode == self.READ_VIEWPORT:
                value = self.EvalField()
                if self.viewport_name not in self.viewport_data.keys():
                    self.viewport_data[self.viewport_name] = {}
                self.viewport_data[self.viewport_name][self.fieldName] = value
            elif self.readMode == self.READ_SEQUENCE:
                value = self.EvalField()
                if self.sequence_name not in self.sequence_data.keys():
                    self.sequence_data[self.sequence_name] = {}
                self.sequence_data[self.sequence_name][self.fieldName] = value
            elif self.readMode == self.READ_GENERIC:
                self.generic_data[self.fieldName] = self.EvalField()
        else:
            self.objectIndent = self.objectIndent - 1
            if self.objectIndent < 1:
                self.readMode = self.READ_GENERIC
        self.line = ""


###############################################################################
# Function: RotateSequence_changeview
#
# Purpose:    This is a callback function for the movie template class's
#             RotateSequence method.
#
# Programmer: Brad Whitlock
# Date:       Thu Sep 28 10:23:57 PDT 2006
#
# Modifications:
#
###############################################################################

def RotateSequence_changeview(i, cb_data):
    template = cb_data[0]
    axis = cb_data[1]
    nSteps = cb_data[2]
    startAngle = cb_data[3]
    endAngle = cb_data[4]
    screenRotate = cb_data[5]
    initialView = cb_data[6]

    # Calculate the angle.
    t = float(i) / float(nSteps-1)
    angle = (1. - t) * startAngle + t * endAngle
    # Set the view view.
    v = template.RotateAxis(axis, angle, screenRotate, initialView)
    SetView3D(v)
    return 1


###############################################################################
# Class: VisItMovieTemplate
#
# Purpose:    This is the base class for movie templates that can be used
#             with "visit -movie".
#
# Programmer: Brad Whitlock
# Date:       Thu Sep 28 10:23:57 PDT 2006
#
# Modifications:
#
###############################################################################

class VisItMovieTemplate:
    # error class that we use to throw exceptions.
    class error:
        def __init__(self, msg):
            self.message = msg

    ###########################################################################
    # Method: __init__
    #
    # Purpose:
    #   Constructor.
    #
    # Arguments:
    #   self : This object reference.
    #   mm   : MovieMaker reference.
    #   tr   : Template reader reference.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def __init__(self, mm, tr):
        self.moviemaker = mm
        self.stereoFilenames = 0

        # Grab a reference to the XML reader's viewer and sequence data.
        self.viewport_data = tr.viewport_data
        self.sequence_data = tr.sequence_data
        self.generic_data = tr.generic_data
        self.Debug(5, " self.viewport_data = %s" % str(self.viewport_data))
        self.Debug(5, " self.sequence_data = %s" % str(self.sequence_data))

        # Supported sequence types.
        self.rotationTypes = ("RotationX", "RotationY", "RotationZ",\
            "ScreenRotationX", "ScreenRotationY", "ScreenRotationZ")
        self.imageProducingTypes = ("CurrentPlots", "Scripting") + self.rotationTypes
        self.transitionTypes = ("Fade", "LRWipe", "RLWipe", "TBWipe", "BTWipe", \
            "LRSlide", "RLSlide", "TBSlide", "BTSlide", "CircularReveal", "RippleFade", "BlockFade")
        self.compositingSequences = ("CompositeLRSlide", "CompositeRLSlide", \
            "CompositeTBSlide", "CompositeBTSlide", "CompositeFadeIn", "CompositeFadeOut")
        self.recognizedSequenceTypes = self.imageProducingTypes + self.transitionTypes \
            + self.compositingSequences

        self.windowSequences = {}
        for i in range(1,17):
            windowName = "Window %d" % i
            self.windowSequences[windowName] = i

    ###########################################################################
    # Method: __del__
    #
    # Purpose:
    #   Destructor.
    #
    # Arguments:
    #   self : This object reference.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def __del__(self):
        pass

    ###########################################################################
    # Logging and communication methods.
    ###########################################################################

    def Debug(self, level, str):
        self.moviemaker.Debug(level, str)

    def Log(self, s):
        self.moviemaker.Log(s)

    def LogAndPrint(self, s):
        self.moviemaker.Log(str(s))
        print str(s)

    def SendClientProgress(self, str, current, total):
        self.moviemaker.SendClientProgress(str, current, total)

    def ClientMessageBox(self, msg):
        self.moviemaker.ClientMessageBox(msg)

    ###########################################################################
    # Method: ValidateTemplate
    #
    # Purpose:    Validates that the template specification options that were
    #             read by the movie template reader are correct.
    #
    # Arguments:
    #   self : This object reference.
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 28 10:23:57 PDT 2006
    #
    # Modifications:
    #
    ###########################################################################

    def ValidateTemplate(self):
        # Do some basic correctness checks on the viewport_data so
        # we know that it has the required fields.
        vkeys = self.viewport_data.keys()
        vkeys.sort()
        for viewport in vkeys:
             requiredKeys = ("coordinates", "compositing", "opacity", "replaceColor", "dropShadow", "sequenceList")
             for k in requiredKeys:
                 if not self.viewport_data[viewport].has_key(k):
                     self.ClientMessageBox("Viewport %s in the options file does not contain a \"%s\" field.\n" % (viewport, k))
                     return 0

        # Make sure that the viewports do not reference sequences that do not exist.
        for viewport in vkeys:
            sequenceList = self.viewport_data[viewport]["sequenceList"]
            if type(sequenceList) == type(()):
                for sequenceName in sequenceList:
                    if sequenceName not in self.sequence_data.keys():
                        self.ClientMessageBox("Viewport %s references a sequence that does not exist." % viewport)
                        return 0
            else:
                sequenceName = sequenceList
                if not sequenceName in self.sequence_data.keys():
                    self.ClientMessageBox("Viewport %s references a sequence that does not exist." % viewport)
                    return 0

        # Make sure that all sequences are recognized sequence types.
        for seqName in self.sequence_data.keys():
            if not self.sequence_data[seqName].has_key("sequenceType"):
                self.ClientMessageBox("Sequence %s does not have a sequenceType field." % seqName)
                return 0
            else:
                st = self.sequence_data[seqName]["sequenceType"]
                if st not in self.recognizedSequenceTypes:
                    self.ClientMessageBox("Sequence %s contains unrecognized sequence type %s." % (seqName, st))
                    return 0
                elif st == "CurrentPlots" and seqName not in self.windowSequences.keys():
                    self.ClientMessageBox("CurrentPlots sequences must have names in " + str(self.windowSequences.keys()))
                    return 0
                elif st in self.transitionTypes:
                    # Transitions must have certain keys.
                    transitionKeys = ("sequenceType", "input1", "color1", "input2", "color2", "nFrames")
                    for k in transitionKeys:
                        if not self.sequence_data[seqName].has_key(k):
                            self.ClientMessageBox("Sequence %s is a transition sequence and it is missing required field \"%s\"." % (seqName, k))
                            return 0
                    # Make sure that input1 and input2's values are "Frames" or "Color".
                    fcValues = ("Frames", "Color")
                    if self.sequence_data[seqName]["input1"] not in fcValues:
                        self.ClientMessageBox("Sequence %s's %s value must be in %s." % (seqName, "input1", str(fcValues)))
                        return 0
                    if self.sequence_data[seqName]["input2"] not in fcValues:
                        self.ClientMessageBox("Sequence %s's %s value must be in %s." % (seqName, "input2", str(fcValues)))
                        return 0
                    # Make sure that nFrames >= 1
                    if self.sequence_data[seqName]["nFrames"] < 1:
                        self.ClientMessageBox("Sequence %s's nFrames value is less than 1." % seqName)
                        return 0
                elif st in self.compositingSequences:
                    # Compositing sequences must have certain keys.
                    transitionKeys = ("sequenceType", "nFrames", "reverse")
                    for k in transitionKeys:
                        if not self.sequence_data[seqName].has_key(k):
                            self.ClientMessageBox("Sequence %s is a compositing sequence and it is missing required field \"%s\"." % (seqName, k))
                            return 0
                    # Make sure that nFrames >= 1
                    if self.sequence_data[seqName]["nFrames"] < 1:
                        self.ClientMessageBox("Sequence %s's nFrames value is less than 1." % seqName)
                        return 0
                elif st in self.rotationTypes:
                    # Rotation sequences must have certain keys.
                    transitionKeys = ("sequenceType", "nSteps", "startAngle", "endAngle")
                    for k in transitionKeys:
                        if not self.sequence_data[seqName].has_key(k):
                            self.ClientMessageBox("Sequence %s is a rotation sequence and it is missing required field \"%s\"." % (seqName, k))
                            return 0
                    # Make sure that nFrames >= 1
                    if self.sequence_data[seqName]["nSteps"] < 1:
                        self.ClientMessageBox("Sequence %s's nSteps value is less than 1." % seqName)
                        return 0

        return 1

    ###########################################################################
    # Method: DetermineImageSizesAndStereo
    #
    # Purpose:    Determines the sizes of the images that will need to be
    #             saved for each sequence, given the movie maker's list of
    #             final movie resolutions. The difference here is that the
    #             sizes of the sequences may vary, depending on the sizes of
    #             the viewports to which they are mapped.
    #
    # Arguments:
    #   movieFormats : The movie maker's list of final file formats.
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 28 10:23:57 PDT 2006
    #
    # Modifications:
    #
    ###########################################################################

    def DetermineImageSizesAndStereo(self, movieFormats):
        # movieFormats is a list of [formatString, fmt, w, h, s]. We need to
        # determine the list of actual sizes needed.
        sizes = []
        for format in movieFormats:
            size = (format[2], format[3], format[4])
            if size not in sizes:
                sizes = sizes + [size]

        # Now that we have the list of sizes, produce the list of sizes that
        # we need for each sequence. We do this by looking through the
        # viewports and we determine to which viewport each sequence is
        # mapped.
        sequence_sizes = {}
        vkeys = self.viewport_data.keys()
        vkeys.sort()
        for viewport in vkeys:
             coordinates = self.viewport_data[viewport]["coordinates"]
             sequenceList = self.viewport_data[viewport]["sequenceList"]
             if type(sequenceList) == type(()):
                 # List of sequence names.
                 for sequenceName in sequenceList:
                     sequence_sizes[sequenceName] = []
                     for size in sizes:
                         # coordinates are llx, lly, urx, ury
                         x0 = coordinates[0] * size[0]
                         x1 = coordinates[2] * size[0]
                         y0 = coordinates[1] * size[1]
                         y1 = coordinates[3] * size[1]
                         width = x1 - x0
                         height = y1 - y0
                         if width < 0:
                             width = -width
                         if height < 0:
                             height = -height
                         sequence_sizes[sequenceName] =  sequence_sizes[sequenceName] + [(width, height, size[2])]                         
             else:
                 # One sequence
                 sequenceName = sequenceList
                 sequence_sizes[sequenceName] = []
                 for size in sizes:
                     # coordinates are llx, lly, urx, ury
                     x0 = coordinates[0] * size[0]
                     x1 = coordinates[2] * size[0]
                     y0 = coordinates[1] * size[1]
                     y1 = coordinates[3] * size[1]
                     width = x1 - x0
                     height = y1 - y0
                     if width < 0:
                         width = -width
                     if height < 0:
                         height = -height
                     sequence_sizes[sequenceName] =  sequence_sizes[sequenceName] + [(width, height, size[2])]                         

        return sequence_sizes

    ###########################################################################
    # Method: CompositingRequired
    #
    # Purpose:    Determines whether image compositing will be necessary.
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 28 10:23:57 PDT 2006
    #
    # Modifications:
    #
    ###########################################################################

    def CompositingRequired(self):
        return len(self.viewport_data.keys()) > 1

    ###########################################################################
    # Method: CalculatePercentagesComplete
    #
    # Purpose:    Determines how much of the frame allocation stage is allocated
    #             to each image sequence and compositing.
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 28 10:23:57 PDT 2006
    #
    # Modifications:
    #
    ###########################################################################

    def CalculatePercentagesComplete(self):
        vkeys = self.viewport_data.keys()
        vkeys.sort()
        sequenceNames = []

        # Look through the viewports for mapped sequences that produce raw images.
        for k in vkeys:
            seqList = self.viewport_data[k]["sequenceList"]
            for seqName in seqList:
                if self.sequence_data[seqName]["sequenceType"] in self.imageProducingTypes:
                    sequenceNames = sequenceNames + [seqName]

        # Look through the viewports for sequences that are transitions. If there is
        # one then add a TRANSITIONS stage.
        for k in vkeys:
            seqList = self.viewport_data[k]["sequenceList"]
            for seqName in seqList:
                if self.sequence_data[seqName]["sequenceType"] in self.transitionTypes:
                    sequenceNames = sequenceNames + ["TRANSITIONS"]
                    break

        # If there are multiple viewports, allow for a compositing stage at the end.
        if self.CompositingRequired():
            sequenceNames = sequenceNames + ["COMPOSITING"]

        # Determine the begin/end percentages that will be reported for each stage.
        incr = self.moviemaker.percentAllocationFrameGen / float(len(sequenceNames))
        percentages = {}
        pct = 0.
        for name in sequenceNames:
            nextpct = pct + incr
            if nextpct >= (1. - 1e-03): 
                nextpct = 1.
            percentages[name] = (pct, nextpct)
            pct = nextpct
        self.Debug(5, "Percentages per sequence = %s" % str(percentages))

        return percentages

    ###########################################################################
    # Method: IterateAndSaveFrames
    #
    # Purpose:    This method iterates over a user-defined range, calls a 
    #             callback function at each iteration and saves an image.
    #
    # Arguments:
    #   filebase      : The format of the name of the image to save.
    #   sizes         : The sizes of the images that we need to save.
    #   stereos       : The stereo types of the images that we need to save.
    #   percents      : The (start,end) percents of this stage - for client
    #                   reporting purposes.
    #   progressMsg   : The message displayed for progress in the client.
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 28 10:23:57 PDT 2006
    #
    # Modifications:
    #
    ###########################################################################

    def IterateAndSaveFrames(self, filebase, formats, percents, progressMsg):
        self.Debug(1, "IterateAndSaveFrames: begin")

        # Make sure that camera view mode is on so that if we have a
        # saved keyframe animation, we really get the keyframed view.
        if(GetWindowInformation().cameraViewMode == 0):
            ToggleCameraViewMode()

        # Save the old rendering attributes
        old_ra = GetRenderingAttributes()

        # Save an image for each frame in the animation.
        numFrames = 0
        drawThePlots = 0
        lastProgress = -1
        for i in range(TimeSliderGetNStates()):
            if TimeSliderGetNStates() > 1:
                t = float(i) / float(TimeSliderGetNStates()-1)
            else:
                t = 1.

            progress = int(((1.-t)*percents[0] + t*percents[1]) * 100.)
            if progress != lastProgress:
                self.SendClientProgress(progressMsg, progress, 100)
                lastProgress = progress

            if(SetTimeSliderState(i) == 0):
                drawThePlots = 1
                self.LogAndPrint("There was an error when trying to set the "\
"time slider state to %d. VisIt will now try to redraw the plots." % i)
            if(drawThePlots):
                if(DrawPlots() == 0):
                    self.LogAndPrint("VisIt could not draw plots for time slider "\
"state %d. You should investigate the files used for that state." % i)
                else:
                    drawThePlots = 0
            if self.SaveImage(filebase, formats, numFrames) == 0:
                self.LogAndPrint("There was an error when trying to save the window "\
"for time slider state %d. VisIt will now try to redraw the plots and re-save "\
"the window." % i)
                if(DrawPlots() == 0):
                    self.LogAndPrint("VisIt could not draw plots for time slider "\
"state %d. You should investigate the files used for that state." % i)
                else:
                    if self.SaveImage(filebase, formats, numFrames) == 0:
                        self.LogAndPrint("VisIt could not re-save the window for "\
"time slider state %d. You should investigate the files used for that state." % i)

            numFrames = numFrames + 1

        # Restore the old rendering attributes.
        SetRenderingAttributes(old_ra)
        self.Debug(1, "IterateAndSaveFrames: end")

        return numFrames

    ###########################################################################
    # Method: IterateCallbackAndSaveFrames
    #
    # Purpose:    This method iterates over a user-defined range, calls a 
    #             callback function at each iteration and saves an image.
    #
    # Arguments:
    #   callback_info : A tuple of (nsteps, callback, callbackdata)
    #   filebase      : The format of the name of the image to save.
    #   sizes         : The sizes of the images that we need to save.
    #   stereos       : The stereo types of the images that we need to save.
    #   percents      : The (start,end) percents of this stage - for client
    #                   reporting purposes.
    #   progressMsg   : The message displayed for progress in the client.
    #
    # Note:       The callback can do whatever you want, change time states,
    #             move clipping planes, etc.
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 28 10:23:57 PDT 2006
    #
    # Modifications:
    #
    ###########################################################################

    def IterateCallbackAndSaveFrames(self, callback_info, filebase, formats,\
                                     percents, progressMsg):
        self.Debug(1, "IterateCallbackAndSaveFrames: begin")

        # Make sure that camera view mode is on so that if we have a
        # saved keyframe animation, we really get the keyframed view.
        if(GetWindowInformation().cameraViewMode == 0):
            ToggleCameraViewMode()

        # Save the old rendering attributes
        old_ra = GetRenderingAttributes()

        # Save an image for each frame in the animation.
        numFrames = 0
        drawThePlots = 0
        lastProgress = -1
        for i in range(callback_info[0]):
            t = float(i) / float(callback_info[0]-1);

            progress = int(((1.-t)*percents[0] + t*percents[1]) * 100.)
            if progress != lastProgress:
                self.SendClientProgress(progressMsg, progress, 100)
                lastProgress = progress

            if(callback_info[1](i, callback_info[2]) == 0):
                drawThePlots = 1
                self.LogAndPrint("There was an error when trying to set the "\
"time slider state to %d. VisIt will now try to redraw the plots." % i)
            if(drawThePlots):
                if(DrawPlots() == 0):
                    self.LogAndPrint("VisIt could not draw plots for time slider "\
"state %d. You should investigate the files used for that state." % i)
                else:
                    drawThePlots = 0
            if self.SaveImage(filebase, formats, numFrames) == 0:
                self.LogAndPrint("There was an error when trying to save the window "\
"for time slider state %d. VisIt will now try to redraw the plots and re-save "\
"the window." % i)
                if(DrawPlots() == 0):
                    self.LogAndPrint("VisIt could not draw plots for time slider "\
"state %d. You should investigate the files used for that state." % i)
                else:
                    if self.SaveImage(filebase, formats, numFrames) == 0:
                        self.LogAndPrint("VisIt could not re-save the window for "\
"time slider state %d. You should investigate the files used for that state." % i)

            numFrames = numFrames + 1

        # Restore the old rendering attributes.
        SetRenderingAttributes(old_ra)
        self.Debug(1, "IterateCallbackAndSaveFrames: end")

        return numFrames

    def CreateFilebase(self, filebase, format):
        name = ""
        prefix = self.moviemaker.tmpDir + self.moviemaker.slash
        stereo = format[2]
        if self.stereoFilenames and stereo > 0:
            sName = self.moviemaker.stereoNameToType.keys()[stereo]
            name = "%s%s_%dx%d_%s" % (prefix, filebase, format[0], format[1], sName)
        else:
            name = "%s%s_%dx%d" % (prefix, filebase, format[0], format[1])
        return name

    def CreateFilename(self, filebase, format, index):
        name = ""
        prefix = self.moviemaker.tmpDir + self.moviemaker.slash
        stereo = format[2]
        if self.stereoFilenames and stereo > 0:
            sName = self.moviemaker.stereoNameToType.keys()[stereo]
            name = "%s%s_%dx%d_%s_%04d.ppm" % (prefix, filebase, format[0], format[1], sName, index)
        else:
            name = "%s%s_%dx%d_%04d.ppm" % (prefix, filebase, format[0], format[1], index)
        return name

    def CreateFilenames(self, filebase, formats, index):
        filenames = []
        for i in range(len(formats)):
            filenames = filenames + [self.CreateFilename(filebase, formats[i], index)]
        return filenames

    ###########################################################################
    # Method: SaveImage
    #
    # Purpose:    This method tells VisIt to save an image.
    #
    # Arguments:
    #   filebase : The root filename that will be used for all size images.
    #   sizes    : The sizes that we need to save.
    #   stereos  : The stereo types of the image that we need to save.
    #   index    : The index of the current file that needs to be saved.
    #
    # Note:       This method saves out PPM images for all of the required
    #             sizes so they can be used in compositing. If compositing
    #             is not required and there is only 1 sequence then the 
    #             MovieMaker class is used to save the images since it has 
    #             a good idea of which images need to be saved.
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 21 11:37:01 PDT 2006
    #
    # Modifications:
    #
    ###########################################################################

    def SaveImage(self, filebase, formats, index):
        retval = 1

        # If compositing is required then save in PPM
        if self.CompositingRequired() or len(self.sequence_data.keys()) > 1:
            s = SaveWindowAttributes()
            old_sw = GetSaveWindowAttributes()
            s.saveTiled = old_sw.saveTiled
            s.family = 0
            s.screenCapture = 0
            s.stereo = old_sw.stereo
            s.format = s.PPM

            currentRA = GetRenderingAttributes()

            # Save the frame in the required sizes.
            filenames = self.CreateFilenames(filebase, formats, index)
            for i in range(len(formats)):
                s.fileName = filenames[i]
                self.Debug(5, "VisItMovieTemplate.SaveImage: %s" % s.fileName)
                # Stereo format
                stereo = formats[i][2]
                ra = GetRenderingAttributes()
                if stereo == self.moviemaker.STEREO_NONE:
                    s.stereo = 0
                    ra.stereoRendering = 0
                elif stereo == self.moviemaker.STEREO_LEFTRIGHT:
                    s.stereo = 1
                    ra.stereoRendering = 0
                elif stereo == self.moviemaker.STEREO_REDBLUE:
                    s.stereo = 0
                    ra.stereoRendering = 1
                    ra.stereoType = ra.RedBlue
                elif stereo == self.moviemaker.STEREO_REDGREEN:
                    s.stereo = 0
                    ra.stereoRendering = 1
                    ra.stereoType = ra.RedGreen
                SetRenderingAttributes(ra)
                currentRA = ra

                s.width = formats[i][0]
                s.height = formats[i][1]
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
        else:
            # No compositing is required and there is just 1 sequence so let 
            # the moviemaker class save all of the required frame sizes and 
            # formats.
            retval = self.moviemaker.SaveImage(index)

        return retval

    ###########################################################################
    # Method: SequenceNameToFilebase
    #
    # Purpose:
    #   Converts a sequence name into a filebase.
    #
    # Arguments:
    #   self : This object reference.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def SequenceNameToFilebase(self, seqName):
        return "seq_" + string.replace(seqName, " ", "_")

    ###########################################################################
    # Method: FindFirstNonTransition
    #
    # Purpose:
    #   Finds the first sequence in the list that is not a transition.
    #
    # Arguments:
    #   self    : This object reference.
    #   seqList : The list of sequences.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def FindFirstNonTransition(self, seqList):
        index = 0
        for i in range(len(seqList)):
            if self.sequence_data[seqList[i]]["sequenceType"] not in self.transitionTypes:
                return i
            else:
                index = index + 1
        return -1

    ###########################################################################
    # Method: FindLastNonTransition
    #
    # Purpose:
    #   Finds the last sequence in the list that is not a transition.
    #
    # Arguments:
    #   self    : This object reference.
    #   seqList : The list of sequences.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def FindLastNonTransition(self, seqList):
        index = 0
        last = len(seqList) - 1
        for i in range(len(seqList)):
            if self.sequence_data[seqList[last - i]]["sequenceType"] not in self.transitionTypes:
                return last-i
            else:
                index = index + 1
        return -1

    ###########################################################################
    # Method: TransitionFrames
    #
    # Purpose:    This method takes completed list of sequences and creates
    #             the desired transitions between them.
    #
    # Arguments:
    #   sequence_frames : A dictionary keyed by the sequence names. The values
    #                     in the dictionary are tuples of (nFrames, 
    #                     sequence file prefix, background color)
    #   movieFormats    : The movie maker's list of file formats.
    #   percents        : The (start,end) percents to report for the compositing
    #                     stage.
    #
    # Programmer: Brad Whitlock
    # Date:       Fri Oct 13 11:19:18 PDT 2006
    #
    # Modifications:
    #
    ###########################################################################

    def TransitionFrames(self, sequence_frames, movieFormats, trans_percents):

        vkeys = self.viewport_data.keys()
        vkeys.sort()
        # Map sequence transition types to "visit -transition" args.
        transitionTypes2Arg = {"Fade" : "fade", "LRWipe":"lrwipe", \
            "RLWipe":"rlwipe", "TBWipe":"tbwipe", "BTWipe":"btwipe", \
            "LRSlide":"lrslide", "RLSlide":"rlslide", "TBSlide":"tbslide",\
            "BTSlide":"btslide", "CircularReveal":"circle", \
            "RippleFade":"ripple", "BlockFade":"block"}

        # Look through the viewports for mapped transition sequences to 
        # get a count.
        numTransitions = 0
        for k in vkeys:
            sequenceList = self.viewport_data[k]["sequenceList"]
            for seqName in sequenceList:
                if self.sequence_data[seqName]["sequenceType"] in self.transitionTypes:
                    numTransitions = numTransitions + 1
        numTransitions = numTransitions * len(movieFormats)

        # Create the sequence transition for each movie format.
        transitionIndex = 0
        lastProgress = -1
        for format in movieFormats:
            # This set of movie frames must be width x height pixels.
            width = format[2]
            height = format[3]
            stereo = format[4]

            for k in vkeys:
                # Determine the size of the viewport in pixels, given the 
                # current movie format size
                coordinates = self.viewport_data[k]["coordinates"]
                x0 = coordinates[0] * width
                x1 = coordinates[2] * width
                y0 = coordinates[1] * height
                y1 = coordinates[3] * height
                vpw = x1 - x0
                vph = y1 - y0
                if vpw < 0:
                    vpw = -vpw
                if vph < 0:
                    vph = -vph

                seqIndex = 0
                sequenceList = self.viewport_data[k]["sequenceList"]
                firstNonTransition = self.FindFirstNonTransition(sequenceList)
                lastNonTransition = self.FindLastNonTransition(sequenceList)
                self.Debug(5,"TransitionFrames: first non-transition sequence: %d" % firstNonTransition)
                self.Debug(5,"TransitionFrames: last non-transition sequence: %d" % lastNonTransition)
                for seqName in sequenceList:
                    thisSequence = self.sequence_data[seqName]

                    seqInputs = ["", "", "", ""]
                    seqBgColor = (0,0,0)

                    if thisSequence["sequenceType"] in self.transitionTypes:
                        self.Debug(4, "Sequence \"%s\" is a transition. We need "\
                                   "to determine its inputs." % seqName)

                        if thisSequence["input1"] == "Color":
                            color1 = self.sequence_data[seqName]["color1"]
                            seqInputs[0] = "-color1"
                            seqInputs[1] = "%d %d %d" % color1
                            self.Debug(4, "Input 1: %s %s" % (seqInputs[0],seqInputs[1]))
                        if thisSequence["input2"] == "Color":
                            color2 = self.sequence_data[seqName]["color2"]
                            seqInputs[2] = "-color2"
                            seqInputs[3] = "%d %d %d" % color2
                            seqBgColor = color2
                            self.Debug(4, "Input 2: %s %s" % (seqInputs[2],seqInputs[3]))

                        if thisSequence["input1"] == "Frames":
                            self.Debug(5, "Determining input1")
                            if seqIndex < firstNonTransition:
                                self.Debug(5, "Sequence index %d < first non transition (%d)"\
                                           % (seqIndex, firstNonTransition))
                                seqBase = sequence_frames[sequenceList[firstNonTransition]][1]
                                seqInputs[0] = "-input1"
                                seqInputs[1] = (seqBase, (vpw, vph, stereo), 0)
                            elif seqIndex > lastNonTransition:
                                self.Debug(5, "Sequence index %d > last non transition (%d)"\
                                           % (seqIndex, lastNonTransition))
                                seqBase = sequence_frames[sequenceList[lastNonTransition]][1]
                                lastIndex = sequence_frames[sequenceList[lastNonTransition]][0] - 1
                                seqInputs[0] = "-input1"
                                seqInputs[1] = (seqBase, (vpw, vph, stereo), lastIndex)
                            else:
                                # Find the previous image-producing sequence
                                prevFrameSeq = seqIndex
                                while self.sequence_data[sequenceList[prevFrameSeq]]["sequenceType"] in self.transitionTypes:
                                     prevFrameSeq = prevFrameSeq - 1
                                self.Debug(5, "Previous non-transition: %d" % prevFrameSeq)
                                seqBase = sequence_frames[sequenceList[prevFrameSeq]][1]
                                lastIndex = sequence_frames[sequenceList[prevFrameSeq]][0] - 1
                                seqInputs[0] = "-input1"
                                seqInputs[1] = (seqBase, (vpw, vph, stereo), lastIndex)
                                
                        if thisSequence["input2"] == "Frames":
                            self.Debug(5, "Determining input2")
                            if seqIndex < firstNonTransition:
                                self.Debug(5, "Sequence index %d < first non transition (%d)"\
                                           % (seqIndex, firstNonTransition))
                                seqBase = sequence_frames[sequenceList[firstNonTransition]][1]
                                seqInputs[2] = "-input2"
                                seqInputs[3] = (seqBase, (vpw, vph, stereo), 0)
                                seqBgColor = sequence_frames[sequenceList[firstNonTransition]][2]
                            elif seqIndex > lastNonTransition:
                                self.Debug(5, "Sequence index %d > last non transition (%d)"\
                                           % (seqIndex, lastNonTransition))
                                seqBase = sequence_frames[sequenceList[lastNonTransition]][1]
                                lastIndex = sequence_frames[sequenceList[lastNonTransition]][0] - 1
                                seqInputs[2] = "-input2"
                                seqInputs[3] = (seqBase, (vpw, vph, stereo), lastIndex)
                                seqBgColor = sequence_frames[sequenceList[lastNonTransition]][2]
                            else:
                                # Find the next image-producing sequence
                                nextFrameSeq = seqIndex
                                while self.sequence_data[sequenceList[nextFrameSeq]]["sequenceType"] in self.transitionTypes:
                                     nextFrameSeq = nextFrameSeq + 1
                                self.Debug(5, "Next non-transition: %d" % nextFrameSeq)
                                seqBase = sequence_frames[sequenceList[nextFrameSeq]][1]
                                seqInputs[2] = "-input2"
                                seqInputs[3] = (seqBase, (vpw, vph, stereo), 0)
                                seqBgColor = sequence_frames[sequenceList[nextFrameSeq]][2]

                        # Now that we have determined the input arguments for the 
                        # transition program, we need to execute it to generate the
                        # frames.
                        nFrames = self.sequence_data[seqName]["nFrames"]
                        seqBase = self.SequenceNameToFilebase(seqName)
                        transitionInputs = [seqInputs[0], seqInputs[1], seqInputs[2], seqInputs[3]]
                        
                        if stereo == self.moviemaker.STEREO_LEFTRIGHT:
                            for eye in ("left_", "right_"):
                                # Convert the tuples into filename strings.
                                if thisSequence["input1"] == "Frames":
                                    transitionInputs[1] = self.CreateFilename(eye + seqInputs[1][0], seqInputs[1][1], seqInputs[1][2]);
                                if thisSequence["input2"] == "Frames":
                                    transitionInputs[3] = self.CreateFilename(eye + seqInputs[3][0], seqInputs[3][1], seqInputs[3][2]);

                                outputName = self.CreateFilebase(eye + seqBase, (vpw, vph, stereo)) + "_%04d.ppm"
                                self.Debug(4, "TransitionFrames: nFrames=%d, "\
                                              "seqBase=%s, outputName=%s" % \
                                              (nFrames, seqBase, outputName))

                                command = "visit -transition %s %s %s %s" % \
                                (transitionInputs[0], transitionInputs[1],\
                                 transitionInputs[2], transitionInputs[3])
                                command = command + " -style %s" % transitionTypes2Arg[thisSequence["sequenceType"]]
                                command = command + " -output %s -nframes %d" % (outputName, nFrames)
                                self.Debug(5, command)
                                os.system(command)
                        else:
                            # Convert the tuples into filename strings.
                            if thisSequence["input1"] == "Frames":
                                transitionInputs[1] = self.CreateFilename(seqInputs[1][0], seqInputs[1][1], seqInputs[1][2]);
                            if thisSequence["input2"] == "Frames":
                                transitionInputs[3] = self.CreateFilename(seqInputs[3][0], seqInputs[3][1], seqInputs[3][2]);

                            outputName = self.CreateFilebase(seqBase, (vpw, vph, stereo)) + "_%04d.ppm"
                            self.Debug(4, "TransitionFrames: nFrames=%d, "\
                                          "seqBase=%s, outputName=%s" % \
                                          (nFrames, seqBase, outputName))

                            command = "visit -transition %s %s %s %s" % \
                            (transitionInputs[0], transitionInputs[1],\
                             transitionInputs[2], transitionInputs[3])
                            command = command + " -style %s" % transitionTypes2Arg[thisSequence["sequenceType"]]
                            command = command + " -output %s -nframes %d" % (outputName, nFrames)
                            self.Debug(5, command)
                            os.system(command)

                        # If we have not added the sequence to the list of 
                        # sequence frames yet then do that now.
                        if not sequence_frames.has_key(seqName):
                            sequence_frames[seqName] = (nFrames, seqBase, seqBgColor)

                        # Report progress
                        transitionIndex = transitionIndex + 1
                        t = 1.
                        if numTransitions > 1:
                            t = float(transitionIndex) / float(numTransitions-1)
                        progress = int(((1.-t)*trans_percents[0] + t*trans_percents[1]) * 100.)
                        if progress != lastProgress:
                            self.SendClientProgress("Creating transitions", progress, 100)
                            lastProgress = progress

                    seqIndex = seqIndex + 1

        return sequence_frames

    ###########################################################################
    # Method: FindFirstNonComposite
    #
    # Purpose:
    #   Finds the first sequence in the list that is not a composite.
    #
    # Arguments:
    #   self    : This object reference.
    #   seqList : The list of sequences.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def FindFirstNonComposite(self, seqList):
        index = 0
        for i in range(len(seqList)):
            st = self.sequence_data[seqList[i]]["sequenceType"]
            if st not in self.compositingSequences:
                return i
            else:
                index = index + 1
        return -1

    ###########################################################################
    # Method: FindLastNonComposite
    #
    # Purpose:
    #   Finds the last sequence in the list that is not a composite.
    #
    # Arguments:
    #   self    : This object reference.
    #   seqList : The list of sequences.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def FindLastNonComposite(self, seqList):
        index = 0
        last = len(seqList) - 1
        for i in range(len(seqList)):
            st = self.sequence_data[seqList[last - i]]["sequenceType"]
            if st not in self.compositingSequences:
                return last-i
            else:
                index = index + 1
        return -1

    ###########################################################################
    # Method: GetFilenameForCompositeSequence
    #
    # Purpose:
    #   Creates a filename for use in compositing.
    #
    # Arguments:
    #   self : This object reference.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def GetFilenameForCompositeSequence(self, sequenceList, seqIndex, sequence_frames, vpw, vph, stereo):
        filename = ""
        firstNC = self.FindFirstNonComposite(sequenceList)
        lastNC = self.FindLastNonComposite(sequenceList)
        prefix = self.moviemaker.tmpDir + self.moviemaker.slash

        if seqIndex < firstNC:
            seqBase = sequence_frames[sequenceList[firstNC]][1]
            filename = self.CreateFilename(seqBase, (vpw, vph, stereo), 0)
        elif seqIndex > lastNC:
            seqBase = sequence_frames[sequenceList[lastNC]][1]
            lastIndex = sequence_frames[sequenceList[lastNC]][0] - 1
            filename = self.CreateFilename(seqBase, (vpw, vph, stereo), lastIndex)
        else:
            # Find the previous image-producing sequence
            prevFrameSeq = seqIndex
            while self.sequence_data[sequenceList[prevFrameSeq]]["sequenceType"] in self.compositingSequences:
                 prevFrameSeq = prevFrameSeq - 1
            # If the previous image producing sequence is the previous sequence
            # then use it. Otherwise, it means there is another
            seqBase = sequence_frames[sequenceList[prevFrameSeq]][1]
            lastIndex = sequence_frames[sequenceList[prevFrameSeq]][0] - 1
            filename = self.CreateFilename(seqBase, (vpw, vph, stereo), lastIndex)

        self.Debug(5, "GetFilenameForCompositeSequence: input for composite " \
            "sequence %d (%s) is %s" % (seqIndex, sequenceList[seqIndex], filename))

        return filename

    ###########################################################################
    # Method: InsertPrefix
    #
    # Purpose:
    #   Inserts a file name prefix into a filename containing a path.
    #
    # Arguments:
    #   self   : This object reference.
    #   s      : The filename
    #   prefix : The prefix to insert.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def InsertPrefix(self, s, prefix):
        ret = s
        if prefix != "":
            index = string.rfind(s, self.moviemaker.slash)
            if index != -1:
                ret = s[:index+1] + prefix + s[index+1:]
            else:
                ret = prefix + s
        return ret

    ###########################################################################
    # Method: CompositeFrames
    #
    # Purpose:    This method takes the input sequence frames and composites
    #             them into completed movie frames in the format desired by
    #             the movieFormats list.
    #
    # Arguments:
    #   sequence_frames : A dictionary keyed by the sequence names. The values
    #                     in the dictionary are tuples of (nFrames, 
    #                     sequence file prefix, background color)
    #   movieFormats    : The movie maker's list of file formats.
    #   percents        : The (start,end) percents to report for the compositing
    #                     stage.
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 21 11:37:01 PDT 2006
    #
    # Modifications:
    #
    ###########################################################################

    def CompositeFrames(self, sequence_frames, movieFormats, percents):
        # Determine the amount of the input percentages should be allocated to
        # each format.
        fmt_incr = (percents[1] - percents[0]) / float(len(movieFormats))
        fmt_start_percent = percents[0];

        vkeys = self.viewport_data.keys()
        vkeys.sort()

        # Composite the frames.
        numFrames = 0
        for format in movieFormats:
            # This set of movie frames must be width x height pixels.
            width = format[2]
            height = format[3]
            stereo = format[4]

            # Determine the percents for this format.
            fmt_percents = [fmt_start_percent, fmt_start_percent+fmt_incr]
            fmt_start_percent = fmt_start_percent+fmt_incr

            # If we're writing PPM images then we can write directly to outputDir
            # instead of to tmpDir. Other formats will go to tmpDir and then will
            # be converted out to outDir.
            doFormatConvert = 0
            formatExtension = ".ppm"
            destDir = self.moviemaker.tmpDir
            convertDir = self.moviemaker.outputDir
            if format[1] == self.moviemaker.FORMAT_PPM:
                destDir = self.moviemaker.outputDir
            elif format[1] == self.moviemaker.outputNeedsInput[format[1]]:
                doFormatConvert = 1
                formatExtension = self.moviemaker.formatExtension[format[1]]
            else:
                # We need to convert for an image encoder that does not take PPM.
                convertDir = self.moviemaker.tmpDir
                doFormatConvert = 1
                cf = self.moviemaker.outputNeedsInput[format[1]]
                formatExtension = self.moviemaker.formatExtension[cf]

            # Determine the name of the sequence going to each viewport.
            current_sequence_for_viewport = {}
            for k in vkeys:
                current_sequence_for_viewport[k] = self.viewport_data[k]["sequenceList"][0]

            # Determine the current image being used in each sequence.
            image_in_sequence = {}
            consumed_all_images_in_sequence = {}
            skeys = sequence_frames.keys()
            skeys.sort()
            for k in skeys:
                image_in_sequence[k] = 0
                consumed_all_images_in_sequence[k] = 0

            # Determine the number of images in each viewport so we can
            # know the number of frames that we'll be compositing. We'll
            # use that information to report progress to the client. 
            nframes_per_viewport = {}
            for k in vkeys:
                # List of sequences for the current viewport.
                seqList = self.viewport_data[k]["sequenceList"]
                for seq in seqList:
                    nframes_in_sequence = sequence_frames[seq][0]
                    if k in nframes_per_viewport.keys():
                        nframes_per_viewport[k] = nframes_per_viewport[k] + nframes_in_sequence
                    else:
                        nframes_per_viewport[k] = nframes_in_sequence
            maxFramesToComposite = 0
            for k in vkeys:
                if nframes_per_viewport[k] > maxFramesToComposite:
                    maxFramesToComposite = nframes_per_viewport[k]
            self.Log("%d frames must be composited." % maxFramesToComposite)

            progress = 0
            lastProgress = 0
            iterating = 1
            composite_index = 0
            prefix = self.moviemaker.tmpDir + self.moviemaker.slash
            while iterating:
                viewport_image = {}
                sequence_t = {}

                # Get the name of the current image file in the sequence being 
                # mapped to the k'th viewport.
                for k in vkeys: 
                    # Determine the size of the viewport in pixels, given the 
                    # current movie format size
                    coordinates = self.viewport_data[k]["coordinates"]
                    x0 = coordinates[0] * width
                    x1 = coordinates[2] * width
                    y0 = coordinates[1] * height
                    y1 = coordinates[3] * height
                    vpw = x1 - x0
                    vph = y1 - y0
                    if vpw < 0:
                        vpw = -vpw
                    if vph < 0:
                        vph = -vph

                    # Look for the current sequence in the viewport's sequenceList
                    seqIndex = -1
                    seq = current_sequence_for_viewport[k]
                    seqType = self.sequence_data[seq]["sequenceType"]
                    seqList = self.viewport_data[k]["sequenceList"]
                    for i in range(len(seqList)):
                        if seqList[i] == seq:
                            seqIndex = i
                            break

                    sequence_t[seq] = float(image_in_sequence[seq]) / float(sequence_frames[seq][0]-1)
                    if seqType in self.compositingSequences:
                        # Get the filename for the composite sequence since 
                        # this sequence is kind of a transition and needs inputs.
                        viewport_image[k] = self.GetFilenameForCompositeSequence(\
                            seqList, seqIndex, sequence_frames, vpw, vph, stereo)
                    else:
                        # The sequence is not a composite transition so
                        # create a filename for it.
                        viewport_image[k] = self.CreateFilename(sequence_frames[seq][1], (vpw, vph, stereo), image_in_sequence[seq])

                    nextImageIndex = image_in_sequence[seq] + 1

                    if nextImageIndex >= sequence_frames[seq][0]:
                        # We got to this stage because we exceeded the number of 
                        # images in a sequence and we have to switch to a new 
                        # sequence.
                        consumed_all_images_in_sequence[seq] = 1
                        sequence_t[seq] = 1.

                        # See if we can switch to a new sequence.
                        if seqIndex+1 < len(seqList):
                            seqIndex = seqIndex + 1
                            seq = seqList[seqIndex]
                            seqType = self.sequence_data[seq]["sequenceType"]
                            current_sequence_for_viewport[k] = seq
                            sequence_t[seq] = 0
                    else:
                        image_in_sequence[seq] = nextImageIndex

                stereoPrefixes = []
                if iterating:
                    if stereo == self.moviemaker.STEREO_LEFTRIGHT:
                        stereoPrefixes = ["left_", "right_"]
                    else:
                        stereoPrefixes = [""]

                for sPrefix in stereoPrefixes:
                    # Now that we've determined which sized image goes in which viewport
                    # Let's use that information to composite the images together.
                    paramFile = "viewport.info"
                    f = open(paramFile, "wt")
                    self.Log("############### frame %04d #####################" % composite_index)
                    f.write("%d\n" % len(vkeys))                      # Number of viewports
                    f.write("%s %s\n" % (width, height))              # Image size
                    f.write("255 255 255\n")                          # Image background color
                    # Output image name.
                    outputName = "%s.ppm" % format[0] % composite_index
                    outputName = self.InsertPrefix(outputName, sPrefix)
                    outputName = destDir + self.moviemaker.slash + outputName
                    f.write(outputName + "\n")
                    index = 0
                    for k in vkeys:
                        coordinates = self.viewport_data[k]["coordinates"]
                        if index == 0:
                            if coordinates[0] == 0. and coordinates[1] == 0. and\
                               coordinates[2] == 1. and coordinates[3] == 1.:
                                f.write("%s\n" % self.InsertPrefix(viewport_image[k], sPrefix))  # Name of background image
                            else:
                                f.write("-\n")                       # No background image
                            f.write("0.02 0.02\n")                   # Shadow x,y offset
                            f.write("0.02\n")                        # Shadow blur radius

                        f.write("%s\n" % self.InsertPrefix(viewport_image[k], sPrefix))          # Name of viewport image

                        #
                        # Get some information about the type of sequence that 
                        # we're processing. If we have a "Composite" modification 
                        # sequence such as a slide, it means that the sequence 
                        # does nothing except affect the compositing phase.
                        #
                        seq = current_sequence_for_viewport[k]
                        sequenceType = self.sequence_data[seq]["sequenceType"]
                        dsOffset = 0.
                        if self.viewport_data[k]["dropShadow"]:
                            dsOffset = 0.02 * 1.5
                        t = sequence_t[seq]
                        # If it's a compositing sequence that we're reversing then reverse t.
                        if sequenceType in self.compositingSequences and self.sequence_data[seq]["reverse"] > 0:
                            t = 1. - t
                        if sequenceType == "CompositeRLSlide":
                            dX = (coordinates[2] - coordinates[0])
                            x0 = (1.-t) + t * coordinates[0]
                            x1 = (1.-t) * (1. + dX) + t * coordinates[2]
                            f.write("%g %g %g %g\n" % (x0, coordinates[1], x1, coordinates[3]))
                        elif sequenceType == "CompositeLRSlide":
                            dX = (coordinates[2] - coordinates[0])
                            x0 = -(coordinates[0] + dX + dsOffset)
                            x1 = x0 + dX
                            lowX = (1. - t) * x0 + t * coordinates[0]
                            highX = (1. - t) * x1 + t * coordinates[2]
                            f.write("%g %g %g %g\n" % (lowX, coordinates[1], highX, coordinates[3]))
                        elif sequenceType == "CompositeTBSlide":
                            dY = coordinates[3] - coordinates[1]
                            y0 = 1. + dsOffset
                            y1 = 1. + dY + dsOffset
                            lowY = (1. - t) * y0 + t * coordinates[1]
                            highY = (1. - t) * y1 + t * coordinates[3]
                            f.write("%g %g %g %g\n" % (coordinates[0], lowY, coordinates[2], highY))
                        elif sequenceType == "CompositeBTSlide":
                            dY = coordinates[3] - coordinates[1]
                            y0 = -dY
                            y1 = 0.
                            lowY = (1. - t) * y0 + t * coordinates[1]
                            highY = (1. - t) * y1 + t * coordinates[3]
                            f.write("%g %g %g %g\n" % (coordinates[0], lowY, coordinates[2], highY))
                        else:
                            f.write("%g %g %g %g\n" % coordinates)    # Viewport coordinates

                        # Viewport opacity
                        if sequenceType == "CompositeFadeOut":
                            opacity = (1.-t)*self.viewport_data[k]["opacity"]
                            f.write("1\n")
                            f.write("%g\n" % opacity)
                        elif sequenceType == "CompositeFadeIn":
                            opacity = t * self.viewport_data[k]["opacity"]
                            f.write("1\n")
                            f.write("%g\n" % opacity)
                        else:
                            # Viewport opaque mode
                            opaque = self.viewport_data[k]["compositing"]
                            if opaque == 3:
                                opaque = 2
                            f.write("%d\n" % opaque)
                            f.write("%g\n" % self.viewport_data[k]["opacity"])

                        # Write the viewport transparent color. If the mode is 3 then use the background
                        # color for the sequence.
                        if self.viewport_data[k]["compositing"] == 3:
                            seq = current_sequence_for_viewport[k]
                            f.write("%d %d %d\n" % sequence_frames[seq][2][:3])
                        else:
                            f.write("%d %d %d\n" % self.viewport_data[k]["replaceColor"])

                        # Write whether the viewport has a drop shadow. 
                        f.write("%d\n" % self.viewport_data[k]["dropShadow"])

                        index = index + 1
                    f.close()

                    command = "visit -composite %s %s" % (paramFile, outputName)
                    self.Debug(5, command)
                    os.system(command)

                    # If we need to do format conversion then do it now with ImageMagick.
                    if doFormatConvert:
                        newFormatName = "%s%s" % (format[0] % composite_index, formatExtension)
                        newFormatName = self.InsertPrefix(newFormatName, sPrefix)
                        newFormatName = convertDir + self.moviemaker.slash + newFormatName
                        command = "convert %s %s" % (outputName, newFormatName)
                        self.Debug(5, command)
                        os.system(command)

                # If we are doing something with the image then report progress.
                if iterating:
                    composite_index = composite_index + 1

                    if maxFramesToComposite > 1:
                        t = float(composite_index) / float(maxFramesToComposite-1)
                        progress = int(((1.-t)*fmt_percents[0] + t*fmt_percents[1]) * 100.)
                        if progress != lastProgress:
                            self.SendClientProgress("Compositing frames", progress, 100)
                            lastProgress = progress


                # See if we've consumed all of the images in all sequences.
                all_consumed = 1
                for k in skeys:
                    if consumed_all_images_in_sequence[k] == 0:
                        all_consumed = 0
                iterating = not all_consumed

            # Remove the compositing settings file.
            try:
                os.unlink("viewport.info")
            except OSError:
                self.Log("Error removing viewport.info")

            numFrames = composite_index

        return numFrames

    ###########################################################################
    # Method: LinkFrames
    #
    # Purpose:    This method is called when we have just one viewport but
    #             have many sequences that need to be linked together.
    #
    # Programmer: Brad Whitlock
    # Date:       Fri Oct 13 15:03:16 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def LinkFrames(self, sequence_frames, movieFormats):
        # There is just 1 viewport if this method was called.
        vpName = self.viewport_data.keys()[0]
        sequenceList = self.viewport_data[vpName]["sequenceList"]
        prefix = self.moviemaker.tmpDir + self.moviemaker.slash 
        index = 0
        self.Debug(5, "Linkframes: prefix=%s" % prefix)
        for format in movieFormats:
            # This set of movie frames must be width x height pixels.
            width = format[2]
            height = format[3]
            stereo = format[4]

            # Determine the size of the viewport.
            coordinates = self.viewport_data[vpName]["coordinates"]
            x0 = coordinates[0] * width
            x1 = coordinates[2] * width
            y0 = coordinates[1] * height
            y1 = coordinates[3] * height
            vpw = x1 - x0
            vph = y1 - y0
            if vpw < 0:
                vpw = -vpw
            if vph < 0:
                vph = -vph

            index = 0
            for seqName in sequenceList:
                nFrames = sequence_frames[seqName][0]
                seqBase = sequence_frames[seqName][1]
                for frame in range(nFrames):
                    imageT = self.moviemaker.outputNeedsInput[format[1]]
                    ext = self.moviemaker.formatExtension[imageT]
                    if stereo == self.moviemaker.STEREO_LEFTRIGHT:
                        for eye in ("left_", "right_"):
                            srcName = self.CreateFilename(eye + seqBase, (vpw, vph, stereo), frame)
                            if format[1] != self.moviemaker.FORMAT_PPM:
                                destName = "%s%s" % (format[0] % index, ext)
                            else:
                                destName = "%s.ppm" % format[0] % index
                            lastSlash = string.find(destName, self.moviemaker.slash)
                            if lastSlash != -1:
                                destName = prefix + destName[:lastSlash+1] + eye + destName[lastSlash+1:]
                            else:
                                destName = prefix + eye + destName
                            if imageT != self.moviemaker.FORMAT_PPM:
                                command = "convert %s %s" % (srcName, destName)
                                self.Debug(5, "LinkFrames: %s" % command)
                                os.system(command)
                            else:
                                self.Debug(5, "LinkFrames: cp %s %s" % (srcName, destName))
                                CopyFile(srcName, destName, 1)
                    else:
                        srcName = self.CreateFilename(seqBase, (vpw, vph, stereo), frame)
                        destName = prefix + "%s%s" % (format[0] % index, ext)
                        if imageT != self.moviemaker.FORMAT_PPM:
                            command = "convert %s %s" % (srcName, destName)
                            self.Debug(5, "LinkFrames: %s" % command)
                            os.system(command)
                        else:
                            self.Debug(5, "LinkFrames: cp %s %s" % (srcName, destName))
                            CopyFile(srcName, destName, 1)
                    index = index + 1

        return index

    ###########################################################################
    # Method: SetupVisualizationFromSession
    #
    # Purpose:
    #   Reads the session file name from the template file speficiation and
    #   any list of alternate sources with which to restore the session and 
    #   tells VisIt to restore the session.
    #
    # Arguments:
    #   self : This object reference.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def SetupVisualizationFromSession(self):
        self.Debug(1,"Calling VisItMovieTemplate.SetupVisualizationFromSession")
        if self.generic_data.has_key("SESSIONFILE"):
            sessionFile = str(self.generic_data["SESSIONFILE"])
            self.Debug(1,"VisItMovieTemplate.SetupVisualizationFromSession using session %s" % sessionFile)

            # When we read the template options file, we read the source list
            # if it was available. The source map lets us know that we should
            # replace certain databases in a session file.
            if self.generic_data.has_key("SOURCES"):
                if len(self.generic_data["SOURCES"]) == 0:
                    # The source list was empty.
                    RestoreSession(sessionFile, 0)
                else:
                    # Use the source list to restore the session file with
                    # different sources.
                    if len(self.generic_data["SOURCES"]) == 1:
                        self.Debug(1,"SetupVisualizationFromSession source list=%s" % str(self.generic_data["SOURCES"][0]))
                        RestoreSessionWithDifferentSources(sessionFile, 0, str(self.generic_data["SOURCES"][0]))
                    else:
                        sources = []                                 
                        for s in self.generic_data["SOURCES"]:
                            sources = sources + [str(s)]
                        sources = tuple(sources)
                        self.Debug(1,"SetupVisualizationFromSession source list=%s" % str(sources))
                        RestoreSessionWithDifferentSources(sessionFile, 0, sources)
            else:
                # There was no list of sources.
                RestoreSession(sessionFile, 0)

    ###########################################################################
    # Method: SetupVisualization
    #
    # Purpose:
    #   Default implementation for setting up plots that the movie needs in
    #   order to successfully handle "CurrentPlots" sequences.
    #
    # Arguments:
    #   self : This object reference.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def SetupVisualization(self):
        self.Debug(1,"Calling VisItMovieTemplate.SetupVisualization()")

    ###########################################################################
    # Method: HandleScriptingSequence
    #
    # Purpose:
    #   Default implementation for handling "Scripting" sequences that lets
    #   us more easily delegate these sequences to derived classes.
    #
    # Arguments:
    #   self : This object reference.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def HandleScriptingSequence(self, seqName, formats, percents):
        self.Debug(1,"Calling VisItMovieTemplate.HandleScriptingSequence()")
        return 0

    ###########################################################################
    # Method: CreateSequenceFrames
    #
    # Purpose:
    #   This method generates movie frames using the information found in
    #   the movie template specification.
    #
    # Arguments:
    #   self                : This object reference.
    #   movieFormats        : The list of movie formats that we need to make.
    #   percentsPerSequence : A map of percents that let us know how much of
    #                         the whole production is allocated to a given
    #                         sequence so we can report progress to the user.
    #
    # Notes:      The VisItMovieTemplate base class calls this method to
    #             get the list of sequences and the images that were produced.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def CreateSequenceFrames(self, movieFormats, percentsPerSequence):
        self.Debug(1, "VisItMovieTemplate.CreateSequenceFrames: begin")

        # Determine the size/stereo of the images required for each sequence
        sequence_formats = self.DetermineImageSizesAndStereo(movieFormats)
        self.Debug(5, "Sequence sizes = %s" % str(sequence_formats))

        # Set up the plots.
        self.SetupVisualization()

        # Insert some number of sequence method calls. Each of the sequence
        # methods returns (nframes, filebase, sizes tuple)
        frames = {}
        vkeys = self.viewport_data.keys()
        vkeys.sort()

        # Look for all "CurrentPlots" sequences to get the list of requested windows.
        requestedWindows = []
        for k in vkeys:
            for seqName in self.viewport_data[k]["sequenceList"]:
                seqType = self.sequence_data[seqName]["sequenceType"]
                if seqType == "CurrentPlots":
                    requestedWindows = requestedWindows + [self.windowSequences[seqName]]
        # Now that we have a list of requested windows, make sure they all exist.
        existingWindows = GetGlobalAttributes().windows
        windowsThatDontExist = []
        for w in requestedWindows:
            if w not in existingWindows:
                windowsThatDontExist = windowsThatDontExist + [w]
        if len(windowsThatDontExist) == 1:
            msg = "The movie template requested window %d, which does not exist." % windowsThatDontExist[0]
            self.Log(msg)
            raise self.error(msg)
        elif len(windowsThatDontExist) > 1:
            msg = "The movie template requested windows %s, which do not exist." % str(windowsThatDontExist)
            self.Log(msg)
            raise self.error(msg)

        # Make sure that all of the requested windows have plots.
        windowsThatDontHavePlots = []
        for w in requestedWindows:
            SetActiveWindow(w)
            if GetNumPlots() < 1:
                windowsThatDontHavePlots = windowsThatDontHavePlots + [w]
        if len(windowsThatDontHavePlots) == 1:
            msg = "The movie template requested window %d, but it has no plots." % windowsThatDontHavePlots[0]
            self.Log(msg)
            raise self.error(msg)
        elif len(windowsThatDontHavePlots) > 1:
            msg = "The movie template requested windows %s, but they have no plots." % str(windowsThatDontHavePlots)
            self.Log(msg)
            raise self.error(msg)

        #
        # For each viewport, go through and create its "CurrentPlots" sequences
        #
        vkeys = self.viewport_data.keys()
        vkeys.sort()
        for k in vkeys:
            currentActiveWindow = -1

            # For each sequence in the sequence list, try and pick out the
            # ones that are custom or come from windows.
            for seqIndex in range(len(self.viewport_data[k]["sequenceList"])):
                seqName = self.viewport_data[k]["sequenceList"][seqIndex]
                seqType = self.sequence_data[seqName]["sequenceType"]
                if seqType == "CurrentPlots":
                    windowIndex = self.windowSequences[seqName]

                    # Set the active window.
                    self.Debug(4, "Making window %d active" % windowIndex)
                    SetActiveWindow(windowIndex)
                    currentActiveWindow = windowIndex

                    # Create the filebase and iterate over all time steps, saving images.
                    filebase = self.SequenceNameToFilebase(seqName)
                    pMsg = "Generating %s frames." % seqName
                    self.Debug(4, "Preparing to iterate over frames for window "\
                               "%d. Saving frames to %s" % (windowIndex, filebase))

                    nFrames = self.IterateAndSaveFrames(filebase, \
                        sequence_formats[seqName], percentsPerSequence[seqName], pMsg)

                    # Add an entry into the sequence frames map.
                    frames[seqName] = (nFrames, filebase, GetAnnotationAttributes().backgroundColor)

                # else do other frame producing sequences
                elif seqType == "Scripting":
                    # Call the HandleScriptingSequence method to create the scripting sequence.
                    sValues = self.HandleScriptingSequence(seqName, sequence_formats[seqName], percentsPerSequence[seqName])

                    # The method to create the scripting sequence did not return the expected
                    # values (int nFrames, string filebase, int bgcolor[3]) so we cannot
                    # continue making the movie.
                    if sValues == 0:
                        raise self.error("The scripting sequence %s could not be created. "
                                         "VisIt cannot generate your movie." % seqName)

                    # Add an entry into the sequence frames map.
                    frames[seqName] = (sValues[0], sValues[1], sValues[2])

                    # We have no idea what a scripting sequence could have done so restore
                    # the session, if one exists, so the state will be compatible with the
                    # other CurrentPlots sequences that might not have been processed yet.
                    self.SetupVisualization()

                    # Get the current window
                    gAtts = GetGlobalAttributes()
                    currentActiveWindow = gAtts.windows[gAtts.activeWindow]

                elif seqType in self.rotationTypes:
                    if currentActiveWindow == -1:
                        # Look for the next "CurrentPlots" or "Scripting" sequences.
                        for sIndex in range(seqIndex+1,len(self.viewport_data[k]["sequenceList"])):
                            sn = self.viewport_data[k]["sequenceList"][sIndex]
                            st = self.sequence_data[sn]["sequenceType"]
                            if st == "CurrentPlots" or st == "Scripting":
                                windowIndex = self.windowSequences[sn]
                                # Set the active window.
                                self.Debug(4, "Making window %d active" % windowIndex)
                                SetActiveWindow(windowIndex)
                                currentActiveWindow = windowIndex
                                # Since we have a rotation before a "current plots" 
                                # sequence, let's be sure to make sure that the
                                # time state is set to zero.
                                if GetWindowInformation().activeTimeSlider != -1:
                                    self.Debug(4, "Going back to first time state.")
                                    SetTimeSliderState(0)
                                break

                    if currentActiveWindow != -1:
                        startAngle = self.sequence_data[seqName]["startAngle"]
                        endAngle = self.sequence_data[seqName]["endAngle"]
                        nSteps = self.sequence_data[seqName]["nSteps"]

                        # Create the filebase and iterate over all time steps, saving images.
                        filebase = self.SequenceNameToFilebase(seqName)
                        pMsg = "Generating %s frames." % seqName
                        self.Debug(4, "Preparing to rotate. Saving frames to %s" % filebase)

                        nFrames = 0
                        if seqType == "RotationX":
                            nFrames = self.RotateSequence(0, startAngle, endAngle, nSteps,\
                                filebase, sequence_formats[seqName], percentsPerSequence[seqName],\
                                pMsg)
                        elif seqType == "RotationY":
                            nFrames = self.RotateSequence(1, startAngle, endAngle, nSteps,\
                                filebase, sequence_formats[seqName], percentsPerSequence[seqName],\
                                pMsg)
                        elif seqType == "RotationZ":
                            nFrames = self.RotateSequence(2, startAngle, endAngle, nSteps,\
                                filebase, sequence_formats[seqName], percentsPerSequence[seqName],\
                                pMsg)
                        elif seqType == "ScreenRotationX":
                            nFrames = self.RotateInScreenSequence(0, startAngle, endAngle, nSteps,\
                                filebase, sequence_formats[seqName], percentsPerSequence[seqName],\
                                pMsg)
                        elif seqType == "ScreenRotationY":
                            nFrames = self.RotateInScreenSequence(1, startAngle, endAngle, nSteps,\
                                filebase, sequence_formats[seqName], percentsPerSequence[seqName],\
                                pMsg)
                        elif seqType == "ScreenRotationZ":
                            nFrames = self.RotateInScreenSequence(2, startAngle, endAngle, nSteps,\
                                filebase, sequence_formats[seqName], percentsPerSequence[seqName],\
                                pMsg)
                        # Add an entry into the sequence frames map.
                        frames[seqName] = (nFrames, filebase, GetAnnotationAttributes().backgroundColor)
                    else:
                        self.Log("Sequence %s is a rotation sequence but it cannot be "\
                                 "applied because there are no image producing plots in "\
                                 "the viewport." % seqName)
                elif seqType in self.transitionTypes:
                    self.Log("Sequence %s is a transition so it is skipped for now." % seqName)

        self.Debug(1, "VisItMovieTemplate.CreateSequenceFrames: end")
        return frames

    ###########################################################################
    # Method: GenerateFrames
    #
    # Purpose:    This method generates the frames for the template.
    #
    # Notes:      Basically, the movie template can do whatever it wants
    #             as long as at the end of this method, there exist image files
    #             of the sizes and formats prescribed by the 
    #             moviemaker.movieFormats list.
    #
    # Programmer: Brad Whitlock
    # Date:       Thu Sep 21 11:37:01 PDT 2006
    #
    # Modifications:
    #
    ###########################################################################

    def GenerateFrames(self, templateOptionsFile, movieFormats):
        # Make ure that the template file contains valid entries.
        if self.ValidateTemplate() == 0:
            return -1

        # Determine if the formats contain different stereo settings.
        self.stereoFilenames = 0
        if len(movieFormats) > 1:
            s = movieFormats[0][4]
            for i in range(1, len(movieFormats)):
                if s != movieFormats[i][4]:
                    self.stereoFilenames = 1
                    break

        nFrames = 0
        try:
            # Determine the percentage of the total time that will be 
            # reported for each sequence.
            percentsPerSequence = self.CalculatePercentagesComplete()

            # Use the derived template movie to create the frames for the
            # various sequences.
            sequence_frames = self.CreateSequenceFrames(movieFormats, percentsPerSequence)

            # Now that the derived class has created the sequences that produce 
            # frames, create the transitions frames.
            pct = (0,0)
            if percentsPerSequence.has_key("TRANSITIONS"):
                pct = percentsPerSequence["TRANSITIONS"]
            sequence_frames = self.TransitionFrames(sequence_frames, movieFormats, pct)

            # If compositing is required then we need to composite the frames.
            if self.CompositingRequired():
                # Create new sequences based on the compositing sequences that 
                # are stored in sequence_data.
                for k in self.viewport_data.keys():
                    for sName in self.viewport_data[k]["sequenceList"]:
                        if self.sequence_data[sName]["sequenceType"] in self.compositingSequences:
                            sequence_frames[sName] = (self.sequence_data[sName]["nFrames"],\
                                self.SequenceNameToFilebase(sName), (0,0,0))

                pct = percentsPerSequence["COMPOSITING"]
                nFrames = self.CompositeFrames(sequence_frames, movieFormats, pct)
            elif len(self.sequence_data.keys()) > 1:
                nFrames = self.LinkFrames(sequence_frames, movieFormats)

        except self.error, value:
            self.LogAndPrint(value.message)
            nFrames = -1

        return nFrames


    ###########################################################################
    # Method: RotateAxis
    #
    # Purpose:    This method helps rotate the 3D view.
    #
    # Programmer: Brad Whitlock
    # Date:       Tue Nov 7 14:05:47 PST 2006
    #
    # Modifications:
    #
    ###########################################################################
    def RotateAxis(self, axis, angle, screenRotate, in_view3d):
        import math

        v1 = [0.,0.,0.]
        v2 = [0.,0.,0.]
        v3 = [0.,0.,0.]
        t1 = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
        t2 = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
        m1 = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
        m2 = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
        r = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
        ma = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
        mb = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
        mc = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
        rM = [1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
        viewNormal = [0.,0.,0.]
        viewUp = [0.,0.,0.]
        viewFocus = [0.,0.,0.]

        # Copy the input view so we can mess it up.
        view3d = View3DAttributes()
        view3d.viewNormal = (in_view3d.viewNormal[0], in_view3d.viewNormal[1], in_view3d.viewNormal[2])
        view3d.focus = (in_view3d.focus[0], in_view3d.focus[1], in_view3d.focus[2])
        view3d.viewUp = (in_view3d.viewUp[0], in_view3d.viewUp[1], in_view3d.viewUp[2])
        view3d.viewAngle = in_view3d.viewAngle
        view3d.parallelScale = in_view3d.parallelScale
        view3d.nearPlane = in_view3d.nearPlane
        view3d.farPlane = in_view3d.farPlane
        view3d.imagePan = (in_view3d.imagePan[0], in_view3d.imagePan[1])
        view3d.imageZoom = in_view3d.imageZoom
        view3d.perspective = in_view3d.perspective
        view3d.eyeAngle = in_view3d.eyeAngle
        view3d.centerOfRotationSet = in_view3d.centerOfRotationSet
        view3d.centerOfRotation = (in_view3d.centerOfRotation[0], in_view3d.centerOfRotation[1], in_view3d.centerOfRotation[2])

        #
        # Calculate the rotation matrix in screen coordinates.
        #
        angleRadians = angle * (3.141592653589793 / 180.)
        if axis == 0:
            r[0]  = 1.
            r[1]  = 0.
            r[2]  = 0.
            r[3]  = 0.
            r[4]  = 0.
            r[5]  = math.cos(angleRadians)
            r[6]  = - math.sin(angleRadians)
            r[7]  = 0.
            r[8]  = 0.
            r[9]  = math.sin(angleRadians)
            r[10] = math.cos(angleRadians)
            r[11] = 0.
            r[12] = 0.
            r[13] = 0.
            r[14] = 0.
            r[15] = 1.
        elif axis == 1:
            r[0]  = math.cos(angleRadians)
            r[1]  = 0.
            r[2]  = math.sin(angleRadians)
            r[3]  = 0.
            r[4]  = 0.
            r[5]  = 1.
            r[6]  = 0.
            r[7]  = 0.
            r[8]  = - math.sin(angleRadians)
            r[9]  = 0.
            r[10]  = math.cos(angleRadians)
            r[11] = 0.
            r[12] = 0.
            r[13] = 0.
            r[14] = 0.
            r[15] = 1.
        elif axis == 2:
            r[0]  = math.cos(angleRadians)
            r[1]  = - math.sin(angleRadians)
            r[2]  = 0.
            r[3]  = 0.
            r[4]  = math.sin(angleRadians)
            r[5]  = math.cos(angleRadians)
            r[6]  = 0.
            r[7]  = 0.
            r[8]  = 0.
            r[9]  = 0.
            r[10]  = 1.
            r[11] = 0.
            r[12] = 0.
            r[13] = 0.
            r[14] = 0.
            r[15] = 1.
 
        #
        # Calculate the matrix to rotate from object coordinates to screen
        # coordinates and its inverse.
        #
        v1[0] = view3d.GetViewNormal()[0]
        v1[1] = view3d.GetViewNormal()[1]
        v1[2] = view3d.GetViewNormal()[2]
 
        v2[0] = view3d.GetViewUp()[0]
        v2[1] = view3d.GetViewUp()[1]
        v2[2] = view3d.GetViewUp()[2]
 
        v3[0] =   v2[1]*v1[2] - v2[2]*v1[1]
        v3[1] = - v2[0]*v1[2] + v2[2]*v1[0]
        v3[2] =   v2[0]*v1[1] - v2[1]*v1[0]

        if screenRotate:
            m1[0]  = v3[0]
            m1[1]  = v2[0]
            m1[2]  = v1[0]
            m1[3]  = 0.
            m1[4]  = v3[1]
            m1[5]  = v2[1]
            m1[6]  = v1[1]
            m1[7]  = 0.
            m1[8]  = v3[2]
            m1[9]  = v2[2]
            m1[10] = v1[2]
            m1[11] = 0.
            m1[12] = 0.
            m1[13] = 0.
            m1[14] = 0.
            m1[15] = 1.

        m2[0]  = m1[0]
        m2[1]  = m1[4]
        m2[2]  = m1[8]
        m2[3]  = m1[12]
        m2[4]  = m1[1]
        m2[5]  = m1[5]
        m2[6]  = m1[9]
        m2[7]  = m1[13]
        m2[8]  = m1[2]
        m2[9]  = m1[6]
        m2[10] = m1[10]
        m2[11] = m1[14]
        m2[12] = m1[3]
        m2[13] = m1[7]
        m2[14] = m1[11]
        m2[15] = m1[15]

        #
        # Calculate the translation to the center of rotation (and its
        # inverse).
        #
        t1[0]  = 1.
        t1[1]  = 0.
        t1[2]  = 0.
        t1[3]  = 0.
        t1[4]  = 0.
        t1[5]  = 1.
        t1[6]  = 0.
        t1[7]  = 0.
        t1[8]  = 0.
        t1[9]  = 0.
        t1[10] = 1.
        t1[11] = 0.
        t1[12] = -view3d.GetCenterOfRotation()[0]
        t1[13] = -view3d.GetCenterOfRotation()[1]
        t1[14] = -view3d.GetCenterOfRotation()[2]
        t1[15] = 1.

        t2[0]  = 1.
        t2[1]  = 0.
        t2[2]  = 0.
        t2[3]  = 0.
        t2[4]  = 0.
        t2[5]  = 1.
        t2[6]  = 0.
        t2[7]  = 0.
        t2[8]  = 0.
        t2[9]  = 0.
        t2[10] = 1.
        t2[11] = 0.
        t2[12] = view3d.GetCenterOfRotation()[0]
        t2[13] = view3d.GetCenterOfRotation()[1]
        t2[14] = view3d.GetCenterOfRotation()[2]
        t2[15] = 1.

        #
        # Form the composite transformation matrix t1 X m1 X r X m2 X t2.
        #
        ma[0]  = t1[0]*m1[0]  + t1[1]*m1[4]  + t1[2]*m1[8]   + t1[3]*m1[12]
        ma[1]  = t1[0]*m1[1]  + t1[1]*m1[5]  + t1[2]*m1[9]   + t1[3]*m1[13]
        ma[2]  = t1[0]*m1[2]  + t1[1]*m1[6]  + t1[2]*m1[10]  + t1[3]*m1[14]
        ma[3]  = t1[0]*m1[3]  + t1[1]*m1[7]  + t1[2]*m1[11]  + t1[3]*m1[15]
        ma[4]  = t1[4]*m1[0]  + t1[5]*m1[4]  + t1[6]*m1[8]   + t1[7]*m1[12]
        ma[5]  = t1[4]*m1[1]  + t1[5]*m1[5]  + t1[6]*m1[9]   + t1[7]*m1[13]
        ma[6]  = t1[4]*m1[2]  + t1[5]*m1[6]  + t1[6]*m1[10]  + t1[7]*m1[14]
        ma[7]  = t1[4]*m1[3]  + t1[5]*m1[7]  + t1[6]*m1[11]  + t1[7]*m1[15]
        ma[8]  = t1[8]*m1[0]  + t1[9]*m1[4]  + t1[10]*m1[8]  + t1[11]*m1[12]
        ma[9]  = t1[8]*m1[1]  + t1[9]*m1[5]  + t1[10]*m1[9]  + t1[11]*m1[13]
        ma[10] = t1[8]*m1[2]  + t1[9]*m1[6]  + t1[10]*m1[10] + t1[11]*m1[14]
        ma[11] = t1[8]*m1[3]  + t1[9]*m1[7]  + t1[10]*m1[11] + t1[11]*m1[15]
        ma[12] = t1[12]*m1[0] + t1[13]*m1[4] + t1[14]*m1[8]  + t1[15]*m1[12]
        ma[13] = t1[12]*m1[1] + t1[13]*m1[5] + t1[14]*m1[9]  + t1[15]*m1[13]
        ma[14] = t1[12]*m1[2] + t1[13]*m1[6] + t1[14]*m1[10] + t1[15]*m1[14]
        ma[15] = t1[12]*m1[3] + t1[13]*m1[7] + t1[14]*m1[11] + t1[15]*m1[15]

        mb[0]  = ma[0]*r[0]  + ma[1]*r[4]  + ma[2]*r[8]   + ma[3]*r[12]
        mb[1]  = ma[0]*r[1]  + ma[1]*r[5]  + ma[2]*r[9]   + ma[3]*r[13]
        mb[2]  = ma[0]*r[2]  + ma[1]*r[6]  + ma[2]*r[10]  + ma[3]*r[14]
        mb[3]  = ma[0]*r[3]  + ma[1]*r[7]  + ma[2]*r[11]  + ma[3]*r[15]
        mb[4]  = ma[4]*r[0]  + ma[5]*r[4]  + ma[6]*r[8]   + ma[7]*r[12]
        mb[5]  = ma[4]*r[1]  + ma[5]*r[5]  + ma[6]*r[9]   + ma[7]*r[13]
        mb[6]  = ma[4]*r[2]  + ma[5]*r[6]  + ma[6]*r[10]  + ma[7]*r[14]
        mb[7]  = ma[4]*r[3]  + ma[5]*r[7]  + ma[6]*r[11]  + ma[7]*r[15]
        mb[8]  = ma[8]*r[0]  + ma[9]*r[4]  + ma[10]*r[8]  + ma[11]*r[12]
        mb[9]  = ma[8]*r[1]  + ma[9]*r[5]  + ma[10]*r[9]  + ma[11]*r[13]
        mb[10] = ma[8]*r[2]  + ma[9]*r[6]  + ma[10]*r[10] + ma[11]*r[14]
        mb[11] = ma[8]*r[3]  + ma[9]*r[7]  + ma[10]*r[11] + ma[11]*r[15]
        mb[12] = ma[12]*r[0] + ma[13]*r[4] + ma[14]*r[8]  + ma[15]*r[12]
        mb[13] = ma[12]*r[1] + ma[13]*r[5] + ma[14]*r[9]  + ma[15]*r[13]
        mb[14] = ma[12]*r[2] + ma[13]*r[6] + ma[14]*r[10] + ma[15]*r[14]
        mb[15] = ma[12]*r[3] + ma[13]*r[7] + ma[14]*r[11] + ma[15]*r[15]
 
        mc[0]  = mb[0]*m2[0]  + mb[1]*m2[4]  + mb[2]*m2[8]   + mb[3]*m2[12]
        mc[1]  = mb[0]*m2[1]  + mb[1]*m2[5]  + mb[2]*m2[9]   + mb[3]*m2[13]
        mc[2]  = mb[0]*m2[2]  + mb[1]*m2[6]  + mb[2]*m2[10]  + mb[3]*m2[14]
        mc[3]  = mb[0]*m2[3]  + mb[1]*m2[7]  + mb[2]*m2[11]  + mb[3]*m2[15]
        mc[4]  = mb[4]*m2[0]  + mb[5]*m2[4]  + mb[6]*m2[8]   + mb[7]*m2[12]
        mc[5]  = mb[4]*m2[1]  + mb[5]*m2[5]  + mb[6]*m2[9]   + mb[7]*m2[13]
        mc[6]  = mb[4]*m2[2]  + mb[5]*m2[6]  + mb[6]*m2[10]  + mb[7]*m2[14]
        mc[7]  = mb[4]*m2[3]  + mb[5]*m2[7]  + mb[6]*m2[11]  + mb[7]*m2[15]
        mc[8]  = mb[8]*m2[0]  + mb[9]*m2[4]  + mb[10]*m2[8]  + mb[11]*m2[12]
        mc[9]  = mb[8]*m2[1]  + mb[9]*m2[5]  + mb[10]*m2[9]  + mb[11]*m2[13]
        mc[10] = mb[8]*m2[2]  + mb[9]*m2[6]  + mb[10]*m2[10] + mb[11]*m2[14]
        mc[11] = mb[8]*m2[3]  + mb[9]*m2[7]  + mb[10]*m2[11] + mb[11]*m2[15]
        mc[12] = mb[12]*m2[0] + mb[13]*m2[4] + mb[14]*m2[8]  + mb[15]*m2[12]
        mc[13] = mb[12]*m2[1] + mb[13]*m2[5] + mb[14]*m2[9]  + mb[15]*m2[13]
        mc[14] = mb[12]*m2[2] + mb[13]*m2[6] + mb[14]*m2[10] + mb[15]*m2[14]
        mc[15] = mb[12]*m2[3] + mb[13]*m2[7] + mb[14]*m2[11] + mb[15]*m2[15]
 
        rM[0]  = mc[0]*t2[0]  + mc[1]*t2[4]  + mc[2]*t2[8]   + mc[3]*t2[12]
        rM[1]  = mc[0]*t2[1]  + mc[1]*t2[5]  + mc[2]*t2[9]   + mc[3]*t2[13]
        rM[2]  = mc[0]*t2[2]  + mc[1]*t2[6]  + mc[2]*t2[10]  + mc[3]*t2[14]
        rM[3]  = mc[0]*t2[3]  + mc[1]*t2[7]  + mc[2]*t2[11]  + mc[3]*t2[15]
        rM[4]  = mc[4]*t2[0]  + mc[5]*t2[4]  + mc[6]*t2[8]   + mc[7]*t2[12]
        rM[5]  = mc[4]*t2[1]  + mc[5]*t2[5]  + mc[6]*t2[9]   + mc[7]*t2[13]
        rM[6]  = mc[4]*t2[2]  + mc[5]*t2[6]  + mc[6]*t2[10]  + mc[7]*t2[14]
        rM[7]  = mc[4]*t2[3]  + mc[5]*t2[7]  + mc[6]*t2[11]  + mc[7]*t2[15]
        rM[8]  = mc[8]*t2[0]  + mc[9]*t2[4]  + mc[10]*t2[8]  + mc[11]*t2[12]
        rM[9]  = mc[8]*t2[1]  + mc[9]*t2[5]  + mc[10]*t2[9]  + mc[11]*t2[13]
        rM[10] = mc[8]*t2[2]  + mc[9]*t2[6]  + mc[10]*t2[10] + mc[11]*t2[14]
        rM[11] = mc[8]*t2[3]  + mc[9]*t2[7]  + mc[10]*t2[11] + mc[11]*t2[15]
        rM[12] = mc[12]*t2[0] + mc[13]*t2[4] + mc[14]*t2[8]  + mc[15]*t2[12]
        rM[13] = mc[12]*t2[1] + mc[13]*t2[5] + mc[14]*t2[9]  + mc[15]*t2[13]
        rM[14] = mc[12]*t2[2] + mc[13]*t2[6] + mc[14]*t2[10] + mc[15]*t2[14]
        rM[15] = mc[12]*t2[3] + mc[13]*t2[7] + mc[14]*t2[11] + mc[15]*t2[15]

        #
        # Calculate the new view normal and view up.
        #
        viewNormal[0] = view3d.GetViewNormal()[0] * rM[0] +\
                        view3d.GetViewNormal()[1] * rM[4] +\
                        view3d.GetViewNormal()[2] * rM[8]
        viewNormal[1] = view3d.GetViewNormal()[0] * rM[1] +\
                        view3d.GetViewNormal()[1] * rM[5] +\
                        view3d.GetViewNormal()[2] * rM[9]
        viewNormal[2] = view3d.GetViewNormal()[0] * rM[2] +\
                        view3d.GetViewNormal()[1] * rM[6] +\
                        view3d.GetViewNormal()[2] * rM[10]
        dist = math.sqrt(viewNormal[0]*viewNormal[0] +\
                         viewNormal[1]*viewNormal[1] +\
                         viewNormal[2]*viewNormal[2])
        viewNormal[0] = viewNormal[0] / dist
        viewNormal[1] = viewNormal[1] / dist
        viewNormal[2] = viewNormal[2] / dist 
        view3d.viewNormal = tuple(viewNormal)
 
        viewUp[0] = view3d.GetViewUp()[0] * rM[0] +\
                    view3d.GetViewUp()[1] * rM[4] +\
                    view3d.GetViewUp()[2] * rM[8]
        viewUp[1] = view3d.GetViewUp()[0] * rM[1] +\
                    view3d.GetViewUp()[1] * rM[5] +\
                    view3d.GetViewUp()[2] * rM[9]
        viewUp[2] = view3d.GetViewUp()[0] * rM[2] +\
                    view3d.GetViewUp()[1] * rM[6] +\
                    view3d.GetViewUp()[2] * rM[10]
        dist = math.sqrt(viewUp[0]*viewUp[0] + viewUp[1]*viewUp[1] +\
                    viewUp[2]*viewUp[2])
        viewUp[0] = viewUp[0] / dist
        viewUp[1] = viewUp[1] / dist
        viewUp[2] = viewUp[2] / dist 
        view3d.viewUp = tuple(viewUp)
 
        if (view3d.GetCenterOfRotationSet()):
            viewFocus[0] = view3d.GetFocus()[0] * rM[0]  +\
                           view3d.GetFocus()[1] * rM[4]  +\
                           view3d.GetFocus()[2] * rM[8]  +\
                           rM[12]
            viewFocus[1] = view3d.GetFocus()[0] * rM[1]  +\
                           view3d.GetFocus()[1] * rM[5]  +\
                           view3d.GetFocus()[2] * rM[9]  +\
                           rM[13]
            viewFocus[2] = view3d.GetFocus()[0] * rM[2]  +\
                           view3d.GetFocus()[1] * rM[6]  +\
                           view3d.GetFocus()[2] * rM[10] +\
                           rM[14]
            view3d.focus = tuple(viewFocus)

        return view3d

    ###########################################################################
    # Method: RotateSequence
    #
    # Purpose:
    #   Generates a rotation sequence
    #
    # Arguments:
    #   self : This object reference.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def RotateSequence(self, axis, startAngle, endAngle, nSteps, \
                       filebase, formats, percents, msg):
        currentView = GetView3D()
        cb_data = (nSteps, RotateSequence_changeview, (self, axis, nSteps, startAngle, endAngle, 0, currentView))
        ret = self.IterateCallbackAndSaveFrames(cb_data, filebase, formats, percents, msg)
        SetView3D(currentView)
        return ret

    ###########################################################################
    # Method: RotateInScreenSequence
    #
    # Purpose:
    #   Generates a rotation in screen space sequence
    #
    # Arguments:
    #   self : This object reference.
    #
    # Programmer: Brad Whitlock
    # Creation:   Tue Nov 14 13:48:13 PST 2006
    #
    # Modifications:
    #
    ###########################################################################

    def RotateInScreenSequence(self, axis, startAngle, endAngle, nSteps, \
                               filebase, formats, percents, msg):
        currentView = GetView3D()
        cb_data = (nSteps, RotateSequence_changeview, (self, axis, nSteps, startAngle, endAngle, 1, currentView))
        ret = self.IterateCallbackAndSaveFrames(cb_data, filebase, formats, percents, msg)
        SetView3D(currentView)
        return ret

#
# Function to instantiate VisItMovieTemplate from makemovie.py
#
def InstantiateMovieTemplate(moviemaker, templateReader):
    return VisItMovieTemplate(moviemaker, templateReader)
