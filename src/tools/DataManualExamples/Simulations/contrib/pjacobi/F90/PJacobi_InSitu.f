c---------------------------------------------------------------------------
c Note the command callback making a call to simulate_one_timestep()
c---------------------------------------------------------------------------
c visitgetmetadata    
c---------------------------------------------------------------------------
      integer function visitgetmetadata()
      use jacobi_module
      implicit none
      include "visitfortransimV2interface.inc"
c     Local variables
      integer md, mmd, vmd, cmd, err

      simtime = iter*0.1
c      print *, 'id ', par_rank, ' m =', m, ' mp=', mp
      if(visitmdsimalloc(md).eq.VISIT_OKAY) then
          err = visitmdsimsetcycletime(md, iter, simtime)

      if( visRunFlag ) then
              err = visitmdsimsetmode(md, VISIT_SIMMODE_RUNNING)
          else
              err = visitmdsimsetmode(md, VISIT_SIMMODE_STOPPED)
          endif

c     Add a 2D rect mesh
          if(visitmdmeshalloc(mmd).eq.VISIT_OKAY) then
              err = visitmdmeshsetname(mmd, "mesh", 4)
              err = visitmdmeshsetmeshtype(mmd, 
     .            VISIT_MESHTYPE_RECTILINEAR)
              err = visitmdmeshsettopologicaldim(mmd, 2)
              err = visitmdmeshsetspatialdim(mmd, 2)
              err = visitmdmeshsetnumdomains(mmd, par_size)
              err = visitmdmeshsetdomaintitle(mmd, "Domains", 7)
              err = visitmdmeshsetdomainpiecename(mmd, "domain", 6)
              err = visitmdmeshsetxunits(mmd, "m", 1)
              err = visitmdmeshsetyunits(mmd, "m", 1)
              err = visitmdmeshsetxlabel(mmd, "Width", 5)
              err = visitmdmeshsetylabel(mmd, "Height", 6)
              err = visitmdsimaddmesh(md, mmd)
          endif

c     Add a zonal variable on mesh2d.
      if(visitmdvaralloc(vmd).eq.VISIT_OKAY) then
          err = visitmdvarsetname(vmd, "temperature", 11)
          err = visitmdvarsetmeshname(vmd, "mesh", 4)
          err = visitmdvarsetcentering(vmd, VISIT_VARCENTERING_NODE)
          err = visitmdvarsettype(vmd, VISIT_VARTYPE_SCALAR)
          err = visitmdsimaddvariable(md, vmd)
      endif

c     Add simulation commands
          err = visitmdcmdalloc(cmd)
          if(err.eq.VISIT_OKAY) then
              err = visitmdcmdsetname(cmd, "halt", 4)
              err = visitmdsimaddgenericcommand(md, cmd)
          endif
          err = visitmdcmdalloc(cmd)
          if(err.eq.VISIT_OKAY) then
              err = visitmdcmdsetname(cmd, "step", 4)
              err = visitmdsimaddgenericcommand(md, cmd)
          endif
          err = visitmdcmdalloc(cmd)
          if(err.eq.VISIT_OKAY) then
              err = visitmdcmdsetname(cmd, "run", 3)
              err = visitmdsimaddgenericcommand(md, cmd)
          endif
          err = visitmdcmdalloc(cmd)
          if(err.eq.VISIT_OKAY) then
              err = visitmdcmdsetname(cmd, "update", 6)
              err = visitmdsimaddgenericcommand(md, cmd)
          endif
      endif
      visitgetmetadata = md
      end

c---------------------------------------------------------------------------
c visitgetmesh
c---------------------------------------------------------------------------
      integer function visitgetmesh(domain, name, lname)
      use jacobi_module
      implicit none
      character*8 name
      integer     domain, lname
      include "visitfortransimV2interface.inc"

      integer h, i, x, y, err
      integer minRealIndex(0:2), maxRealIndex(0:2)
      real hsize, cx(0:m+1), cy(0:mp+1)

c      if( par_rank == 0) print *, 'id ', par_rank, ' m =', m, ' mp=', mp
      hsize = 1.0/float(m+1)
