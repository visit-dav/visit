// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ***************************************************************************
//  Program: add_visit_searchpath
//
//  Purpose:
//    Add the variable "_visit_searchpath" with the specified search path
//    to a Silo file. If the search path is present then the Silo reader
//    will only search those directories for variables. This may speed up
//    the reading of the Silo file and improve the responsivness of VisIt
//    if the file has many variables stored in the directories being ignored.
//
//  Arguments:
//    Takes a Silo filename and an optional search path. If the search path
//    is not specified then it stores an empty search path, which will cause
//    all the directories to be ignored.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Oct  1 08:38:30 PDT 2015
//
// ***************************************************************************

#include <stdio.h>
#include <string.h>

#include <silo.h>

void
usage()
{
    fprintf(stderr, "Usage: add_visit_searchpath filename [searchpath]\n");
}

int
main(int argc, char *argv[])
{
    //
    // Parse the arguments.
    //
    char *filename = NULL, *searchpath = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-forceversion") == 0)
        {
            i++;
        }
        else
        {
            if (filename != NULL && searchpath != NULL)
            {
                usage();
                return 1;
            }

            if (filename != NULL)
                searchpath = argv[i];
            else
                filename = argv[i];
        }
    }
    if (filename == NULL)
    {
        usage();
        return 2;
    }

    //
    // Open the file.
    //
    DBfile *db = NULL;
    if ((db = DBOpen(filename, DB_HDF5, DB_APPEND)) == NULL)
    {
        fprintf(stderr, "Unable to open file %s\n", argv[1]);
        return 3;
    }

    //
    // Write the path.
    //
    if (searchpath == NULL)
    {
        int length = 1;
        DBWrite(db, "_visit_searchpath", ";", &length, 1, DB_CHAR);
    }
    else
    {
        int length = strlen(searchpath);
        DBWrite(db, "_visit_searchpath", searchpath, &length, 1, DB_CHAR);
    }

    //
    // Close the file.
    //
    DBClose(db);

    return 0;
}
