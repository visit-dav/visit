// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <silo.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
    DBfile *dbfile = NULL;
    /* Open the Silo file */
    dbfile = DBCreate("basic.silo", DB_CLOBBER, DB_LOCAL,
      "Comment about the data", DB_HDF5);
    if(dbfile == NULL)
    {
        fprintf(stderr, "Could not create Silo file!\n");
        return -1;
    }

    /* Add other Silo calls here. */

    /* Close the Silo file. */
    DBClose(dbfile);

    return 0;
}
