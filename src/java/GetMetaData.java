// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

import java.lang.ArrayIndexOutOfBoundsException;
import llnl.visit.avtDatabaseMetaData;

// ****************************************************************************
// Class: GetMetaData
//
// Purpose:
//   This example program opens a database and gets the metadata, printing
//   it to the console.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 25 12:01:43 PDT 2008
//
// Modifications:
//
// ****************************************************************************

public class GetMetaData extends RunViewer
{
    public GetMetaData()
    {
        super();
    }

    protected void work(String[] args)
    {
        // Try and open a database
        if(viewer.GetViewerMethods().RequestMetaData(viewer.GetDataPath() + "noise.silo",0))
        {
            avtDatabaseMetaData md = viewer.GetViewerState().GetDatabaseMetaData();
            System.out.print(md.toString());
        }
        else
            System.out.println("Could not get the metadata for the database!");
    }

    public static void main(String args[])
    {
        GetMetaData r = new GetMetaData();
        r.run(args);
    }
}
