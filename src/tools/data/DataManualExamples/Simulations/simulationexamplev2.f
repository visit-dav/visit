c-----------------------------------------------------------------
c simulationarguments : The routine handles command line arguments
c-----------------------------------------------------------------
      subroutine simulationarguments()
      implicit none
      character (len=80) str
      integer err, i, N, len
      integer visitsetoptions, visitsetdirectory, visitopentracefile
      N = iargc()
      i = 1
      len = 80
5     if (i.le.N) then
          call getarg(i, str)
          if(str.eq."-dir") then
              call getarg(i+1, str)
              err = visitsetdirectory(str, len)
              i = i + 1
          elseif(str.eq."-options") then
              call getarg(i+1, str)
              err = visitsetoptions(str, len)
              i = i + 1
          elseif(str.eq."-trace") then
              call getarg(i+1, str)
              err = visitopentracefile(str, len)
              i = i + 1
          endif
          i = i + 1
          goto 5
      endif
      end
