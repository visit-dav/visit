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
// File:  Shape.C
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
//    Added Line and Vertex shapes.  Added error messages for default 
//    cases in switch statements.
//
// ----------------------------------------------------------------------------

#include "Shape.h"
#include "Vector.h"
#include <qgl.h>
#include "Font.h"
#include "Transforms.h"

#include "../../visit_vtk/lightweight/vtkTriangulationTables.C"
#include "DataSet.h"

using std::vector;
using std::swap;

int  Shape::duplicateFacesRemoval = 0;
bool Shape::lighting = false;
bool Shape::numbering = true;

int triangleFaces[1][3] = {{0,1,2}};
int quadFaces[1][4] = {{0,1,2,3}};
int pixelFaces[1][4] = {{0,1,3,2}};

void
DrawFuzzyPoint(float x,float y,float z,float d)
{
    glVertex3f(x-d, y,   z);
    glVertex3f(x+d, y,   z);

    glVertex3f(x,   y-d, z);
    glVertex3f(x,   y+d, z);

    glVertex3f(x,   y,   z-d);
    glVertex3f(x,   y,   z+d);

    glVertex3f(x-d, y-d, z-d);
    glVertex3f(x+d, y+d, z+d);

    glVertex3f(x-d, y-d, z+d);
    glVertex3f(x+d, y+d, z-d);

    glVertex3f(x-d, y+d, z-d);
    glVertex3f(x+d, y-d, z+d);

    glVertex3f(x-d, y+d, z+d);
    glVertex3f(x+d, y-d, z-d);
}

