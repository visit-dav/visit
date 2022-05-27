// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

import java.lang.ArrayIndexOutOfBoundsException;
import java.util.Vector;
import llnl.visit.AttributeSubject;
import llnl.visit.SimpleObserver;
import llnl.visit.QueryAttributes;

// ****************************************************************************
// Class: TryQuery
//
// Purpose:
//   This example program does a plot and queries some values in it.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 1 12:51:29 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Dec 12 10:44:31 PDT 2002
//   Updated because of changse to color table methods.
//
//   Brad Whitlock, Thu Jan 2 16:05:48 PST 2003
//   Changed because of Lineout method interface change.
//
//   Brad Whitlock, Mon Jun 6 17:25:34 PST 2005
//   I made it use GetDataPath to locate the data.
//
//   Brad Whitlock, Mon Feb 25 11:07:24 PDT 2008
//   Changed to new ViewerProxy interface.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
//
// ****************************************************************************

public class TryQuery extends RunViewer implements SimpleObserver
{
    public TryQuery()
    {
        super();
        doUpdate = true;

        // Make this object observe the query and pick attributes.
        viewer.GetViewerState().GetQueryAttributes().Attach(this);
        viewer.GetViewerState().GetPickAttributes().Attach(this);
    }

    protected void work(String[] args)
    {
        // Try and open a database
        if(viewer.GetViewerMethods().OpenDatabase(viewer.GetDataPath() + "curv2d.silo"))
        {
            viewer.GetViewerMethods().AddPlot("Mesh", "curvmesh2d");
            viewer.GetViewerMethods().AddPlot("Pseudocolor", "d");
            viewer.GetViewerMethods().DrawPlots();

            // Set the colortable to one that has white at the bottom values.
            viewer.GetViewerMethods().SetDefaultContinuousColorTable("calewhite");

            // Create the variable list.
            Vector vars = new Vector();
            vars.addElement(new String("default"));

            // Do some picks.
            viewer.GetViewerMethods().Pick(300, 300, vars);
            viewer.GetViewerMethods().Pick(450, 350, vars);
            viewer.GetViewerMethods().Pick(600, 400, vars);

            // Do some lineouts.
            viewer.GetViewerMethods().Lineout(-4.01261, 1.91818, 2.52975, 3.78323, vars);
            viewer.GetViewerMethods().SetActiveWindow(1);
            viewer.GetViewerMethods().Lineout(-3.89903, 1.79309, 2.91593, 3.40794, vars);

            // Change the window layout.
            viewer.GetViewerMethods().SetWindowLayout(2);
        }
        else
            System.out.println("Could not open the database!");
    }

    public void Update(AttributeSubject s)
    {
        System.out.println(s.toString(""));
    }

    public void SetUpdate(boolean val) { doUpdate = val; }
    public boolean GetUpdate() { return doUpdate; }

    public static void main(String args[])
    {
        TryQuery r = new TryQuery();
        r.run(args);
    }

    private boolean doUpdate;
}
