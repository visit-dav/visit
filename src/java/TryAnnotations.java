// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

import java.lang.ArrayIndexOutOfBoundsException;
import java.util.Vector;
import llnl.visit.Axes3D;
import llnl.visit.View3DAttributes;
import llnl.visit.AnnotationAttributes;
import llnl.visit.AnnotationObject;
import llnl.visit.AnnotationObjectList;
import llnl.visit.ColorAttribute;
import llnl.visit.SaveWindowAttributes;

// ****************************************************************************
// Class: TryAnnotations
//
// Purpose:
//   This example program shows how to create annotation objects and set
//   their properties.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 27 09:38:43 PDT 2008
//
// Modifications:
//   Brad Whitlock, Wed Jun 13 17:00:08 PDT 2012
//   Set some legend options.
//
// ****************************************************************************

public class TryAnnotations extends RunViewer
{
    public TryAnnotations()
    {
        super();
    }

    protected void SetCustomDefaultAnnotations()
    {
        // Change some annotation attributes.
        AnnotationAttributes a = viewer.GetViewerState().GetAnnotationAttributes();
        a.SetBackgroundMode(AnnotationAttributes.BACKGROUNDMODE_GRADIENT);
        a.SetGradientBackgroundStyle(AnnotationAttributes.GRADIENTSTYLE_RADIAL);
        a.SetGradientColor1(new ColorAttribute(0,0,255));
        a.SetGradientColor2(new ColorAttribute(0,0,0));
        a.SetForegroundColor(new ColorAttribute(255,255,255));
        Axes3D a3d = new Axes3D(a.GetAxes3D());
        a3d.SetAxesType(Axes3D.AXES_STATICEDGES);
        a3d.SetVisible(true);
        a.SetAxes3D(a3d);
        a.Notify();
        viewer.GetViewerMethods().SetAnnotationAttributes();
    }