Shape::Shape(ShapeType st, int sc, DataSet *ds)
{
    shapeType   = st;
    parentShape = NULL;
    dataset     = ds;
    color       = 0;

    switch (shapeType)
    {
      case ST_HEX:
        nverts = 8;

        xc[0] = -1;        yc[0] = -1;        zc[0] = -1;
        xc[1] =  1;        yc[1] = -1;        zc[1] = -1;
        xc[2] =  1;        yc[2] = -1;        zc[2] =  1;
        xc[3] = -1;        yc[3] = -1;        zc[3] =  1;
        xc[4] = -1;        yc[4] =  1;        zc[4] = -1;
        xc[5] =  1;        yc[5] =  1;        zc[5] = -1;
        xc[6] =  1;        yc[6] =  1;        zc[6] =  1;
        xc[7] = -1;        yc[7] =  1;        zc[7] =  1;

        nedges = 12;
        edges = hexVerticesFromEdges;

        ntris = 0;
        tris = NULL;

        nquads = 6;
        quads = hexQuadFaces;
        break;

      case ST_VOXEL:
        nverts = 8;

        xc[0] = -1;        yc[0] = -1;        zc[0] = -1;
        xc[1] =  1;        yc[1] = -1;        zc[1] = -1;
        xc[2] = -1;        yc[2] = -1;        zc[2] =  1;
        xc[3] =  1;        yc[3] = -1;        zc[3] =  1;
        xc[4] = -1;        yc[4] =  1;        zc[4] = -1;
        xc[5] =  1;        yc[5] =  1;        zc[5] = -1;
        xc[6] = -1;        yc[6] =  1;        zc[6] =  1;
        xc[7] =  1;        yc[7] =  1;        zc[7] =  1;

        nedges = 12;
        edges = voxVerticesFromEdges;

        ntris = 0;
        tris = NULL;

        nquads = 6;
        quads = voxQuadFaces;
        break;

      case ST_TET:
        nverts = 4;

        xc[0] = -1;        yc[0] = -.8;        zc[0] = -.8;
        xc[1] =  1;        yc[1] = -.8;        zc[1] = -.8;
        xc[2] =  0;        yc[2] = -.8;        zc[2] =  1;
        xc[3] =  0;        yc[3] =  1;         zc[3] =  0;

        nedges = 6;
        edges = tetVerticesFromEdges;

        ntris = 4;
        tris = tetTriangleFaces;

        nquads = 0;
        quads = NULL;
        break;

      case ST_PYRAMID:
        nverts = 5;

        xc[0] = -1;        yc[0] = -1;        zc[0] = -1;
        xc[1] =  1;        yc[1] = -1;        zc[1] = -1;
        xc[2] =  1;        yc[2] = -1;        zc[2] =  1;
        xc[3] = -1;        yc[3] = -1;        zc[3] =  1;
        xc[4] =  0;        yc[4] =  1;        zc[4] =  0;

        nedges = 8;
        edges = pyramidVerticesFromEdges;

        ntris = 4;
        tris = pyramidTriangleFaces;

        nquads = 1;
        quads = pyramidQuadFaces;
        break;

      case ST_WEDGE:
        nverts = 6;

        xc[0] = -1;        yc[0] =  1;        zc[0] = -1;
        xc[1] =  1;        yc[1] =  1;        zc[1] = -1;
        xc[2] =  0;        yc[2] =  1;        zc[2] =  1;
        xc[3] = -1;        yc[3] = -1;        zc[3] = -1;
        xc[4] =  1;        yc[4] = -1;        zc[4] = -1;
        xc[5] =  0;        yc[5] = -1;        zc[5] =  1;

        nedges = 9;
        edges = wedgeVerticesFromEdges;

        ntris = 2;
        tris = wedgeTriangleFaces;

        nquads = 3;
        quads = wedgeQuadFaces;
        break;

      case ST_QUAD:
        nverts = 4;

        xc[0] = -1;        yc[0] =  -1;        zc[0] =  0;
        xc[1] =  1;        yc[1] =  -1;        zc[1] =  0;
        xc[2] =  1;        yc[2] =   1;        zc[2] =  0;
        xc[3] = -1;        yc[3] =   1;        zc[3] =  0;

        nedges = 4;
        edges = quadVerticesFromEdges;

        ntris = 0;
        tris = NULL;

        nquads = 1;
        quads = quadFaces;
        break;

      case ST_PIXEL:
        nverts = 4;

        xc[0] = -1;        yc[0] =  -1;        zc[0] =  0;
        xc[1] =  1;        yc[1] =  -1;        zc[1] =  0;
        xc[2] = -1;        yc[2] =   1;        zc[2] =  0;
        xc[3] =  1;        yc[3] =   1;        zc[3] =  0;

        nedges = 4;
        edges = pixelVerticesFromEdges;

        ntris = 0;
        tris = NULL;

        nquads = 1;
        quads = pixelFaces;
        break;

      case ST_TRIANGLE:
        nverts = 3;

        xc[0] = -1;        yc[0] =  -1;        zc[0] =  0;
        xc[1] =  1;        yc[1] =  -1;        zc[1] =  0;
        xc[2] =  0;        yc[2] =   1;        zc[2] =  0;

        nedges = 3;
        edges = triVerticesFromEdges;

        ntris = 1;
        tris = triangleFaces;

        nquads = 0;
        quads = NULL;
        break;

      case ST_LINE:
        nverts = 2;

        xc[0] =  0;        yc[0] =  -1;        zc[0] =  0;
        xc[1] =  0;        yc[1] =  +1;        zc[1] =  0;

        nedges = 1;
        edges = lineVerticesFromEdges;

        ntris = 0;
        tris = NULL;

        nquads = 0;
        quads = NULL;
        break;

      case ST_VERTEX:
        nverts = 1;

        xc[0] =  0;        yc[0] =   0;        zc[0] =  0;

        nedges = 0;
        edges = NULL;

        ntris = 0;
        tris = NULL;

        nquads = 0;
        quads = NULL;
        break;

      case ST_POINT:
        cerr << "Error\n";
        exit(1);

      default:
        cerr << "Error\n";
        exit(1);
    }

    splitCase = sc;

    Init();
}

