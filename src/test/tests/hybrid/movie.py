# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  movie.py
#
#  Tests:      Tests various aspects of "visit -movie" movie-making.
#
#  Notes:      
#
#  Programmer: Brad Whitlock
#  Date:       Fri Sep 18 17:29:49 PDT 2015
#
#  Modifications:
#     Kathleen Biagas, Tue Sep 29 15:47:21 MST 2015
#     Use TestEnv.params["visit_bin"] for correct path to visit executable
#     on all platforms.  Use raw string for database path when writing to
#     fb_wave.py.
#
#     Kathleen Biagas, Mon Oct 23 17:41:43 MST 2017
#     Use Image lib for image conversion instead of 'convert'.
#
# ----------------------------------------------------------------------------
import os, string, subprocess, visit_utils, Image

def GenerateMovie(movieArgs):
    if TestEnv.params["parallel"]:
        args = [TestEnv.params["visit_bin"], "-movie", "-np", "2", "-l", TestEnv.params["parallel_launch"]] + movieArgs
    else:
        args = [TestEnv.params["visit_bin"], "-movie"] + movieArgs
    p = subprocess.check_output(args)
    return p

def GetFiles(prefix):
    files = os.listdir(".")
    names = []
    for f in files:
        if prefix in f:
            names = names + [f]
    names = sorted(names)
    sizes = []
    for f in names:
        s = os.stat(f)
        sizes = sizes + [s.st_size]
    return list(zip(names, sizes))

def RemoveFiles(files):
    for f in files:
        os.unlink(f)

def TestMovieFrames(testFormatString, startindex, framefiles, percents=[], label=""):
    files = []
    if len(percents) > 0:
        for p in percents:
            idx = int(p*float(len(framefiles)-1))
            files = files + [framefiles[idx][0]]
    else:
        files = [x[0] for x in framefiles]
    annot = None
    if len(label) > 0:
        annot = CreateAnnotationObject("Text2D")
        annot.position = (0.02,0.92)
        annot.height = 0.06
        annot.text = label
    testid = startindex
    for f in files:
        OpenDatabase(f)
        AddPlot("Truecolor", "color")
        DrawPlots()
        ResetView()
        v = GetView2D()
        v.viewportCoords = (0,1,0,1)
        SetView2D(v)
        testname = testFormatString % testid
        testid = testid + 1
        Test(testname)
        DeleteAllPlots()
        CloseDatabase(f)
    if annot != None:
        annot.Delete()
    return testid

def FileSubstitution(infile, outfile, replacements):
    lines = open(infile, "rt").readlines()
    out = open(outfile, "wt")
    for line in lines:
        s = line
        for token in replacements:
            if token in line:
                s = string.replace(s, token, replacements[token])
        out.write(s)
    out.close()

