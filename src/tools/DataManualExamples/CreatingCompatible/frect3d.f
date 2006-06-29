      program main
      implicit none
      include "silo.inc"
      integer dbfile, ierr

c The 9 and 21 arguments represent the lengths of strings
      ierr = dbcreate("frect3d.silo", 12, DB_CLOBBER, DB_LOCAL, 
     .                "3D rectilinear mesh", 19, DB_HDF5, dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          goto 10000
      endif
c Add other Silo calls here.
      call write_rect3d(dbfile)
c Close the Silo file.
      ierr = dbclose(dbfile)
10000 stop
      end

      subroutine write_rect3d(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, dims(3), ndims, NX, NY, NZ
      parameter (NX = 4)
      parameter (NY = 5)
      parameter (NZ = 3)
      real x(NX), y(NY), z(NZ)
      data x/0., 1., 2.5, 5./
      data y/0., 2., 2.25, 2.55,  5./
      data z/0., 1., 3./
      ndims = 3
      data dims/NX, NY, NZ/
      err = dbputqm (dbfile, "quadmesh", 8, "xc", 2, 
     . "yc", 2, "zc", 2, x, y, z, dims, ndims, 
     . DB_FLOAT, DB_COLLINEAR, DB_F77NULL, ierr)
      end
