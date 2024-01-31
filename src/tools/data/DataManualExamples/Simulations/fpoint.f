c Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
c Project developers.  See the top-level LICENSE file for dates and other
c details.  No copyright assignment is required to contribute to VisIt.

c-----------------------------------------------------------------
c Program: main
c
c Programmer: Brad Whitlock
c Date:       Thu Mar 11 12:13:06 PST 2010
c
c Modifications:
c
c-----------------------------------------------------------------
      program main
      implicit none
      include "visitfortransimV2interface.inc"
ccc   local variables
      integer err

      call simulationarguments()
      err = visitsetupenv()
      err = visitinitializesim("fpoint", 6,
     . "Demonstrates creating a point mesh", 34,
     . "/no/useful/path", 15,
     . VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN,
     . VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN,
     . VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN)
      call mainloop()
      stop
      end

c-----------------------------------------------------------------
c mainloop
c-----------------------------------------------------------------
      subroutine mainloop()
      implicit none
      include "visitfortransimV2interface.inc"
ccc   local variables
      integer visitstate, result, blocking
ccc   SIMSTATE common block
      integer runflag, simcycle, simUpdate
      double precision simtime, simangle
      common /SIMSTATE/ simtime, simangle, runflag, simcycle, simUpdate
      save /SIMSTATE/

c     main loop
      runflag = 0
      simcycle = 0
      simtime = 0.
      simUpdate = 0
      simangle = 0.

      call simulate_one_timestep()

      do 10
          if(runflag.eq.1) then
              blocking = 0 
          else
              blocking = 1
          endif

          visitstate = visitdetectinput(blocking, -1)

          if (visitstate.lt.0) then
              goto 1234
          elseif (visitstate.eq.0) then
              call simulate_one_timestep()
          elseif (visitstate.eq.1) then
              runflag = 0
              result = visitattemptconnection()
              if (result.eq.1) then
                  write (6,*) 'VisIt connected!'
                  simUpdate = 1
              else
                  write (6,*) 'VisIt did not connect!'
              endif
          elseif (visitstate.eq.2) then
              runflag = 0
              if (visitprocessenginecommand().eq.0) then
                  result = visitdisconnect()
                  runflag = 1
              endif
          endif
10    continue
1234  end

      subroutine simulate_one_timestep()
      implicit none
      include "visitfortransimV2interface.inc" 
ccc   SIMSTATE common block
      integer runFlag, simcycle, simUpdate
      double precision simtime, simangle
      common /SIMSTATE/ simtime, simangle, runflag, simcycle, simUpdate
ccc   POINTMESH common block
      integer NPTS
      parameter (NPTS = 1000)
      real pmx(NPTS), pmy(NPTS), pmz(NPTS)
      common /POINTMESH/ pmx, pmy, pmz
      save /POINTMESH/
ccc   Local vars
      integer i, err
      real t, a

c Simulate one time step
      do 2000 i = 1,NPTS
          t = float(i-1) / float(NPTS-1)
          a = 3.14159 * 10. * t
          pmx(i) = t * cos(a + (0.5 + 0.5 * t) * simangle);
          pmy(i) = t * sin(a + (0.5 + 0.5 * t) * simangle);
          pmz(i) = t
2000  continue

      simangle = simangle + 0.05
      simcycle = simcycle + 1
      simtime = simtime + 3.14159 / 10.
      write (6,*) 'Simulating time step: cycle=',simcycle, 
     .            ' time=', simtime

      if(simUpdate.eq.1) then
c         Tell VisIt that the timestep changed
          err = visittimestepchanged()
c         Tell VisIt to update its plots
          err = visitupdateplots()
      endif
      end

ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c These functions must be defined to satisfy the visitfortransimV2interface lib.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

c---------------------------------------------------------------------------
c visitcommandcallback
c---------------------------------------------------------------------------
      subroutine visitcommandcallback (cmd, lcmd, args, largs) 
      implicit none
      character*8 cmd, args
      integer     lcmd, largs
      include "visitfortransimV2interface.inc"
ccc   SIMSTATE common block
      integer runFlag, simcycle, simUpdate
      double precision simtime, simangle
      common /SIMSTATE/ simtime, simangle, runflag, simcycle, simUpdate

c     Handle the commands that we define in visitgetmetadata.
      if(visitstrcmp(cmd, lcmd, "halt", 4).eq.0) then
          runflag = 0
      elseif(visitstrcmp(cmd, lcmd, "step", 4).eq.0) then
          call simulate_one_timestep()
      elseif(visitstrcmp(cmd, lcmd, "run", 3).eq.0) then
          runflag = 1
      endif
      end

c---------------------------------------------------------------------------
c visitbroadcastintfunction
c---------------------------------------------------------------------------
      integer function visitbroadcastintfunction(value, sender)
      implicit none
      integer value, sender
c     REPLACE WITH MPI COMMUNICATION IF SIMULATION IS PARALLEL
      visitbroadcastintfunction = 0
      end

c---------------------------------------------------------------------------
c visitbroadcaststringfunction
c---------------------------------------------------------------------------
      integer function visitbroadcaststringfunction(str, lstr, sender)
      implicit none
      character*8 str
      integer     lstr, sender
