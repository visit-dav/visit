import os, pickle, Image, movietools, arial

###############################################################################
# Class: ImageOpsBase
#
# Purpose:    This class is responsible for managing and processing a list
#             of image operations.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

class ImageOpsBase:
    def __init__(self):
        self.operations = {"invalid":0}
        self.imageops = []
        self.fileIndex = 0
        self.baseFormat = "frame%04d.png"
        self.saveType = "PNG"

    #
    # Declares an image processing operation and the function to call when
    # the operation is found in the image operation list.
    #
    def declareOperation(self, name, callback):
        self.operations[name] = callback

    #
    # Adds an image processing operation to the image operation list so it
    # can be processed later.
    #
    def addOperation(self, operationName, args):
        self.imageops = self.imageops + [(operationName, args)]

    #
    # Backs up the image operation list to a file.
    #
    def backup(self, filename):
        retval = 0
        try:
            f = open(filename, "w")
            pickle.dump(self.imageops, f)
            f.close()
            retval = 1
        except IOError:
            print "Could not save the imageops!"
        return retval

    #
    # Reads the image operation list from a file.
    #
    def restore(self, filename):
        retval = 0
        self.imageops = []
        try:
            f = open(filename, "r")
            self.imageops = pickle.load(f)
            f.close()
            retval = 1
        except IOError:
            print "Could not read the imageops!"
        return retval

    #
    # Prints the image operation list.
    #
    def showOperations(self):
        print self.imageops

    #
    # Sets the base format used in saving images.
    #
    def setBaseFormat(self, bf):
        self.baseFormat = bf
 
    #
    # Processes the image operation list which results in new image files.
    #
    def process(self, baseFormat, startIndex):
        self.baseFormat = baseFormat
        self.fileIndex = startIndex
        self.saveType = movietools.getTypeFromExtension(baseFormat)
        for instruction in self.imageops:
            # Get the function name and arguments.
            functionName = instruction[0]
            arguments = instruction[1:][0]
            try:
                # Lookup the function using the function name.
                function = self.operations[functionName]
                function(arguments)
            except TypeError:
                print "ImageOpsBase.process(): Failed to execute: ", functionName, "(", arguments, ")"
            except KeyError:
                print "ImageOpsBase.process(): \"%s\" is an invalid instruction." % functionName

        return self.fileIndex

    #
    # Increments the file index.
    #
    def incrementFileIndex(self):
        self.fileIndex = self.fileIndex + 1

    #
    # Gets the name of the file that will be saved next.
    #
    def getSaveName(self):
        return self.baseFormat % self.fileIndex


###############################################################################
# Class: ImageOps
#
# Purpose:    This class is responsible for managing and processing a list
#             of image operations. This class defines operations for image
#             processing such as interpolating images. This class exists so
#             as a movie is being generated, we can log the desired image
#             processing operations so they are a function of the image files
#             that the movie creates. We do it like this so we don't have to
#             rewrite special image processing code when the list of files
#             used to generate the movie changes.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

