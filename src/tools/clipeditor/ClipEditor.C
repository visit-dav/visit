/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
// File:  ClipEditor.C
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
//
// Modifications:
//    Jeremy Meredith, Mon Sep 15 17:21:30 PDT 2003
//    Allowed centroid-points to use the color associated with them, and
//    added a NOCOLOR option (i.e. the centroid-point is on the intersection
//    between the two materials).
//
//    Jeremy Meredith, Thu Sep 18 11:29:12 PDT 2003
//    Added Quad and Triangle shapes.
//
//    Jeremy Meredith, Thu Jun 24 10:38:05 PDT 2004
//    Added Voxel and Pixel shapes.
//
//    Jeremy Meredith, Tue Aug 29 16:13:43 EDT 2006
//    Added Line and Vertex shapes.  Added copyright string.
//
// ----------------------------------------------------------------------------

#include "ClipEditor.h"

#include "Shape.h"
#include "Viewer.h"

#include <stdlib.h>
#include <visitstream.h>

std::string copyright_str = 
"/*****************************************************************************\n"
"*\n"
"* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC\n"
"* Produced at the Lawrence Livermore National Laboratory\n"
"* LLNL-CODE-400124\n"
"* All rights reserved.\n"
"*\n"
"* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The\n"
"* full copyright notice is contained in the file COPYRIGHT located at the root\n"
"* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.\n"
"*\n"
"* Redistribution  and  use  in  source  and  binary  forms,  with  or  without\n"
"* modification, are permitted provided that the following conditions are met:\n"
"*\n"
"*  - Redistributions of  source code must  retain the above  copyright notice,\n"
"*    this list of conditions and the disclaimer below.\n"
"*  - Redistributions in binary form must reproduce the above copyright notice,\n"
"*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the\n"
"*    documentation and/or other materials provided with the distribution.\n"
"*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may\n"
"*    be used to endorse or promote products derived from this software without\n"
"*    specific prior written permission.\n"
"*\n"
"* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS \"AS IS\"\n"
"* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE\n"
"* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE\n"
"* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,\n"
"* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY\n"
"* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL\n"
"* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR\n"
"* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER\n"
"* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT\n"
"* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY\n"
"* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH\n"
"* DAMAGE.\n"
"*\n"
"*****************************************************************************/\n";

inline int ConvertedCaseIndex(int i, bool qpconv, bool hvconv)
{
    // qpconv: true if converting quad to pixel
    // hvconv: true if converting hex to voxel
    int caseIndex;
    if (qpconv)
    {
        int bits[8] = {i&0x01, i&0x02, i&0x04, i&0x08};
        caseIndex = ((bits[2] ? 0x08:0) +
                     (bits[3] ? 0x04:0) +
                     (bits[1] ? 0x02:0) +
                     (bits[0] ? 0x01:0));
    }
    else if (hvconv)
    {
        int bits[8] = {i&0x01, i&0x02, i&0x04, i&0x08,
                       i&0x10, i&0x20, i&0x40, i&0x80};
        caseIndex = ((bits[6] ? 0x80:0) +
                     (bits[7] ? 0x40:0) +
                     (bits[5] ? 0x20:0) +
                     (bits[4] ? 0x10:0) +
                     (bits[2] ? 0x08:0) +
                     (bits[3] ? 0x04:0) +
                     (bits[1] ? 0x02:0) +
                     (bits[0] ? 0x01:0));
    }
    else
    {
        caseIndex = i;
    }
    return caseIndex;
}

inline const char *NodeToStr(char c)
{
    switch (c)
    {
      case '0': return "P0";
      case '1': return "P1";
      case '2': return "P2";
      case '3': return "P3";
      case '4': return "P4";
      case '5': return "P5";
      case '6': return "P6";
      case '7': return "P7";

      case 'a': return "EA";
      case 'b': return "EB";
      case 'c': return "EC";
      case 'd': return "ED";
      case 'e': return "EE";
      case 'f': return "EF";
      case 'g': return "EG";
      case 'h': return "EH";
      case 'i': return "EI";
      case 'j': return "EJ";
      case 'k': return "EK";
      case 'l': return "EL";

      case 'p': return "N0";
      case 'q': return "N1";
      case 'r': return "N2";
      case 's': return "N3";
      case 't': return "N4";

      default: return "!!UNKNOWN!!\n";
    }
}

