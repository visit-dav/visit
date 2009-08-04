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
// File:  Shape.h
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
//
// Modifications:
//    Jeremy Meredith, Thu Sep 18 11:29:12 PDT 2003
//    Added Quad and Triangle shapes.
//
//    Jeremy Meredith, Thu Jun 24 10:38:05 PDT 2004
//    Added Voxel and Pixel shapes.
//
//    Jeremy Meredith, Tue Aug 29 16:13:43 EDT 2006
//    Added Line and Vertex shapes.
//
// ----------------------------------------------------------------------------

#ifndef SHAPE_H
#define SHAPE_H

#include <vector>
#include "Vector.h"
#include "Transforms.h"

struct DataSet;

enum ShapeType
{
    ST_HEX,
    ST_VOXEL,
    ST_WEDGE,
    ST_PYRAMID,
    ST_TET,
    ST_QUAD,
    ST_TRIANGLE,
    ST_PIXEL,
    ST_VERTEX,
    ST_LINE,
    ST_POINT
};

struct Shape
{
    ShapeType shapeType;

    int nverts;
    float xc[8];
    float yc[8];
    float zc[8];

    int nedges;
    typedef int edge_t[2];
    edge_t *edges;

    int ntris;
    typedef int tri_t[3];
    tri_t *tris;

    int nquads;
    typedef int quad_t[4];
    quad_t *quads;

    bool pointcase[8];

    bool edgepointsvalid[12];
    float exc[12];
    float eyc[12];
    float ezc[12];

    int splitCase;
    Shape *parentShape;
    char parentNodes[8];

    float xcent;
    float ycent;
    float zcent;
    bool valid;

    int color;

    // Rendering options
    static int  duplicateFacesRemoval;
    static bool lighting;
    static bool numbering;

    // Methods
    Shape() {}
    Shape(ShapeType, int sc, DataSet *ds);
    Shape(ShapeType st, Shape *parent, const char *nodes, DataSet *ds);
    Shape(ShapeType st, Shape *parent, int c, int n, const char *nodes, DataSet *ds);
    Shape(Shape *copy, int xformID, Shape *parent, DataSet *ds);
    void DrawPolyData(Vector &up, Vector &right);
    void Init();
    void Invert();
    int  CheckCopyOf(Shape*);
    void MakeCopyOf(Shape *s, HexTransform &xform);
    void MakeCopyOf(Shape *s, VoxTransform &xform);
    void MakeCopyOf(Shape *s, WedgeTransform &xform);
    void MakeCopyOf(Shape *s, PyramidTransform &xform);
    void MakeCopyOf(Shape *s, TetTransform &xform);
    void MakeCopyOf(Shape *s, QuadTransform &xform);
    void MakeCopyOf(Shape *s, PixelTransform &xform);
    void MakeCopyOf(Shape *s, TriTransform &xform);
    void MakeCopyOf(Shape *s, LineTransform &xform);

    DataSet *dataset;
};


#endif
