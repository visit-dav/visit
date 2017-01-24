c-----------------------------------------------------------------------------c
c Copyright (c) 2000 - 2016, Lawrence Livermore National Security, LLC
c Produced at the Lawrence Livermore National Laboratoryc LLNL-CODE-442911
c All rights reserved.c
c This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
c full copyright notice is contained in the file COPYRIGHT located at the root
c of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
c Redistribution  and  use  in  source  and  binary  forms,  with  or  without
c modification, are permitted provided that the following conditions are met:
c
c  - Redistributions of  source code must  retain the above  copyright notice,
c    this list of conditions and the disclaimer below.
c  - Redistributions in binary form must reproduce the above copyright notice,
c    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
c    documentation and/or other materials provided with the distribution.
c  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
c    be used to endorse or promote products derived from this software without
c    specific prior written permission.
c
c THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
c AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
c IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
c ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
c LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
c DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
c DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
c SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
c CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
c LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
c OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
c DAMAGE.
c
c-----------------------------------------------------------------------------
      program main
      implicit none
      include "silo.inc"
      include "fspatialextents.inc"
      double precision data_extents(2,NXDOMS*NYDOMS*NZDOMS)
      double precision spatial_extents(6,NXDOMS*NYDOMS*NZDOMS)
      integer err, ierr, dbfile
c Create a new silo file
      ierr = dbcreate("fmultimat.silo", 14, DB_CLOBBER,
     . DB_LOCAL, "multimaterial file example", 25, DB_HDF5,
     . dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          stop 'KO'
      endif
c Set the maximum string length to 20
      err = dbset2dstrlen(20)
c Write the multimesh and multivar and multimat objects
      call write_domains(dbfile, spatial_extents, data_extents)
      call write_multimesh(dbfile, spatial_extents)
      call write_multivar(dbfile, data_extents)
      call write_multimat(dbfile, data_extents)
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
      function getmatid(i,j,k,nx,ny,nz)
      integer ::i,j,k,nx,ny,nz
      integer :: getmatid
      if (k >= 5 .and. k <= 7 .and. i >= 5 .and. i <= nx-5
     .     .and. j >= 5 .and. j <= ny-5) then
         getmatid = 2
      elseif (k <= nz-5 .and. k >= nz-7 .and. i >= 5 .and. i <= nx-5
     .        .and. j >= 5 .and. j <= ny-5) then
         getmatid = 2
      elseif (i >= 5 .and. i <= 7 .and. j >= 5 .and. j <= ny-5
     .        .and. k >= 5 .and. k <= nz-5) then
         getmatid = 2
      elseif (i <= nx-5 .and. i >= nx-7 .and. j >= 5 .and. j <= ny-5
     .        .and. k >= 5 .and. k <= nz-5) then
         getmatid = 2
      else
         getmatid = 1
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
      integer imat((NX+2)*(NY+2)*(NZ+2))
      real xc(NX+2), yc(NY+2), zc(NZ+2)
      real xstart, xend, ystart, yend, zstart, zend
      real dx, dy, dz, cx, cy, cz, t
      character*9 dirname  /'DomainXXX'/
c Adding the material specific infos:
      integer matnos(2)
      integer optmist
      integer, dimension(2) :: matlens, loptnames
      character(LEN=20) :: optcolors(2)
      character(LEN=20) :: optnames(2)
      integer getmatid
c Determine the size of a cell
      cx = XSIZE / float(NX-1)
      cy = YSIZE / float(NY-1)
      cz = ZSIZE / float(NZ-1)
c Making the material specific attributes:
      matnos(1) = 1
      matnos(2) = 2 
      err = dbmkoptlist(2, optmist)
      optcolors(1) = '#FF0000'
      optcolors(2) = '#00FF00'
      optnames(1) = 'MAT1'
      optnames(2) = 'MAT2'
      loptnames(1) = 4
      loptnames(2) = 4
      matlens(1) = 7
      matlens(2) = 7
      err = dbaddcaopt(optmist, DBOPT_MATCOLORS, 2, optcolors, matlens)
      err = dbaddcaopt(optmist, DBOPT_MATNAMES, 2, optnames, loptnames)
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
c Create the material values.
          index = 1
          do 10061 k=1,zzones
          do 10071 j=1,yzones
          do 10081 i=1,xzones
             imat(index) = getmatid(
     .            i+(xdom-1)*NX,j+(ydom-1)*NY,k+(zdom-1)*NZ,
     .            NX*NXDOMS,NY*NYDOMS,NZ*NZDOMS
     .            )
             index = index + 1
10081     continue
10071     continue
10061     continue
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
c Write the material
          err = dbputmat(dbfile, "mat",3, "quadmesh",8, 2,matnos, imat,
     .    vardims, ndims, DB_F77NULL,DB_F77NULL,DB_F77NULL,DB_F77NULL,
     .    0, DB_FLOAT, optmist, ierr)
c Go back to the parent directory
          err = dbsetdir(dbfile, "..", 2)
          dom = dom + 1
10020 continue
10010 continue
10000 continue
      err = dbfreeoptlist(optmist)
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

      subroutine create_matname(matname, dom)
      implicit none
      character*20 matname
      character*20 domstr /'DomainXXX/mat       '/
      integer dom, i
      do 40000 i=1,20
          matname(i:) = domstr(i:)
40000 continue
      call create_dirname(matname, dom)
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

      subroutine write_multimat(dbfile)
      implicit none
      include "silo.inc"
      include "fspatialextents.inc"
      integer err, ierr, dbfile, nmat, optlist
      character*20 matnames(NDOMS)
      integer lmatnames(NDOMS), i
      integer, dimension(2) :: matlens, loptnames
      character(LEN=20) :: optcolors(2)
      character(LEN=20) :: optnames(2)
      nmat = NDOMS
      do 20000 i=1,nmat
          call create_matname(matnames(i), i)
          lmatnames(i) = 13
20000 continue
c Add the data extents to the optlist that we use to write the multivar
      err = dbmkoptlist(2, optlist)
      optcolors(1) = '#FF0000'
      optcolors(2) = '#00FF00'
      optnames(1) = 'MAT1'
      optnames(2) = 'MAT2'
      loptnames(1) = 4
      loptnames(2) = 4
      matlens(1) = 7
      matlens(2) = 7
      err = dbaddcaopt(optlist, DBOPT_MATCOLORS, 2, optcolors, matlens)
      err = dbaddcaopt(optlist, DBOPT_MATNAMES, 2, optnames, loptnames)
      err = dbputmmat(dbfile, "mat", 3, nmat, matnames, lmatnames,
     . optlist, ierr)
      err = dbfreeoptlist(optlist)
      end























