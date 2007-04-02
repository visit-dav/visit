c-----------------------------------------------------------------------------
c
c Copyright (c) 2000 - 2007, The Regents of the University of California
c Produced at the Lawrence Livermore National Laboratory
c All rights reserved.
c
c This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
c    documentation and/or materials provided with the distribution.
c  - Neither the name of the UC/LLNL nor  the names of its contributors may be
c    used to  endorse or  promote products derived from  this software without
c    specific prior written permission.
c
c THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
c AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
c IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
c ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
c CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
c ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
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

c Create the Silo file
      ierr = dbcreate("fpointvar3d.silo", 16, DB_CLOBBER, DB_LOCAL, 
     .                "3D point variables", 18, DB_HDF5, dbfile)
      if(dbfile.eq.-1) then
          write (6,*) 'Could not create Silo file!\n'
          goto 10000
      endif
c Add other Silo calls here.
      call write_point3d(dbfile)
      call write_pointvar(dbfile)
c Close the Silo file.
      ierr = dbclose(dbfile)
10000 stop
      end

      subroutine write_point3d(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, i, ndims, NPTS
      parameter (NPTS = 100)
      real x(NPTS), y(NPTS), z(NPTS), t, angle
      do 10000 i = 0,NPTS-1
          t = float(i) / float(NPTS-1)
          angle =  3.14159 * 10. * t
          x(i+1) = t * cos(angle);
          y(i+1) = t * sin(angle);
          z(i+1) = t
10000 continue
      ndims = 3
      err = dbputpm (dbfile, "pointmesh", 9, ndims, x, y, z,
     . NPTS, DB_FLOAT, DB_F77NULL, ierr)
      end

      subroutine write_pointvar(dbfile)
      implicit none
      integer dbfile
      include "silo.inc"
      integer err, ierr, i, NPTS
      parameter (NPTS = 100)
      real var(NPTS)
      do 10010 i = 1,NPTS
          var(i) = float(i-1)
10010 continue
      err = dbputpv1(dbfile, "pointvar", 8, "pointmesh", 9, var, NPTS,
     . DB_FLOAT, DB_F77NULL, ierr)
      end
