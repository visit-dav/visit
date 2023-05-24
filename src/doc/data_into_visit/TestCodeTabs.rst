.. _TestCodeTabs:

Testing code tabs
=================


Example for creating a new silo file:

.. tabs::

  .. code-tab:: c

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

  .. code-tab:: fortran

        progam main
        implicit none
        include "silo.inc"
        integer dbfile, ierr
    c The 11 and 22 arguments represent the lengths of strings
        ierr = dbcreate("fbasic.silo", 11, DB_CLOBBER, DB_LOCAL,
        .               "Comment about the data", 22, DB_HDF5, dbfile)
        if(dbfile.eq.-1) then
            write (6,*) 'Could not create Silo file!\n'
            goto 10000
        endif
    c Add other Silo calls here.
    c Close the Silo file.
        ierr = dbclose(dbfile)
    10000 stop
        end


Now we are done testing.
