import Image, os, arial

###############################################################################
# Function: getTypeFromExtension
#
# Purpose:    This function determines the file type from the file extension.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def getTypeFromExtension(filename):
    extension = filename[-3:]
    toType = "TIFF"
    if(extension == "tif"):
        toType = "TIFF"
    elif(extension == "png"):
        toType = "PNG"
    elif(extension == "ppm"):
        toType = "PPM"
    return toType

###############################################################################
# Function: commandInPath
#
# Purpose:    This function determines if a command is in the user's path.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def commandInPath(command):
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
    return noFail1 and noFail2

###############################################################################
# Function: getBasenameFromBaseFormat
#
# Purpose:    This function determines the basename from the base format string.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def getBasenameFromBaseFormat(baseFormat):
    basename = ""
    for i in range(len(baseFormat)):
        if(baseFormat[i] != '%'):
            basename = basename + baseFormat[i]
        else:
            break
    return basename

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
# Function: resizeImage
#
# Purpose:    This function resaves an open image to a new file with the
#             specified image resolution.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def resizeImage(im, outputname, xres, yres):
    try:
        saveFormat = getTypeFromExtension(outputname)
        # If the image is not changing size, save it out in the new format.
        if(im.size[0] == xres and im.size[1] == yres):
            im.save(outputname, saveFormat)
        # Otherwise, if the image is square, so a simple resize.
        elif(xres == yres):
            try:
                newImage = im.resize((xres, yres), Image.ANTIALIAS)
            except ValueError:
                newImage = im.resize((xres, yres), Image.BICUBIC)
            newImage.save(outputname, saveFormat)
        # Finally, create a non-square, letterboxed image.
        else:
            # Create a letter-boxed image.
            minsize = xres
            if(yres < minsize):
                minsize = yres
            try:
                pasteImage = im.resize((minsize, minsize), Image.ANTIALIAS)
            except ValueError:
                pasteImage = im.resize((minsize, minsize), Image.BICUBIC)
            newImage = Image.new("RGBA", (xres, yres), (0,0,0))
            px = (xres - minsize) / 2
            py = (yres - minsize) / 2
            newImage.paste(pasteImage, (px, py, px+pasteImage.size[0], py+pasteImage.size[1]))
            newImage.save(outputname, saveFormat)
        print "Saved", outputname
    except IOError:
        print "Could not process frame %s. We could not write the new file." % outputname

###############################################################################
# Function: resizeFramesHelper
#
# Purpose:    This function reads in input frames and writes out smaller
#             version(s) of each one.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def resizeFramesHelper(threadID, start, end, conversionargs):
    print "resizeFramesHelper(%d, %d" % (start, end), conversionargs,")"

    baseFormat = conversionargs[0]
    for i in range(start, end):
        inputname = baseFormat % i
        try:
            originalImage = Image.open(inputname)
            for args in conversionargs[1:][0]:
                smallbaseFormat = args[0]
                xres = args[1]
                yres = args[2]
                outputname = smallbaseFormat % i
                # Resize the image
                resizeImage(originalImage, outputname, xres, yres)
        except IOError:
            print "Could not process frame %d because of the input file." % i

###############################################################################
# Function: convertFramesHelper
#
# Purpose:    This function reads in a frame and saves it out in another format.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def convertFramesHelper(threadID, start, end, conversionargs):
    # Get the arguments out of the tuple.
    print "convertFramesHelper(%d, %d," % (start, end), conversionargs,")"
    fromNameFormat = conversionargs[0]
    toNameFormat = conversionargs[1]
    toType = getTypeFromExtension(toNameFormat)

    # Save the images in their new format.
    for i in range(start, end):
        im1name = fromNameFormat % i
        im2name = toNameFormat % i
        try:
            im1 = Image.open(im1name)
            im1.save(im2name, toType)
        except IOError:
            print "Could not open the source image %s" % im1name

###############################################################################
# Function: removeFilesHelper
#
# Purpose:    This function removes files from the disk.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def removeFilesHelper(threadID, start, end, conversionargs):
    # Get the arguments out of the tuple.
    print "removeFilesHelper(%d, %d," % (start, end), conversionargs,")"

    # Create the list of files to remove.
    format = conversionargs
    command = "rm "
    for i in range(start, end):
        fileName = format % i
        command = command + " " + fileName
    # Remove the files.
    print command
    os.system(command)

