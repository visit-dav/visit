// ----------------------------------------------------------------------------
// File:  ClipEditor.C
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
// ----------------------------------------------------------------------------

#include "ClipEditor.h"

#include "Shape.h"
#include "Viewer.h"

#include <stdlib.h>
#include <fstream.h>

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

ClipEditor::ClipEditor(const QString &st,
                     QWidget *parent, const QString &name) :
        QMainWindow(parent, name)
{
    if (st.left(3) == "hex")
    {
        ncases    = 256;
        shapetype = ST_HEX;
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
                    datasets[caseindex]->shapes[datasets[caseindex]->selectedShape].color = 
                        1 - datasets[caseindex]->shapes[datasets[caseindex]->selectedShape].color;
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
                cerr << "Adding new shape, please choose Hex, Wedge, Pyramid, Tet, or NewPoint\n";
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
                    vector<Shape> &shapes = datasets[caseindex]->shapes;
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
      case ST_WEDGE:   lower="Wdg"; upper="WDG"; break;
      case ST_PYRAMID: lower="Pyr"; upper="PYR"; break;
      case ST_TET:     lower="Tet"; upper="TET"; break;
      default: cerr << "Error\n"; break;
    }
    sprintf(fname, "ClipCases%s.C", lower);

    ifstream in(fname, ios::in);
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
        int sz;
        in >> buff;
        if (buff[0] == '/')
        {
            in.getline(buff, 200);
            in >> buff;
        }
        if (buff[strlen(buff)-1] == ',') buff[strlen(buff)-1] = '\0';
        sz = atoi(buff);
        datasets[i]->shapes.resize(sz+1);
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
        DataSet *d = datasets[i];
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
            else if (!strcmp(buff,"ST_PNT"))
            {
                st=ST_POINT;
                in >> buff; // point id
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
                    cerr << "Bad file: bad color!\n";
                    exit(4);
                }
            }

            for (int i=0; i<nv; i++)
            {
                in >> buff;
                if (buff[strlen(buff)-1] == ',') buff[strlen(buff)-1] = '\0';
                pts[i] = StrToNode(buff);
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
      case ST_WEDGE:   lower="Wdg"; upper="WDG"; break;
      case ST_PYRAMID: lower="Pyr"; upper="PYR"; break;
      case ST_TET:     lower="Tet"; upper="TET"; break;
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
                    index++;
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
                out << "  ST_PNT, "<<ptcounter<<", "<<s->nverts<<", ";
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
                      case ST_WEDGE:   out << "  ST_WDG, "; break;
                      case ST_PYRAMID: out << "  ST_PYR, "; break;
                      case ST_TET:     out << "  ST_TET, "; break;
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
