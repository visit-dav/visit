c Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
c Project developers.  See the top-level LICENSE file for dates and other
c details.  No copyright assignment is required to contribute to VisIt.

      program main
      implicit none
      include "silo.inc"
      include "fspatialextents.inc"
      double precision data_extents(2,NXDOMS*NYDOMS*NZDOMS)
      double precision spatial_extents(6,NXDOMS*NYDOMS*NZDOMS)
      integer err, ierr, dbfile
c Create a new silo file
      ierr = dbcreate("fspatialextents.silo", 20, DB_CLOBBER, DB_LOCAL,
     . "spatial extents root", 20, DB_HDF5, dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          goto 10000
      endif
c Set the maximum string length to 20
      err = dbset2dstrlen(20)

c Write the multimesh and multivar objects
      call write_domains(dbfile, spatial_extents, data_extents)
      call write_multimesh(dbfile, spatial_extents)
      call write_multivar(dbfile, data_extents)

c Close the Silo file
      ierr = dbclose(dbfile)
10000 stop
      end


      subroutine create_dirname(dirname, dom)
      implicit none
      character*9 dirname
      integer dom, d100, d10, d
      d100 = dom / 100
      d10 = (dom - (d100 * 100)) / 10
      d = dom - (d100 * 100) - (d10 * 10)
      if(dom >= 100) then
          dirname(7:) = char(48 + d100)
          dirname(8:) = char(48 + d10)
          dirname(9:) = char(48 + d)
      else if(dom >= 10) then
          dirname(7:) = '0'
          dirname(8:) = char(48 + d10)
          dirname(9:) = char(48 + d)
      else
          dirname(7:) = '0'
          dirname(8:) = '0'
          dirname(9:) = char(48 + dom)
      endif
      end


      subroutine write_domains(dbfile, spatial_extents, data_extents)
      implicit none
      include "silo.inc"
      include "fspatialextents.inc"
      double precision data_extents(2,NXDOMS*NYDOMS*NZDOMS)
      double precision spatial_extents(6,NXDOMS*NYDOMS*NZDOMS)
      integer dbfile, err, ierr, i,j,k, index
      integer dom, dims(3), vardims(3), ndims, nmesh, loopflag
      integer xdom, ydom, zdom
      real var(NX-1,NY-1,NZ-1), dx, dy, dz, t
      real xc(NX), yc(NY), zc(NZ)
      character*9 dirname  /'DomainXXX'/
      data dims/NX, NY, NZ/
      data vardims/NX1, NY1, NZ1/

      ndims = 3
      dom = 1
      do 10000 zdom=1,NZDOMS
      do 10010 ydom=1,NYDOMS
      do 10020 xdom=1,NXDOMS
c Create a new directory
          call create_dirname(dirname, dom)
          err = dbmkdir(dbfile, dirname, 9, ierr)
          err = dbsetdir(dbfile, dirname, 9)

c Create the mesh coordinates
          do 10030 i=1,NX
              t = float(i-1) / float(NX-1)
              xc(i) = float(xdom) * XSIZE + t * XSIZE
10030     continue
          do 10040 i=1,NY
              t = float(i-1) / float(NY-1)
              yc(i) = float(ydom) * YSIZE + t * YSIZE
10040     continue
          do 10050 i=1,NZ
              t = float(i-1) / float(NZ-1)
              zc(i) = float(zdom) * ZSIZE + t * ZSIZE
10050     continue

c Create the variable value.
          do 10060 k=1,NZ-1
          do 10070 j=1,NY-1
          do 10080 i=1,NX-1
              dx = xc(i) - 5.
              dy = yc(j) - 5.
              dz = zc(k) - 5.
              var(i,j,k) = sqrt(dx*dx + dy*dy + dz*dz)
10080     continue
10070     continue
10060     continue

c Figure out the spatial extents for the domain
          spatial_extents(1, dom) = xc(1)
          spatial_extents(2, dom) = yc(1)
          spatial_extents(3, dom) = zc(1)
          spatial_extents(4, dom) = xc(NX)
          spatial_extents(5, dom) = yc(NY)
          spatial_extents(6, dom) = zc(NZ)

c Figure out the data extents for the domain
          loopflag = 1
          do 10090 k=1,NZ-1
          do 10100 j=1,NY-1
          do 10110 i=1,NX-1
              if(loopflag.eq.1) then
                  data_extents(1,dom) = var(i,j,k)
                  data_extents(2,dom) = var(i,j,k)
                  loopflag = 0
              else
                  if(var(i,j,k) < data_extents(1,dom)) then
                      data_extents(1,dom) = var(i,j,k)
                  endif
                  if(var(i,j,k) > data_extents(2,dom)) then
                      data_extents(2,dom) = var(i,j,k)
                  endif
              endif
10110     continue
10100     continue
10090     continue

c Write the quadmesh
          err = dbputqm (dbfile, "quadmesh", 8, "xc", 2, 
     .    "yc", 2, "zc", 2, xc, yc, zc, dims, ndims, 
     .    DB_FLOAT, DB_COLLINEAR, DB_F77NULL, ierr)
c Write the quadvar
          err = dbputqv1(dbfile, "var", 3, "quadmesh", 8, var, vardims,
     .    ndims, DB_F77NULL, 0, DB_FLOAT, DB_ZONECENT, DB_F77NULL,
     .    ierr)

c Go back to the parent directory
          err = dbsetdir(dbfile, "..", 2)
          dom = dom + 1
10020 continue
10010 continue
10000 continue
      end


      subroutine create_meshname(meshname, dom)
      implicit none
      character*20 meshname
      character*20 domstr /'DomainXXX/quadmesh  '/
      integer dom, i
      do 30000 i=1,20
          meshname(i:) = domstr(i:)
30000 continue
      call create_dirname(meshname, dom)
      end


      subroutine create_varname(varname, dom)
      implicit none
      character*20 varname
      character*20 domstr /'DomainXXX/var       '/
      integer dom, i
      do 40000 i=1,20
          varname(i:) = domstr(i:)
40000 continue
      call create_dirname(varname, dom)
      end


      subroutine write_multimesh(dbfile, spatial_extents)
      implicit none
      include "silo.inc"
      include "fspatialextents.inc"
      double precision spatial_extents(6,NDOMS)
      integer err, ierr, dbfile, optlist, nmesh
      character*20 meshnames(NDOMS)
      integer lmeshnames(NDOMS), meshtypes(NDOMS), i

      nmesh = NDOMS
      do 20000 i=1,nmesh
          call create_meshname(meshnames(i), i)
          lmeshnames(i) = 18
          meshtypes(i) = DB_QUAD_RECT
20000 continue

      err = dbmkoptlist(2, optlist)
      err = dbaddiopt(optlist, DBOPT_EXTENTS_SIZE, 6)
      err = dbadddopt(optlist, DBOPT_EXTENTS, spatial_extents)
      err = dbputmmesh(dbfile, "quadmesh", 8, nmesh, meshnames,
     . lmeshnames, meshtypes, optlist, ierr)
      err = dbfreeoptlist(optlist)
      end


      subroutine write_multivar(dbfile, data_extents)
      implicit none
      include "silo.inc"
      include "fspatialextents.inc"
      double precision data_extents(2,NDOMS)
      integer err, ierr, dbfile, nvar, optlist
      character*20 varnames(NDOMS)
      integer lvarnames(NDOMS), vartypes(NDOMS), i

      nvar = NDOMS
      do 20000 i=1,nvar
          call create_varname(varnames(i), i)
          lvarnames(i) = 13
          vartypes(i) = DB_QUADVAR
20000 continue

c Add the data extents to the optlist that we use to write the multivar
      err = dbmkoptlist(2, optlist)
      err = dbaddiopt(optlist, DBOPT_EXTENTS_SIZE, 2)
      err = dbadddopt(optlist, DBOPT_EXTENTS, data_extents)
      err = dbputmvar(dbfile, "var", 3, nvar, varnames, lvarnames,
     . vartypes, optlist, ierr)
      err = dbfreeoptlist(optlist)
      end
