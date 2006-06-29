      program main
      implicit none
      call write_domains()
      call write_master()
      stop
      end

      subroutine write_domains()
      implicit none
      include "silo.inc"
      integer dbfile, err, ierr, i, dom, dims(2), ndims, nmesh
      real x(4), y(5)
      real xc(4), yc(5), tx(4), ty(4)
      character*11 filename  /'multimesh.X'/
      data x/0., 1., 2.5, 5./
      data y/0., 2., 2.25, 2.55,  5./
      data tx /0., -5., -5., 0./
      data ty /0., 0., -5., -5./
      data dims/4, 5/

      ndims = 2
      nmesh = 4
      do 10020 dom=1,nmesh
c Poke a number into the filename.
          filename(11:) = char(48 + dom)
c Create a new silo file.
          ierr = dbcreate(filename, 11, DB_CLOBBER, DB_LOCAL,
     .                    "multimesh data", 14, DB_HDF5, dbfile)
          if(dbfile.eq.-1) then
              write (6,*) 'Could not create Silo file!\n'
              return
          endif
c Displace the coordinates
          do 10000 i=1,4
              xc(i) = x(i) + tx(dom)
10000     continue
          do 10010 i=1,5
              yc(i) = y(i) + ty(dom)
10010     continue
c Write the multimesh
          err = dbputqm (dbfile, "quadmesh", 8, "xc", 2, 
     .    "yc", 2, "zc", 2, xc, yc, DB_F77NULL, dims, ndims, 
     .    DB_FLOAT, DB_COLLINEAR, DB_F77NULL, ierr)
c Close the Silo file
          ierr = dbclose(dbfile)
10020 continue
      end


      subroutine write_master()
      implicit none
      include "silo.inc"
      integer err, ierr, dbfile, nmesh, oldlen
      character*20 meshnames(4) /'multimesh.1:quadmesh',
     .                           'multimesh.2:quadmesh',
     .                           'multimesh.3:quadmesh',
     .                           'multimesh.4:quadmesh'/
      integer lmeshnames(4) /20,20,20,20/
      integer meshtypes(4) /DB_QUAD_RECT, DB_QUAD_RECT,
     .                      DB_QUAD_RECT, DB_QUAD_RECT/
c Create a new silo file
      ierr = dbcreate("multimesh.root", 14, DB_CLOBBER, DB_LOCAL,
     . "multimesh root", 14, DB_HDF5, dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          return
      endif
c Set the maximum string length to 20 since that's how long our strings are
      oldlen = dbget2dstrlen()
      err = dbset2dstrlen(20)
c Write the multimesh object.
      nmesh = 4
      err = dbputmmesh(dbfile, "quadmesh", 8, nmesh, meshnames,
     . lmeshnames, meshtypes, DB_F77NULL, ierr)
c Restore the previous value for maximum string length
      err = dbset2dstrlen(oldlen)
c Close the Silo file
      ierr = dbclose(dbfile)
      end
