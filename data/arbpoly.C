/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <silo.h>

//
// Purpose: Build a one zone, arbitrary polyhedral mesh consisting of
// a single hex that has had one corner cut off.
//
// Programmer: Mark C. Miller, Sat Oct 10 01:32:23 PDT 2009
//

int
main(int argc, char **argv)
{
    DBfile *dbfile;
    int N = 21;
    int driver = DB_PDB;

    int i = 1;
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

    dbfile = DBCreate("arbpoly.silo", DB_CLOBBER, DB_LOCAL,
                      "Simple arbitrary polyhedral clipped hex", driver);

    const float t = 0.75;
    // node ids  0   1   2   3   4   5   6   7   8   9
    float x[] = {0,  0,  1,  1,  0,  0,  t,  1,  1,  1};
    float y[] = {0,  0,  0,  0,  1,  1,  1,  1,  1,  t};
    float z[] = {0,  1,  1,  0,  0,  1,  1,  t,  0,  1};
    float *coords[3]={x, y, z};
    // face ids       0         1         2         3           4           5           6
    int nodecnts[] = {4,        4,        4,        5,          5,          5,          3};
    int nodelist[] = {0,3,2,1,  5,4,0,1,  4,8,3,0,  1,2,9,6,5,  2,3,8,7,9,  4,5,6,7,8,  6,7,9};
    int facecnts[] = {7};
    int facelist[] = {0,1,2,3,4,5,6};

    DBoptlist *optlist = DBMakeOptlist(3);
    char *phzlname = "phzl";
    char *coordnames[] = {"x","y","z"};
    DBAddOption(optlist, DBOPT_PHZONELIST, phzlname);
    DBPutUcdmesh(dbfile, "clipped_hex", 3, coordnames, (float **)coords, 10,
                 1, 0, 0, DB_FLOAT, optlist);
    DBPutPHZonelist(dbfile, "phzl", 7, nodecnts, 30, nodelist,
                    0, 1, facecnts, 7, facelist, 0, 0, 0, 0);
    DBClose(dbfile);
}
