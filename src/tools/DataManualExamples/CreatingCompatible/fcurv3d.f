      program main
      implicit none
      include "silo.inc"
      integer dbfile, ierr

c Open the Silo file
      ierr = dbcreate("fcurv3d.silo", 12, DB_CLOBBER, DB_LOCAL, 
     .                "3D curvilinear mesh", 19, DB_HDF5, dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          goto 10000
      endif
c Add other Silo calls here.
      call write_curv3d(dbfile)
c Close the Silo file.
      ierr = dbclose(dbfile)
10000 stop
      end

      subroutine write_curv3d(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, dims(3), ndims, NX, NY, NZ
      parameter (NX = 4)
      parameter (NY = 3)
      parameter (NZ = 2)
      real x(NX,NY,NZ), y(NX,NY,NZ), z(NX,NY,NZ)
      data x/0.,1.,2.,3., 0.,1.,2.,3., 0.,1.,2.,3.,
     . 0.,1.,2.,3., 0.,1.,2.,3., 0.,1.,2.,3./
      data y/0.5,0.,0.,0.5, 1.,1.,1.,1., 1.5,2.,2.,1.5,
     . 0.5,0.,0.,0.5, 1.,1.,1.,1., 1.5,2.,2.,1.5/
      data z/0.,0.,0.,0., 0.,0.,0.,0., 0.,0.,0.,0,
     . 1.,1.,1.,1., 1.,1.,1.,1., 1.,1.,1.,1./
      ndims = 3
      data dims/NX,NY,NZ/
      err = dbputqm (dbfile, "quadmesh", 8, "xc", 2, 
     . "yc", 2, "zc", 2, x, y, z, dims, ndims, 
     . DB_FLOAT, DB_NONCOLLINEAR, DB_F77NULL, ierr)
      end