c all ranks send their block including their south boundary wall
c the northern-most rank sends also the northern boundary wall
      minRealIndex(0) = 0
      maxRealIndex(0) = m+1 ! no ghost in the X direction

      minRealIndex(1) = 0
      if( par_rank == par_size-1) then
        maxRealIndex(1) = mp+1
      else
        maxRealIndex(1) = mp
      endif

      minRealIndex(2) = 0
      maxRealIndex(2) = 0
      DO i = 0, m+1
         cx(i) = float(i) * hsize
      END DO
      DO i = 0, mp+1
         cy(i) = float(i + par_rank*mp) * hsize
      END DO

      h = VISIT_INVALID_HANDLE
      if(visitstrcmp(name, lname, "mesh", 4).eq.0) then          
c Create a rect mesh here
          if(visitrectmeshalloc(h).eq.VISIT_OKAY) then
              err = visitvardataalloc(x)
              err = visitvardataalloc(y)
              err = visitvardatasetf(x,VISIT_OWNER_COPY, 1, m+2, cx)
              err = visitvardatasetf(y,VISIT_OWNER_COPY, 1, mp+2, cy)
              err = visitrectmeshsetcoordsxy(h, x, y)
              err = visitrectmeshsetrealindices(h, minRealIndex, 
     .                          maxRealIndex)
          endif
      endif
      visitgetmesh = h
      end

c---------------------------------------------------------------------------
c visitgetvariable
c---------------------------------------------------------------------------
      integer function visitgetvariable(domain, name, lname)
      use jacobi_module
      implicit none
      character*8 name
      integer     domain, lname
      include "visitfortransimV2interface.inc"

      integer h

      if(visitvardataalloc(h).eq.VISIT_OKAY) then
        if(visitstrcmp(name, lname, "temperature", 11).eq.0) then 
        ierr = visitvardatasetd(h, VISIT_OWNER_SIM, 1,
     .                          (m+2)*(mp+2), Temp)
        endif
      endif

      visitgetvariable = h
      end

c---------------------------------------------------------------------------
c visitcommandcallback
c---------------------------------------------------------------------------
      subroutine visitcommandcallback(cmd, lcmd, args, largs)
      use jacobi_module
      implicit none
      character*8 cmd, args
      integer     lcmd, largs
      include "visitfortransimV2interface.inc"

c     Handle the commands that we define in visitgetmetadata.
      if(visitstrcmp(cmd, lcmd, "halt", 4).eq.0) then
           visRunFlag = .false.
      elseif(visitstrcmp(cmd, lcmd, "step", 4).eq.0) then
           call simulate_one_timestep()
           ierr = visitupdateplots()
      elseif(visitstrcmp(cmd, lcmd, "run", 3).eq.0) then
          visRunFlag = .true.
      elseif(visitstrcmp(cmd, lcmd, "update", 6).eq.0) then
         ierr = visitupdateplots()
      endif
      end

c---------------------------------------------------------------------------
c visitbroadcastintfunction
c---------------------------------------------------------------------------
      integer function visitbroadcastintfunction(value, sender)
      implicit none
      include "mpif.h"
      integer value, sender, ierr
      call MPI_BCAST(value,1,MPI_INTEGER,sender,MPI_COMM_WORLD,ierr)
      visitbroadcastintfunction = 0
      end

c---------------------------------------------------------------------------
c visitbroadcaststringfunction
c---------------------------------------------------------------------------
      integer function visitbroadcaststringfunction(str, lstr, sender)
      implicit none
      include "mpif.h"
      character*8 str
      integer     lstr, sender, ierr
      call MPI_BCAST(str,lstr,MPI_CHARACTER,sender,MPI_COMM_WORLD,ierr)
      visitbroadcaststringfunction = 0
      end

