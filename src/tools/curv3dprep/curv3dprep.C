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

#include <stdlib.h>
#include <visitstream.h>
#include <silo.h>

//**********************************************************************
//
// This program transforms the domain connectivity information for a
// 3d multi-block structured mesh in a form that can be read more
// efficiently.  It takes as input a list of files to transform.  It
// reads the connectivity information from the file and then appends
// the information to the file in its new format.
//
//**********************************************************************

void AppendFile (char *fileName);

void IgnoreError (char *msg) { }

int
main (int argc, char **argv)
{
    int       i;

    //
    // Check the execute line arguments.
    //
    if (argc < 2)
    {
        cerr << "usage: " << argv[0] << "filelist" << endl;
        exit (1);
    }

    //
    // Loop over all the files appending the connectivity information in
    // a form which is read more quickly.
    //
    DBShowErrors (DB_ALL, IgnoreError);
    DBForceSingle (1);

    for (i = 1; i < argc; i++)
    {
        AppendFile (argv[i]);
    }
}

void
AppendFile (char *fileName)
{
    DBfile    *dbfile=NULL;

    int       i, j, k;

    int       ndomains;
    int       *extents=NULL;
    int       *nneighbors=NULL;
    int       lneighbors;
    int       *neighbors=NULL;

    char      dirname[256], varname[256];

    int       dims, ndims;

    //
    // Open the old file.
    //
    dbfile = DBOpen (fileName, DB_UNKNOWN, DB_APPEND);

    //
    // Read the domain boundary information.
    //
    bool found = false;
    if (DBSetDir(dbfile, "Global") == 0)
    {
        if (DBSetDir(dbfile, "Decomposition") == 0)
        {
            found = true;
        }
        else
        {
            DBSetDir(dbfile, "..");
        }
    }
    if (!found && DBSetDir(dbfile, "Decomposition") == 0)
    {
        found = true;
    }

    if (!found)
    {
        DBClose (dbfile);
        cerr << "File " << fileName << " doesn't contain any connectivity."
             << endl;
        return;
    }

    ndomains = 0;
    DBReadVar(dbfile, "NumDomains", &ndomains);

    extents = new int[ndomains*6];
    nneighbors = new int[ndomains];
    lneighbors = 0;
    for (i = 0; i < ndomains; i++)
    {
        sprintf(dirname, "Domain_%d", i);
        DBSetDir(dbfile, dirname);

        DBReadVar(dbfile, "Extents", &extents[i*6]);
        DBReadVar(dbfile, "NumNeighbors", &nneighbors[i]);

        DBSetDir(dbfile, "..");
        lneighbors += nneighbors[i] * 11;
    }

    neighbors = new int[lneighbors];
    k = 0;
    for (i = 0; i < ndomains; i++)
    {
        sprintf(dirname, "Domain_%d", i);
        DBSetDir(dbfile, dirname);

        for (j = 0; j < nneighbors[i]; j++)
        {
            sprintf(varname, "Neighbor_%d", j);
            DBReadVar(dbfile, varname, &neighbors[k]);
            k += 11;
        }

        DBSetDir(dbfile, "..");
    }

    DBSetDir(dbfile, "/");

    //
    // Write out the neighbor information in a more compact fashion.
    //
    ndims = 1;
    dims = 1;
    DBWrite(dbfile, "ConnectivityNumDomains", &ndomains, &dims, ndims, DB_INT);
    dims = ndomains * 6;
    DBWrite(dbfile, "ConnectivityExtents", extents, &dims, ndims, DB_INT);
    dims = ndomains;
    DBWrite(dbfile, "ConnectivityNumNeighbors", nneighbors, &dims, ndims,
            DB_INT);
    dims = lneighbors;
    DBWrite(dbfile, "ConnectivityNeighbors", neighbors, &dims, ndims, DB_INT);
    dims = 1;
    DBWrite(dbfile, "ConnectivityLenNeighbors", &lneighbors, &dims, ndims,
            DB_INT);

    //
    // Close the file.
    //
    DBClose (dbfile);

    //
    // Free dynamic memory.
    //
    delete [] neighbors;
    delete [] nneighbors;
    delete [] extents;
}
