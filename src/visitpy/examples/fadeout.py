# ----------------------------------------------------------------------------
#  Test script that changes animates an isosurface by changing its iso-value.
#
#  Modifications:
#    Hank Childs, Tue May  7 09:27:44 PDT 2002
#    Updated for 0.9.9.  Made it use a file in the /data directory.
#    Also converted bulk of routine to a function and put in a cast that
#    avoids an unusual crash on the SGIs (0 / 2. -> core, 0. / 2. -> okay).
#    Updated clipping planes to not cut off the dataset.
#
#    Brad Whitlock, Tue Apr 1 09:35:23 PDT 2003
#    Updated for 1.1.3.
#
#    Eric Brugger, Fri Aug 29 16:26:53 PDT 2003
#    I modified the script to use the new interface for setting views.
#
# ----------------------------------------------------------------------------

import os

# Check the version.
ScriptVersion = "1.2.1"
if(Version() != ScriptVersion):
    print "This script is for VisIt %s. It may not work with version %s" % (ScriptVersion, Version())

def fadeout():
    OpenDatabase("localhost:../../data/globe.silo")
    AddPlot("Pseudocolor", "u")
    DisableRedraw()
    DrawPlots()
    
    annot = AnnotationAttributes()
    annot.gradientColor1 = (0, 255, 255)
    annot.gradientColor2 = (0, 0, 150)
    annot.gradientBackgroundStyle = annot.Radial
    annot.backgroundMode = annot.Gradient
    annot.foregroundColor = (255, 255, 255)
    SetAnnotationAttributes(annot)
    startGradient = [0, 255, 255, 0, 0, 150]
    endGradient = [0, 0, 0, 255, 0, 0]
    gc = [0, 255, 255, 0, 0, 150]
    
    # Create camera 1 and set its attributes.
    a = View3DAttributes()
    a.SetViewNormal(-0.383063, 0.307038, 0.871201)
    a.SetFocus(-0.863862, -0.03381, -0.36792)
    a.SetViewUp(0.0888824, 0.951017, -0.296086)
    a.SetViewAngle(30)
    a.SetParallelScale(17.3205)
    a.SetNearPlane(-40)
    a.SetFarPlane(40.)
    a.SetPerspective(1)
    # Set the view and redraw the window.
    SetView3D(a) 
    RedrawWindow()
    
    # Create camera 2 and set its attributes.
    b = View3DAttributes()
    b.SetViewNormal(-0.67373, 0.661551, -0.329301)
    b.SetFocus(-0.867492, -0.0726593, -0.355825)
    b.SetViewUp(0.653625, 0.741375, 0.152113)
    b.SetViewAngle(30)
    b.SetParallelScale(17.3205)
    b.SetNearPlane(-40.)
    b.SetFarPlane(40.)
    b.SetPerspective(1)
    
    # Create camera 3 and set its attributes.
    c = View3DAttributes()
    c.SetViewNormal(-0.586145, 0.500894, -0.636819)
    c.SetFocus(-0.867492, -0.0726593, -0.355825)
    c.SetViewUp(0.468759, 0.850743, 0.2377)
    c.SetViewAngle(30)
    c.SetParallelScale(20.)
    c.SetNearPlane(-40.)
    c.SetFarPlane(40.)
    c.SetPerspective(1)
    
    # Create camera 4 and set its attributes.
    d = View3DAttributes()
    d.SetViewNormal(0.298261, -0.196029, -0.934138)
    d.SetFocus(-0.867492, -0.0726593, -0.355825)
    d.SetViewUp(0.247141, 0.961168, -0.122791)
    d.SetViewAngle(30)
    d.SetParallelScale(7.84054)
    d.SetNearPlane(-40.)
    d.SetFarPlane(40.)
    d.SetPerspective(1)
    
    # Create some pc attributes.
    pc = PseudocolorAttributes()
    vecPlotCreated = 0
    
    # Do cubic Bezier interpolation between all four cameras.
    #nsteps = 100
    for i in range(50):
        t = float(i) / (50. - 1.)
        print "t = %g" % t
        t2 = t * t
        t3 = t * t2
        omt = 1. - t
        omt2 = omt * omt
        omt3 = omt2 * omt
        c0 = a * omt3
        c1 = b * 3. * t * omt2
        c2 = c * 3. * t2 * omt
        c3 = d * t3
        camera = (c0 + c1) + (c2 + c3)
        # Disable redraws
        DisableRedraw()
        # Set the view
        SetView3D(camera)
    
        # Set the gradient background color
        for j in range(6):
            gc[j] = int(omt*float(startGradient[j]) + t * float(endGradient[j]))
        annot.gradientColor1 = (gc[0], gc[1], gc[2])
        annot.gradientColor2 = (gc[3], gc[4], gc[5])
        SetAnnotationAttributes(annot)
    
        # Figure out what the pc plot's opacity should be.
        startVal = .666
        if(t > startVal):
            opacity = (1. - ((t - startVal) / (1. - startVal)))
            if(opacity < 0.):
                opacity = 0.
            pc.opacity = opacity
            SetActivePlots(0)
            SetPlotOptions(pc)
            # If t > 0.666 then create a pc plot.
            if(vecPlotCreated == 0):
                print "Creating vector plot"
                vec = VectorAttributes()
                vec.colorByMag = 1
                SetDefaultPlotOptions(vec)
                AddPlot("Vector", "vel")
                DrawPlots()
                vecPlotCreated = 1
    
        # Redraw the window
        RedrawWindow()
    
    # Zoom out a bit
    startScale = camera.parallelScale
    endScale = 2. * startScale
    for i in range(15):
        t = float(i) / 14.
        scale = (1. - t) * startScale + t * endScale
        camera.parallelScale = scale
        SetView3D(camera)
    

if(not os.path.isfile("../../data/globe.silo")):
    print "This script requires the file globe.silo to be built in the data directory"
else:
    fadeout()