def resizeMultipleFrames(baseFormat, nframes, conversionargs):
    args = (baseFormat, conversionargs)
    applyFunctionToFrames(resizeFramesHelper, nframes, args)

def resizeFrames(smallbaseFormat, baseFormat, nframes, xres, yres):
    resizeMultipleFrames(baseFormat, nframes, (smallbaseFormat, xres, yres))

def convertFrames(fromFormat, toFormat, nframes):
    conversionargs = (fromFormat, toFormat)
    applyFunctionToFrames(convertFramesHelper, nframes, conversionargs)

def removeFiles(format, nframes):
    conversionargs = format
    applyFunctionToFrames(removeFilesHelper, nframes, conversionargs)

###############################################################################
# Function: createStreamingMovie
#
# Purpose:    This function creates a streaming movie from all of the frames.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
# Modifications:
#   Brad Whitlock, Mon Feb 3 07:17:18 PDT 2003
#   I fixed a bug that prevented it from working with tiff images.
#
##############################################################################

def createStreamingMovie(moviename, baseFormat, nframes):
    if(not commandInPath("img2sm")):
        print """
***
*** The createStreamingMovie command cannot create a movie because
*** the \"img2sm\" command cannot be found in your path.
***
"""
        return

    format = baseFormat[-3:]
    if(format == "tif"):
        format = "tiff"
    command = "img2sm -rle -first 0 -last %d -form %s %s %s" % (nframes-1, format, baseFormat, moviename)
    #print command
    os.system(command)

###############################################################################
# Function: createQuickTimeMovie
#
# Purpose:    This function creates a QuickTime movie from all of the frames.
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
    command = "makemovie -c qt_anim -o %s -s %d,%d -f qt -r 15 %s" % (moviename, xres, yres, names)
    #print command
    os.system(command)

def createQuickTimeMovieHelper(threadID, start, end, conversionargs):
    # Extract the arguments.
    moviename = conversionargs[0]
    baseFormat = conversionargs[1]
    xres = conversionargs[2]
    yres = conversionargs[3]
    # Create the name of the part of the movie and the names of the files to use.
    subMovieName = "%s.%d" % (moviename, threadID)
    createQuickTimeFile(subMovieName, baseFormat, start, end, xres, yres)

def createQuickTimeMovie(moviename, baseFormat, nframes, xres, yres):
    if(not commandInPath("makemovie")):
        print """
***
*** The createQuickTimeMovie function cannot create a movie because
*** because the \"makemovie\" command cannot be found in your path.
***
"""
        return

    baseType = getTypeFromExtension(baseFormat)
    if(baseType != "TIFF"):
        baseExt = baseFormat[-3:]
        # We need to convert the files to TIFF.
        newBaseFormat = "%s-%dx%d-%%04d.tif" % (moviename, xres, yres)
        convertFrames(baseFormat, newBaseFormat, nframes)
        # Call this function recursively
        createQuickTimeMovie(moviename, newBaseFormat, nframes, xres, yres)
        removeFiles(newBaseFormat, nframes)
    else:
        # Create small quicktime movies
        conversionargs = (moviename, baseFormat, xres, yres)
        framesPerMovie = 50
        applyFunctionToNFrames(createQuickTimeMovieHelper, framesPerMovie, nframes, conversionargs)
        # Glue the submovies together.
        nSubMovies = nframes / framesPerMovie
        if(nSubMovies * framesPerMovie < nframes):
            nSubMovies = nSubMovies + 1
        subMovieFormat = "%s.%%d" % moviename
        createQuickTimeFile(moviename, subMovieFormat, 0, nSubMovies, xres, yres)
        # Delete the submovies.
        removeFiles(subMovieFormat, nSubMovies)            