Shape::Shape(ShapeType st, Shape *parent, int c, int n, const char *nodes, DataSet *ds)
{
    shapeType   = st;
    parentShape = parent;
    dataset     = ds;
    color       = c;

    switch (shapeType)
    {
      case ST_HEX:
        nverts = 8;
        nedges = 12;
        edges  = hexVerticesFromEdges;
        ntris  = 0;
        tris   = NULL;
        nquads = 6;
        quads  = hexQuadFaces;
        break;

      case ST_VOXEL:
        nverts = 8;
        nedges = 12;
        edges  = voxVerticesFromEdges;
        ntris  = 0;
        tris   = NULL;
        nquads = 6;
        quads  = voxQuadFaces;
        break;

      case ST_TET:
        nverts = 4;
        nedges = 6;
        edges  = tetVerticesFromEdges;
        ntris  = 4;
        tris   = tetTriangleFaces;
        nquads = 0;
        quads  = NULL;
        break;

      case ST_PYRAMID:
        nverts = 5;
        nedges = 8;
        edges  = pyramidVerticesFromEdges;
        ntris  = 4;
        tris   = pyramidTriangleFaces;
        nquads = 1;
        quads  = pyramidQuadFaces;
        break;

      case ST_WEDGE:
        nverts = 6;
        nedges = 9;
        edges  = wedgeVerticesFromEdges;
        ntris  = 2;
        tris   = wedgeTriangleFaces;
        nquads = 3;
        quads  = wedgeQuadFaces;
        break;

      case ST_QUAD:
        nverts = 4;
        nedges = 4;
        edges  = quadVerticesFromEdges;
        ntris  = 0;
        tris   = NULL;
        nquads = 1;
        quads  = quadFaces;
        break;

      case ST_PIXEL:
        nverts = 4;
        nedges = 4;
        edges  = pixelVerticesFromEdges;
        ntris  = 0;
        tris   = NULL;
        nquads = 1;
        quads  = pixelFaces;
        break;

      case ST_TRIANGLE:
        nverts = 3;
        nedges = 3;
        edges  = triVerticesFromEdges;
        ntris  = 1;
        tris   = triangleFaces;
        nquads = 0;
        quads  = NULL;
        break;

      case ST_POINT:
        nverts = n;
        nedges = 0;
        ntris  = 0;
        nquads = 0;
        break;

      case ST_LINE:
        nverts = 2;
        nedges = 1;
        edges  = lineVerticesFromEdges;
        ntris  = 0;
        tris   = NULL;
        nquads = 0;
        quads  = NULL;
        break;

      case ST_VERTEX:
        nverts = 1;
        nedges = 0;
        edges  = NULL;;
        ntris  = 0;
        tris   = NULL;
        nquads = 0;
        quads  = NULL;
        break;

      default:
        cerr << "ERROR";
        exit(1);
    }

    splitCase = 0;

    for (int i=0; i<nverts; i++)
    {
        parentNodes[i] = nodes[i];
    }

    Init();
}

void
Shape::Init()
{
    valid = true;

    //
    xcent = 0;
    ycent = 0;
    zcent = 0;
    if (parentShape)
    {
        for (int i=0; i<nverts; i++)
        {
            char n = parentNodes[i];
            if (n>='0' && n<='9')
            {
                int index = n - '0';
                xc[i] = parentShape->xc[index];
                yc[i] = parentShape->yc[index];
                zc[i] = parentShape->zc[index];
            }
            else if (n>='a' && n<='l')
            {
                int index = n - 'a';
                xc[i] = parentShape->exc[index];
                yc[i] = parentShape->eyc[index];
                zc[i] = parentShape->ezc[index];
                if (!parentShape->edgepointsvalid[index])
                {
                    valid = false;
                    cerr << "Error: used invalid point index " << n << "\n";
                }
            }
            else if (n>='p' && n<='z')
            {
                xc[i] = .5;
                yc[i] = .5;
                zc[i] = .5;
                int index = (n - 'p') + 1;
                int found = 0;
                for (int s=0; s<dataset->shapes.size() && found<index; s++)
                {
                    if (dataset->shapes[s].shapeType == ST_POINT)
                    {
                        found++;
                        if (found == index)
                        {
                            xc[i] = dataset->shapes[s].xcent;
                            yc[i] = dataset->shapes[s].ycent;
                            zc[i] = dataset->shapes[s].zcent;
                        }
                    }
                }
                if (found < index)
                    valid = false;
            }
            xcent += xc[i];
            ycent += yc[i];
            zcent += zc[i];
        }
    }
    xcent /= float(nverts);
    ycent /= float(nverts);
    zcent /= float(nverts);

    for (int i=0; i<nedges; i++)
    {
        exc[i] = (xc[edges[i][0]] + xc[edges[i][1]]) / 2.;
        eyc[i] = (yc[edges[i][0]] + yc[edges[i][1]]) / 2.;
        ezc[i] = (zc[edges[i][0]] + zc[edges[i][1]]) / 2.;
    }

    for (int i=0; i<nverts; i++)
    {
        pointcase[i] = (splitCase & (1<<i));
    }

    for (int i=0; i<nedges; i++)
    {
        edgepointsvalid[i] = pointcase[edges[i][0]] != pointcase[edges[i][1]];
    }
}

