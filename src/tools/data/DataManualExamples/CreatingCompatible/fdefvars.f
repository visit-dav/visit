c Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
c Project developers.  See the top-level LICENSE file for dates and other
c details.  No copyright assignment is required to contribute to VisIt.

      program main
      implicit none
      include "silo.inc"
      integer dbfile, ierr

c Create the Silo file
      ierr = dbcreate("fdefvars.silo", 13, DB_CLOBBER, DB_LOCAL, 
     .                "This file has defvars", 21, DB_HDF5, dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          goto 10000
      endif
c Add other Silo calls here.
      call write_curv3d(dbfile)
      call write_zonecent_quadvar(dbfile)
      call write_nodecent_quadvar(dbfile)
      call write_defvars(dbfile)
c Close the Silo file.
      ierr = dbclose(dbfile)
10000 stop
      end

      subroutine write_curv3d(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, dims(3), ndims
      real x(4,3,2), y(4,3,2), z(4,3,2)
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

c Write the mesh coordinates as nodal variables.
      err = dbputqv1(dbfile, "xc", 2, "quadmesh", 8, x, dims, 
     . ndims, DB_F77NULL, 0, DB_INT, DB_NODECENT, DB_F77NULL, ierr)
      err = dbputqv1(dbfile, "yc", 2, "quadmesh", 8, y, dims, 
     . ndims, DB_F77NULL, 0, DB_INT, DB_NODECENT, DB_F77NULL, ierr)
      err = dbputqv1(dbfile, "zc", 2, "quadmesh", 8, z, dims, 
     . ndims, DB_F77NULL, 0, DB_INT, DB_NODECENT, DB_F77NULL, ierr)
      end

      subroutine write_zonecent_quadvar(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, dims(3), ndims, i,j,k,index
      integer    zonal(2,3,1)
      data dims/3,2,1/
      index = 0
      do 10020 k=1,1
      do 10010 j=1,3
      do 10000 i=1,2
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
      integer err, ierr, dims(3), ndims, i,j,k,index
      real    nodal(4,3,2)
      data dims/4,3,2/
      index = 0
      do 20020 k=1,2
      do 20010 j=1,3
      do 20000 i=1,4
          nodal(i,j,k) = float(index)
          index = index + 1
20000 continue
20010 continue
20020 continue
      ndims = 3
      err = dbputqv1(dbfile, "nodal", 5, "quadmesh", 8, nodal, dims, 
     . ndims, DB_F77NULL, 0, DB_FLOAT, DB_NODECENT, DB_F77NULL, ierr)
      end

      subroutine write_defvars(dbfile)
            implicit none
      integer dbfile
      include "silo.inc"
      integer     err, ierr, types(2), lnames(2), ldefs(2), oldlen
      integer     optlists(2)
c Initialize some 20 character length strings
      character*20 names(2) /'velocity            ',
     .                       'speed               '/
      character*20 defs(2)  /'{xc,yc,zc}          ',
     .                       'magnitude(velocity) '/
c Store the length of each string
      data lnames/8, 5/
      data ldefs/10, 19/
      data types/DB_VARTYPE_VECTOR, DB_VARTYPE_SCALAR/
c Store NULL in the option lists for dbputdefvars
      data optlists/DB_F77NULL, DB_F77NULL/
c Set the maximum string length to 20 since that's how long our strings are
      oldlen = dbget2dstrlen()
      err = dbset2dstrlen(20)
c Write out the expressions
      err = dbputdefvars(dbfile, "defvars", 7, 2, names, lnames, types,
     . defs, ldefs, optlists, ierr)
c Restore the previous value for maximum string length
      err = dbset2dstrlen(oldlen) 
      end
