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
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#define ONE_MEG 1048576

static void
build_curve (DBfile *dbfile)
{
   float        x[20], y[2][20] ;
   int          i ;
   DBoptlist    *opts ;

   // Sine and cosine for 1/2 cycle.  Both curves have the same
   // X data points and share the data in the file.
   for (i=0; i<20; i++) {
      x[i] = i * M_PI / 20.0 ;
      y[0][i] = sin (x[i]) ;
      y[1][i] = cos (x[i]) ;
   }

   opts = DBMakeOptlist (10) ;
   char *xaxis = "X Axis";
   char *yaxis = "Y Axis";
   char *xunits = "radians";
   char *xvarnm = "sincurve_xvals";

   DBAddOption(opts, DBOPT_XLABEL, xaxis);
   DBAddOption(opts, DBOPT_YLABEL, yaxis); 
   DBAddOption(opts, DBOPT_XUNITS, xunits);
   DBAddOption(opts, DBOPT_XVARNAME, xvarnm); 

   // Write the `sincurve' curve. The hdf5 driver allows the user to specify
   // the name which will be used to store the x values, but the pdb driver
   // requires us to know where the values were stored.
   DBPutCurve (dbfile, "sincurve", x, y[0], DB_FLOAT, 20, opts);

   // Write the `coscurve' curve. It shares x values with the `sincurve'
   // curve.
   DBPutCurve (dbfile, "coscurve", NULL, y[1], DB_FLOAT, 20, opts) ;
   DBFreeOptlist (opts) ;
}


/*-------------------------------------------------------------------------
 * Function:     main
 *
 * Purpose:      Build a > 2Gig file with something stored at offsets
 *               greater than 2 Gig that VisIt can try to display.
 *
 * Notes:        This can only work on the HDF5 driver
 *
 * Programmer:   Mark C. Miller, March 23, 2006
 *
 *-------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    
    int            i, j, ndims=1, dims[]={ONE_MEG/sizeof(float)};
    float          val[ONE_MEG/sizeof(float)];
    char          filename[1024];
    DBfile        *dbfile;

    
    if (argc == 1)
    {
        fprintf(stderr, "You must specify a filename.\n"
            "Make sure the file (and path) you specify is\n"
            "to a filesystem that supports large files.\n");
        exit(1);
    }
    else
    {
        strncpy(filename, argv[1], sizeof(filename));
        filename[1023] = '\0';
    }

    DBShowErrors(DB_TOP, NULL);
    DBForceSingle(1);

    // Create a file that contains a simple variables.
    printf("Creating file: \"%s\"\n", filename);
    dbfile = DBCreate(filename, 0, DB_LOCAL, "Large File Test", DB_HDF5);

    if (dbfile == NULL)
    {
        fprintf(stderr, "Error creating file\n");
        exit(1);
    }

    // This will generate a 2.5 Gigabyte file
    for (j = 0; j < 2500; j++)
    {
        char tmpname[64];

        if (j % 100 == 0)
            printf("Iterations %04d to %04d of %04d\n", j, j+100-1, 2500);

        sprintf(tmpname, "simple_%04d", j);

        for (i = 0; i < dims[0]; i++)
            val[i] = (float) dims[0] * j + i;

        if (DBWrite(dbfile, tmpname, val, dims, ndims, DB_FLOAT) != 0)
        {
            fprintf(stderr, "Error creating file\n");
            exit(1);
        }
    }

    // Put some objects VisIt can process at the end of the file
    build_curve(dbfile);

    DBClose(dbfile);

    return 0;
}