class ImageOps(ImageOpsBase):
    def __init__(self):
        ImageOpsBase.__init__(self)
        # Add known operations
        self.declareOperation("addfile", self._ImageOps__addFileCallback)
        self.declareOperation("loadfile", self._ImageOps__loadFileCallback)
        self.declareOperation("savefile", self._ImageOps__saveFileCallback)
        self.declareOperation("savefileas", self._ImageOps__saveFileAsCallback)
        self.declareOperation("interpolate", self._ImageOps__interpolateCallback)
        self.declareOperation("resizesaveas", self._ImageOps__resizeSaveAsCallback)
        self.declareOperation("comment", self._ImageOps__commentCallback)
        self.declareOperation("repeat", self._ImageOps__repeatCallback)
        self.declareOperation("addtime", self._ImageOps__addTimeCallback)
        self.declareOperation("drawtext", self._ImageOps__drawTextCallback)
        self.declareOperation("drawrotatedtext", self._ImageOps__drawRotatedTextCallback)
        self.declareOperation("system", self._ImageOps__systemCallback)

    #
    # Image processing methods.
    #
    def addFile(self, filename):
        self.addOperation("addfile", filename)

    def loadFile(self, filename):
        self.addOperation("loadfile", filename)

    def saveFile(self):
        self.addOperation("savefile", 0)

    def saveFileAs(self, filename):
        self.addOperation("savefileas", filename)

    def interpolateFrames(self, file1, file2, nframes):
        self.addOperation("interpolate", (file1, file2, nframes))

    def resizeSaveAs(self, filename, xres, yres):
        self.addOperation("resizesaveas", (filename, xres, yres))

    def repeatFrame(self, ntimes):
        self.addOperation("repeat", ntimes)

    def comment(self, commentStr):
        self.addOperation("comment", commentStr)

    def addTimeAnnotation(self, dtime, units):
        self.addOperation("addtime", (dtime, units))

    def drawText(self, x0, y0, text, scale, fillColor):
        self.addOperation("drawtext", (x0, y0, text, scale, fillColor))

    def drawRotatedText(self, x0, y0, text, scale, angle, fillColor):
        self.addOperation("drawrotatedtext", (x0, y0, text, scale, angle, fillColor))

    def system(self, command):
        self.addOperation("system", command)

    #
    # Private callbacks to do the work.
    #

    def _ImageOps__addFileCallback(self, arguments):
        self._ImageOps__loadFileCallback(self, arguments)
        self._ImageOps__saveFileCallback(self, arguments)

    def _ImageOps__loadFileCallback(self, arguments):
        #print "loadFile(", arguments, ")"
        filename = arguments
        try:
            self.im = Image.open(filename)
        except IOError:
            print "Could not open image: %s" % filename
            self.im = 0

    def _ImageOps__saveFileCallback(self, arguments):
        #print "saveFile(", arguments, ")"
        if(self.im != 0):
            try:
                name = self.getSaveName()
                print "Saving frame as %s." % name
                self.im.save(name, self.saveType)
                self.incrementFileIndex()
            except IOError:
                print "Could not save the image!"
        else:
            print "Can't save image because no image was loaded."

    def _ImageOps__saveFileAsCallback(self, arguments):
        #print "saveFileAs(", arguments, ")"
        if(self.im != 0):
            try:
                name = arguments
                print "Saving frame as %s." % name
                saveasType = movietools.getTypeFromExtension(name)
                self.im.save(name, saveasType)
            except IOError:
                print "Could not save the image!"
        else:
            print "Can't save image because no image was loaded."

    def _ImageOps__interpolateCallback(self, arguments):
        #print "interpolate(", arguments, ")"
        f1 = arguments[0]
        f2 = arguments[1]
        nframes = arguments[2]
        print "Interpolating %s and %s with %d steps" % (f1, f2, nframes)
        if(movietools.interpolateFrames(f1, f2, self.baseFormat, nframes, self.fileIndex) == 1):
            self.fileIndex = self.fileIndex + nframes

    def _ImageOps__resizeSaveAsCallback(self, arguments):
        #print "resizeSaveAs(", arguments, ")"
        if(self.im != 0):
            outputname = arguments[0]
            xres = arguments[1]
            yres = arguments[2]
            movietools.resizeImage(self.im, outputname, xres, yres)
        
    def _ImageOps__repeatCallback(self, arguments):
        #print "repeat(", arguments, ")"
        if(self.im != 0):
            try:
                ntimes = arguments
                for n in range(0,ntimes):
                    name = self.getSaveName()
                    print "Saving frame as %s." % name
                    self.im.save(name, self.saveType)
                    self.incrementFileIndex()
            except IOError:
                print "Could not save the image!"
        else:
            print "Can't repeat image because no image was loaded."

    def _ImageOps__addTimeCallback(self, arguments):
        #print "addtime(", arguments, ")"
        if(self.im != 0):
            t = arguments[0]
            units = arguments[1]
            print "Adding time annotation."
            movietools.drawTimeAnnotation(self.im, t, units)
        else:
            print "Can't add time annotation because no image was loaded."

    def _ImageOps__drawTextCallback(self, arguments):
        if(self.im != 0):
            x0 = arguments[0]
            y0 = arguments[1]
            text = arguments[2]
            scale = arguments[3]
            fillColor = arguments[4]
            arial.drawText(self.im, x0, y0, text, scale, fillColor)
        else:
            print "Can't draw text because no image is loaded."

    def _ImageOps__drawRotatedTextCallback(self, arguments):
        if(self.im != 0):
            x0 = arguments[0]
            y0 = arguments[1]
            text = argumments[2]
            scale = arguments[3]
            angle = arguments[4]
            fillColor = arguments[5]
            arial.drawRotatedText(self.im, x0, y0, text, scale, angle, fillColor)
        else:
            print "Can't draw rotated text because no image is loaded."

    def _ImageOps__commentCallback(self, arguments):
        print arguments

    def _ImageOps__systemCallback(self, arguments):
        #print "system(", arguments, ")"
        os.system(arguments)

#
# Example of how to use the class.
#
def test_module():
    # Test it out.
    a = ImageOps()
    a.loadFile("visit0000.tif")
    a.drawText(0.5, 0.5, "TEXT!!!", 0.12, (255,255,255))
    a.saveFileAs("visit0.png")
    a.loadFile("visit0001.tif")
    a.drawText(0.5, 0.5, "TEXT!!!", 0.12, (255,255,0))
    a.saveFileAs("visit1.png")
    a.loadFile("visit0002.tif")
    a.drawText(0.5, 0.5, "TEXT!!!", 0.12, (255,0,0))
    a.saveFileAs("visit2.png")
    a.loadFile("visit0.png")
    a.repeatFrame(5)
    # Fade in the text
    a.interpolateFrames("visit0000.tif", "visit0.png", 5)
    # Fade in the mesh plot
    a.interpolateFrames("visit0.png", "visit1.png", 10)
    # Fade out the air material
    a.interpolateFrames("visit1.png", "visit2.png", 10)
    a.system("ls")
    a.backup("imageops.dat")

    b = ImageOps()
    b.restore("imageops.dat")
    b.showOperations()
    b.process("frame%04d.tif", 0)