def test012():
    # Set up a movie script.
    f = open("fb_wave.py", "wt")
    f.write("print \"MOVIE SCRIPT EXECUTING\"\n")
    f.write("OpenDatabase(r\"%s\")\n" % silo_data_path("wave*.silo database"))
    f.write("AddPlot(\"FilledBoundary\", \"Material\")\n")
    f.write("DrawPlots()\n")
    f.write("v = GetView3D()\n")
    f.write("v.viewNormal = (-0.6041, 0.689904, 0.398868)\n")
    f.write("v.focus = (5, 0.753448, 2.5)\n")
    f.write("v.viewUp = (0.601718, 0.723053, -0.339309)\n")
    f.write("v.viewAngle = 30\n")
    f.write("v.parallelScale = 5.6398\n")
    f.write("v.nearPlane = -11.2796\n")
    f.write("v.farPlane = 11.2796\n")
    f.write("v.imagePan = (0.0453988, 0.0661949)\n")
    f.write("v.imageZoom = 1.09294\n")
    f.write("v.perspective = 1\n")
    f.write("v.eyeAngle = 2\n")
    f.write("v.centerOfRotationSet = 0\n")
    f.write("v.centerOfRotation = (5, 0.753448, 2.5)\n")
    f.write("v.axis3DScaleFlag = 0\n")
    f.write("v.axis3DScales = (1, 1, 1)\n")
    f.write("v.shear = (0, 0, 1)\n")
    f.write("v.windowValid = 1\n")
    f.write("SetView3D(v)\n")
    f.write("a = GetAnnotationAttributes()\n")
    f.write("a.axes2D.visible = 0\n")
    f.write("a.axes3D.visible = 0\n")
    f.write("a.axes3D.triadFlag = 0\n")
    f.write("a.axes3D.bboxFlag = 0\n")
    f.write("a.userInfoFlag = 0\n")
    f.write("a.databaseInfoFlag = 0\n")
    f.write("a.legendInfoFlag = 0\n")
    f.write("SetAnnotationAttributes(a)\n")
    f.write("f = FilledBoundaryAttributes(1)\n")
    f.write("f.SetMultiColor(0, (255,100,100,255))\n")
    f.write("f.SetMultiColor(1, (255,255,100,255))\n")
    f.write("f.SetMultiColor(2, (100,255,180,255))\n")
    f.write("SetPlotOptions(f)\n")
    f.write("ts = CreateAnnotationObject(\"TimeSlider\")\n")
    f.write("ts.height = 0.15\n")
    f.write("ts.width = 0.98\n")
    f.write("ts.startColor = (255,140,80,250)\n")
    f.write("ts.rounded = 0\n")
    f.write("for i in xrange(TimeSliderGetNStates()):\n")
    f.write("    SetTimeSliderState(i)\n")
    f.write("    SaveWindow()\n")
    f.close()

    WH = "%dx%d" % (TestEnv.params["width"], TestEnv.params["height"])

    ############################################################################
    # Regular movie. 1 format. Force mpeg2encode.
    ############################################################################
    def test0():
        TestSection("Script, Save MPEG")
        output = GenerateMovie(["-scriptfile", "fb_wave.py", "-output", "test_0", "-noffmpeg", "-format", "mpeg", "-geometry", WH])

        # Look at the mpeg movie file size.
        files = GetFiles("test_0")
        print(files)
        txt = ""
        for f in files:
            if f[0] == "test_0.mpg":
                if f[1] > 1100000:
                    txt = txt + "MPEG movie file size is plausible."
                else:
                    txt = txt + "MPEG movie file size is suspect."
                break
        TestText("movie_0_00", txt)

        # extract movie and let's plot some frames.
        visit_utils.encoding.extract("test_0.mpg", "frame%04d.png")
        framefiles = GetFiles("frame")
        txt = string.join([x[0] for x in framefiles], "\n")
        TestText("movie_0_01", txt)

        # Plot some of the extracted frames
        nextid = TestMovieFrames("movie_0_%02d", 2, framefiles, percents=[0., 0.25, 0.5, 0.75, 1.], label="MPEG")
        RemoveFiles([x[0] for x in framefiles])

    ############################################################################
    # Regular movie with 2 formats
    ############################################################################
    def test1():
        TestSection("Script, Save 2 formats (PNG,MPEG)")
        output = GenerateMovie(["-scriptfile", "fb_wave.py", "-output", "test_1", "-format", "png,mpeg", "-geometry", WH+",800x600"])

        # Look at the mpeg movie file size.
        files = GetFiles("test_1")
        txt = string.join([x[0] for x in files], "\n") + "\n\n"
        for f in files:
            if f[0] == "test_1.mpg":
                if f[1] > 1400000:
                    txt = txt + "MPEG movie file size is plausible."
                else:
                    txt = txt + "MPEG movie file size is suspect."
                break
        TestText("movie_1_00", txt)

        # extract movie and let's plot some frames.
        visit_utils.encoding.extract("test_1.mpg", "frame%04d.png")
        framefiles = GetFiles("frame")
        txt = string.join([x[0] for x in framefiles], "\n")
        TestText("movie_1_01", txt)

        # Plot some of the png files we made.
        pngs = []
        for f in files:
            if f[0][-4:] == ".png":
                pngs = pngs + [f]
        nextid = TestMovieFrames("movie_1_%02d", 2, pngs, percents=[0., 0.25, 0.5, 0.75, 1.], label="")
        RemoveFiles([x[0] for x in pngs])

        # Plot some of the extracted frames
        nextid = TestMovieFrames("movie_1_%02d", nextid, framefiles, percents=[0., 0.25, 0.5, 0.75, 1.], label="MPEG")
        RemoveFiles([x[0] for x in framefiles])

    ############################################################################
    # Stereo movie with 2 sizes
    ############################################################################
    def test2():
        TestSection("Script, Stereo with 2 sizes")
        output = GenerateMovie(["-scriptfile", "fb_wave.py", "-output", "test_2", "-stereo", "leftright", "-format", "png,png", "-geometry", WH+",600x600"])

        # Look at the list of files that were created.
        leftWH = GetFiles("left_test_2_"+WH)
        rightWH = GetFiles("right_test_2_"+WH)
        left600 = GetFiles("left_test_2_600x600")
        right600 = GetFiles("right_test_2_600x600")
        TestText("movie_2_00", string.join([x[0] for x in leftWH], "\n"))
        TestText("movie_2_01", string.join([x[0] for x in rightWH], "\n"))
        TestText("movie_2_02", string.join([x[0] for x in left600], "\n"))
        TestText("movie_2_03", string.join([x[0] for x in right600], "\n"))

        # Plot some of the files we made.
        nextid = TestMovieFrames("movie_2_%02d", 4, leftWH, percents=[0., 0.25, 0.5, 0.75, 1.], label="Left Eye, "+ WH)
        RemoveFiles([x[0] for x in leftWH])
        nextid = TestMovieFrames("movie_2_%02d", nextid, rightWH, percents=[0., 0.25, 0.5, 0.75, 1.], label="Right Eye, "+ WH)
        RemoveFiles([x[0] for x in rightWH])

        nextid = TestMovieFrames("movie_2_%02d", nextid, left600, percents=[0., 0.25, 0.5, 0.75, 1.], label="Left Eye, 600x600")
        RemoveFiles([x[0] for x in left600])
        nextid = TestMovieFrames("movie_2_%02d", nextid, right600, percents=[0., 0.25, 0.5, 0.75, 1.], label="Right Eye, 600x600")
        RemoveFiles([x[0] for x in right600])


    test0()
    test1()
    test2()

