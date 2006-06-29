      progam main
      implicit none
      include "silo.inc"
      integer dbfile, ierr

c The 11 and 22 arguments represent the lengths of strings
      ierr = dbcreate("fbasic.silo", 11, DB_CLOBBER, DB_LOCAL, 
     . "Comment about the data", 22, DB_HDF5, dbfile)

      if(dbfile.eq.-1) then
          write (6,*) `Could not create Silo file!\n'
          goto 10000
      endif

c Add other Silo calls here.

c Close the Silo file.
      ierr = dbclose(dbfile)
10000 stop
      end
