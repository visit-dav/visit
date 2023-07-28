// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ----------------------------------------------------------------------------
// File:  main.C
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
//
//    Jeremy Meredith, Tue Aug 29 16:13:43 EDT 2006
//    Added Line and Vertex shapes.  Added missing help text for other shapes.
//
//    Jeremy Meredith, Mon Jul  9 15:22:06 EDT 2012
//    Added 5- thru 8-sided polygon shapes.
//
//    Kathleen Biagas, Wed Apr  5 12:45:21 PDT 2023
//    Remove call to obsolete function setColorSpec.
//
// ----------------------------------------------------------------------------

#include <QApplication>

#include <QMainWindow>

#include <iostream>
#include <stdlib.h>

#include "ClipEditor.h"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication *a = new QApplication( argc, argv );

    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <shape>\n";
        cerr << "   where <shape> = { hex, wdg, pyr, tet, qua, pix, tri, lin, vtx, poly5, poly6, poly7, poly8 }\n";
        exit(1);
    }

    cerr << ">>>>>>>>>> Valid keys <<<<<<<<<\n";
    cerr << "\n";
    cerr << "Display commands:\n";
    cerr << " D:  Change duplicate face removal algorithm {no-dupes, all+dupes, only-dupes}\n";
    cerr << " L:  Toggle lighting  {alpha+unlit, solid+lit}\n";
    cerr << " N:  Toggle numbering {on, off}\n";
    cerr << "\n";
    cerr << "File saving commands:\n";
    cerr << " S:  Save file, making full backup\n";
    cerr << "\n";
    cerr << "Case selection commands:\n";
    cerr << " up    :  previous case\n";
    cerr << " pgup  :  previous unique case\n";
    cerr << " down  :  next case\n";
    cerr << " pgdown:  next unique case\n";
    cerr << "\n";
    cerr << "Output subshape selection commands (shape '0' shows all output shapes):\n";
    cerr << " left  :  previous subshape (cycles past beginning)\n";
    cerr << " right :  next subshape (cycles past end)\n";
    cerr << " home  :  go back to shape '0' (i.e. show all shapes)\n";
    cerr << "\n";
    cerr << "Output subshape modification commands:\n";
    cerr << " space :  toggle shape color (toggles default color when no subshape selected)\n";
    cerr << " minus :  invert current subshape\n";
    cerr << " delete:  delete current subshape\n";
    cerr << " insert:  add new subshape, and enter shape insertion mode\n";
    cerr << "\n";
    cerr << "Shape insertion mode:\n";
    cerr << " escape:  abort shape insertion at any pont\n";
    cerr << "\n";
    cerr << " Step 1: choose output shape\n";
    cerr << "   H:  hex\n";
    cerr << "   W:  wedge\n";
    cerr << "   P:  pyramid\n";
    cerr << "   T:  tetrahedron\n";
    cerr << "   Q:  quad\n";
    cerr << "   R:  triangle\n";
    cerr << "   L:  line\n";
    cerr << "   V:  vertex\n";
    cerr << "   N:  newpoint (centroid based) (four max newpoints per case)\n";
    cerr << "\n";
    cerr << " Step 2: choose number of points (NewPoint only)\n";
    cerr << "   1-8:  number of points\n";
    cerr << "\n";
    cerr << " Step 3: choose points\n";
    cerr << "   0-7:  index from original shape node\n";
    cerr << "   A-L:  index from original shape edge\n";
    cerr << "   P-S:  index from a newpoint\n";
    cerr << "\n";
    cerr << "\n";

    ClipEditor *editor = new ClipEditor(argv[1], NULL);

    editor->show();

    return a->exec();
}
