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