c---------------------------------------------------------------------------
c visitslaveprocesscallback
c---------------------------------------------------------------------------
      subroutine visitslaveprocesscallback ()
      implicit none
      include "mpif.h"
      integer c, VISIT_COMMAND_PROCESS, ierr
      parameter (VISIT_COMMAND_PROCESS = 0)
      c = VISIT_COMMAND_PROCESS
      call MPI_BCAST(c,1,MPI_INTEGER,0,MPI_COMM_WORLD,ierr)
      end

c---------------------------------------------------------------------------
c visitactivatetimestep
c---------------------------------------------------------------------------
      integer function visitactivatetimestep()
      implicit none
      include "visitfortransimV2interface.inc"
      visitactivatetimestep = VISIT_OKAY
      end

c---------------------------------------------------------------------------
c visitgetcurve
c---------------------------------------------------------------------------
      integer function visitgetcurve(handle, name, lname)
      implicit none
      character*8 name
      integer     handle, lname
      include "visitfortransimV2interface.inc"
      visitgetcurve = VISIT_ERROR
      end

c---------------------------------------------------------------------------
c visitgetdomainlist
c---------------------------------------------------------------------------
      integer function visitgetdomainlist()
      use jacobi_module
      implicit none
      include "visitfortransimV2interface.inc"
ccc   local vars
      integer h, dl

      h = VISIT_INVALID_HANDLE
      if(visitdomainlistalloc(h).eq.VISIT_OKAY) then
          if(visitvardataalloc(dl).eq.VISIT_OKAY) then
              ierr = visitvardataseti(dl,VISIT_OWNER_SIM,1,1,par_rank)
              ierr = visitdomainlistsetdomains(h, par_size, dl)
          endif
      endif
      visitgetdomainlist = h
      end

c---------------------------------------------------------------------------
c visitgetdomainbounds
c---------------------------------------------------------------------------
      integer function visitgetdomainbounds(name, lname)
      implicit none
      character*8 name
      integer     lname
      include "visitfortransimV2interface.inc"
      visitgetdomainbounds = VISIT_INVALID_HANDLE
      end

c---------------------------------------------------------------------------
c visitgetdomainnesting
c---------------------------------------------------------------------------
      integer function visitgetdomainnesting(name, lname)
      implicit none
      character*8 name
      integer     lname
      include "visitfortransimV2interface.inc"
      visitgetdomainnesting = VISIT_INVALID_HANDLE
      end


c---------------------------------------------------------------------------
c visitgetmaterial
c---------------------------------------------------------------------------
      integer function visitgetmaterial(domain, name, lname)
      implicit none
      character*8 name
      integer     domain, lname
      include "visitfortransimV2interface.inc"
      visitgetmaterial = VISIT_ERROR
      end

c-----------------------------------------------------------------
c processvisitcommand
c-----------------------------------------------------------------

      integer function processvisitcommand()
      use jacobi_module
      implicit none

      include "visitfortransimV2interface.inc"
      integer command, e, doloop, success, ret
      integer VISIT_COMMAND_PROCESS
      integer VISIT_COMMAND_SUCCESS
      integer VISIT_COMMAND_FAILURE
      parameter (VISIT_COMMAND_PROCESS = 0)
      parameter (VISIT_COMMAND_SUCCESS = 1)
      parameter (VISIT_COMMAND_FAILURE = 2)
      integer h, dl

      if(par_rank.eq.0) then
          success = visitprocessenginecommand()

          if(success.gt.0) then
              command = VISIT_COMMAND_SUCCESS
              ret = 1
          else
              command = VISIT_COMMAND_FAILURE
              ret = 0
          endif

          call MPI_BCAST(command,1,MPI_INTEGER,0,MPI_COMM_WORLD,e)
      else
          doloop = 1
2345      call MPI_BCAST(command,1,MPI_INTEGER,0,MPI_COMM_WORLD,e)
          if(command.eq.VISIT_COMMAND_PROCESS) then
              success = visitprocessenginecommand()
          elseif(command.eq.VISIT_COMMAND_SUCCESS) then
              ret = 1
              doloop = 0
          else
              ret = 0
              doloop = 0
          endif

          if(doloop.ne.0) then
              goto 2345
          endif
      endif
      processvisitcommand = ret
      end
 
