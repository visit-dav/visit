// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

import java.util.Vector;

import llnl.visit.ColorAttribute;
import llnl.visit.plots.MeshAttributes;

// ****************************************************************************
// Class: TryPointGlyphing
//
// Purpose:
//   This example program sets up a Pseudocolor plot with threshold operator.
//
// Notes:      Based on threshold.py of test-suite.
//
// Programmer: Kathleen Biagas
// Creation:   March 31, 2017
//
// Modifications:
//
// ****************************************************************************

public class TryPointGlyphing extends RunViewer
{
    public TryPointGlyphing()
    {
        super();
    }

    protected void work(String[] args)
    {
        // Try and open a database
        if(viewer.GetViewerMethods().OpenDatabase(viewer.GetDataPath() + "noise.silo"))
        {
            viewer.GetViewerMethods().AddPlot("Mesh", "PointMesh");
            viewer.GetViewerMethods().DrawPlots();

            // Set the pseudocolor attributes
            MeshAttributes m = (MeshAttributes)viewer.GetPlotAttributes("Mesh");
            m.SetPointSize(1.0);
            m.SetPointSizePixels(5);

            m.SetMeshColorSource(1);
            m.SetMeshColor(new ColorAttribute(204,153,255,0));
            // Run through all the point types
            for (int i = 0; i < 8; ++i)
            {
                m.SetPointType(i);
                m.Notify();
                viewer.GetViewerMethods().SetPlotOptions("Mesh");

                viewer.GetViewerMethods().SaveWindow();
            }
        }
        else
            System.out.println("Could not open the database!");
    }


    public static void main(String args[])
    {
        TryPointGlyphing r = new TryPointGlyphing();
        r.run(args);
    }
}