void
Shape::DrawPolyData(Vector &up, Vector &right)
{
    glColor4f(1.,1.,1.,1.);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    for (int i=0; i<nedges; i++)
    {
        glVertex3f(xc[edges[i][0]],yc[edges[i][0]],zc[edges[i][0]]);
        glVertex3f(xc[edges[i][1]],yc[edges[i][1]],zc[edges[i][1]]);
    }
    glEnd();

    if (!parentShape && numbering)
    {
        glDisable(GL_LIGHTING);
        //glEnable(GL_POLYGON_SMOOTH);
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);
        //glDisable(GL_DEPTH_TEST);
        glBegin(GL_TRIANGLES);
        glColor4f(1.,.7,1.,.5);
        for (int i=0; i<nedges; i++)
        {
            DrawCharacter('A'+i, .08, Vector(exc[i],eyc[i],ezc[i])+(.02*up)+(0.02*right),
                          up, right);
        }
        glColor4f(1.,1.,.6,.5);
        for (int i=0; i<nverts; i++)
        {
            DrawCharacter('0'+i, .08, Vector(xc[i],yc[i],zc[i])+(.02*up)+(0.02*right),
                          up, right);
        }
        glEnd();
        //glDisable(GL_BLEND);
        //glDisable(GL_POLYGON_SMOOTH);
        //glEnable(GL_DEPTH_TEST);
    }

    if (!parentShape)
    {
        const float d = .025;

        glColor4f(1.,0.,0.,1.);
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        for (int i=0; i<nedges; i++)
        {
            if (edgepointsvalid[i])
            {
                DrawFuzzyPoint(exc[i], eyc[i], ezc[i], d);
            }
        }
        glEnd();

        if (dataset->selectedShape == 0)
        {
            glColor4f(.4,.4,1.,1.);
            glDisable(GL_LIGHTING);
            glBegin(GL_LINES);
            for (int i=0; i<nverts; i++)
            {
                if (!pointcase[i])
                {
                    DrawFuzzyPoint(xc[i], yc[i], zc[i], d);
                }
            }
            glEnd();

            glColor4f(.4,1.,.4,1.);
            glDisable(GL_LIGHTING);
            glBegin(GL_LINES);
            for (int i=0; i<nverts; i++)
            {
                if (pointcase[i])
                {
                    DrawFuzzyPoint(xc[i], yc[i], zc[i], d);
                }
            }
            glEnd();
        }

        return;
    }

    if (shapeType == ST_POINT)
    {
        const float d = .025;

        if (&(dataset->shapes[dataset->selectedShape]) == this)
        {
            glColor4f(1,.5,0,1);
            glDisable(GL_LIGHTING);
            glBegin(GL_LINES);
            for (int i=0; i<nverts; i++)
            {
                DrawFuzzyPoint(xc[i]+d*.2, yc[i]+d*.2, zc[i]+d*.2, d);
            }
            glEnd();
        }

        glColor4f(1,1,0,1);
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        DrawFuzzyPoint(xcent, ycent, zcent, d);
        glEnd();

        if (color==0)
        {
            glColor4f(0,0,1,1);
        }
        else if (color==1)
        {
            glColor4f(0,1,0,1);
        }
        else
        {
            glColor4f(1,1,0,1);
        }
        glBegin(GL_LINES);
        for (int i=0; i<nverts; i++)
        {
            DrawFuzzyPoint(xcent+d*.2, ycent+d*.2, zcent+d*.2, d);
        }
        glEnd();

    }


    float goodcolor0[] = {.5, .5, .8, .2};
    float goodcolor1[] = {.5, .8, .5, .2};
    float badcolor[]  = {1,  0, 0, .2};

    if (valid)
    {
        if (color==0)
        {
            glColor4fv(goodcolor0);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, goodcolor0);
        }
        else
        {
            glColor4fv(goodcolor1);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, goodcolor1);
        }
    }
    else
    {
        glColor4fv(badcolor);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, badcolor);
    }

    //glColor3f(.5, .5, .8);
    if (lighting)
    {
        glEnable(GL_LIGHTING);
    }
    else
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glBegin(GL_TRIANGLES);
    for (int i=0; i<ntris; i++)
    {
        int i0 = tris[i][0];
        int i1 = tris[i][1];
        int i2 = tris[i][2];

        char ci0 = parentNodes[tris[i][0]];
        char ci1 = parentNodes[tris[i][1]];
        char ci2 = parentNodes[tris[i][2]];

        bool duplicate = false;
        for (int s=1; s<dataset->shapes.size() && !duplicate; s++)
        {
            if (&dataset->shapes[s] == this)
                continue;

            for (int j=0; j<dataset->shapes[s].ntris && !duplicate; j++)
            {
                int j0 = dataset->shapes[s].tris[j][0];
                int j1 = dataset->shapes[s].tris[j][1];
                int j2 = dataset->shapes[s].tris[j][2];

                char cj0 = dataset->shapes[s].parentNodes[j0];
                char cj1 = dataset->shapes[s].parentNodes[j1];
                char cj2 = dataset->shapes[s].parentNodes[j2];

                if ((ci0==cj2 && ci1==cj1 && ci2==cj0) ||
                    (ci0==cj1 && ci1==cj0 && ci2==cj2) ||
                    (ci0==cj0 && ci1==cj2 && ci2==cj1))
                {
                    if (duplicateFacesRemoval != 2 ||
                        color != dataset->shapes[s].color)
                        duplicate = true;
                }
            }
        }
        if (( duplicate && duplicateFacesRemoval==0) ||
            (!duplicate && duplicateFacesRemoval==2))
        {
            continue;
        }

        Vector p1(xc[i0], yc[i0], zc[i0]);
        Vector p2(xc[i1], yc[i1], zc[i1]);
        Vector p3(xc[i2], yc[i2], zc[i2]);

        Vector v12 = p2 - p1;
        Vector v13 = p3 - p1;
        Vector normal = (v12 % v13).normalized();

        glNormal3f(normal.x, normal.y, normal.z);
        glVertex3f(p1.x,p1.y,p1.z);
        glVertex3f(p2.x,p2.y,p2.z);
        glVertex3f(p3.x,p3.y,p3.z);
    }
    glEnd();

    glBegin(GL_QUADS);
    for (int i=0; i<nquads; i++)
    {
        int i0 = quads[i][0];
        int i1 = quads[i][1];
        int i2 = quads[i][2];
        int i3 = quads[i][3];

        char ci0 = parentNodes[quads[i][0]];
        char ci1 = parentNodes[quads[i][1]];
        char ci2 = parentNodes[quads[i][2]];
        char ci3 = parentNodes[quads[i][3]];

        bool duplicate = false;
        for (int s=1; s<dataset->shapes.size() && !duplicate; s++)
        {
            if (&dataset->shapes[s] == this)
                continue;

            for (int j=0; j<dataset->shapes[s].nquads && !duplicate; j++)
            {
                int j0 = dataset->shapes[s].quads[j][0];
                int j1 = dataset->shapes[s].quads[j][1];
                int j2 = dataset->shapes[s].quads[j][2];
                int j3 = dataset->shapes[s].quads[j][3];

                char cj0 = dataset->shapes[s].parentNodes[j0];
                char cj1 = dataset->shapes[s].parentNodes[j1];
                char cj2 = dataset->shapes[s].parentNodes[j2];
                char cj3 = dataset->shapes[s].parentNodes[j3];

                if ((ci0==cj3 && ci1==cj2 && ci2==cj1 && ci3==cj0) ||
                    (ci0==cj2 && ci1==cj1 && ci2==cj0 && ci3==cj3) ||
                    (ci0==cj1 && ci1==cj0 && ci2==cj3 && ci3==cj2) ||
                    (ci0==cj0 && ci1==cj3 && ci2==cj2 && ci3==cj1))
                {
                    if (duplicateFacesRemoval != 2 ||
                        color != dataset->shapes[s].color)
                        duplicate = true;
                }
            }
        }
        if (( duplicate && duplicateFacesRemoval==0) ||
            (!duplicate && duplicateFacesRemoval==2))
        {
            continue;
        }

        Vector p1(xc[i0], yc[i0], zc[i0]);
        Vector p2(xc[i1], yc[i1], zc[i1]);
        Vector p3(xc[i2], yc[i2], zc[i2]);
        Vector p4(xc[i3], yc[i3], zc[i3]);

        Vector v12 = p2 - p1;
        Vector v13 = p3 - p1;
        Vector normal = (v12 % v13).normalized();

        glNormal3f(normal.x, normal.y, normal.z);
        glVertex3f(p1.x,p1.y,p1.z);
        glVertex3f(p2.x,p2.y,p2.z);
        glVertex3f(p3.x,p3.y,p3.z);
        glVertex3f(p4.x,p4.y,p4.z);
    }
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void
Shape::Invert()
{
    switch (shapeType)
    {
      case ST_HEX:
      case ST_VOXEL:
        swap(parentNodes[0],parentNodes[4]);
        swap(parentNodes[1],parentNodes[5]);
        swap(parentNodes[2],parentNodes[6]);
        swap(parentNodes[3],parentNodes[7]);
        break;

      case ST_TET:
        swap(parentNodes[1],parentNodes[2]);
        break;

      case ST_PYRAMID:
        swap(parentNodes[1],parentNodes[3]);
        break;

      case ST_WEDGE:
        swap(parentNodes[0],parentNodes[3]);
        swap(parentNodes[1],parentNodes[4]);
        swap(parentNodes[2],parentNodes[5]);
        break;

      case ST_TRIANGLE:
        swap(parentNodes[0],parentNodes[2]);
        break;

      case ST_QUAD:
        swap(parentNodes[0],parentNodes[3]);
        swap(parentNodes[1],parentNodes[2]);
        break;

      case ST_PIXEL:
        swap(parentNodes[1],parentNodes[2]);
        break;

      case ST_POINT:
        break;

      default:
        cerr << "ERROR";
        exit(1);
    }

    Init();
}


