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
      integer NX, NY, NZ
      parameter (NX = 20)
      parameter (NY = 30)
      parameter (NZ = 40)
      real values(NX, NY, NZ)
      integer i, j, k, output, index, status
      output = 20

c     Fill values into the array
      index = 0
      do 20 k=1,NZ
         do 20 j=1,NY
             do 10 i=1,NX
                  values(i,j,k) = index
                  index = index + 1
10            continue
20       continue
30    continue

c     Write the BOV data file
      open (unit=output, file='fbov.values', status='replace',
     . form='unformatted', err=99)
      write(output) values
      close (output)

c     Write the BOV header file.
      open (unit=output, file='fbov.bov', status='replace',
     .err=99)
      write(output,*) 'TIME: 0.0'
      write(output,*) 'DATA_FILE: fbov.values'
      write(output,*) 'DATA_SIZE: ', NX, NY, NZ
      write(output,*) 'DATA_FORMAT: FLOAT'
      write(output,*) 'VARIABLE: var'
c     How to detect little vs. big endian automatically in Fortran?
      write(output,*) 'DATA_ENDIAN: LITTLE'
      write(output,*) 'CENTERING: zonal'
      write(output,*) 'BYTE_OFFSET: 4'
      write(output,*) 'BRICK_ORIGIN: 0.0 0.0 0.0'
      write(output,*) 'BRICK_SIZE: 10.0 10.0 10.0'
      close (output)
99    stop
      end