inline char StrToNode(const char *c)
{
    if (!strcmp(c,"P0")) return '0';
    if (!strcmp(c,"P1")) return '1';
    if (!strcmp(c,"P2")) return '2';
    if (!strcmp(c,"P3")) return '3';
    if (!strcmp(c,"P4")) return '4';
    if (!strcmp(c,"P5")) return '5';
    if (!strcmp(c,"P6")) return '6';
    if (!strcmp(c,"P7")) return '7';

    if (!strcmp(c,"EA")) return 'a';
    if (!strcmp(c,"EB")) return 'b';
    if (!strcmp(c,"EC")) return 'c';
    if (!strcmp(c,"ED")) return 'd';
    if (!strcmp(c,"EE")) return 'e';
    if (!strcmp(c,"EF")) return 'f';
    if (!strcmp(c,"EG")) return 'g';
    if (!strcmp(c,"EH")) return 'h';
    if (!strcmp(c,"EI")) return 'i';
    if (!strcmp(c,"EJ")) return 'j';
    if (!strcmp(c,"EK")) return 'k';
    if (!strcmp(c,"EL")) return 'l';

    if (!strcmp(c,"N0")) return 'p';
    if (!strcmp(c,"N1")) return 'q';
    if (!strcmp(c,"N2")) return 'r';
    if (!strcmp(c,"N3")) return 's';
    if (!strcmp(c,"N4")) return 't';

    cerr << "Error: Bad node string '"<<c<<"'\n";
    return 0;
}

inline char StrToNodeWithConversion(const char *c)
{
    // This is the same as StrToNode, but it also converts
    // indexing from Quad to Pixel or Hex to Voxel
    if (!strcmp(c,"P0")) return '0';
    if (!strcmp(c,"P1")) return '1';
    if (!strcmp(c,"P2")) return '3';
    if (!strcmp(c,"P3")) return '2';
    if (!strcmp(c,"P4")) return '4';
    if (!strcmp(c,"P5")) return '5';
    if (!strcmp(c,"P6")) return '7';
    if (!strcmp(c,"P7")) return '6';

    if (!strcmp(c,"EA")) return 'a';
    if (!strcmp(c,"EB")) return 'b';
    if (!strcmp(c,"EC")) return 'c';
    if (!strcmp(c,"ED")) return 'd';
    if (!strcmp(c,"EE")) return 'e';
    if (!strcmp(c,"EF")) return 'f';
    if (!strcmp(c,"EG")) return 'g';
    if (!strcmp(c,"EH")) return 'h';
    if (!strcmp(c,"EI")) return 'i';
    if (!strcmp(c,"EJ")) return 'j';
    if (!strcmp(c,"EK")) return 'k';
    if (!strcmp(c,"EL")) return 'l';

    if (!strcmp(c,"N0")) return 'p';
    if (!strcmp(c,"N1")) return 'q';
    if (!strcmp(c,"N2")) return 'r';
    if (!strcmp(c,"N3")) return 's';
    if (!strcmp(c,"N4")) return 't';

    cerr << "Error: Bad node string '"<<c<<"'\n";
    return 0;
}

ClipEditor::ClipEditor(const QString &st,
                     QWidget *parent, const QString &name) :
        QMainWindow(parent, name)
{
    if (st.left(3) == "hex")
    {
        ncases    = 256;
        shapetype = ST_HEX;
    }
    else if (st.left(3) == "vox")
    {
        ncases    = 256;
        shapetype = ST_VOXEL;
    }
    else if (st.left(3) == "wed" || st.left(3) == "wdg")
    {
        ncases    = 64;
        shapetype = ST_WEDGE;
    }
    else if (st.left(3) == "pyr")
    {
        ncases    = 32;
        shapetype = ST_PYRAMID;
    }
    else if (st.left(3) == "tet")
    {
        ncases    = 16;
        shapetype = ST_TET;
    }
    else if (st.left(3) == "qua")
    {
        ncases    = 16;
        shapetype = ST_QUAD;
    }
    else if (st.left(3) == "pix")
    {
        ncases    = 16;
        shapetype = ST_PIXEL;
    }
    else if (st.left(3) == "tri")
    {
        ncases    = 8;
        shapetype = ST_TRIANGLE;
    }
    else if (st.left(3) == "lin")
    {
        ncases    = 4;
        shapetype = ST_LINE;
    }
    else if (st.left(3) == "vtx")
    {
        ncases    = 2;
        shapetype = ST_VERTEX;
    }
    else
    {
        cerr << "Expected a valid shape type\n";
        exit(2);
    }


    viewer = new Viewer(this, "Viewer Widget");
    setCentralWidget(viewer);
    textMode = TM_MAIN;
    defaultcolor = 0;

    for (int i=0; i<ncases; i++)
    {
        datasets.push_back(new DataSet(shapetype, i));
    }
    caseindex = 0;
    viewer->setDataSet(datasets[caseindex]);

    cerr << "Unique cases: ";
    int counter = 0;
    for (int i=0; i<ncases; i++)
    {
        bool copy = false;
        for (int j=0; j<i; j++)
        {
            int xform = datasets[i]->shapes[0].CheckCopyOf(&datasets[j]->shapes[0]);

            if (xform >= 0)
            {
                copy = true;
                datasets[i]->copyOfDataset = datasets[j];
                datasets[i]->copyOfIndex   = j;
                datasets[i]->transformNumber = xform;
                break;
            }
        }
        if (!copy)
        {
            ++counter;
            cerr << i << " ";
        }
    }
    cerr << endl;

    LoadFromFile();
}