c     REPLACE WITH MPI COMMUNICATION IF SIMULATION IS PARALLEL
      visitbroadcaststringfunction = 0
      end

c---------------------------------------------------------------------------
c visitworkerprocesscallback
c---------------------------------------------------------------------------
      subroutine visitworkerprocesscallback ()
      implicit none
c     REPLACE WITH MPI COMMUNICATION IF SIMULATION IS PARALLEL
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
c visitgetmetadata
c---------------------------------------------------------------------------
      integer function visitgetmetadata()
      implicit none
      include "visitfortransimV2interface.inc"
ccc   SIMSTATE common block
      integer runFlag, simcycle, simUpdate
      double precision simtime, simangle
      common /SIMSTATE/ simtime, simangle, runflag, simcycle, simUpdate
ccc   Local vars
      integer md, mmd, cmd, err

      if(visitmdsimalloc(md).eq.VISIT_OKAY) then
          err = visitmdsimsetcycletime(md, simcycle, simtime)
          if(runflag.eq.1) then
              err = visitmdsimsetmode(md, VISIT_SIMMODE_RUNNING)
          else
              err = visitmdsimsetmode(md, VISIT_SIMMODE_STOPPED)
          endif

c         Add a point mesh
          err = visitmdmeshalloc(mmd);
          if(err.eq.VISIT_OKAY) then
              err = visitmdmeshsetname(mmd, "point3d", 7)
              err = visitmdmeshsetmeshtype(mmd, 
     .            VISIT_MESHTYPE_POINT)
              err = visitmdmeshsettopologicaldim(mmd, 0)
              err = visitmdmeshsetspatialdim(mmd, 3)
              err = visitmdmeshsetnumdomains(mmd, 1)
              err = visitmdmeshsetdomaintitle(mmd, "Domains", 7)
              err = visitmdmeshsetdomainpiecename(mmd, "domain", 6)
              err = visitmdmeshsetxunits(mmd, "cm", 2)
              err = visitmdmeshsetyunits(mmd, "cm", 2)
              err = visitmdmeshsetzunits(mmd, "cm", 2)
              err = visitmdmeshsetxlabel(mmd, "Width", 5)
              err = visitmdmeshsetylabel(mmd, "Height", 6)
              err = visitmdmeshsetylabel(mmd, "Depth", 5)
              err = visitmdmeshsetcellorigin(mmd, 1)
              err = visitmdmeshsetnodeorigin(mmd, 1)
              err = visitmdsimaddmesh(md, mmd)
          endif

c         Add simulation commands
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
      endif
      visitgetmetadata = md
      end

c---------------------------------------------------------------------------
c visitgetmesh
c---------------------------------------------------------------------------
      integer function visitgetmesh(domain, name, lname)
      implicit none
      character*8 name
      integer     domain, lname
      include "visitfortransimV2interface.inc" 
ccc   POINTMESH common block
      integer NPTS
      parameter (NPTS = 1000)
      real pmx(NPTS), pmy(NPTS), pmz(NPTS)
      common /POINTMESH/ pmx, pmy, pmz
ccc   local vars
      integer h, hx, hy, hz, err

      h = VISIT_INVALID_HANDLE
      if(visitstrcmp(name, lname, "point3d", 7).eq.0) then
          if(visitpointmeshalloc(h).eq.VISIT_OKAY) then
              err = visitvardataalloc(hx)
              err = visitvardataalloc(hy)
              err = visitvardataalloc(hz)
              err = visitvardatasetf(hx, VISIT_OWNER_SIM, 1, NPTS, pmx)
              err = visitvardatasetf(hy, VISIT_OWNER_SIM, 1, NPTS, pmy)
              err = visitvardatasetf(hz, VISIT_OWNER_SIM, 1, NPTS, pmz)
              err = visitpointmeshsetcoordsxyz(h, hx, hy, hz)
          endif
      endif
      visitgetmesh = h
      end

c---------------------------------------------------------------------------
c visitgetvariable
c---------------------------------------------------------------------------
      integer function visitgetvariable(domain, name, lname)
      implicit none
      character*8 name
      integer     domain, lname
      include "visitfortransimV2interface.inc"
      visitgetvariable = VISIT_INVALID_HANDLE
      end

c---------------------------------------------------------------------------
c visitgetmixedvariable
c---------------------------------------------------------------------------
      integer function visitgetmixedvariable(domain, name, lname)
      implicit none
      character*8 name
      integer     domain, lname
      include "visitfortransimV2interface.inc"
      visitgetmixedvariable = VISIT_INVALID_HANDLE
      end

c---------------------------------------------------------------------------
c visitgetcurve
c---------------------------------------------------------------------------
      integer function visitgetcurve(name, lname)
      implicit none
      character*8 name
      integer     lname
      include "visitfortransimV2interface.inc"
      visitgetcurve = VISIT_INVALID_HANDLE
      end

c---------------------------------------------------------------------------
c visitgetdomainlist
c---------------------------------------------------------------------------
      integer function visitgetdomainlist(name, lname)
      implicit none
      character*8 name
      integer     lname
      include "visitfortransimV2interface.inc"
      visitgetdomainlist = VISIT_INVALID_HANDLE
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
      visitgetmaterial = VISIT_INVALID_HANDLE
      end





























