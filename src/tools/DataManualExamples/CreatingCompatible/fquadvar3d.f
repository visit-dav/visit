      program main
      implicit none
      include "silo.inc"
      integer dbfile, ierr

c Create the Silo file
      ierr = dbcreate("fquadvar3d.silo", 15, DB_CLOBBER, DB_LOCAL, 
     .                "Quadvars in 3D", 14, DB_HDF5, dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          goto 10000
      endif
c Add other Silo calls here.
      call write_curv3d(dbfile)
      call write_zonecent_quadvar(dbfile)
      call write_nodecent_quadvar(dbfile)
c Close the Silo file.
      ierr = dbclose(dbfile)
10000 stop
      end

      subroutine write_curv3d(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, dims(3), ndims, ZX,ZY,ZZ,NX,NY,NZ
      parameter (NX = 4)
      parameter (NY = 3)
      parameter (NZ = 2)
      parameter (ZX = NX-1)
      parameter (ZY = NY-1)
      parameter (ZZ = NZ-1)
      real x(NX,NY,NZ), y(NX,NY,NZ), z(NX,NY,NZ)
      data x/0.,1.,2.,3., 0.,1.,2.,3., 0.,1.,2.,3.,
     . 0.,1.,2.,3., 0.,1.,2.,3., 0.,1.,2.,3./
      data y/0.5,0.,0.,0.5, 1.,1.,1.,1., 1.5,2.,2.,1.5,
     . 0.5,0.,0.,0.5, 1.,1.,1.,1., 1.5,2.,2.,1.5/
      data z/0.,0.,0.,0., 0.,0.,0.,0., 0.,0.,0.,0,
     . 1.,1.,1.,1., 1.,1.,1.,1., 1.,1.,1.,1./
      ndims = 3
      data dims/4, 3, 2/
      err = dbputqm (dbfile, "quadmesh", 8, "xc", 2, 
     . "yc", 2, "zc", 2, x, y, z, dims, ndims, 
     . DB_FLOAT, DB_NONCOLLINEAR, DB_F77NULL, ierr)
      end

      subroutine write_zonecent_quadvar(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, dims(3), ndims,i,j,k,index,ZX,ZY,ZZ,NX,NY,NZ
      parameter (NX = 4)
      parameter (NY = 3)
      parameter (NZ = 2)
      parameter (ZX = NX-1)
      parameter (ZY = NY-1)
      parameter (ZZ = NZ-1)
      integer    zonal(ZX,ZY,ZZ)
      data dims/ZX,ZY,ZZ/
      index = 0
      do 10020 k=1,ZZ
      do 10010 j=1,ZY
      do 10000 i=1,ZX
          zonal(i,j,k) = index
          index = index + 1
10000 continue
10010 continue
10020 continue
      ndims = 3
      err = dbputqv1(dbfile, "zonal", 5, "quadmesh", 8, zonal, dims, 
     . ndims, DB_F77NULL, 0, DB_INT, DB_ZONECENT, DB_F77NULL, ierr)
      end

      subroutine write_nodecent_quadvar(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, dims(3), ndims,i,j,k,index,ZX,ZY,ZZ,NX,NY,NZ
      parameter (NX = 4)
      parameter (NY = 3)
      parameter (NZ = 2)
      parameter (ZX = NX-1)
      parameter (ZY = NY-1)
      parameter (ZZ = NZ-1)
      real    nodal(NX,NY,NZ)
      data dims/NX,NY,NZ/
      index = 0
      do 20020 k=1,NZ
      do 20010 j=1,NY
      do 20000 i=1,NX
          nodal(i,j,k) = float(index)
          index = index + 1
20000 continue
20010 continue
20020 continue
      ndims = 3
      err = dbputqv1(dbfile, "nodal", 5, "quadmesh", 8, nodal, dims, 
     . ndims, DB_F77NULL, 0, DB_FLOAT, DB_NODECENT, DB_F77NULL, ierr)
      end
