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
