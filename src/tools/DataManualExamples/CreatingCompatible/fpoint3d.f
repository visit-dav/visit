      program main
      implicit none
      include "silo.inc"
      integer dbfile, ierr

c Create the Silo file
      ierr = dbcreate("fpoint3d.silo", 13, DB_CLOBBER, DB_LOCAL, 
     .                "3D point mesh", 13, DB_HDF5, dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          goto 10000
      endif
c Add other Silo calls here.
      call write_point3d(dbfile)
c Close the Silo file.
      ierr = dbclose(dbfile)
10000 stop
      end

      subroutine write_point3d(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, i, ndims, NPTS
      parameter (NPTS = 100)
      real x(NPTS), y(NPTS), z(NPTS), t, angle
      do 10000 i = 0,NPTS-1
          t = float(i) / float(NPTS-1)
          angle =  3.14159 * 10. * t
          x(i+1) = t * cos(angle);
          y(i+1) = t * sin(angle);
          z(i+1) = t
10000 continue
      ndims = 3
      err = dbputpm (dbfile, "pointmesh", 9, ndims, x, y, z,
     . NPTS, DB_FLOAT, DB_F77NULL, ierr)
      end
