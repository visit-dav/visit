      program main
      implicit none
      include "silo.inc"
      integer dbfile, ierr

c Create the Silo file
      ierr = dbcreate("fcurv2d.silo", 12, DB_CLOBBER, DB_LOCAL, 
     .                "2D curvilinear mesh", 19, DB_HDF5, dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          goto 10000
      endif
c Add other Silo calls here.
      call write_curv2d(dbfile)
c Close the Silo file.
      ierr = dbclose(dbfile)
10000 stop
      end

      subroutine write_curv2d(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, i, dims(2), ndims, NX, NY
      parameter (NX = 4)
      parameter (NY = 3)
      real x(NX, NY), y(NX, NY)
      data x/0., 1., 3., 3.5, 
     .       0., 1., 2.5, 3.5,    
     .       0.7, 1.3, 2.3, 3.5/
      data y/0., 0., 0., 0., 
     .       1.5, 1.5, 1.25, 1.5,
     .       3., 2.75, 2.75, 3./
      ndims = 2
      data dims/NX, NY/
      err = dbputqm (dbfile, "quadmesh", 8, "xc", 2, 
     . "yc", 2, "zc", 2, x, y, DB_F77NULL, dims, ndims, 
     . DB_FLOAT, DB_NONCOLLINEAR, DB_F77NULL, ierr)
      end
