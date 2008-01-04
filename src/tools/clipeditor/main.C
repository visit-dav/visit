/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ----------------------------------------------------------------------------
// File:  main.C
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
//
//    Jeremy Meredith, Tue Aug 29 16:13:43 EDT 2006
//    Added Line and Vertex shapes.  Added missing help text for other shapes.
//
// ----------------------------------------------------------------------------

#include <qapplication.h>

#include <qmainwindow.h>

#include <visitstream.h>
#include <stdlib.h>

#include "ClipEditor.h"

int main(int argc, char *argv[])
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication *a = new QApplication( argc, argv );

    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <shape>\n";
        cerr << "   where <shape> = { hex, wedge, pyramid, tet }\n";
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

    ClipEditor *editor = new ClipEditor(argv[1], NULL, "Viewer");

    a->setMainWidget(editor);

    editor->show();

    return a->exec();
}
