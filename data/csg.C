/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <silo.h>
#include <math.h>

static void build_csg(DBfile *dbfile, char *name);
static void build_greenman_csg(DBfile *dbfile, char *name);
static void build_primitives_csg(DBfile *dbfile);

int
main(int argc, char *argv[])
{
    DBfile        *dbfile;
    int            i=1, driver = DB_PDB;

    DBShowErrors(DB_ALL, NULL);

    while (i < argc)
    {
        if (strcmp(argv[i], "-driver") == 0)
        {
            i++;

            if (strcmp(argv[i], "DB_HDF5") == 0)
            {
                driver = DB_HDF5;
            }
            else if (strcmp(argv[i], "DB_PDB") == 0)
            {
                driver = DB_PDB;
            }
            else
            {
               fprintf(stderr,"Uncrecognized driver name \"%s\"\n",
                   argv[i]);
               exit(-1);
            }
        }
        i++;
    }

    dbfile = DBCreate("csg.silo", 0, DB_LOCAL, "csg test file", driver);
    build_csg(dbfile, "csgmesh");
    build_greenman_csg(dbfile, "greenman_mesh");
    build_primitives_csg(dbfile);
    DBClose(dbfile);

    return 0;
}

static void
build_csg(DBfile *dbfile, char *name)
{
    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_SPHERE_PR,
            DBCSG_PLANE_X,
            DBCSG_PLANE_X,
            DBCSG_CYLINDER_PNLR,
            //DBCSG_QUADRIC_G,
            DBCSG_SPHERE_PR,
            DBCSG_SPHERE_PR
        };

        float coeffs[] =
        {
            0.0, 0.0, 0.0, 5.0,                // point-radius form of sphere
           -2.0,                               // x-intercept form of plane
            2.0,                               // x-intercept form of plane
           -10.0, 0.0, 0.0, 1.0, 0.0, 0.0, 20.0, 4.5, // point-normal-length-radius form of cylinder
          //  0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -20.25,
            0.0, 0.0, 49.5, 50.0,              // point-radius form of sphere
            0.0, 0.0, -49.5, 50.0              // point radius form of sphere
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-5.0, -5.0, -5.0, 5.0, 5.0, 5.0};

        DBPutCsgmesh(dbfile, name, 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "csgzl", NULL);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER,       // 0: inside of sphere
            DBCSG_OUTER,       // 1: in-front of -X plane
            DBCSG_INNER,       // 2: in-back of +X plane 
            DBCSG_INNER,       // 3: inside of quadric cylinder
            DBCSG_INTERSECT,   // 4: sphere cut to -X plane
            DBCSG_INTERSECT,   // 5: -X-cut-sphere cut to +X plane
            DBCSG_DIFF,        // 6: cut-sphere minus quadric cylinder
            DBCSG_INNER,       // 7: inside of large upper sphere
            DBCSG_INNER,       // 8: inside of large lower sphere
            DBCSG_INTERSECT,   // 9: intersection of large spheres
            DBCSG_INTERSECT,   //10: 9 cut to inside of quadric cylinder 
            DBCSG_INTERSECT    //11: 10 cut to inside of smaller sphere 
        };
        //                 0   1   2   3   4   5   6   7   8   9  10  11
        int leftids[] =  { 0,  1,  2,  3,  0,  4,  5,  4,  5,  7,  9, 10};
        int rightids[] = {-1, -1, -1, -1,  1,  2,  3, -1, -1,  8,  3, 0}; 
        int zonelist[] = {6, 11};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"ring housing", "lens-shaped fin"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);

        DBFreeOptlist(optlist);
    }

    // output a csg variable
    {
        void *pv[1];
        double var1_data[] = {10.0, 100.0};
        char *pname[1];
        char name1[] = "var1";

        pv[0] = var1_data;
        pname[0] = name1;

        DBPutCsgvar(dbfile, "var1", "csgmesh", 1, pname, pv, 2, DB_DOUBLE,
            DB_ZONECENT, 0);
    }

    // output a material for this csg mesh
    {
        int matnos[] = {2, 3};
        int matlist[] = {2, 3};
        int dims = 2;
        DBPutMaterial(dbfile, "mat", "csgmesh", 2,  matnos,  matlist, &dims, 1,
            0, 0, 0, 0, 0, DB_FLOAT, 0);
    }
}

