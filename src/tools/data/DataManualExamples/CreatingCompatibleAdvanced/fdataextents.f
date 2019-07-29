c Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
c Project developers.  See the top-level LICENSE file for dates and other
c details.  No copyright assignment is required to contribute to VisIt.

      program main
      implicit none
      double precision extents(2,4)
      call write_domains(extents)
      call write_master(extents)
      stop
      end

      subroutine write_domains(extents)
      implicit none
      include "silo.inc"
      double precision extents(2,4)
      integer dbfile, err, ierr, i,j, dom, dims(2), ndims, nmesh
      real x(4), y(5), var(4,5), dx, dy
      real xc(4), yc(5), tx(4), ty(4)
      character*14 filename  /'fdataextents.X'/
      data x/0., 1., 2.5, 5./
      data y/0., 2., 2.25, 2.55,  5./
      data tx /0., -5., -5., 0./
      data ty /0., 0., -5., -5./
      data dims/4, 5/

      ndims = 2
      nmesh = 4
      do 10030 dom=1,nmesh
c Poke a number into the filename.
          filename(14:) = char(48 + dom)
c Create a new silo file.
          ierr = dbcreate(filename, 14, DB_CLOBBER, DB_LOCAL,
     .                    "dataextents data", 16, DB_HDF5, dbfile)
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
                  dx = xc(i) - 5.
                  dy = yc(j) - 5.
                  var(i, j) = sqrt(dx*dx + dy*dy)

c Determine the extents for this domain.
                  if(i==1 .and. j==1) then
                      extents(1, dom) = var(i,j)
                  else if(var(i,j) < extents(1,dom)) then
                      extents(1, dom) = var(i,j)
                  endif
                  if(i==1 .and. j==1) then
                      extents(2, dom) = var(i,j)
                  else if(var(i,j) > extents(2,dom)) then
                      extents(2, dom) = var(i,j)
                  endif
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
      character*25 meshnames(4) /'fdataextents.1:quadmesh  ',
     .                           'fdataextents.2:quadmesh  ',
     .                           'fdataextents.3:quadmesh  ',
     .                           'fdataextents.4:quadmesh  '/
      integer lmeshnames(4) /23,23,23,23/
      integer meshtypes(4) /DB_QUAD_RECT, DB_QUAD_RECT,
     .                      DB_QUAD_RECT, DB_QUAD_RECT/
      nmesh = 4
      err = dbputmmesh(dbfile, "quadmesh", 8, nmesh, meshnames,
     . lmeshnames, meshtypes, DB_F77NULL, ierr)
      end


      subroutine write_multivar(dbfile, extents)
      implicit none
      include "silo.inc"
      double precision extents(2,4)
      integer err, ierr, dbfile, nvar, optlist
      character*25 varnames(4)  /'fdataextents.1:var       ',
     .                           'fdataextents.2:var       ',
     .                           'fdataextents.3:var       ',
     .                           'fdataextents.4:var       '/
      integer lvarnames(4) /18,18,18,18/
      integer vartypes(4) /DB_QUADVAR,DB_QUADVAR,
     .                     DB_QUADVAR,DB_QUADVAR/
      nvar = 4
c Add the data extents to the optlist that we use to write the multivar
      err = dbmkoptlist(2, optlist)
      err = dbaddiopt(optlist, DBOPT_EXTENTS_SIZE, 2)
      err = dbadddopt(optlist, DBOPT_EXTENTS, extents)
      err = dbputmvar(dbfile, "var", 3, nvar, varnames, lvarnames,
     . vartypes, optlist, ierr)
      err = dbfreeoptlist(optlist)
      end


      subroutine write_master(extents)
      implicit none
      include "silo.inc"
      double precision extents(2,4)
      integer err, ierr, dbfile, oldlen
c Create a new silo file
      ierr = dbcreate("fdataextents.root", 17, DB_CLOBBER, DB_LOCAL,
     . "dataextents root", 17, DB_HDF5, dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          return
      endif
c Set the maximum string length to 25
      oldlen = dbget2dstrlen()
      err = dbset2dstrlen(25)

c Write the multimesh and multivar objects
      call write_multimesh(dbfile)
      call write_multivar(dbfile, extents)

c Restore the previous value for maximum string length
      err = dbset2dstrlen(oldlen)
c Close the Silo file
      ierr = dbclose(dbfile)
      end