void
ClipEditor::keyPressEvent(QKeyEvent *kev)
{
    //cerr << "Got text='"<<kev->text()<<"', int="<<kev->key()<< endl;
    //cerr << "Got text="<<kev->text()<<"  int="<<kev->key()<< endl;
    if (textMode == TM_MAIN)
    {
        if (kev->key() == 'D')
        {
            Shape::duplicateFacesRemoval = (Shape::duplicateFacesRemoval+1)%3;
            cerr << "Duplicate faces are now " <<
                (Shape::duplicateFacesRemoval==0 ? "removed" :
                 (Shape::duplicateFacesRemoval==1 ? "left alone" : "drawn alone")) << endl;
        }
        else if (kev->key() == 'L')
        {
            Shape::lighting = !Shape::lighting;
            cerr << "Lighting is now " <<
                (Shape::lighting ? "on" : "off") << endl;
        }
        else if (kev->key() == 'N')
        {
            Shape::numbering = !Shape::numbering;
            cerr << "Number display is now " <<
                (Shape::numbering ? "on" : "off") << endl;
        }
        else if (kev->key() == Qt::Key_Up)
        {
            if (caseindex > 0)
            {
                caseindex--;
                datasets[caseindex]->ReInit();
                viewer->setDataSet(datasets[caseindex]);
                cerr << ">>>  Now on case " << caseindex;
                if (! datasets[caseindex]->copyOfDataset)
                    cerr << ": Unique";
                cerr << endl;
            }
        }
        else if (kev->key() == Qt::Key_Down)
        {
            if (caseindex < ncases-1)
            {
                caseindex++;
                datasets[caseindex]->ReInit();
                viewer->setDataSet(datasets[caseindex]);
                cerr << ">>>  Now on case " << caseindex;
                if (! datasets[caseindex]->copyOfDataset)
                    cerr << ": Unique";
                cerr << endl;
            }
        }
        else if (kev->key() == Qt::Key_Prior)
        {
            int oldindex = caseindex;
            caseindex--;
            while (caseindex >= 0)
            {
                if (! datasets[caseindex]->copyOfDataset)
                {
                    datasets[caseindex]->ReInit();
                    viewer->setDataSet(datasets[caseindex]);
                    cerr << ">>>  Now on case " << caseindex;
                    if (! datasets[caseindex]->copyOfDataset)
                        cerr << ": Unique";
                    cerr << endl;
                    break;
                }
                caseindex--;
            }
            if (caseindex < 0)
            {
                caseindex = oldindex;
                cerr << "No previous unique cases\n";
            }
        }
        else if (kev->key() == Qt::Key_Next)
        {
            int oldindex = caseindex;
            caseindex++;
            while (caseindex < ncases)
            {
                if (! datasets[caseindex]->copyOfDataset)
                {
                    datasets[caseindex]->ReInit();
                    viewer->setDataSet(datasets[caseindex]);
                    cerr << ">>>  Now on case " << caseindex;
                    if (! datasets[caseindex]->copyOfDataset)
                        cerr << ": Unique";
                    cerr << endl;
                    break;
                }
                caseindex++;
            }
            if (caseindex >= ncases)
            {
                caseindex = oldindex;
                cerr << "No more unique cases\n";
            }
        }
        else if (kev->key() == Qt::Key_Home)
        {
            if (datasets[caseindex]->copyOfDataset)
            {
                cerr << "Read-only case!\n";
            }
            else
            {
                datasets[caseindex]->selectedShape = 0;
                cerr << "Selected subshape: All\n";
            }
        }
        else if (kev->key() == Qt::Key_Left)
        {
            if (datasets[caseindex]->copyOfDataset)
            {
                cerr << "Read-only case!\n";
            }
            else
            {
                datasets[caseindex]->selectedShape--;
                if (datasets[caseindex]->selectedShape < 0)
                    datasets[caseindex]->selectedShape = datasets[caseindex]->shapes.size()-1;
                cerr << "Selected subshape: ";
                if (datasets[caseindex]->selectedShape==0)
                    cerr << "All\n";
                else
                    cerr << datasets[caseindex]->selectedShape << endl;
            }
        }
        else if (kev->key() == Qt::Key_Right)
        {
            if (datasets[caseindex]->copyOfDataset)
            {
                cerr << "Read-only case!\n";
            }
            else
            {
                datasets[caseindex]->selectedShape++;
                if (datasets[caseindex]->selectedShape >= datasets[caseindex]->shapes.size())
                    datasets[caseindex]->selectedShape = 0;
                cerr << "Selected subshape: ";
                if (datasets[caseindex]->selectedShape==0)
                    cerr << "All\n";
                else
                    cerr << datasets[caseindex]->selectedShape << endl;
            }
        }
        else if (kev->key() == Qt::Key_Space)
        {
            if (datasets[caseindex]->copyOfDataset)
            {
                cerr << "Read-only case!\n";
            }
            else
            {
                if (datasets[caseindex]->selectedShape > 0 && 
                    datasets[caseindex]->selectedShape < datasets[caseindex]->shapes.size())
                {
                    if (datasets[caseindex]->shapes[datasets[caseindex]->selectedShape].shapeType == ST_POINT)
                    {
                        datasets[caseindex]->shapes[datasets[caseindex]->selectedShape].color = 
                            (datasets[caseindex]->shapes[datasets[caseindex]->selectedShape].color + 1) % 3;
                    }
                    else
                    {
                        datasets[caseindex]->shapes[datasets[caseindex]->selectedShape].color = 
                            1 - datasets[caseindex]->shapes[datasets[caseindex]->selectedShape].color;
                    }
                    switch (datasets[caseindex]->shapes[datasets[caseindex]->selectedShape].color)
                    {
                      case 0:
                        cerr << "Set shape color to blue\n";
                        break;
                      case 1:
                        cerr << "Set shape color to green\n";
                        break;
                      case 2:
                        cerr << "Set shape color to NOCOLOR\n";
                        break;
                    }
                }
                else if (datasets[caseindex]->selectedShape == 0)
                {
                    defaultcolor = 1 - defaultcolor;
                    cerr << "Default color = " << (defaultcolor ? "green" : "blue") << endl;
                }
            }
        }
        else if (kev->key() == Qt::Key_Minus)
        {
            if (datasets[caseindex]->copyOfDataset)
            {
                cerr << "Read-only case!\n";
            }
            else
            {
                if (datasets[caseindex]->selectedShape > 0 && 
                    datasets[caseindex]->selectedShape < datasets[caseindex]->shapes.size())
                {
                    datasets[caseindex]->shapes[datasets[caseindex]->selectedShape].Invert();
                }
            }
        }
        else if (kev->key() == Qt::Key_Insert)
        {
            if (datasets[caseindex]->copyOfDataset)
            {
                cerr << "Read-only case!\n";
            }
            else
            {
                textMode = TM_ADD;
                addedpoints = -2;
                pts = "";
                cerr << "Adding new shape, please choose\n Hex, Wedge, Pyramid, Tet, Quad, tRiangle, Line, Vertex, or NewPoint\n";
            }
        }
        else if (kev->key() == Qt::Key_Delete)
        {
            if (datasets[caseindex]->copyOfDataset)
            {
                cerr << "Read-only case!\n";
            }
            else
            {
                if (datasets[caseindex]->selectedShape > 0 && 
                    datasets[caseindex]->selectedShape < datasets[caseindex]->shapes.size())
                {
                    cerr << "Deleting shape\n";
                    std::vector<Shape> &shapes = datasets[caseindex]->shapes;
                    for (int i=datasets[caseindex]->selectedShape; i<shapes.size(); i++)
                    {
                        shapes[i] = shapes[i+1];
                    }
                    shapes.resize(shapes.size()-1);
                    datasets[caseindex]->ReInit();
                }
                else
                {
                    cerr << "Can't delete: no shape selected\n";
                }
            }
        }
        else if (kev->key() == 'S')
        {
            cerr << "Trying to save\n";
            SaveToFile();
        }
    }
    else if (textMode == TM_ADD)
    {
        if (addedpoints == -2)
        {
            if (kev->key() == 'H')
            {
                npts = 8;
                addingShape = ST_HEX;
                addedpoints = 0;
                cerr << "Chose Hex; please choose 8 points\n";
            }
            else if (kev->key() == 'W')
            {
                npts = 6;
                addingShape = ST_WEDGE;
                addedpoints = 0;
                cerr << "Chose Wedge; please choose 6 points\n";
            }
            else if (kev->key() == 'P')
            {
                npts = 5;
                addingShape = ST_PYRAMID;
                addedpoints = 0;
                cerr << "Chose Pyramid; please choose 5 points\n";
            }
            else if (kev->key() == 'T')
            {
                npts = 4;
                addingShape = ST_TET;
                addedpoints = 0;
                cerr << "Chose Tet; please choose 4 points\n";
            }
            else if (kev->key() == 'Q')
            {
                npts = 4;
                addingShape = ST_QUAD;
                addedpoints = 0;
                cerr << "Chose Quad; please choose 4 points\n";
            }
            else if (kev->key() == 'R')
            {
                npts = 3;
                addingShape = ST_TRIANGLE;
                addedpoints = 0;
                cerr << "Chose Triangle; please choose 4 points\n";
            }
            else if (kev->key() == 'L')
            {
                npts = 2;
                addingShape = ST_LINE;
                addedpoints = 0;
                cerr << "Chose Line; please choose 2 points\n";
            }
            else if (kev->key() == 'V')
            {
                npts = 1;
                addingShape = ST_VERTEX;
                addedpoints = 0;
                cerr << "Chose Vertex; please choose 1 point\n";
            }
            else if (kev->key() == 'N')
            {
                addingShape = ST_POINT;
                addedpoints = -1;
                cerr << "Chose NewPoint; please choose the number of points (1-8)\n";
            }
            else if (kev->key() == Qt::Key_Escape)
            {
                addedpoints = 0;
                textMode = TM_MAIN;
                cerr << "Aborted adding shape\n";
            }
            else
            {
                cerr << "Invalid shape type " << kev->text() << endl;
            }
        }
        else if (addedpoints == -1)
        {
            if (kev->key() >= '1' && kev->key() <= '8')
            {
                npts = kev->key() - '0';
                addedpoints = 0;
                cerr << "Please enter "<<npts<<" points\n";
            }
            else if (kev->key() == Qt::Key_Escape)
            {
                addedpoints = 0;
                textMode = TM_MAIN;
                cerr << "Aborted adding shape\n";
            }
            else
            {
                cerr << "Please choose 1 - 8.\n";
            }
        }
        else
        {
            if (kev->key() >= '0' && kev->key() <= '7')
            {
                pts += tolower(kev->key());
                addedpoints++;
                cerr << "    point "<<char(kev->key()) << endl;
            }
            else if (kev->key() >= 'A' && kev->key() <= 'L')
            {
                pts += tolower(kev->key());
                addedpoints++;
                cerr << "    point "<<char(kev->key()) << endl;
            }
            else if (kev->key() >= 'P' && kev->key() <= 'S')
            {
                pts += tolower(kev->key());
                addedpoints++;
                cerr << "    point "<<char(kev->key()) << endl;
            }
            else if (kev->key() == Qt::Key_Escape)
            {
                addedpoints = 0;
                textMode = TM_MAIN;
                cerr << "Aborted adding shape\n";
            }
            else
            {
                cerr << "Invalid key " << kev->text() << endl;
            }

            if (addedpoints == npts)
            {
                cerr << "Succesfully added new shape, nodes='"<<pts<<"'\n";
                datasets[caseindex]->shapes.resize(datasets[caseindex]->shapes.size()+1);
                datasets[caseindex]->shapes[datasets[caseindex]->shapes.size()-1] =
                    Shape(addingShape,
                          &datasets[caseindex]->shapes[0],
                          defaultcolor,
                          addedpoints,
                          pts.c_str(),
                          datasets[caseindex]);

                datasets[caseindex]->ReInit();

                textMode = TM_MAIN;
            }
        }
    }

    viewer->updateGL();
}