###############################################################################
# Function: createMPEG
#
# Purpose:    This function creates an mpeg movie from all of the frames.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def createMPEG(moviename, baseFormat, nframes, xres, yres):
    if(not commandInPath("mpeg_encode")):
        print """
***
*** The createMPEG function cannot create a movie because
*** the \"mpeg_encode\" command cannot be found in your path.
***
"""
        return

    baseType = getTypeFromExtension(baseFormat)
    base = "%s-%dx%d-" % (moviename, xres, yres)
    if(baseType != "PPM"):
        # Convert the files to PPM
        fromFormat = baseFormat
        toFormat = "%s%%04d.ppm" % base
        convertFrames(fromFormat, toFormat, nframes)
        createMPEG(moviename, toFormat, nframes, xres, yres)
        # Remove the extra files.
        command = "rm %s" % paramFile
        os.system(command)
        removeFiles(toFormat, nframes)
    else:
        paramFile = "%s-mpeg_encode-params" % moviename
        try:
            f = open(paramFile, "w")
            f.write("PATTERN             IBBPBBPBBPBBPBB\n");
            f.write("OUTPUT		    %s\n" % moviename);
            f.write("BASE_FILE_FORMAT    PPM\n");
            f.write("GOP_SIZE	     30\n");
            f.write("SLICES_PER_FRAME    1\n");
            f.write("PIXEL               HALF\n");
            f.write("RANGE               10\n");
            f.write("PSEARCH_ALG         LOGARITHMIC\n");
            f.write("BSEARCH_ALG         CROSS2\n");
            f.write("IQSCALE             8\n");
            f.write("PQSCALE             10\n");
            f.write("BQSCALE             25\n");
            f.write("REFERENCE_FRAME     ORIGINAL\n");
            f.write("FORCE_ENCODE_LAST_FRAME\n");
            f.write("YUV_SIZE	    %dx%d\n" % (xres, yres))
            f.write("INPUT_CONVERT       *\n");
            f.write("INPUT_DIR           .\n");
            f.write("INPUT\n");
            f.write("%s*.ppm  [0000-%04d]\n" % (base, nframes - 1))
            f.write("END_INPUT\n")
            f.close();

            # Create the movie
            command = "mpeg_encode %s" % paramFile
            os.system(command)
        except IOError:
            print "Could not create param file!"

###############################################################################
# Function: createTarFile
#
# Purpose:    This function creates a tar file from all of the frames.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
###############################################################################

def createTarFile(tarName, basename):
    command = "tar -cf %s %s????.???" % (tarName, basename)
    #print command
    os.system(command)

###############################################################################
# Function: interpolateFrames
#
# Purpose:    This function interpolates two images and saves out the
#             resulting images.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
##############################################################################

def interpolateFrames(file1, file2, baseFormat, nframes, startindex):
    retval = 0
    try:
        im1 = Image.open(file1)
        im2 = Image.open(file2)
        saveType = getTypeFromExtension(baseFormat)

        index = startindex
        for i in range(0, nframes):
            t = float(i) / float(nframes - 1)
            outname = baseFormat % index
            im3 = Image.blend(im1, im2, t)
            try:
                im3.save(outname, saveType)
                index = index + 1
            except IOError:
                print "Can't save", outname

        retval = 1
    except IOError:
        print "Can't open one of the input files."
    return retval

###############################################################################
# Function: drawTimeAnnotation
#
# Purpose:    This function draws a time annotation in the upper left corner.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
##############################################################################

def drawTimeAnnotation(im, t, units):
    text = "Time = %1.8g %s" % (t, units)
    textScale = 0.03
    tx = 0.005
    ty = 1. - textScale
    textColor = (0,0,0)
    arial.drawText(im, tx, ty, text, textScale, textColor)

###############################################################################
# Function: ReadDataFile
#
# Purpose:    This function reads data from a text file and returns it in
#             a tuple.
#
# Programmer: Brad Whitlock
# Date:       Wed Dec 18 11:33:22 PDT 2002
#
##############################################################################

def ReadDataFile(filename):
    data = []
    try:
        f = open(filename, "r")
        str = "start"
        while(str != ''):
            str = f.readline()
            if(str != ''):
                if(str[0] != '#'):
                    data = data + [str[:-1]]
    except IOError:
        print "Could not read from", filename
    return data

def testShrink():
    baseFormat = "frame%04d.png"
    smallbaseFormat = "framesmall%04d.png"
    smallestbaseFormat = "framesmallest%04d.png"
    nframes = 159
    conversions = ((smallbaseFormat, 1024, 768), (smallestbaseFormat, 500, 500))
    shrinkMultipleFrames(baseFormat, nframes, conversions)
