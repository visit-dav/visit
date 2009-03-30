c-----------------------------------------------------------------------------
c
c Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
c Produced at the Lawrence Livermore National Laboratory
c LLNL-CODE-400142
c All rights reserved.
c
c This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
c full copyright notice is contained in the file COPYRIGHT located at the root
c of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
c
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
      integer dbfile, ierr

c Create the Silo file.
      ierr = dbcreate("fquadvar2d.silo", 15, DB_CLOBBER, DB_LOCAL, 
     .                "Quadvar example", 15 , DB_HDF5, dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          goto 10000
      endif
c Add other Silo calls here.
      call write_rect2d(dbfile)
      call write_zonecent_quadvar2d(dbfile)
      call write_nodecent_quadvar2d(dbfile)
c Close the Silo file.
      ierr = dbclose(dbfile)
10000 stop
      end

      subroutine write_rect2d(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, dims(2), ndims, optlistid
      real x(4), y(5)
      data x/0., 1., 2.5, 5./
      data y/0., 2., 2.25, 2.55,  5./
      ndims = 2
      data dims/4, 5/
      err = dbmkoptlist(4, optlistid)
      err = dbaddcopt(optlistid, DBOPT_XLABEL, "Pressure", 8);
      err = dbaddcopt(optlistid, DBOPT_XUNITS, "kP", 2)
      err = dbaddcopt(optlistid, DBOPT_YLABEL, "Temperature", 11)
      err = dbaddcopt(optlistid, DBOPT_YUNITS, "Celsius", 7)
      err = dbputqm (dbfile, "quadmesh", 8, "xc", 2, 
     . "yc", 2, "zc", 2, x, y, DB_F77NULL, dims, ndims, 
     . DB_FLOAT, DB_COLLINEAR, optlistid, ierr)
      err = dbfreeoptlist(optlistid)
      end

      subroutine write_zonecent_quadvar2d(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, dims(2), ndims, NX, NY, ZX, ZY
      parameter (NX = 4)
      parameter (NY = 5)
      parameter (ZX = NX-1)
      parameter (ZY = NY-1)
      real             var1(ZX,ZY)
      double precision var2(ZX,ZY)
      integer          var3(ZX,ZY)
      character        var4(ZX,ZY)
      data var1/0., 1., 2.,
     . 3.,  4.,  5.,
     . 6.,  7.,  8.,
     . 9., 10., 11./
      data var2/0.,1.11,2.22,
     . 3.33, 4.44, 5.55,
     . 6.66, 7.77, 8.88,
     . 9.99, 10.1, 11.11/
      data var3/0,1,2,
     . 3, 4, 5,
     . 6, 7, 8,
     . 9, 10, 11/
      data var4/'a','b','c',
     . 'd', 'e', 'f',
     . 'g', 'h', 'i',
     . 'j', 'k', 'l'/
      data dims/ZX, ZY/
      ndims = 2
      err = dbputqv1(dbfile, "var1", 4, "quadmesh", 8, var1, dims, 
     . ndims, DB_F77NULL, 0, DB_FLOAT, DB_ZONECENT, DB_F77NULL, ierr)
c Write a double-precision variable
      err = dbputqv1(dbfile, "var2", 4, "quadmesh", 8, var2, dims,
     . ndims, DB_F77NULL, 0, DB_DOUBLE, DB_ZONECENT, DB_F77NULL, ierr)
c Write an integer variable
      err = dbputqv1(dbfile, "var3", 4, "quadmesh", 8, var3, dims,
     . ndims, DB_F77NULL, 0, DB_INT, DB_ZONECENT, DB_F77NULL, ierr)
c Write a char variable
      err = dbputqv1(dbfile, "var4", 4, "quadmesh", 8, var4, dims,
     . ndims, DB_F77NULL, 0, DB_CHAR, DB_ZONECENT, DB_F77NULL, ierr)
      end

      subroutine write_nodecent_quadvar2d(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, dims(2), ndims, NX, NY
      parameter (NX = 4)
      parameter (NY = 5)
      real    nodal(NX, NY)
      data dims/NX, NY/
      data nodal/0.,   1.,  2., 3.,
     . 4.,   5.,  6., 7.,
     . 8.,   9., 10., 11.,
     . 12., 13., 14., 15.,
     . 16., 17., 18., 19./
      ndims = 2
      err = dbputqv1(dbfile, "nodal", 5, "quadmesh", 8, nodal, dims, 
     . ndims, DB_F77NULL, 0, DB_FLOAT, DB_NODECENT, DB_F77NULL, ierr)
      end
