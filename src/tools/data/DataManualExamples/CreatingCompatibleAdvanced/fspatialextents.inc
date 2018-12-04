c
c Includes for fspatialextents.f
c
      integer NXDOMS, NYDOMS, NZDOMS, NDOMS
      integer NX, NY, NZ, NX1, NY1, NZ1
      real    XSIZE, YSIZE, ZSIZE
      parameter (NXDOMS = 5)
      parameter (NYDOMS = 5)
      parameter (NZDOMS = 5)
      parameter (NDOMS = NXDOMS * NYDOMS * NZDOMS)
      parameter (NX = 10)
      parameter (NY = 15)
      parameter (NZ = 20)
      parameter (NX1 = NX-1)
      parameter (NY1 = NY-1)
      parameter (NZ1 = NZ-1)
      parameter (XSIZE = 5.)
      parameter (YSIZE = 5.)
      parameter (ZSIZE = 5.)
