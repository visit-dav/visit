// ****************************************************************************
//
// Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
// Produced at the Lawrence Livermore National Laboratory
// LLNL-CODE-400124
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

import llnl.visit.Axes3D;
import llnl.visit.ViewerProxy;
import llnl.visit.ColorAttribute;
import llnl.visit.AnnotationAttributes;

// ****************************************************************************
// Class: RunViewer
//
// Purpose:
//   This class implements an example program that shows how to use the 
//   ViewerProxy class and control VisIt's viewer from Java.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:47:31 PDT 2002
//
// Modifications:
//   Brad Whitlock, Fri Nov 22 12:33:00 PDT 2002
//   Updated because of changes to AnnotationAttributes.
//
//   Brad Whitlock, Thu Dec 12 10:43:50 PDT 2002
//   Updated because of changes to color tables.
//
//   Brad Whitlock, Thu Mar 20 10:53:39 PDT 2003
//   I made it use port 5600.
//
//   Brad Whitlock, Mon Jun 6 10:18:10 PDT 2005
//   I added a little code to reduce CPU usage. I also made it use
//   GetDataPath to locate the data.
//
//   Brad Whitlock, Thu Jul 26 15:44:08 PST 2007
//   Added support for -dv instead of -vob.
//
//   Brad Whitlock, Mon Feb 25 11:07:24 PDT 2008
//   Changed to new ViewerProxy interface.
//
// ****************************************************************************

public class RunViewer
{
    public RunViewer()
    {
        viewer = new ViewerProxy();
    }

    public void run(String[] args)
    {
        // Pass command line options to the viewer viewer
        boolean stay = false;
        boolean sync = true;
        boolean verbose = false;

        for(int i = 0; i < args.length; ++i)
        {
            if(args[i].equals("-stay"))
                stay = true;
            else if(args[i].equals("-dv"))
                viewer.SetBinPath("../bin");
            else if(args[i].equals("-sync"))
                sync = true;
            else if(args[i].equals("-async"))
                sync = false;
            else if(args[i].equals("-verbose"))
                verbose = true;
            else if(args[i].equals("-quiet"))
                verbose = false;
            else if(args[i].equals("-path") && ((i + 1) < args.length))
            {
                viewer.SetBinPath(args[i + 1]);
                ++i;
            }
            else if(args[i].equals("-help"))
            {
                printUsage();
                return;
            }
            else
                viewer.AddArgument(args[i]);
        }

        // Set the viewer proxy's verbose flag.
        viewer.SetVerbose(verbose);

        // Try and open the viewer using the viewer proxy.
        if(viewer.Create(5600))
        {
            System.out.println("ViewerProxy opened the viewer.");

            // Set some viewer properties based on command line args.
            viewer.SetSynchronous(sync);

            // Show the windows.
            viewer.GetViewerMethods().ShowAllWindows();

            work(args);

            // If we have the -stay argument on the command line, keep the
            // viewer around so we can do stuff with it.
            while(stay)
            {
                try
                {
                    // Sleep a little so we don't hog the CPU.
                    Thread.currentThread().sleep(200);
                }
                catch(java.lang.InterruptedException e)
                {
                    stay = false;
                }
            }

            viewer.Close();
        }
        else
            System.out.println("ViewerProxy could not open the viewer.");
    }

    protected void printUsage()
    {
        System.out.println("Options:");
        System.out.println("    -stay      Keeps the viewer around after it is done processing commands.");
        System.out.println("    -vob       Runs the viewer located in /data_vobs/VisIt/bin.");
        System.out.println("    -sync      Runs the viewer in synchronous mode. This is the default.");
        System.out.println("    -async     Runs the viewer in asynchronous mode.");
        System.out.println("    -verbose   Prints information to the console.");
        System.out.println("    -quiet     Prevents information from being printed to the console.");
        System.out.println("    -path dir  Sets the directory that is searched for the visit script.");
        System.out.println("    -help      Displays options and exits program.");
    }

    protected void work(String[] args)
    {
        // Do a plot
        if(viewer.GetViewerMethods().OpenDatabase(viewer.GetDataPath() + "globe.silo"))
        {
            viewer.GetViewerMethods().AddPlot("Pseudocolor", "u");
            viewer.GetViewerMethods().AddPlot("Mesh", "mesh1");
            viewer.GetViewerMethods().DrawPlots();
            viewer.GetViewerMethods().SaveWindow();

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

            // Change the active color table
            viewer.GetViewerMethods().SetActiveContinuousColorTable("rainbow");

            viewer.GetViewerMethods().SetActivePlot(0);
            viewer.GetViewerMethods().ChangeActivePlotsVar("v");
            viewer.GetViewerMethods().SaveWindow();

            viewer.GetViewerMethods().ChangeActivePlotsVar("t");
            viewer.GetViewerMethods().SaveWindow();
        }
        else
        {
            System.out.println("Could not open the database!");
        }
    }

    public static void main(String args[])
    {
        RunViewer r = new RunViewer();
        r.run(args);
    }

    protected ViewerProxy viewer;
}