int
Shape::CheckCopyOf(Shape *s)
{
    int ncases;
    int i;
    switch (shapeType)
    {
      case ST_HEX:
        ncases = 48;
        for (i=0; i<ncases; i++)
        {
            bool okay = true;
            for (int p=0; p<8; p++)
            {
                if (s->pointcase[p] !=
                    this->pointcase[hexTransforms[i].n[p]])
                {
                    okay = false;
                }
            }
            if (okay)
            {
                return i;
            }
        }
        break;

      case ST_VOXEL:
        ncases = 48;
        for (i=0; i<ncases; i++)
        {
            bool okay = true;
            for (int p=0; p<8; p++)
            {
                if (s->pointcase[p] !=
                    this->pointcase[voxTransforms[i].n[p]])
                {
                    okay = false;
                }
            }
            if (okay)
            {
                return i;
            }
        }
        break;

      case ST_TET:
        ncases = 24;
        for (i=0; i<ncases; i++)
        {
            bool okay = true;
            for (int p=0; p<4; p++)
            {
                if (s->pointcase[p] !=
                    this->pointcase[tetTransforms[i].n[p]])
                {
                    okay = false;
                }
            }
            if (okay)
            {
                return i;
            }
        }
        break;

      case ST_PYRAMID:
        ncases = 8;
        for (i=0; i<ncases; i++)
        {
            bool okay = true;
            for (int p=0; p<5; p++)
            {
                if (s->pointcase[p] !=
                    this->pointcase[pyramidTransforms[i].n[p]])
                {
                    okay = false;
                }
            }
            if (okay)
            {
                return i;
            }
        }
        break;

      case ST_WEDGE:
        ncases = 12;
        for (i=0; i<ncases; i++)
        {
            bool okay = true;
            for (int p=0; p<6; p++)
            {
                if (s->pointcase[p] !=
                    this->pointcase[wedgeTransforms[i].n[p]])
                {
                    okay = false;
                }
            }
            if (okay)
            {
                return i;
            }
        }
        break;

      case ST_QUAD:
        ncases = 8;
        for (i=0; i<ncases; i++)
        {
            bool okay = true;
            for (int p=0; p<4; p++)
            {
                if (s->pointcase[p] !=
                    this->pointcase[quadTransforms[i].n[p]])
                {
                    okay = false;
                }
            }
            if (okay)
            {
                return i;
            }
        }
        break;

      case ST_PIXEL:
        ncases = 8;
        for (i=0; i<ncases; i++)
        {
            bool okay = true;
            for (int p=0; p<4; p++)
            {
                if (s->pointcase[p] !=
                    this->pointcase[pixelTransforms[i].n[p]])
                {
                    okay = false;
                }
            }
            if (okay)
            {
                return i;
            }
        }
        break;

      case ST_TRIANGLE:
        ncases = 6;
        for (i=0; i<ncases; i++)
        {
            bool okay = true;
            for (int p=0; p<3; p++)
            {
                if (s->pointcase[p] !=
                    this->pointcase[triTransforms[i].n[p]])
                {
                    okay = false;
                }
            }
            if (okay)
            {
                return i;
            }
        }
        break;

      case ST_POINT:
        cerr << "Error\n";
        exit(1);
        break;

      case ST_LINE:
        ncases = 2;
        for (i=0; i<ncases; i++)
        {
            bool okay = true;
            for (int p=0; p<2; p++)
            {
                if (s->pointcase[p] !=
                    this->pointcase[lineTransforms[i].n[p]])
                {
                    okay = false;
                }
            }
            if (okay)
            {
                return i;
            }
        }
        break;

      case ST_VERTEX:
        // no copied cases for vertices; just fall through
        break;

      default:
        cerr << "Error\n";
        exit(1);
        break;
    }

    return -1;
}

