// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

import llnl.visit.ViewerProxy;
import java.util.Vector;

// ****************************************************************************
// Class: OpenGUI
//
// Purpose:
//   This class implements an example program that shows how to use the
//   ViewerProxy class and control VisIt's viewer from Java.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 17 13:40:40 PDT 2009
//
// Modifications:
//
// ****************************************************************************

public class OpenGUI extends RunViewer
{
    public OpenGUI()
    {
        super();
    }

    protected void work(String[] args)
    {
        // Do a plot of the data.
        String db = new String("globe.silo");
        if(viewer.GetViewerMethods().OpenDatabase(viewer.GetDataPath() + db))
        {
            viewer.GetViewerMethods().AddPlot("Pseudocolor", "u");
            viewer.GetViewerMethods().AddPlot("Mesh", "mesh1");
            viewer.GetViewerMethods().DrawPlots();
        }
        else
        {
            System.out.println("Could not open the database!");
        }

        // Open the VisIt GUI.
        String clientName = new String("GUI");
        String clientProgram = new String("visit");
        Vector clientArgs = new Vector();
        clientArgs.add(new String("-gui"));
        viewer.GetViewerMethods().OpenClient(clientName, clientProgram, clientArgs);
    }

    public static void main(String args[])
    {
        OpenGUI r = new OpenGUI();
        r.run(args);
    }
}