def test34():
    WH = "%dx%d" % (TestEnv.params["width"], TestEnv.params["height"])

    def PrepareSession(infile, outfile, replacements):
        token = "WAVE_VISIT_DATABASE"
        lines = open(infile, "rt").readlines()
        out = open(outfile, "wt")
        for line in lines:
            s = line
            if token in line:
                s = string.replace(line, token, "localhost:" + silo_data_path("wave.visit"))
            out.write(s)
        out.close()

    ############################################################################
    # Movie using session files.
    ############################################################################
    def test3():
        TestSection("Session")
        output = GenerateMovie(["-sessionfile", "movie.session", "-output", "test3_", "-format", "png", "-geometry", WH])

        # Get the frame files
        files = GetFiles("test3_")[1:]
        txt = string.join([x[0] for x in files], "\n")
        TestText("movie_3_00", txt)

        # Plot some of the frames
        nextid = TestMovieFrames("movie_3_%02d", 1, files, percents=[0., 0.25, 0.5, 0.75, 1.], label="")
        RemoveFiles([x[0] for x in files])

    ############################################################################
    # Movie using session files where we also test -start,-end,-framestep
    ############################################################################
    def test4():
        TestSection("Session, -start,-end,-framestep")
        output = GenerateMovie(["-sessionfile", "movie.session", "-output", "test4_", "-format", "png", "-geometry", WH, "-start", "10", "-end", "50", "-framestep", "5"])

        # Get the frame files
        files = GetFiles("test4_")[1:]
        print(files)
        txt = string.join([x[0] for x in files], "\n")
        TestText("movie_4_00", txt)

        # Plot some of the frames
        nextid = TestMovieFrames("movie_4_%02d", 1, files, percents=[0., 0.25, 0.5, 0.75, 1.], label="")
        RemoveFiles([x[0] for x in files])

    # Make the session file suitable for testing.
    replacements = {"WAVE_VISIT_DATABASE" : "localhost:" + silo_data_path("wave.visit")}
    infile = string.replace(TestEnv.params["script"], "movie.py", "movie.session")
    FileSubstitution(infile, "movie.session", replacements)

    test3()
    test4()

def test5():
    TestSection("Template movie")

    f = open("shortwave.visit", "wt")
    for i in range(0,71,5):
        line = silo_data_path("wave%04d.silo" % (i*10))
        print(line)
        f.write("%s\n" % line)
    f.close()

    # Make the session file suitable for testing.
    replacements = {"WAVE_VISIT_DATABASE" : "localhost:%s" % os.path.abspath("shortwave.visit")}
    infile = string.replace(TestEnv.params["script"], "movie.py", "movie5.session")
    FileSubstitution(infile, "movie5.session", replacements)

    # Make the template file suitable for testing.
    replacements = {"PATH_TO_VISITMOVIETEMPLATE_PY" : os.path.join(os.getenv("VISITHOME"), "resources", "movietemplates", "visitmovietemplate.py"),
                    "TEST_SESSION_FILE" : os.path.abspath("movie5.session")}
    infile = string.replace(TestEnv.params["script"], "movie.py", "movie5.opt")
    FileSubstitution(infile, "movie5.opt", replacements)

    output = GenerateMovie(["-templatefile", "movie5.opt", "-output", "test5_", "-format", "bmp", "-geometry", "300x300"])

    files = GetFiles("test5_")
    img = []
    for f in files:
        if f[0][-4:] == ".bmp":
            #
            # NOTE: I could not get ffmpeg to work with png output from visit-composite
            # now that it uses VTK. I also could not get VisIt to read PNG, TIFF, or 
            # BMP files made with that VTK export. The files looked okay on my Mac viewer
            # but VisIt didn't like them. Maybe they had alpha.
            #
            bmp = f[0]
            png = f[0][:-4] + ".png"
            print("convert %s %s" % (bmp, png))
            im1 = Image.open(bmp)
            im1.save(png, "png")
            img = img + [(png,0)]
    txt = string.join([x[0] for x in img], "\n")
    TestText("movie_5_00", txt)

    nextid = TestMovieFrames("movie_5_%02d", 1, img)
    RemoveFiles([x[0] for x in img])

def main():
    test012()
    test34()
    test5()

main()
Exit()
