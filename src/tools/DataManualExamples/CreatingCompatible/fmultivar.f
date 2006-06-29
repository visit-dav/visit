      program main
      implicit none
      call write_domains()
      call write_master()
      stop
      end

      subroutine write_domains()
      implicit none
      include "silo.inc"
      integer dbfile, err, ierr, i,j, dom, dims(2), ndims, nmesh, index
      real x(4), y(5), var(4,5)
      real xc(4), yc(5), tx(4), ty(4)
      character*10 filename  /'multivar.X'/
      data x/0., 1., 2.5, 5./
      data y/0., 2., 2.25, 2.55,  5./
      data tx /0., -5., -5., 0./
      data ty /0., 0., -5., -5./
      data dims/4, 5/

      ndims = 2
      nmesh = 4
      index = 0
      do 10030 dom=1,nmesh
c Poke a number into the filename.
          filename(10:) = char(48 + dom)
c Create a new silo file.
          ierr = dbcreate(filename, 10, DB_CLOBBER, DB_LOCAL,
     .                    "multivar data", 13, DB_HDF5, dbfile)
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
          do 10020 j=1,5
              do 10021 i=1,4
                  var(i, j) = float(index)
                  index = index + 1
10021         continue
10020     continue
c Write the quadmesh
          err = dbputqm (dbfile, "quadmesh", 8, "xc", 2, 
     .    "yc", 2, "zc", 2, xc, yc, DB_F77NULL, dims, ndims, 
     .    DB_FLOAT, DB_COLLINEAR, DB_F77NULL, ierr)
c Write the quadvar
          err = dbputqv1(dbfile, "var", 3, "quadmesh", 8, var, dims,
     .    ndims, DB_F77NULL, 0, DB_FLOAT, DB_NODECENT, DB_F77NULL,
     .    ierr)
c Close the Silo file
          ierr = dbclose(dbfile)
10030 continue
      end


      subroutine write_multimesh(dbfile)
      implicit none
      include "silo.inc"
      integer err, ierr, dbfile, nmesh
      character*20 meshnames(4) /'multivar.1:quadmesh ',
     .                           'multivar.2:quadmesh ',
     .                           'multivar.3:quadmesh ',
     .                           'multivar.4:quadmesh '/
      integer lmeshnames(4) /19,19,19,19/
      integer meshtypes(4) /DB_QUAD_RECT, DB_QUAD_RECT,
     .                      DB_QUAD_RECT, DB_QUAD_RECT/
      nmesh = 4
      err = dbputmmesh(dbfile, "quadmesh", 8, nmesh, meshnames,
     . lmeshnames, meshtypes, DB_F77NULL, ierr)
      end


      subroutine write_multivar(dbfile)
      implicit none
      include "silo.inc"
      integer err, ierr, dbfile, nvar
      character*20 varnames(4) /'multivar.1:var     ',
     .                          'multivar.2:var     ',
     .                          'multivar.3:var     ',
     .                          'multivar.4:var     '/
      integer lvarnames(4) /14,14,14,14/
      integer vartypes(4) /DB_QUADVAR,DB_QUADVAR,
     .                     DB_QUADVAR,DB_QUADVAR/
      nvar = 4
      err = dbputmvar(dbfile, "var", 3, nvar, varnames, lvarnames,
     . vartypes, DB_F77NULL, ierr)
      end


      subroutine write_master()
      implicit none
      include "silo.inc"
      integer err, ierr, dbfile, oldlen
c Create a new silo file
      ierr = dbcreate("multivar.root", 13, DB_CLOBBER, DB_LOCAL,
     . "multimesh root", 14, DB_HDF5, dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          return
      endif
c Set the maximum string length to 20 since that's how long our strings are
      oldlen = dbget2dstrlen()
      err = dbset2dstrlen(20)

c Write the multimesh and multivar objects
      call write_multimesh(dbfile)
      call write_multivar(dbfile)

c Restore the previous value for maximum string length
      err = dbset2dstrlen(oldlen)
c Close the Silo file
      ierr = dbclose(dbfile)
      end
