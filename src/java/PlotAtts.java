
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
// ****************************************************************************

public class PlotAtts extends RunViewer
{
    public PlotAtts()
    {
        super();
    }

    protected void work(String[] args)
    {
        if(viewer.OpenDatabase("localhost:/usr/gapps/visit/data/globe.silo"))
        {
            // Create a plot.
            viewer.AddPlot("Pseudocolor", "u");

            // Set the pseudocolor attributes
            PseudocolorAttributes p = (PseudocolorAttributes)viewer.GetPlotAttributes("Pseudocolor");
            p.SetOpacity(0.3);
            p.Notify();
            viewer.SetPlotOptions("Pseudocolor");

            // Draw the plot
            viewer.DrawPlots();

            // Set the view
            View3DAttributes v = viewer.GetView3D();
            v.SetCamera(0.456808, 0.335583, 0.823839);
            v.SetFocus(-0.927295, -1.22113, 1.01159);
            v.SetViewUp(-0.184554, 0.941716, -0.281266);
            v.SetParallelScale(15.7041);
            v.SetNearPlane(-34.641);
            v.SetFarPlane(34.641);
            v.Notify();
            viewer.SetView3D();
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
