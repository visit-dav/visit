// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

import java.util.Vector;
import llnl.visit.AttributeSubject;
import llnl.visit.View3DAttributes;

import llnl.visit.plots.PseudocolorAttributes;
import llnl.visit.operators.ThresholdAttributes;

// ****************************************************************************
// Class: TryThreshold
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

public class TryThreshold extends RunViewer
{
    public TryThreshold()
    {
        super();
    }

    protected void work(String[] args)
    {
        // Try and open a database
        if(viewer.GetViewerMethods().OpenDatabase(viewer.GetDataPath() + "globe.silo"))
        {
            viewer.GetViewerMethods().AddPlot("Pseudocolor", "u");
            viewer.GetViewerMethods().AddOperator("Threshold");

            // Set the pseudocolor attributes
            ThresholdAttributes t = (ThresholdAttributes)viewer.GetOperatorAttributes("Threshold");
            t.SetOutputMeshType(0);
            Vector zp = new Vector();
            zp.add(1);
            t.SetZonePortions(zp);
            Vector lb = new Vector();
            lb.add(-4.0);
            t.SetLowerBounds(lb);
            Vector ub = new Vector();
            ub.add(4.0);
            t.SetUpperBounds(ub);
            t.Notify();
            viewer.GetViewerMethods().SetOperatorOptions("Threshold");
            viewer.GetViewerMethods().DrawPlots();

            // Set the view
            View3DAttributes v = viewer.GetViewerState().GetView3DAttributes();
            v.SetViewNormal(-0.528889, 0.367702, 0.7649);
            v.SetViewUp(0.176641, 0.929226, -0.324558);
            v.SetParallelScale(17.3205);
            v.SetPerspective(true);
            v.Notify();
            viewer.GetViewerMethods().SetView3D();

            viewer.GetViewerMethods().SaveWindow();

            // Change zone inclusion criteria
            zp.set(0, 0);
            t.SetZonePortions(zp);
            t.Notify();
            viewer.GetViewerMethods().SetOperatorOptions("Threshold");
            viewer.GetViewerMethods().SaveWindow();

            //  Threshold by a variable different than the PC coloring variable.

            zp.set(0, 1);
            t.SetZonePortions(zp);
            lb.set(0, 140.0);
            t.SetLowerBounds(lb);
            ub.set(0, 340.0);
            t.SetUpperBounds(ub);
            Vector vn = new Vector();
            vn.add("t");
            t.SetListedVarNames(vn);
            t.Notify();
            viewer.GetViewerMethods().SetOperatorOptions("Threshold");
            viewer.GetViewerMethods().SaveWindow();

        }
        else
            System.out.println("Could not open the database!");
    }


    public static void main(String args[])
    {
        TryThreshold r = new TryThreshold();
        r.run(args);
    }
}