Shape::Shape(Shape *copy, int xformID, Shape *parent, DataSet *ds)
{
    shapeType   = copy->shapeType;
    parentShape = parent;
    dataset     = ds;
    color       = copy->color;
    nverts = copy->nverts;
    nedges = copy->nedges;
    edges  = copy->edges;
    ntris  = copy->ntris;
    tris   = copy->tris;
    nquads = copy->nquads;
    quads  = copy->quads;

    splitCase = 0;

    switch (parent->shapeType)
    {
      case ST_HEX:
        MakeCopyOf(copy, hexTransforms[xformID]);
        break;

      case ST_VOXEL:
        MakeCopyOf(copy, voxTransforms[xformID]);
        break;

      case ST_TET:
        MakeCopyOf(copy, tetTransforms[xformID]);
        break;

      case ST_PYRAMID:
        MakeCopyOf(copy, pyramidTransforms[xformID]);
        break;

      case ST_WEDGE:
        MakeCopyOf(copy, wedgeTransforms[xformID]);
        break;

      case ST_QUAD:
        MakeCopyOf(copy, quadTransforms[xformID]);
        break;

      case ST_PIXEL:
        MakeCopyOf(copy, pixelTransforms[xformID]);
        break;

      case ST_TRIANGLE:
        MakeCopyOf(copy, triTransforms[xformID]);
        break;

      case ST_LINE:
        MakeCopyOf(copy, lineTransforms[xformID]);
        break;

      case ST_VERTEX:
        cerr << "Error: ST_VERTEX has no transformations.\n";
        exit(1);
        break;

      case ST_POINT:
        cerr << "Error: ST_POINT does not support transformations.\n";
        exit(1);
        break;

      default:
        cerr << "Error\n";
        exit(1);
        break;
    }

    Init();
}


