#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
using namespace std;

#include "VisItPythonConnection.h"

VisItPythonConnection visit;
bool initialized;

// ****************************************************************************
//  Method:  VisualizeFile
//
//  Purpose:
//    Intended to be called from the "debug" function.
//    Starts, or re-uses, a connection the visit CLI,
//    launches a GUI (or opens a new window), opens the
//    given file, and makes a couple plots from it.
//
//  Arguments:
//    fname        the file name
//    allowRetry   allow for one attempt to relaunch visit in case of failure
//                 (this accounts for a user closing visit in between calls)
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 13, 2007
//
//  Modifications:
//    Jeremy Meredith, Wed Jun 20 17:06:21 EDT 2007
//    Modified to open the GUI before a database.  It had been done in the
//    other order to avoid a warning (which has now been fixed).
//
// ****************************************************************************
void
VisualizeFile(const char *fname, bool allowRetry=true)
{
    bool success = true;
    if (!visit.IsOpen())
    {
        initialized = false;
        success = visit.Open();
        if (!success)
        {
            visit.Close();
            initialized = false;
            cerr << "VisualizeFile: could not open connection to VisIt";
            return;
        }
    }

    char opencmd[2000];
    if (!initialized)
    {
        success = success && visit.SendCommand("OpenGUI()");
        // Other one-time-initialization goes here
        initialized = true;
    }
    else
    {
        success = success && visit.SendCommand("AddWindow()");
        success = success && visit.SendCommand("DeleteAllPlots()");
    }
    sprintf(opencmd, "OpenDatabase('%s', 0, 'VTK_1.0')", fname);
    success = success && visit.SendCommand(opencmd);
    success = success && visit.SendCommand("AddPlot('Pseudocolor','nodal')");
    success = success && visit.SendCommand("AddPlot('Mesh','mesh')");
    success = success && visit.SendCommand("DrawPlots()");
    if (!success)
    {
        visit.Close();
        initialized = false;
        if (allowRetry)
        {
            VisualizeFile(fname, false);
        }
        else
        {
            cerr << "VisualizeFile: could not communicate commands to VisIt";
        }
    }
}

float data[4] = {1,3,0,2};

// ****************************************************************************
//  Method:  Step
//
//  Purpose:
//    Run one time step.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 13, 2007
//
// ****************************************************************************
void Step()
{
    data[2] = data[2] + 0.5;
}

// ****************************************************************************
//  Method:  VisualizeFile
//
//  Purpose:
//    Write a simple VTK data set to a temporary file and return the path.
//
//  Arguments:
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 13, 2007
//
//  Modifications:
//
// ****************************************************************************
std::string Write()
{
    char *filename = tempnam(NULL,"visit");
    ofstream out(filename,ios::out);
    out << "# vtk DataFile Version 3.0" << endl;
    out << "vtk output" << endl;
    out << "ASCII" << endl;
    out << "DATASET UNSTRUCTURED_GRID" << endl;
    out << "POINTS 4 float" << endl;
    out << "0 0 0" << endl;
    out << "1 0 0" << endl;
    out << "0 1 0 " << endl;
    out << "1.1 1.1 0" << endl;
    out << "CELLS 1 5" << endl;
    out << "4 0 1 3 2" << endl;
    out << "CELL_TYPES 1" << endl;
    out << "9" << endl;
    out << "CELL_DATA 1" << endl;
    out << "POINT_DATA 4" << endl;
    out << "FIELD FieldData 1" << endl;
    out << "nodal 1 4 float" << endl;
    out << data[0] <<" "<< data[1] <<" "<< data[2] <<" "<< data[3] << endl;
    out.close();
    return filename;
}

// ****************************************************************************
//  Method:  Debug
//
//  Purpose:
//    Dumps a temporary file and calls visit to launch it.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 13, 2007
//
// ****************************************************************************
void Debug()
{
    std::string filename = Write();
    VisualizeFile(filename.c_str());
}

// ****************************************************************************
//  Method:  main
//
//  Purpose:
//    Run the simulation.  To test the capabilities under GDB, break
//    in this routine and 'call Debug()' to visualize the current mesh.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 13, 2007
//
// ****************************************************************************
int main (int argc, char **argv)
{
    // To test:
    //    gdb argv[0]
    //    break main
    //    run
    //    then repeat:
    //       next 2
    //       call Debug()
    for (int step=0; step<10; step++)
    {
        Step();
    }
}
