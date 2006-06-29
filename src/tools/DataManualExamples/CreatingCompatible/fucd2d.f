      program main
      implicit none
      include "silo.inc"
      integer dbfile, ierr

c Create the Silo file
      ierr = dbcreate("fucd2d.silo", 11, DB_CLOBBER, DB_LOCAL, 
     .                "Unstructured 2D mesh", 20, DB_HDF5, dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          goto 10000
      endif
c Add other Silo calls here.
      call write_ucd2d(dbfile)
c Close the Silo file.
      ierr = dbclose(dbfile)
10000 stop
      end

c    
c   *---*---*--*   nodes (5,6,7,8)
c   |   |\5/ 3 |
c   | 1 |4*----*   nodes (3,4)
c   |   |/  2  |
c   *---*------*   nodes(0,1,2)
c 
c   cell 1 quad(0,1,6,5)
c   cell 2 quad(1,2,4,3)
c   cell 3 quad(3,4,8,7)
c   cell 4 tri(1,3,6)
c   cell 5 tri(3,7,6)
c 
      subroutine write_ucd2d(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, ndims, nshapetypes, nnodes, nzones
c Node coordinates
      real x(9) /0., 2., 5., 3., 5., 0., 2., 4., 5./
      real y(9) /0., 0., 0., 3., 3., 5., 5., 5., 5./
c Connectivity
      integer LNODELIST
      parameter (LNODELIST = 18)
      integer nodelist(LNODELIST) /2,4,7,
     .  4,8,7,
     .  1,2,7,6,
     .  2,3,5,4,
     .  4,5,9,8/
c Shape type 1 has 3 nodes (tri), shape type 2 is quad
      integer shapesize(2) /3, 4/
c We have 2 tris and 3 quads
      integer shapecounts(2) /2, 3/
      nshapetypes = 2
      nnodes = 9
      nzones = 5
      ndims = 2
c Write out connectivity information.
      err = dbputzl(dbfile, "zonelist", 8, nzones, ndims, nodelist,
     . LNODELIST, 1, shapesize, shapecounts, nshapetypes, ierr)
c Write an unstructured mesh
      err = dbputum(dbfile, "mesh", 4, ndims, x, y, DB_F77NULL,
     . "X", 1, "Y", 1, DB_F77NULL, 0, DB_FLOAT, nnodes, nzones,
     . "zonelist", 8, DB_F77NULL, 0, DB_F77NULL, ierr)
      end
