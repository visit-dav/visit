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
