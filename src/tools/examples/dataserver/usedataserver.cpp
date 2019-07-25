// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataServer.h>

#include <vtkDataSet.h>
#include <vtkDataSetWriter.h>

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

// ****************************************************************************
// Method: GetData
//
// Purpose: 
//   Set up a network on the engine, execute it, and return VTK data.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed May 23 15:47:28 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet **
GetData(VisItDataServer &server, const std::string plotId,
    const std::string &filename, int timeState, const std::string &var,
    int &n)
{
    //
    // Open the database.
    //
    server.OpenDatabase(filename, timeState);

    //
    // Read the variable of interest.
    //
    server.ReadData(var);

    //
    // Create an IndexSelect operator
    //
    std::string id("IndexSelect_1.0");
    AttributeSubject *atts = server.CreateOperatorAttributes(id);
    atts->SetValue("xMin", 20);
    atts->SetValue("xMax", 25);
    server.AddOperator(id, atts);
    delete atts;

    //
    // Add a plot to the network.
    //
    server.AddPlot(plotId);

    //
    // Execute the plot.
    //
    return server.Execute(n);
}

// ****************************************************************************
// Method: GetContourData
//
// Purpose: 
//   Set up a network on the engine, execute it, and return VTK data.
//
// Arguments:
//
// Returns:    
//
// Note:       Return contour surfaces.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 23 15:47:28 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet **
GetContourData(VisItDataServer &server, 
    const std::string &filename, int timeState, const std::string &var,
    int &n)
{
    //
    // Open the database.
    //
    server.OpenDatabase(filename, timeState);

    //
    // Read the variable of interest.
    //
    server.ReadData(var);

    //
    // Create an IndexSelect operator
    //
    std::string id("IndexSelect_1.0");
    AttributeSubject *atts = server.CreateOperatorAttributes(id);
    atts->SetValue("xMin", 20);
    atts->SetValue("xMax", 25);
    server.AddOperator(id, atts);
    delete atts;

    //
    // Create an Isosurface operator
    //
    std::string id2("Isosurface_1.0");
    atts = server.CreateOperatorAttributes(id2);
    atts->SetValue("contourMethod", 1); // Select by value
    std::vector<double> values;         // Make up some contour values
    values.push_back(1.3);
    values.push_back(2.4);
    values.push_back(3.7);
    values.push_back(4.9);
    atts->SetValue("contourValue", values);
    server.AddOperator(id2, atts);
    delete atts;

    //
    // Add a plot to the network.
    //
    server.AddPlot("Pseudocolor_1.0");

    //
    // Execute the plot.
    //
    return server.Execute(n);
}

// ****************************************************************************
// Function: main
//
// Purpose: 
//   The main program. Show how to get a dataset directly from the engine.
//
// Note:       This program assumes that the "visit" launcher is in your
//             path and it is the same version as the libraries to which
//             this program is linked.
//
// Example usage:
//    cd src/bin
//    ../exe/VisItDataClient -plugindir ../plugins -debug 5 -clobber_vlogs
//
// Programmer: Brad Whitlock
// Creation:   Wed May 23 15:34:29 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

int
main(int argc, char *argv[])
{
    VisItDataServer server;

    // Add arguments.
    for(int i = 1; i < argc; ++i)
    {
        if(i+1 < argc && strcmp(argv[i], "-plugindir") == 0)
        {
            server.SetPluginDir(argv[i+1]);
            i++;
        }
        else if(i+1 < argc && strcmp(argv[i], "-debug") == 0)
        {
            server.AddArgument("-debug");
            server.AddArgument(argv[i+1]);
            i++;
        }
        else if(strcmp(argv[i], "-clobber_vlogs") == 0 ||
                strcmp(argv[i], "-quiet") == 0)
        {
            server.AddArgument(argv[i]);
        }
        else if(strcmp(argv[i], "-debugengine") == 0)
        {
            server.DebugEngine();
        }
    }

    TRY
    {
        //
        // Try opening the server
        //
        server.Open(argc, argv);

        std::string filename("/usr/gapps/visit/data/noise.silo");
        int         timeState = 0;
        const char *varnames[] = {"hardyglobal", "grad", "hardyglobal"};
        const char *filebases[] = {"scalar", "vector", "contour"};
        const char *bestplot[] = {"Pseudocolor_1.0", "Label_1.0", "Pseudocolor_1.0"};
        for(int j = 0; j < 3; ++j)
        {
            //
            // Use the server to return some VTK data.
            //
            int n = 0;
            vtkDataSet **datasets = NULL;
            if(j == 0)
                datasets = GetData(server, bestplot[j], filename, timeState, varnames[j], n);
            else if(j == 1)
                datasets = GetData(server, bestplot[j], filename, timeState, varnames[j], n);
            else if(j == 2)
                datasets = GetContourData(server, filename, timeState, varnames[j], n);

            //
            // Now, do something with the VTK datasets. We're writing them out.
            //
            cout << "Plot of " << varnames[j] << " contains " << n << " datasets." << endl;
            for(int i = 0; i < n; ++i)
            {
                char vtkfile[100];
                SNPRINTF(vtkfile, 100, "%s%04d.vtk", filebases[j], i);
                cout << "\tWriting " << vtkfile << endl;
                vtkDataSetWriter *writer = vtkDataSetWriter::New();
                writer->SetInputData(datasets[i]);
                writer->SetFileName(vtkfile);
                writer->Update();
                writer->Delete();
            }

            // Free the VTK datasets
            for(int i = 0; i < n; ++i)
                datasets[i]->Delete();
            delete [] datasets;
        }
    }
    CATCH2(VisItException, e)
    {
        cerr << "Caught exception " << e.GetExceptionType() << ". "
             << e.GetFilename() << ":" << e.GetLine() << ":  "
             << e.Message() << endl;
    }
    ENDTRY

    // Close the mdserver & server
    server.Close();

    return 0;
}