void
ClipEditor::LoadFromFile()
{
    char fname[200];
    const char *lower, *upper;
    switch (shapetype)
    {
      case ST_HEX:     lower="Hex"; upper="HEX"; break;
      case ST_VOXEL:   lower="Vox"; upper="VOX"; break;
      case ST_WEDGE:   lower="Wdg"; upper="WDG"; break;
      case ST_PYRAMID: lower="Pyr"; upper="PYR"; break;
      case ST_TET:     lower="Tet"; upper="TET"; break;
      case ST_QUAD:    lower="Qua"; upper="QUA"; break;
      case ST_PIXEL:   lower="Pix"; upper="PIX"; break;
      case ST_TRIANGLE:lower="Tri"; upper="TRI"; break;
      case ST_LINE:    lower="Lin"; upper="LIN"; break;
      case ST_VERTEX:  lower="Vtx"; upper="VTX"; break;
      default: cerr << "Error\n"; break;
    }
    sprintf(fname, "ClipCases%s.C", lower);

    ifstream in(fname, ios::in);

    bool qpconv = false;
    bool hvconv = false;
    if (!in && shapetype==ST_PIXEL)
    {
        cerr << "WARNING: Couldn't load from file "<<fname<<"!\n";
        lower = "Qua";
        upper = "QUA";
        sprintf(fname, "ClipCases%s.C", lower);
        qpconv = true;
        cerr << "Attempting to load from file "<<fname<<".\n";
        in.open(fname, ios::in);
    }

    if (!in && shapetype==ST_VOXEL)
    {
        cerr << "WARNING: Couldn't load from file "<<fname<<"!\n";
        lower = "Hex";
        upper = "HEX";
        hvconv = true;
        cerr << "Attempting to load from file "<<fname<<".\n";
        sprintf(fname, "ClipCases%s.C", lower);
        in.open(fname, ios::in);
    }

    if (!in)
    {
        cerr << "WARNING: Couldn't load from file "<<fname<<"!\n";
        cerr << "WARNING: Assuming you want to start from scratch!\n";
        return;
    }

    // Get to the sizes
    char buff[200];
    in >> buff;
    while (strcmp(buff, "="))
    {
        in >> buff;
    }
    in >> buff;
    while (strcmp(buff, "="))
    {
        in >> buff;
    }
    in >> buff;

    // Read the sizes
    for (int i=0; i<ncases; i++)
    {
        int caseIndex = ConvertedCaseIndex(i, qpconv, hvconv);

        int sz;
        in >> buff;
        if (buff[0] == '/')
        {
            in.getline(buff, 200);
            in >> buff;
        }
        if (buff[strlen(buff)-1] == ',') buff[strlen(buff)-1] = '\0';
        sz = atoi(buff);
        datasets[caseIndex]->shapes.resize(sz+1);
    }

    // Get to the offsets
    in >> buff;
    while (strcmp(buff, "="))
    {
        in >> buff;
    }

    // Get to the shapes
    in >> buff;
    while (strcmp(buff, "="))
    {
        in >> buff;
    }
    in >> buff;

    for (int i=0; i<ncases; i++)
    {
        int caseIndex = ConvertedCaseIndex(i, qpconv, hvconv);
        DataSet *d = datasets[caseIndex];
        //cerr << "Case "<<i<<": reading "<<(d->shapes.size()-1)<<" shapes\n";

        in >> buff;
        in.getline(buff,200);

        for (int j=1; j<d->shapes.size(); j++)
        {
            ShapeType st;
            int nv;
            char pts[8];
            int color = 0;

            in >> buff;
            if (buff[strlen(buff)-1] == ',') buff[strlen(buff)-1] = '\0';

            if (!strcmp(buff,"ST_HEX"))
            {
                st=ST_HEX;
                nv=8;
            }
            else if (!strcmp(buff,"ST_VOX"))
            {
                st=ST_VOXEL;
                nv=8;
            }
            else if (!strcmp(buff,"ST_WDG"))
            {
                st=ST_WEDGE;
                nv=6;
            }
            else if (!strcmp(buff,"ST_PYR"))
            {
                st=ST_PYRAMID;
                nv=5;
            }
            else if (!strcmp(buff,"ST_TET"))
            {
                st=ST_TET;
                nv=4;
            }
            else if (!strcmp(buff,"ST_QUA"))
            {
                st=ST_QUAD;
                nv=4;
            }
            else if (!strcmp(buff,"ST_TRI"))
            {
                st=ST_TRIANGLE;
                nv=3;
            }
            else if (!strcmp(buff,"ST_LIN"))
            {
                st=ST_LINE;
                nv=2;
            }
            else if (!strcmp(buff,"ST_VTX"))
            {
                st=ST_VERTEX;
                nv=1;
            }
            else if (!strcmp(buff,"ST_PNT"))
            {
                st=ST_POINT;
                in >> buff; // point id

                // color
                in >> buff;
                if (buff[strlen(buff)-1] == ',') buff[strlen(buff)-1] = '\0';
                if (buff[strlen(buff)-1] == '0')
                    color = 0;
                else if (buff[strlen(buff)-1] == '1')
                    color = 1;
                else if (!strcmp(buff, "NOCOLOR"))
                    color = 2;
                else
                {
                    cerr << "Bad file: bad color '"<<buff<<"'!\n";
                    exit(5);
                }

                // num verts
                in >> buff;
                if (buff[strlen(buff)-1] == ',') buff[strlen(buff)-1] = '\0';
                nv = atoi(buff);
            }
            else
            {
                cerr << "Bad file: bad shape type!\n";
                exit(3);
            }

            if (st != ST_POINT)
            {
                in >> buff;
                if (buff[strlen(buff)-1] == ',') buff[strlen(buff)-1] = '\0';
                if (buff[strlen(buff)-1] == '0')
                    color = 0;
                else if (buff[strlen(buff)-1] == '1')
                    color = 1;
                else
                {
                    cerr << "Bad file: bad color '"<<buff<<"'!\n";
                    exit(4);
                }
            }

            for (int p=0; p<nv; p++)
            {
                in >> buff;
                if (buff[strlen(buff)-1] == ',') buff[strlen(buff)-1] = '\0';

                if (hvconv || qpconv)
                    pts[p] = StrToNodeWithConversion(buff);
                else
                    pts[p] = StrToNode(buff);
            }

            d->shapes[j] = Shape(st, &d->shapes[0], color, nv, pts, d);
        }

        d->ReInit();
    }

    in.close();
}

