// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.


import llnl.visit.View3DAttributes;
import llnl.visit.plots.PseudocolorAttributes;

// ****************************************************************************
// Class: PlotAtts
//
// Purpose:
//   This is an example program that shows how to set plot attributes.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 15 16:09:03 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Sep 24 08:05:51 PDT 2002
//   I changed it so the view is set after the plot is drawn.
//
//   Eric Brugger, Wed Aug 27 09:04:55 PDT 2003
//   I modified it to use the new view interface.
//
//   Brad Whitlock, Mon Jun 6 17:25:34 PST 2005
//   I made it use GetDataPath to locate the data.
//
//   Brad Whitlock, Thu Jul 14 12:15:42 PDT 2005
//   Updated.
//
//   Brad Whitlock, Mon Feb 25 11:07:24 PDT 2008
//   Changed to new ViewerProxy interface.
//
// ****************************************************************************

public class PlotAtts extends RunViewer
{
    public PlotAtts()
    {
        super();
    }

    protected void work(String[] args)
    {
        if(viewer.GetViewerMethods().OpenDatabase(viewer.GetDataPath() + "globe.silo"))
        {
            // Create a plot.
            viewer.GetViewerMethods().AddPlot("Pseudocolor", "u");

            // Set the pseudocolor attributes
            PseudocolorAttributes p = (PseudocolorAttributes)viewer.GetPlotAttributes("Pseudocolor");
            // set Pseudocolor's opacity type to constant
            p.SetOpacityType(p.OPACITYTYPE_CONSTANT);
            p.SetOpacity(0.3);
            p.Notify();
            viewer.GetViewerMethods().SetPlotOptions("Pseudocolor");

            // Draw the plot
            viewer.GetViewerMethods().DrawPlots();

            // Set the view
            View3DAttributes v = viewer.GetViewerState().GetView3DAttributes();
            v.SetViewNormal(0.456808, 0.335583, 0.823839);
            v.SetFocus(-0.927295, -1.22113, 1.01159);
            v.SetViewUp(-0.184554, 0.941716, -0.281266);
            v.SetParallelScale(15.7041);
            v.SetNearPlane(-34.641);
            v.SetFarPlane(34.641);
            v.Notify();
            viewer.GetViewerMethods().SetView3D();
        }
        else
            System.out.println("Could not open the database!");
    }

    public static void main(String args[])
    {
        PlotAtts r = new PlotAtts();
        r.run(args);
    }
}
