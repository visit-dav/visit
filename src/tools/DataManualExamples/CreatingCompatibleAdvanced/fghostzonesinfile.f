      program main
      implicit none
      include "silo.inc"
      include "fspatialextents.inc"
      double precision data_extents(2,NXDOMS*NYDOMS*NZDOMS)
      double precision spatial_extents(6,NXDOMS*NYDOMS*NZDOMS)
      integer err, ierr, dbfile
c Create a new silo file
      ierr = dbcreate("fghostzonesinfile.silo", 22, DB_CLOBBER,
     . DB_LOCAL, "Ghost zones in file example", 26, DB_HDF5,
     . dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          return
      endif
c Set the maximum string length to 20
      err = dbset2dstrlen(20)

c Write the multimesh and multivar objects
      call write_domains(dbfile, spatial_extents, data_extents)
      call write_multimesh(dbfile, spatial_extents)
      call write_multivar(dbfile, data_extents)

c Close the Silo file
      ierr = dbclose(dbfile)
      stop
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
      integer dbfile, optlist, err, ierr, i,j,k, index
      integer dom, dims(3), vardims(3), ndims
      integer xdom, ydom, zdom
      integer xzones, yzones, zzones, nzones
      integer xnodes, ynodes, znodes
      integer lo_offset(3), hi_offset(3)
      real var((NX+2)*(NY+2)*(NZ+2))
      real xc(NX+2), yc(NY+2), zc(NZ+2)
      real xstart, xend, ystart, yend, zstart, zend
      real dx, dy, dz, cx, cy, cz, t
      character*9 dirname  /'DomainXXX'/

c Determine the size of a cell
      cx = XSIZE / float(NX-1)
      cy = YSIZE / float(NY-1)
      cz = ZSIZE / float(NZ-1)

c Create each of the domain meshes and data
      ndims = 3
      dom = 1
      do 10000 zdom=1,NZDOMS
      do 10010 ydom=1,NYDOMS
      do 10020 xdom=1,NXDOMS
c Create a new directory
          call create_dirname(dirname, dom)
          err = dbmkdir(dbfile, dirname, 9, ierr)
          err = dbsetdir(dbfile, dirname, 9)

c Determine the default start, end coordinates
          xstart = float(xdom-1) * XSIZE
          xend   = float(xdom) * XSIZE
          xzones = NX-1

          ystart = float(ydom-1) * YSIZE
          yend   = float(ydom) * YSIZE
          yzones = NY-1

          zstart = float(zdom-1) * ZSIZE
          zend   = float(zdom) * ZSIZE
          zzones = NZ-1

c Set the starting hi/lo offsets
          lo_offset(1) = 0
          lo_offset(2) = 0
          lo_offset(3) = 0

          hi_offset(1) = 0
          hi_offset(2) = 0
          hi_offset(3) = 0

c Adjust the start and end coordinates based on whether or not we
c have ghost zones
          if(xdom > 1) then
              xstart = xstart - cx
              lo_offset(1) = 1
              xzones = xzones + 1
          endif
          if(xdom < NXDOMS) then
              xend = xend + cx
              hi_offset(1) = 1
              xzones = xzones + 1
          endif
          if(ydom > 1) then
              ystart = ystart - cy
              lo_offset(2) = 1
              yzones = yzones + 1
          endif
          if(ydom < NYDOMS) then
              yend = yend + cy
              hi_offset(2) = 1
              yzones = yzones + 1
          endif
          if(zdom > 1) then
              zstart = zstart - cz
              lo_offset(3) = 1
              zzones = zzones + 1
          endif
          if(zdom < NZDOMS) then
              zend = zend + cz
              hi_offset(3) = 1
              zzones = zzones + 1
          endif

          xnodes = xzones + 1
          ynodes = yzones + 1
          znodes = zzones + 1

          nzones = xzones * yzones * zzones
          dims(1) = xnodes
          dims(2) = ynodes
          dims(3) = znodes

          vardims(1) = xzones
          vardims(2) = yzones
          vardims(3) = zzones

c Create the mesh coordinates
          do 10030 i=1,xnodes
              t = float(i-1) / float(xnodes-1)
              xc(i) = (1.-t)*xstart + t*xend
10030     continue
          do 10040 i=1,ynodes
              t = float(i-1) / float(ynodes-1)
              yc(i) = (1.-t)*ystart + t*yend
10040     continue
          do 10050 i=1,znodes
              t = float(i-1) / float(znodes-1)
              zc(i) = (1.-t)*zstart + t*zend
10050     continue

c Create the variable value.
          index = 1
          do 10060 k=1,zzones
          do 10070 j=1,yzones
          do 10080 i=1,xzones
              dx = xc(i) - 5.
              dy = yc(j) - 5.
              dz = zc(k) - 5.
              var(index) = sqrt(dx*dx + dy*dy + dz*dz)
              index = index + 1
10080     continue
10070     continue
10060     continue

c Figure out the spatial extents for the domain
          spatial_extents(1, dom) = xc(1)
          spatial_extents(2, dom) = yc(1)
          spatial_extents(3, dom) = zc(1)
          spatial_extents(4, dom) = xc(xnodes)
          spatial_extents(5, dom) = yc(ynodes)
          spatial_extents(6, dom) = zc(znodes)

c Figure out the data extents for the domain
          data_extents(1,dom) = var(1)
          data_extents(2,dom) = var(1)
          do 10090 index=2,nzones
              if(var(index) < data_extents(1,dom)) then
                  data_extents(1,dom) = var(index)
              endif
              if(var(index) > data_extents(2,dom)) then
                  data_extents(2,dom) = var(index)
              endif
10090     continue

c Write the quadmesh
          err = dbmkoptlist(2, optlist)
          err = dbaddiopt(optlist, DBOPT_HI_OFFSET, hi_offset)
          err = dbaddiopt(optlist, DBOPT_LO_OFFSET, lo_offset)
          err = dbputqm (dbfile, "quadmesh", 8, "xc", 2, 
     .    "yc", 2, "zc", 2, xc, yc, zc, dims, ndims, 
     .    DB_FLOAT, DB_COLLINEAR, optlist, ierr)
          err = dbfreeoptlist(optlist)

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