void
ClipEditor::SaveToFile()
{
    for (int c=0; c<ncases; c++)
    {
        datasets[c]->ReInit();
    }

    char fname[200];
    const char *lower, *upper;
    switch (shapetype)
    {
      case ST_HEX:     lower="Hex"; upper="HEX"; break;
      case ST_VOXEL:   lower="Vox"; upper="VOX"; break;
      case ST_WEDGE:   lower="Wdg"; upper="WDG"; break;
      case ST_PYRAMID: lower="Pyr"; upper="PYR"; break;
      case ST_TET:     lower="Tet"; upper="TET"; break;
      case ST_QUAD:    lower="Qua"; upper="QUA"; break;
      case ST_PIXEL:   lower="Pix"; upper="PIX"; break;
      case ST_TRIANGLE:lower="Tri"; upper="TRI"; break;
      case ST_LINE:    lower="Lin"; upper="LIN"; break;
      case ST_VERTEX:  lower="Vtx"; upper="VTX"; break;
      default: cerr << "Error\n"; break;
    }
    sprintf(fname, "ClipCases%s.C", lower);

    ifstream in(fname, ios::in);
    if (in)
    {
        in.close();
        int i=0;
        bool backedup = false;
        char bakname[200];
        while (!backedup)
        {
            sprintf(bakname, "ClipCases%s.C.bak%04d", lower, i);
            ifstream in2(bakname, ios::in);
            if (in2)
            {
                in2.close();
            }
            else
            {
                char cmd[256];
                sprintf(cmd, "cp %s %s", fname, bakname);
                system(cmd);
                cerr << "backed up "<<fname<<" to " << bakname << endl;
                backedup = true;
            }
            i++;
            if (i>100)
            {
                cerr << "Too many backups; skipping\n";\
                break;
            }
        }
    }

    ofstream out(fname, ios::out);
    if (!out)
    {
        cerr << "ERROR: Couldn't write file!!!\n";
        return;
    }

    out << copyright_str.c_str() << endl;

    out << "#include \"ClipCases.h\"\n";

    out << "" << endl;
    out << "// This file is meant to be read and created by a program other than a" << endl;
    out << "// compiler.  If you must modify it by hand, at least be nice to the " << endl;
    out << "// parser and don't add anything else to this file or rearrange it." << endl;
    out << "" << endl;
    out << "int numClipCases"<<lower<<" = " << ncases << ";" << endl;
    out << "" << endl;

    out << "int numClipShapes"<<lower<<"["<<ncases<<"] = {" << endl;
    for (int c=0; c<ncases; c++)
    {
        out << "  " << (datasets[c]->shapes.size()-1);
        if (c < ncases-1)
            out << ",";
        else
            out << " ";
        if (((c+1)%8)==0)
        {
            out << " // cases "<< int(c/8)*8 << " - " << c << "\n";
        }
        else
        {
            out << "";
        }
    }
    out << "};" << endl;
    out << "" << endl;


    out << "int startClipShapes"<<lower<<"["<<ncases<<"] = {" << endl;
    for (int c=0; c<ncases; c++)
    {
        int index = 0;
        for (int c2=0; c2<c; c2++)
        {
            DataSet *d = datasets[c2];
            for (int i=1; i<d->shapes.size(); i++)
            {
                Shape *s = &(d->shapes[i]);
                index += 2 + s->nverts;
                if (s->shapeType==ST_POINT)
                    index += 2;
            }
        }
        if ((c%8)==0)
            out << "  ";
        out << index;
        if (c < ncases-1)
            out << ",";
        else
            out << " ";
        if (((c+1)%8)==0)
        {
            out << " // cases "<< int(c/8)*8 << " - " << c << "\n";
        }
        else
        {
            out << " ";
        }
    }
    out << "};" << endl;
    out << "" << endl;

    out << "unsigned char clipShapes"<<lower<<"[] = {" << endl;
    int uniquecounter = 0;
    for (int c=0; c<ncases; c++)
    {
        out << " // Case #"<<c<<": ";
        DataSet *d = datasets[c];
        if (d->copyOfDataset)
        {
            out << "(cloned #"<<d->copyOfIndex<<")\n";
        }
        else
        {
            out << "Unique case #"<<(++uniquecounter)<<"\n";
        }

        int ptcounter = 0;
        for (int i=1; i<d->shapes.size(); i++)
        {
            Shape *s = &(d->shapes[i]);
            if (s->shapeType == ST_POINT)
            {
                out << "  ST_PNT, "<<ptcounter<<", ";
                if (s->color==0)
                    out << "COLOR0, ";
                else if (s->color==1)
                    out << "COLOR1, ";
                else if (s->color==2)
                    out << "NOCOLOR, ";
                else
                {
                    cerr << "bad color for point\n";
                }
                out <<s->nverts<<", ";
                for (int j=0; j<s->nverts; j++)
                {
                    out << NodeToStr(s->parentNodes[j]) << ", ";
                }
                out << endl;
                ptcounter++;
            }
        }

        for (int pass = 0; pass <= 1 ; pass++)
        {
            for (int i=1; i<d->shapes.size(); i++)
            {
                Shape *s = &(d->shapes[i]);
                if (s->color != pass)
                    continue;

                if (s->shapeType != ST_POINT)
                {
                    switch (s->shapeType)
                    {
                      case ST_HEX:     out << "  ST_HEX, "; break;
                      case ST_VOXEL:   out << "  ST_VOX, "; break;
                      case ST_WEDGE:   out << "  ST_WDG, "; break;
                      case ST_PYRAMID: out << "  ST_PYR, "; break;
                      case ST_TET:     out << "  ST_TET, "; break;
                      case ST_QUAD:    out << "  ST_QUA, "; break;
                      case ST_PIXEL:   out << "  ST_PIX, "; break;
                      case ST_TRIANGLE:out << "  ST_TRI, "; break;
                      case ST_LINE:    out << "  ST_LIN, "; break;
                      case ST_VERTEX:  out << "  ST_VTX, "; break;
                      default: cerr << "Error\n";
                    }

                    out << "COLOR"<<s->color<<", ";

                    for (int j=0; j<s->nverts; j++)
                    {
                        out << NodeToStr(s->parentNodes[j]) << ", ";
                    }
                    out << endl;
                }
            }
        }
    }
    out << " // Dummy\n";
    out << "  0\n";
    out << "};" << endl;
    out << "" << endl;
    out.close();

    cerr << "Saved!\n";
}
