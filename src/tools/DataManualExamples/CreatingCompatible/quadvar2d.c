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

#include <silo.h>
#include <stdio.h>

void
write_rect2d(DBfile *dbfile)
{
    /* Write a rectilinear mesh. */
    float x[] = {0., 1., 2.5, 5.};
    float y[] = {0., 2., 2.25, 2.55,  5.};
    int dims[] = {4, 5};
    int ndims = 2;
    float *coords[] = {x, y};
    /* Create an option list to contain labels and units. */
    DBoptlist *optlist = DBMakeOptlist(4);
    DBAddOption(optlist, DBOPT_XLABEL, (void *)"Pressure");
    DBAddOption(optlist, DBOPT_XUNITS, (void *)"kP");
    DBAddOption(optlist, DBOPT_YLABEL, (void *)"Temperature");
    DBAddOption(optlist, DBOPT_YUNITS, (void *)"Celsius");
    /* Write a quadmesh with an option list. */
    DBPutQuadmesh(dbfile, "quadmesh", NULL, coords, dims, ndims,
                  DB_FLOAT, DB_COLLINEAR, optlist);
    /* Free the option list. */
    DBFreeOptlist(optlist);
}

void
write_zonecent_quadvar2d(DBfile *dbfile)
{
    /* The data must be (NX-1) * (NY-1) since it is zonal. */
    float var1[] = {
        0.,  1.,  2.,
        3.,  4.,  5.,
        6.,  7.,  8.,
        9., 10., 11.
    };
    double var2[] = {
        0.00, 1.11, 2.22,
        3.33, 4.44, 5.55,
        6.66, 7.77, 8.88,
        9.99, 10.1, 11.11
    };
    int var3[] = {
        0,  1,  2,
        3,  4,  5,
        6,  7,  8,
        9, 10, 11
    };
    char var4[] = {
        0,  1,  2,
        3,  4,  5,
        6,  7,  8,
        9, 10, 11
    };
    /* Note dims are 1 less than mesh's dims in each dimension. */
    int dims[]={3, 4};
    int ndims = 2;
    DBPutQuadvar1(dbfile, "var1", "quadmesh", var1, dims,
                  ndims, NULL, 0, DB_FLOAT, DB_ZONECENT, NULL);
    /* Write a double-precision variable. */
    DBPutQuadvar1(dbfile, "var2", "quadmesh", (float*)var2, dims,
                  ndims, NULL, 0, DB_DOUBLE, DB_ZONECENT, NULL);
    /* Write an integer variable */
    DBPutQuadvar1(dbfile, "var3", "quadmesh", (float*)var3, dims,
                  ndims, NULL, 0, DB_INT, DB_ZONECENT, NULL);
    /* Write a char variable */
    DBPutQuadvar1(dbfile, "var4", "quadmesh", (float*)var4, dims,
                  ndims, NULL, 0, DB_CHAR, DB_ZONECENT, NULL);
}

void
write_nodecent_quadvar2d(DBfile *dbfile)
{
    /* The data must be NX * NY since it is nodal. */
    float nodal[] = {
        0.,   1.,  2., 3.,
        4.,   5.,  6., 7.,
        8.,   9., 10., 11.,
        12., 13., 14., 15.,
        16., 17., 18., 19.
    };
    /* Nodal variables have same #values as #nodes in mesh */
    int dims[]={4, 5};
    int ndims = 2;
    DBPutQuadvar1(dbfile, "nodal", "quadmesh", nodal, dims,
                  ndims, NULL, 0, DB_FLOAT, DB_NODECENT, NULL);
}

int
main(int argc, char *argv[])
{
    DBfile *dbfile = NULL;
    /* Open the Silo file */
    dbfile = DBCreate("quadvar2d.silo", DB_CLOBBER, DB_LOCAL,
        "Quadvar example", DB_HDF5);
    if(dbfile == NULL)
    {
        fprintf(stderr, "Could not create Silo file!\n");
        return -1;
    }

    /* Add other Silo calls here. */
    write_rect2d(dbfile);
    write_zonecent_quadvar2d(dbfile);
    write_nodecent_quadvar2d(dbfile);

    /* Close the Silo file. */
    DBClose(dbfile);
    return 0;
}
