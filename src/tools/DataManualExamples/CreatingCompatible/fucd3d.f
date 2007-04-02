      program main
      implicit none
      include "silo.inc"
      integer dbfile, ierr

c The 10 and 20 arguments represent the lengths of strings
      ierr = dbcreate("ucd3d.silo", 10, DB_CLOBBER, DB_LOCAL, 
     .                "Unstructured 3d mesh", 20, DB_HDF5, dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          goto 10000
      endif
c Add other Silo calls here.
      call write_ucd3d(dbfile)
c Close the Silo file.
      ierr = dbclose(dbfile)
10000 stop
      end


      subroutine write_ucd3d(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, ndims, nzones
      integer NSHAPETYPES, NNODES
      parameter (NSHAPETYPES = 4)
      parameter (NNODES = 16)
c Node coordinates
      real x(NNODES) 
     ./0.,2.,2.,0.,0.,2.,2.,0.,0.,2.,2.,0.,1.,2.,4.,4./
      real y(NNODES) /0.,0.,0.,0.,2.,2.,2.,2.,4.,4.,4.,4.,6.,0.,0.,0./
      real z(NNODES) /2.,2.,0.,0.,2.,2.,0.,0.,2.,2.,0.,0.,1.,4.,2.,0./
c Connectivity
      integer LNODELIST
      parameter (LNODELIST = 31)
      integer nodelist(LNODELIST) /1,2,3,4,5,6,7,8,
     . 5,6,7,8,9,10,11,12,
     . 9,10,11,12,13,
     . 2,3,16,15,6,7,
     . 2,15,14,6/
c Shape type 1 has 8 nodes (hex)
c Shape type 2 has 5 nodes (pyramid)
c Shape type 3 has 6 nodes (prism)
c Shape type 4 has 4 nodes (tet)
      integer shapesize(NSHAPETYPES) /8, 5, 6, 4/
c We have 2 hex, 1 pyramid, 1 prism, 1 tet
      integer shapecounts(NSHAPETYPES) /2, 1, 1, 1/
      nzones = 5
      ndims = 3
c Write out connectivity information.
      err = dbputzl(dbfile, "zonelist", 8, nzones, ndims, nodelist,
     . LNODELIST, 1, shapesize, shapecounts, NSHAPETYPES, ierr)
c Write an unstructured mesh
      err = dbputum(dbfile, "mesh", 4, ndims, x, y, z,
     . "X", 1, "Y", 1, "Z", 1, DB_FLOAT, NNODES, nzones,
     . "zonelist", 8, DB_F77NULL, 0, DB_F77NULL, ierr)
      end