void
Shape::MakeCopyOf(Shape *s, HexTransform &xform)
{
    for (int i=0; i<nverts; i++)
    {
        char c1 = s->parentNodes[i];
        char c2 = c1;
        if (c1 >= '0' && c1 <= '9')
        {
            c2 = xform.n[c1 - '0'] + '0';
        }
        else if (c1 >= 'a' && c1 <= 'l')
        {
            c2 = xform.e[c1 - 'a'];
        }
        parentNodes[i] = c2;
    }
    if (xform.f)
        Invert();
}

void
Shape::MakeCopyOf(Shape *s, VoxTransform &xform)
{
    for (int i=0; i<nverts; i++)
    {
        char c1 = s->parentNodes[i];
        char c2 = c1;
        if (c1 >= '0' && c1 <= '9')
        {
            c2 = xform.n[c1 - '0'] + '0';
        }
        else if (c1 >= 'a' && c1 <= 'l')
        {
            c2 = xform.e[c1 - 'a'];
        }
        parentNodes[i] = c2;
    }
    if (xform.f)
        Invert();
}

void
Shape::MakeCopyOf(Shape *s, WedgeTransform &xform)
{
    for (int i=0; i<nverts; i++)
    {
        char c1 = s->parentNodes[i];
        char c2 = c1;
        if (c1 >= '0' && c1 <= '9')
        {
            c2 = xform.n[c1 - '0'] + '0';
        }
        else if (c1 >= 'a' && c1 <= 'l')
        {
            c2 = xform.e[c1 - 'a'];
        }
        parentNodes[i] = c2;
    }
    if (xform.f)
        Invert();
}