    protected void work(String[] args)
    {
        // Try and open a database
        if(viewer.GetViewerMethods().OpenDatabase(viewer.GetDataPath() + "wave*.silo database"))
        {
            viewer.GetViewerMethods().AddPlot("Pseudocolor", "pressure");
            viewer.GetViewerMethods().AddPlot("Mesh", "quadmesh");
            viewer.GetViewerMethods().DrawPlots();

            // Set a 3D view.
            View3DAttributes v = viewer.GetViewerState().GetView3DAttributes();
            v.SetViewNormal(-0.705386, 0.57035, 0.42087);
            v.SetFocus(5, 0.353448, 2.5);
            v.SetViewUp(0.49514, 0.821357, -0.283213);
            v.SetViewAngle(30.);
            v.SetParallelScale(5.6009);
            v.SetNearPlane(-11.2018);
            v.SetFarPlane(11.2018);
            v.SetImagePan(0.0300266, 0.0519825);
            v.SetImageZoom(1.10796);
            v.SetPerspective(true);
            v.SetEyeAngle(2.);
            v.SetCenterOfRotationSet(false);
            v.SetCenterOfRotation(5, 0.353448, 2.5);
            v.Notify();
            viewer.GetViewerMethods().SetView3D();

            SetCustomDefaultAnnotations();

            AnnotationObjectList aol = viewer.GetViewerState().GetAnnotationObjectList();

            //
            // Set up a time slider annotation.
            //
            viewer.GetViewerMethods().AddAnnotationObject(AnnotationObject.ANNOTATIONTYPE_TIMESLIDER, "timeSlider");
            aol.SetTimeSliderOptions("timeSlider",
                0.675, 0.01, 0.3, 0.1,
                "Wave time = $time", "%1.3f",
                new ColorAttribute(255,0,0,255), new ColorAttribute(255,255,0,255),
                new ColorAttribute(0,255,0,255), false,
                0,
                true, false, false);
            aol.Notify();
            viewer.GetViewerMethods().SetAnnotationObjectOptions();
            // Advance through time to test the time slider.
            viewer.GetViewerMethods().SetTimeSliderState(30);

            System.out.println("After time slider: " + viewer.GetViewerState().GetAnnotationObjectList().toString());

            //
            // Create a 2D text annotation
            //
            viewer.GetViewerMethods().AddAnnotationObject(AnnotationObject.ANNOTATIONTYPE_TEXT2D, "text");
            aol.SetText2DOptions("text", 0.4, 0.95, 0.05,
                "Wave simulation",
                new ColorAttribute(255,0,0,255), false, 
                2, true, true, true, true);
            aol.Notify();
            viewer.GetViewerMethods().SetAnnotationObjectOptions();
            SaveImage("text2Dannot.png", 300, 300);
            System.out.println("After 2D text: " + viewer.GetViewerState().GetAnnotationObjectList().toString());

            //
            // Create a 3D text annotation
            //
            viewer.GetViewerMethods().AddAnnotationObject(AnnotationObject.ANNOTATIONTYPE_TEXT3D, "text3D");
            aol.SetText3DOptions("text3D",
                0., 2.5, 1.5, 
                "Wave simulation, 3D text",
                true, 0., 3,
                true,
                15., 0., 0.,
                new ColorAttribute(255,255,0,255), false, 
                true);
            aol.Notify();
            viewer.GetViewerMethods().SetAnnotationObjectOptions();
            SaveImage("text3D.png", 300, 300);
            System.out.println("After 3D text: " + viewer.GetViewerState().GetAnnotationObjectList().toString());

            // Create a line 2D/arrow annotation
            //
            viewer.GetViewerMethods().AddAnnotationObject(AnnotationObject.ANNOTATIONTYPE_LINE2D, "line");
            aol.SetLine2DOptions("line",
                0.5, 0.9495, 0.5, 0.6,
                2,
                0,
                2,
                new ColorAttribute(255,0,0,255), false, 
                true);
            aol.Notify();
            viewer.GetViewerMethods().SetAnnotationObjectOptions();
            SaveImage("line2Dannot.png", 300, 300);
            System.out.println("After line: " + viewer.GetViewerState().GetAnnotationObjectList().toString());

            //
            // Create a 3D line/arrow annotation
            //
            viewer.GetViewerMethods().AddAnnotationObject(AnnotationObject.ANNOTATIONTYPE_LINE3D, "line3d");
            aol.SetLine3DOptions("line3d",
                6.0, 0.0, 0.0, // startpoint
                6.0, 3.0, 0.0, // endpoint
                1,        // lineWidth
                1,        // lineType 1=tube
                1,        //tubeQuality,
                0.04,     //tubeRadius,
                false,    // arrow1
                16,       // arrow1Resolution
                0.120831, // arrow1Radius
                0.338327, // arrow1Height
                true,    // arrow2
                16,       // arrow2Resolution
                0.120831, // arrow2Radius
                0.338327, // arrow2Height
                new ColorAttribute(255,153,0,255), false, 
                true);
            aol.Notify();
            viewer.GetViewerMethods().SetAnnotationObjectOptions();
            System.out.println("After 3D line: " + viewer.GetViewerState().GetAnnotationObjectList().toString());
            SaveImage("line3Dannot.png", 300, 300);

            //
            // Save a small image to use for an annotation.
            //
            AnnotationAttributes annot = viewer.GetViewerState().GetAnnotationAttributes();
            ColorAttribute transColor = new ColorAttribute(50,0,100,255);
            annot.SetBackgroundColor(transColor);
            annot.SetBackgroundMode(annot.BACKGROUNDMODE_SOLID);
            annot.Notify();
            viewer.GetViewerMethods().SetAnnotationAttributes();
            SaveImage("imageannot.png", 300, 300);
            annot.SetBackgroundColor(new ColorAttribute(0,0,0,255));
            annot.Notify();
            viewer.GetViewerMethods().SetAnnotationAttributes();

            viewer.GetViewerMethods().AddAnnotationObject(AnnotationObject.ANNOTATIONTYPE_IMAGE, "image");
            aol.SetImageOptions("image",
                "imageannot.png",
                0.02, 0.63,
                1., 1., true,
                transColor, true,
                1.,
                true);
            aol.Notify();
            viewer.GetViewerMethods().SetAnnotationObjectOptions();
            System.out.println("After image: " + viewer.GetViewerState().GetAnnotationObjectList().toString());


            // Set some legend attributes. You'd get the name from the PlotList object
            // but here we're just hard-coding the plot name since the Pseudocolor is
            // called Plot0000.
            SetCustomDefaultAnnotations();
            aol.SetLegendOptions("Plot0000",
                false, // managePosition,
                0.2, 0.1, // x,y
                1.5, 0.5, // scaleX, scaleY
                7, // numTicks
                true, // drawBox
                false, // drawLabels,
                true,  // horizontalLegend,
                true, // alternateText, (false=normal text position, true=opposite position)
                false, // drawTitle,
                false, // drawMinMax,
                true, // controlTicks,
                true, // minMaxInclusive,
                true, // drawValues
                0.03, // fontheight
                new ColorAttribute(100,255,100), false,
                2, true, true, true,
                true);
            aol.Notify();
            viewer.GetViewerMethods().SetAnnotationObjectOptions();
            SaveImage("legendChange.png", 300, 300);
        }
        else
            System.out.println("Could not open the database!");
    }

    private void SaveImage(String filename, int xres, int yres)
    {
        viewer.GetViewerMethods().SetAnnotationAttributes();
        SaveWindowAttributes saveAtts = viewer.GetViewerState().GetSaveWindowAttributes();
        saveAtts.SetFileName(filename);
        saveAtts.SetWidth(xres);
        saveAtts.SetHeight(yres);
        saveAtts.SetFamily(false);
        saveAtts.SetFormat(saveAtts.FILEFORMAT_PNG);
        saveAtts.SetResConstraint(saveAtts.RESCONSTRAINT_NOCONSTRAINT);
        saveAtts.Notify();
        viewer.GetViewerMethods().SaveWindow();
    }

    public static void main(String args[])
    {
        TryAnnotations r = new TryAnnotations();
        r.run(args);
    }
}
