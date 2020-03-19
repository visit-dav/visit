c Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
c Project developers.  See the top-level LICENSE file for dates and other
c details.  No copyright assignment is required to contribute to VisIt.

      program main
      implicit none
      include "silo.inc"
      integer dbfile, ierr

c Create the Silo file
      ierr = dbcreate("fmixedmaterials.silo", 20, DB_CLOBBER, DB_LOCAL, 
     . "2D rectilinear mesh with mixed materials", 40, DB_HDF5, dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          goto 10000
      endif
c Add other Silo calls here.
      call write_rect2d(dbfile)
      call write_mixedmaterial(dbfile)
c Close the Silo file.
      ierr = dbclose(dbfile)
10000 stop
      end

      subroutine write_rect2d(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, dims(2), ndims, NX, NY
      parameter (NX = 5)
      parameter (NY = 4)
      real x(NX), y(NY)
      data x/0., 0.5, 1.0, 1.5, 2.0/
      data y/0., 0.5, 1.0, 1.5/
      ndims = 2
      data dims/NX, NY/
      err = dbputqm (dbfile, "quadmesh", 8, "xc", 2, 
     . "yc", 2, "zc", 2, x, y, DB_F77NULL, dims, ndims, 
     . DB_FLOAT, DB_COLLINEAR, DB_F77NULL, ierr)
      end

      subroutine write_mixedmaterial(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer NX, NY, NX1, NY1
      parameter (NX = 5)
      parameter (NY = 4)
      parameter (NX1 = NX-1)
      parameter (NY1 = NY-1)
      integer err, ierr, optlist, ndims, nmats, mixlen
      integer mdims(2) /NX1, NY1/
      integer matnos(3) /1,2,3/

      integer matlist(12) /3, -1, -3, 1,
     . 3, -5, -7, 1,
     . 3, -9, -11, -14/

      real mix_vf(15) /0.75,0.25,     0.1875,0.8125,
     . 0.625,0.375,   0.4375,0.56250,
     . 0.3,0.7,       0.2,0.4,0.4,      0.45,0.55/

      integer mix_zone(15) /1,1,  2,2,
     . 5,5,  6,6,
     . 9,9,  10,10,10,  11,11/

      integer mix_mat(15) /2,3,  2,1,
     . 2,3,  2,1,
     . 2,3,  1,2,3,  2,1/

      integer mix_next(15) /2,0,  4,0,
     . 6,0,  8,0,
     . 10,0, 12,13,0,  15,0/

      ndims = 2
      nmats = 3
      mixlen = 15
c Write out the material
      err = dbputmat(dbfile, "mat", 3, "quadmesh", 8, nmats, matnos,
     . matlist, mdims, ndims, mix_next, mix_mat, mix_zone, mix_vf,
     . mixlen, DB_FLOAT, DB_F77NULL, ierr)
      end