void
Shape::MakeCopyOf(Shape *s, PyramidTransform &xform)
{
    for (int i=0; i<nverts; i++)
    {
        char c1 = s->parentNodes[i];
        char c2 = c1;
        if (c1 >= '0' && c1 <= '9')
        {
            c2 = xform.n[c1 - '0'] + '0';
        }
        else if (c1 >= 'a' && c1 <= 'l')
        {
            c2 = xform.e[c1 - 'a'];
        }
        parentNodes[i] = c2;
    }
    if (xform.f)
        Invert();
}

void
Shape::MakeCopyOf(Shape *s, TetTransform &xform)
{
    for (int i=0; i<nverts; i++)
    {
        char c1 = s->parentNodes[i];
        char c2 = c1;
        if (c1 >= '0' && c1 <= '9')
        {
            c2 = xform.n[c1 - '0'] + '0';
        }
        else if (c1 >= 'a' && c1 <= 'l')
        {
            c2 = xform.e[c1 - 'a'];
        }
        parentNodes[i] = c2;
    }
    if (xform.f)
        Invert();
}

void
Shape::MakeCopyOf(Shape *s, QuadTransform &xform)
{
    for (int i=0; i<nverts; i++)
    {
        char c1 = s->parentNodes[i];
        char c2 = c1;
        if (c1 >= '0' && c1 <= '9')
        {
            c2 = xform.n[c1 - '0'] + '0';
        }
        else if (c1 >= 'a' && c1 <= 'l')
        {
            c2 = xform.e[c1 - 'a'];
        }
        parentNodes[i] = c2;
    }
    if (xform.f)
        Invert();
}

void
Shape::MakeCopyOf(Shape *s, PixelTransform &xform)
{
    for (int i=0; i<nverts; i++)
    {
        char c1 = s->parentNodes[i];
        char c2 = c1;
        if (c1 >= '0' && c1 <= '9')
        {
            c2 = xform.n[c1 - '0'] + '0';
        }
        else if (c1 >= 'a' && c1 <= 'l')
        {
            c2 = xform.e[c1 - 'a'];
        }
        parentNodes[i] = c2;
    }
    if (xform.f)
        Invert();
}

void
Shape::MakeCopyOf(Shape *s, TriTransform &xform)
{
    for (int i=0; i<nverts; i++)
    {
        char c1 = s->parentNodes[i];
        char c2 = c1;
        if (c1 >= '0' && c1 <= '9')
        {
            c2 = xform.n[c1 - '0'] + '0';
        }
        else if (c1 >= 'a' && c1 <= 'l')
        {
            c2 = xform.e[c1 - 'a'];
        }
        parentNodes[i] = c2;
    }
    if (xform.f)
        Invert();
}

void
Shape::MakeCopyOf(Shape *s, LineTransform &xform)
{
    for (int i=0; i<nverts; i++)
    {
        char c1 = s->parentNodes[i];
        char c2 = c1;
        if (c1 >= '0' && c1 <= '9')
        {
            c2 = xform.n[c1 - '0'] + '0';
        }
        else if (c1 >= 'a' && c1 <= 'l')
        {
            c2 = xform.e[c1 - 'a'];
        }
        parentNodes[i] = c2;
    }
    if (xform.f)
        Invert();
}