// This mesh is based on some test data from Greg Greenman
static void
build_greenman_csg(DBfile *dbfile, char *name)
{
    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_QUADRIC_G,
            DBCSG_QUADRIC_G,
            DBCSG_QUADRIC_G,
            DBCSG_QUADRIC_G,
            DBCSG_QUADRIC_G,
            DBCSG_QUADRIC_G
        };

        const float c0 = .464102;
        const float c1 = .707107;
        const float c2 = -1.0718;
        float coeffs[] =
        {
         // x^2  y^2  z^2  xy   yz   xz    x    y    z    c
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,    // 0: "bottom" 
            1.0,  c0,  c0, 0.0,  c2, 0.0, 0.0, 0.0, 0.0, 0.0,    // 1: "cone"
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  c1,  c1, 0.0,    // 2: "cone_apex"
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  c1,  c1,-4.0,    // 3: "cone_end"
            1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -100.0, // 4: "cylinder"
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, -10.0   // 5: "top" 
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        //double extents[] = {0.0, 0.0, 0.0, 10.0, 10.0, 10.0};
        double extents[] = {-10.0, -10.0, -10.0, 20.0, 20.0, 20.0};

        char *bndnames[] = {"bottom", "cone", "cone_apex", "cone_end",
                            "cylinder", "top"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, name, 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "greenman_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_OUTER,          // 0: above bottom plane
            DBCSG_INNER,          // 1: below top plane
            DBCSG_INNER,          // 2: inside of cylinder
            DBCSG_INTERSECT,      // 3: between top/bottom
            DBCSG_INTERSECT,      // 4: cylinder clipped to between top/bottom
            DBCSG_OUTER,          // 5: outside of cylinder
            DBCSG_INTERSECT,      // 6: oustide of cylinder cliped to between top/bottom
            DBCSG_INNER,          // 7: inside of cone
            DBCSG_INNER,          // 8: below cone_end plane
            DBCSG_OUTER,          // 9: above cone_apex plane
            DBCSG_INTERSECT,      //10: between cone planes 
            DBCSG_INTERSECT,      //11: inside of cone between cone planes
            DBCSG_DIFF            //12: cylinder minus cone
        };
        //                 0   1   2   3   4   5   6   7   8   9  10  11  12
        int leftids[] =  { 0,  5,  4,  0,  3,  4,  5,  1,  3,  2,  8, 10,  4};
        int rightids[] = {-1, -1, -1,  1,  2, -1,  3, -1, -1, -1,  9,  7, 11};
        int zonelist[] = {6, 11, 12};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"void", "uranium", "air"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "greenman_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }
}

