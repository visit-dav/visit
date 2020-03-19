// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <silo.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

// suppress the following since silo uses char * in its API
#if defined(__clang__)
# pragma clang diagnostic ignored "-Wwritable-strings"
#elif defined(__GNUC__)
# pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#if 0
    varnames[0] = "u";
    vals[0] = dc;
    DBPutUcdvar(dbfile, "ud_1d", "ucd_1d", 1, varnames, (float**)vals,
        NX, NULL, 0, DB_DOUBLE, DB_NODECENT, 0);
    vals[0] = fc;
    DBPutUcdvar(dbfile, "uf_1d", "ucd_1d", 1, varnames, (float**)vals,
        NX, NULL, 0, DB_FLOAT, DB_NODECENT, 0);
#endif

int
main(int argc, char *argv[])
{
    int i;
    float ucdx[] = {0,1,2,0,1,2,0,1,2};
    float ucdy[] = {0,0,0,1,1,1,2,2,2};
    int shapesize = 4;
    int shapecnt = 4;
    int shapetype = DB_ZONETYPE_QUAD;
    int zonelist[] = {0,1,4,3,  1,2,5,4,  3,4,7,6,  4,5,8,7};
    int trishapesize = 3;
    int trishapecnt = 8;
    int trishapetype = DB_ZONETYPE_TRIANGLE;
    int trizonelist[] = {4,3,0, 4,0,1,  4,1,2, 4,2,5,  4,6,3, 4,7,6,  4,8,7, 4,5,8};
    char *coordnames[] = {"x","y"};
    float *ucdcoords[2] = {ucdx,ucdy};
    double nodal_field[] = {0, 0, 0, 0, 10, 0, 0, 0, 0};
    float qx[] = {0,.1,.2,.3,.4,.5,.6,.7,.8,.9,1.0,1.1,1.2,1.3,1.4,1.5,1.6,1.7,1.8,1.9,2.0};
    float *qcoords[] = {qx, qx};
    int qdims[] = {21,21};
    double nodal_field_highres[21*21];

    // Check for the right driver.
    int driver = DB_PDB;
    for(int j = 1; j < argc; ++j)
    {
        if (strcmp(argv[j], "DB_HDF5") == 0)
            driver = DB_HDF5;
        else if (strcmp(argv[j], "DB_PDB") == 0)
            driver = DB_PDB;
        else
           fprintf(stderr,"Uncrecognized driver name \"%s\"\n", argv[j]);
    }

    // Open the Silo file
    DBfile *dbfile = DBCreate("big_quads.silo", DB_CLOBBER, DB_LOCAL,
        "2D mesh with large quads to demonstrate shading artifacts", driver);
    if(dbfile == NULL)
    {
        fprintf(stderr, "Could not create Silo file!\n");
        return -1;
    }

    // Output mesh as 4 quads
    DBPutUcdmesh(dbfile, "quads", 2, coordnames, ucdcoords,
                9, 4, "quads_zl", NULL, DB_FLOAT, NULL);
    DBPutZonelist2(dbfile, "quads_zl", 4, 2, zonelist, (int) sizeof(zonelist),
                0, 0, 0, &shapetype, &shapesize, &shapecnt, 1, NULL);
    DBPutUcdvar1(dbfile, "quads_field", "quads", nodal_field,
        9, NULL, 0, DB_DOUBLE, DB_NODECENT, 0);


    // Output mesh as 8 triangles with diagonals chosen "correctly"
    DBPutUcdmesh(dbfile, "tris", 2, coordnames, ucdcoords,
                9, 8, "tris_zl", NULL, DB_FLOAT, NULL);
    DBPutZonelist2(dbfile, "tris_zl", 8, 2, trizonelist, (int) sizeof(trizonelist),
                0, 0, 0, &trishapetype, &trishapesize, &trishapecnt, 1, NULL);
    DBPutUcdvar1(dbfile, "tris_field", "tris", nodal_field,
        9, NULL, 0, DB_DOUBLE, DB_NODECENT, 0);


    // Output much finer resolution version of 4 quad case by doing
    // bi-linear sampling of each quad, the interpolation function
    // VisIt should be using internally to describe the field variation
    // over a quad.
    {
        for (int i = 0; i < 21; i++)
        {
            double h00, h10, h01, h11, xoff, yoff;
            for (int j = 0; j < 21; j++)
            {
                if (qx[i] <= 1.0)
                {
                    xoff = 0;
                    if (qx[j] <= 1.0)
                    {
                        yoff = 0;
                        h00=0; h10=0; h01=0; h11=10;
                    }
                    else
                    {
                        yoff = 1;
                        h00=0; h10=10; h01=0; h11=0;
                    }
                }
                else
                {
                    xoff = 1;
                    if (qx[j] <= 1.0)
                    {
                        yoff = 0;
                        h00=0; h10=00; h01=10; h11=0;
                    }
                    else
                    {
                        yoff = 1;
                        h00=10; h10=00; h01=0; h11=0;
                    }
                }
                double dx = qx[i] - xoff;
                double dy = qx[j] - yoff;
                double h0x = dx*h10 + (1-dx)*h00;
                double h1x = dx*h11 + (1-dx)*h01;
                double h = dy*h1x + (1-dy)*h0x;
                nodal_field_highres[j*21+i] = h; // [j*nx+i]
            }
        }
        DBPutQuadmesh(dbfile, "qmesh", coordnames, qcoords, qdims, 2,
            DB_FLOAT, DB_COLLINEAR, NULL);
        DBPutQuadvar1(dbfile, "highres_field", "qmesh", nodal_field_highres,
            qdims, 2, NULL, 0, DB_DOUBLE, DB_NODECENT, NULL);
    }

    // Compute various qualities of lineouts from (0,1.5,0) to (1.5,0,0)
    {
        // Equation of the line of the lineout (y = mx + b)
        // b=1.5, m=-1, so points along the line are given by
        // y=-x+1.5. But, we really want a ray-type representation
        // for the lineout with an initial point, p0, and a
        // direction vector, r. A point, p(t), along the lineout
        // ray is given by p(t) = p0 + t*r where p0=(0,1.5) and
        // r = (1x -1y) / sqrt(2) (making a unit step in t along
        // the ray the same geometric length as x or y.

        // Equation of a piecewise-linear interpolant over a quad given
        // dofs at the corners and u,v in [0,1]
        //
        //        ^ y/v
        //        |
        //        |
        //         
        //       h01---+-------h11
        //        |    |        |
        //        |    |        |
        //        |    |        |
        //      v +----x--------+
        //        |    |        |
        //       h00---+-------h10   ---> x/u
        //             u
        //
        // h(u,v) = v*(u*h11+(1-u)*h01) + (1-v)*(u*h10+(1-u)*h00) 
        // h(u,v) = h00 + (h10-h00)*u + (h01-h00)*v + (h11+h00-(h10+h01))*u*v
        //
        // For a given quad, we need to calculate the point of entrance of
        // the lineout ray with the quad. We then convert that point to 
        // the quad's local u,v coordinate system. That point becomes the
        // local ray origin. The direction vector is the same since, in
        // this example anyways, the quad's local u,v coordinate system
        // aligns with the global x,y coordinate system. That, however,
        // is not true in general.
        //
        // Labeling the Quads in this mesh as 
        //
        //       +-+-+
        //       |A|B|
        //       +-+-+
        //       |C|D|
        //       +-+-+
        //
        // ...the lineout starts in Quad A, then C and then D
        //
        //
        // For Quad A: P0(x,y)=(0,1.5) ==> P0(u,v)=(0,0.5)
        //             r(x,y)=(x-y)/sqrt(2) ==> r(u,v)=(u-v)/sqrt(2)
        //             p(t') = P0(u,v)+r(u,v)*t'
        // So, for points along the ray, t', we have
        //             p(t'),u = 0.0 + 1/q*t'
        //             p(t'),v = 0.5 - 1/q*t'
        // where q=sqrt(2)
        //
        // Substituting into h(u,v), above and solving for t' we have...
        //
        // h(t') = h00 +
        //         (h10-h00)*(0.0+1/q*t') +
        //         (h01-h00)*(0.5-1/q*t') +
        //         (h11+h00-(h10+h01))*(0.0+1/q*t')*(0.5-1/q*t')
        //
        //         h00 + 0.0*(h10-h00) + 0.5*(h01-h00) + 0.0*0.5*(h11+h00-(h10+h01)) +
        //         t'*[(h10-h00)*1/q - (h01-h00)*1/q - 1/q*0.0*(h11+h00-(h10+h01)) + 1/q*0.5*(h11+h00-(h10+h01))] +
        //         t'*t'*[1/q*-1/q*(h11+h00-(h10+h01))]
        //
        // or
        //
        //         h00 + A*(h10-h00) + B*(h01-h00) + A*B*(h11+h00-(h10+h01)) +
        //         t'*[(h10-h00)*P + (h01-h00)*Q + Q*A*(h11+h00-(h10+h01)) + P*B*(h11+h00-(h10+h01))] +
        //         t'*t'*[P*Q*(h11+h00-(h10+h01))]
        //
        //         where P0(u,v)=(A,B) and r = (Pu+Qv)
    }

#if 0
    WriteLineouts(dbfile, 0, 1.5, 1, -1, "quads_lineouts", ucdcoords,
        zonelist, (int) sizeof(zonelist), &shapetype, &shapesize, &shapecnt);
#endif

    // Close the Silo file.
    DBClose(dbfile);
    return 0;
}
