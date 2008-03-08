// ****************************************************************************
//
// Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
// Produced at the Lawrence Livermore National Laboratory
// LLNL-CODE-400142
// All rights reserved.
//
// This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
// full copyright notice is contained in the file COPYRIGHT located at the root
// of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
//
// Redistribution  and  use  in  source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of  source code must  retain the above  copyright notice,
//    this list of conditions and the disclaimer below.
//  - Redistributions in binary form must reproduce the above copyright notice,
//    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
//    documentation and/or other materials provided with the distribution.
//  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
//    be used to endorse or promote products derived from this software without
//    specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
// ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
// LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
// DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
// SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
// CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
// OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ****************************************************************************

import java.lang.ArrayIndexOutOfBoundsException;
import java.util.Vector;
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
//
// ****************************************************************************

public class TryAnnotations extends RunViewer
{
    public TryAnnotations()
    {
        super();
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
            aol.SetText2DOptions("text", 0.4, 0.95, 0.2,
                "Wave simulation",
                new ColorAttribute(255,0,0,255), false, 
                2, true, true, true, true);
            aol.Notify();
            viewer.GetViewerMethods().SetAnnotationObjectOptions();
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
            System.out.println("After 3D text: " + viewer.GetViewerState().GetAnnotationObjectList().toString());

            //
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
            System.out.println("After line: " + viewer.GetViewerState().GetAnnotationObjectList().toString());

            //
            // Save a small image to use for an annotation.
            //
            AnnotationAttributes annot = viewer.GetViewerState().GetAnnotationAttributes();
            ColorAttribute transColor = new ColorAttribute(50,0,100,255);
            annot.SetBackgroundColor(transColor);
            annot.SetBackgroundMode(annot.BACKGROUNDMODE_SOLID);
            annot.Notify();
            viewer.GetViewerMethods().SetAnnotationAttributes();
            SaveImage("imageannot.tif", 300, 300);
            annot.SetBackgroundColor(new ColorAttribute(0,0,0,255));
            annot.Notify();
            viewer.GetViewerMethods().SetAnnotationAttributes();

            viewer.GetViewerMethods().AddAnnotationObject(AnnotationObject.ANNOTATIONTYPE_IMAGE, "image");
            aol.SetImageOptions("image",
                "imageannot.tif",
                0.02, 0.63,
                1., 1., true,
                transColor, true,
                1.,
                true);
            aol.Notify();
            viewer.GetViewerMethods().SetAnnotationObjectOptions();
            System.out.println("After image: " + viewer.GetViewerState().GetAnnotationObjectList().toString());
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
        saveAtts.SetFormat(saveAtts.FILEFORMAT_TIFF);
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