static void
build_primitives_csg(DBfile *dbfile)
{

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_SPHERE_PR,
            DBCSG_PLANE_X
        };

        float coeffs[] =
        {
            0.0, 0.0, 0.0, 1.0,               // point-radius form of sphere
            0.0                               // x-intercept form of plane
        };

        double extents[] = {-2.0, -2.0, -2.0, 2.0, 2.0, 2.0};

        char *bndnames[] = {"sphere", "plane"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "sphere", 3, 1, typeflags,
            NULL, coeffs, 4, DB_FLOAT, extents, "sphere_csgzl", optlist);

        DBPutCsgmesh(dbfile, "half_sphere", 3, 2, typeflags,
            NULL, coeffs, 5, DB_FLOAT, extents, "half_sphere_csgzl", optlist);

        DBPutCsgmesh(dbfile, "half_sphere_compliment", 3, 2, typeflags,
            NULL, coeffs, 5, DB_FLOAT, extents, "half_sphere_comp_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER,          // 0: inside of sphere 
            DBCSG_OUTER,          // 1: +x side of plane
            DBCSG_INTERSECT,      // 2: the +half sphere
            DBCSG_COMPLIMENT      // 3: the not of 2
        };
        //                 0   1   2   3
        int leftids[] =  { 0,  1,  1,  2};
        int rightids[] = {-1, -1,  0, -1};
        int zonelist1[] = {0};
        int zonelist2[] = {2};
        int zonelist3[] = {3};

        char *zonenames[] = {"region_1"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "sphere_csgzl", 1, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, 1, zonelist1, optlist);

        DBPutCSGZonelist(dbfile, "half_sphere_csgzl", 3, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, 1, zonelist2, optlist);

        DBPutCSGZonelist(dbfile, "half_sphere_comp_csgzl", 4, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, 1, zonelist3, optlist);
    }

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_CYLINDER_PNLR,      // 0: cylinderY
            DBCSG_CYLINDER_PNLR,      // 1: cylinderX
            DBCSG_CYLINDER_PNLR       // 2: cylinderZ
        };

        float coeffs[] =
        {
           //0.0, -10.0, 0.0, 0.707, 0.0, 0.707, 20.0, 4.5 point-normal-length-radius form of cylinder
         -10.0, 0.0, 0.0, 1.0, 0.0, 0.0, 20.0, 4.5, // point-normal-length-radius form of cylinder
           0.0, -10.0, 0.0, 0.0, 1.0, 0.0, 20.0, 4.5, // point-normal-length-radius form of cylinder
           0.0, 0.0, -10.0, 0.0, 0.0, 1.0, 20.0, 4.5  // point-normal-length-radius form of cylinder
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-10.0, -10.0, -10.0, 10.0, 10.0, 10.0};

        char *bndnames[] = {"cylinderY", "cylinderX", "cylinderZ"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "cylinders", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "cylinders_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER,          // 0: inside of cylinderY
            DBCSG_INNER,          // 1: inside of cylinderX 
            DBCSG_INNER,          // 2: inside of cylinderZ
            DBCSG_UNION,          // 3: 0 and 1 
            DBCSG_UNION           // 4: 2 and 3 
        };
        //                 0   1   2   3   4
        int leftids[] =  { 0,  1,  2,  0,  2};
        int rightids[] = {-1, -1, -1,  1,  3};
        int zonelist[] = {0, 1, 2, 4};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"cylinderY","cylinderX","cylinderZ","cylinderAll"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "cylinders_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_PLANE_PN
        };

        float coeffs[] =
        {
            1.0, 0.0, 0.0, 1.0, 0.0, 0.0  // point-normal form of plane
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-2.0, -2.0, -2.0, 3.0, 3.0, 3.0};

        char *bndnames[] = {"plane"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "plane_pn_X", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "plane_pn_X_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER,           // 0: inside (everything to left) of plane
            DBCSG_OUTER            // 1: outside (everything to right) of plane
        };
        //                 0
        int leftids[] =  { 0,  0};
        int rightids[] = {-1, -1};
        int zonelist[] = {0, 1};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"left-of-plane",
                             "right-of-plane"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "plane_pn_X_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_PLANE_X
        };

        float coeffs[] =
        {
            1.0  // x-intercept form of plane
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-2.0, -2.0, -2.0, 2.0, 2.0, 2.0};

        char *bndnames[] = {"plane"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "plane_X", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "plane_X_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER,          // 0: inside (everything to left) of plane
            DBCSG_OUTER           // 1: outside (everything to right) of plane
        };
        //                 0   1
        int leftids[] =  { 0,  0};
        int rightids[] = {-1, -1};
        int zonelist[] = { 0,  1};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"left-of-plane", "right-of-plane"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "plane_X_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_PLANE_Y
        };

        float coeffs[] =
        {
            1.0  // y-intercept form of plane
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-2.0, -2.0, -2.0, 2.0, 2.0, 2.0};

        char *bndnames[] = {"plane"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "plane_Y", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "plane_Y_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER,          // 0: inside (everything to bottom) of plane
            DBCSG_OUTER           // 1: outside (everything to top) of plane
        };
        //                 0   1
        int leftids[] =  { 0,  0};
        int rightids[] = {-1, -1};
        int zonelist[] = { 0,  1};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"bottom-of-plane", "top-of-plane"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "plane_Y_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_PLANE_Z
        };

        float coeffs[] =
        {
            1.0  // z-intercept form of plane
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-2.0, -2.0, -2.0, 2.0, 2.0, 2.0};

        char *bndnames[] = {"plane"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "plane_Z", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "plane_Z_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER,          // 0: inside (everything to back) of plane
            DBCSG_OUTER           // 1: outside (everything to front) of plane
        };
        //                 0   1
        int leftids[] =  { 0,  0};
        int rightids[] = {-1, -1};
        int zonelist[] = { 0,  1};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"back-of-plane", "front-of-plane"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "plane_Z_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_PLANE_X,
            DBCSG_PLANE_X,
            DBCSG_PLANE_Y,
            DBCSG_PLANE_Y,
            DBCSG_PLANE_Z,
            DBCSG_PLANE_Z
        };

        float coeffs[] =
        {
            1.0, -1.0,  // x-intercept form of plane
            1.0, -1.0,  // y-intercept form of plane
            1.0, -1.0   // z-intercept form of plane
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-2.0, -2.0, -2.0, 2.0, 2.0, 2.0};

        char *bndnames[] = {"XAplane","XBplane","YAplane","YBplane","ZAplane","ZBplane"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "Box", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "box_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER,          // 0: inside (everything to left) of plane
            DBCSG_OUTER,          // 1: outside (everything to right) of plane
            DBCSG_INNER,          // 2: inside (everything to left) of plane
            DBCSG_OUTER,          // 3: outside (everything to right) of plane
            DBCSG_INNER,          // 4: inside (everything to left) of plane
            DBCSG_OUTER,          // 5: outside (everything to right) of plane
            DBCSG_INTERSECT,      // 6: between X planes
            DBCSG_INTERSECT,      // 7: between Y planes
            DBCSG_INTERSECT,      // 8: between Z planes
            DBCSG_INTERSECT,      // 9: between XY planes
            DBCSG_INTERSECT,      //10: between XY & Z planes
        };
        //                 0   1   2   3   4   5   6   7   8   9   10
        int leftids[] =  { 0,  1,  2,  3,  4,  5,  0,  2,  4,  6,  8};
        int rightids[] = {-1, -1, -1, -1, -1, -1,  1,  3,  5,  7,  9};
        int zonelist[] = {10};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"box"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "box_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_PLANE_PN
        };

        float coeffs[] =
        {
            1.0, 1.0, 1.0, 0.707, 0.707, 0.0  // point-normal form of plane
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-2.0, -2.0, -2.0, 2.0, 2.0, 2.0};

        char *bndnames[] = {"plane"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "plane_PN", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "plane_PN_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER,          // 0: inside (everything to back) of plane
            DBCSG_OUTER           // 1: outside (everything to front) of plane
        };
        //                 0   1
        int leftids[] =  { 0,  0};
        int rightids[] = {-1, -1};
        int zonelist[] = { 0,  1};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"sideA", "sideB"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "plane_PN_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_QUADRIC_G
        };

        float coeffs[] =
        {
            0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -20.25 // cylinder
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-5.0, -5.0, -5.0, 5.0, 5.0, 5.0};

        char *bndnames[] = {"cylinder"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "quadric_cylinder", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "quadric_cylinder_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER,          // 0: inside (everything to back) of plane
            DBCSG_OUTER           // 1: outside (everything to front) of plane
        };
        //                 0   1
        int leftids[] =  { 0,  0};
        int rightids[] = {-1, -1};
        int zonelist[] = { 0,  1};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"inside", "outside"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "quadric_cylinder_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_QUADRIC_G
        };

        float coeffs[] =
        {
            1.0, 2.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -16.0 // spheriod
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-5.0, -5.0, -5.0, 5.0, 5.0, 5.0};

        char *bndnames[] = {"spheriod"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "quadric_spheriod", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "quadric_spheroid_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER,          // 0: inside (everything to back) of plane
            DBCSG_OUTER           // 1: outside (everything to front) of plane
        };
        //                 0   1
        int leftids[] =  { 0,  0};
        int rightids[] = {-1, -1};
        int zonelist[] = { 0,  1};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"inside", "outside"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "quadric_spheroid_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_QUADRIC_G
        };

        float coeffs[] =
        {
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, -1.0 // plane
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-5.0, -5.0, -5.0, 5.0, 5.0, 5.0};

        char *bndnames[] = {"plane"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "quadric_plane", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "quadric_plane_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER,          // 0: inside (everything to back) of plane
            DBCSG_OUTER           // 1: outside (everything to front) of plane
        };
        //                 0   1
        int leftids[] =  { 0,  0};
        int rightids[] = {-1, -1};
        int zonelist[] = { 0,  1};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"inside", "outside"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "quadric_plane_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_QUADRIC_G
        };

        const float c0 = .464102;
        const float c2 = -1.0718;
        float coeffs[] =
        {
            1.0,  c0,  c0, 0.0,  c2, 0.0, 0.0, 0.0, 0.0, 0.0    // cone
            //1/9.0,  1/9.0,  -0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0    // good-cone
            //1/9.0,  1/9.0,  -0.01, 0.0, -.10, 0.0, 0.0, 0.0, 0.0, 0.0    // cone
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        //double extents[] = {-1.0, -1.0, -1.0, 1.0, 1.0, 1.0};
        double extents[] = {-10.0, -10.0, -10.0, 10.0, 10.0, 10.0};

        char *bndnames[] = {"cone"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "quadric_cone", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "quadric_cone_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER,          // 0: inside (everything to back) of plane
            DBCSG_OUTER           // 1: outside (everything to front) of plane
        };
        //                 0   1
        int leftids[] =  { 0,  0};
        int rightids[] = {-1, -1};
        int zonelist[] = { 0,  1};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"inside", "outside"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "quadric_cone_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_SPHERE_PR,
            DBCSG_SPHERE_PR
        };

        float coeffs[] =
        {
            0.0, 0.0, 0.0, 1.0,
            0.0, 0.0, 0.0, 0.9
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-3.0, -3.0, -3.0, 3.0, 3.0, 3.0};

        char *bndnames[] = {"outer-sphere", "inner-sphere"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "quadric_shell", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "quadric_shell_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER,          // 0: inside of outer sphere 
            DBCSG_INNER,          // 1: inside of inner sphere 
            DBCSG_DIFF            // 2: difference 
        };
        //                 0   1   2
        int leftids[] =  { 0,  1,  0};
        int rightids[] = {-1, -1,  1};
        int zonelist[] = {2};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"difference"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "quadric_shell_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }
    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_QUADRIC_G
        };

        double theta = 45.0 * 3.1415926/180.0;
        double a = cos(theta);
        double b = sin(theta); 
        float coeffs[] =
        {
         // x^2  y^2  z^2  xy   yz      xz    x    y    z    c
            a*a, 1.0, b*b, 0.0, 0.0, 2*a*b, 0.0, 0.0, 0.0, -1
            //1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -4
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-5.0, -5.0, -5.0, 5.0, 5.0, 5.0};

        char *bndnames[] = {"quadric_cylinder"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "quadric_cylinder2", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "quadric_cylinder2_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER           // 0: inside of cylinder 
        };
        //                 0
        int leftids[] =  { 0};
        int rightids[] = {-1};
        int zonelist[] = {0};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"cylinder"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "quadric_cylinder2_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_QUADRIC_G
        };

        double theta = 45.0 * 3.1415926/180.0;
        double phi  = 45.0 * 3.1415926/180.0;
        double a = cos(theta);
        double b = sin(theta); 
        double q = cos(-phi);
        double r = sin(-phi);
        double a2 = a*a;
        double b2 = b*b;
        double q2 = q*q;
        double r2 = r*r;
        double R = 2.0;
        float coeffs[] =
        {
         // x^2     y^2      z^2        xy         yz           xz         x  y  z    c
         a2+r2*b2,  q2,    b2+r2*a2,  2*q*r*b,   2*q*r*a, -2*a*b+2*a*b*r2, 0, 0, 0, -R*R
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-5.0, -5.0, -5.0, 5.0, 5.0, 5.0};

        char *bndnames[] = {"quadric_cylinder"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "quadric_cylinder3", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "quadric_cylinder3_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER           // 0: inside of cylinder 
        };
        //                 0
        int leftids[] =  { 0};
        int rightids[] = {-1};
        int zonelist[] = {0};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"cylinder"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "quadric_cylinder3_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_CONE_PNLA
        };

        float coeffs[] =
        {
            0.0, 0.0, 0.0, 0.577, 0.577, 0.577, 5.0, 30.0
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-5.0, -5.0, -5.0, 5.0, 5.0, 5.0};

        char *bndnames[] = {"cone_pnla"};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);
        DBAddOption(optlist, DBOPT_BNDNAMES, bndnames);

        DBPutCsgmesh(dbfile, "cone_pnla", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "cone_pnla_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER           // 0: inside of cylinder 
        };
        //                 0
        int leftids[] =  { 0};
        int rightids[] = {-1};
        int zonelist[] = {0};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"cone"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "cone_pnla_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }

    // build and output the csg mesh (boundaries)
    {
        int typeflags[] =
        {
            DBCSG_SPHERE_PR,
            DBCSG_SPHERE_PR,
            DBCSG_CYLINDER_PNLR
        };

        float coeffs[] =
        {
            0.0, 0.0, 0.0, 0.975,
            0.0, 0.0, 0.0,  1.0,
            0.4, 0.4, 0.4, 0.57, 0.578, 0.578, 2.0, 0.1
        };

        int nbounds = sizeof(typeflags) / sizeof(typeflags[0]);
        int lcoeffs = sizeof(coeffs) / sizeof(coeffs[0]);

        double extents[] = {-2.0, -2.0, -2.0, 2.0, 2.0, 2.0};

        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_EXTENTS, extents);

        DBPutCsgmesh(dbfile, "sphere_shell_by_intersection", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "ssbi_csgzl", optlist);

        DBPutCsgmesh(dbfile, "sphere_shell_by_diff", 3, nbounds, typeflags,
            NULL, coeffs, lcoeffs, DB_FLOAT, extents, "ssbd_csgzl", optlist);

        DBFreeOptlist(optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_OUTER,          // 0: outside of inner sphere
            DBCSG_INNER,          // 1: inside of outer sphere 
            DBCSG_INTERSECT,      // 2: intersection between them 
            DBCSG_INNER,          // 3: inside of cylinder
            DBCSG_DIFF            // 4: sphere shell minus cylinder
        };
        //                 0   1   2   3   4
        int leftids[] =  { 0,  1,  0,  2,  2};
        int rightids[] = {-1, -1,  1, -1,  3};
        int zonelist[] = {4};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"shelli"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "ssbi_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }

    // build and output the csg zonelist
    {
        int typeflags[] =
        {
            DBCSG_INNER,          // 0: inside of inner sphere
            DBCSG_INNER,          // 1: inside of outer sphere 
            DBCSG_DIFF,           // 2: outer minus inner 
            DBCSG_INNER,          // 3: inside of cylinder
            DBCSG_DIFF            // 4: sphere shell minus cylinder
        };
        //                 0   1   2   3   4
        int leftids[] =  { 0,  1,  1,  2,  2};
        int rightids[] = {-1, -1,  0, -1,  3};
        int zonelist[] = {2};

        int nregs = sizeof(typeflags) / sizeof(typeflags[0]);
        int nzones = sizeof(zonelist) / sizeof(zonelist[0]);

        char *zonenames[] = {"shelli"};

        DBoptlist *optlist = DBMakeOptlist(1);
        DBAddOption(optlist, DBOPT_ZONENAMES, zonenames);

        DBPutCSGZonelist(dbfile, "ssbd_csgzl", nregs, typeflags, leftids, rightids,
                         NULL, 0, DB_INT, nzones, zonelist, optlist);
    }
}
