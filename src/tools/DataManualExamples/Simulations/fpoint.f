c-----------------------------------------------------------------
c Program: main
c
c-----------------------------------------------------------------
      program main
      implicit none
      include "visitfortransiminterface.inc"
ccc   local variables
      integer err

      err = visitsetupenv()
      err = visitinitializesim("fpoint", 6,
     . "Demonstrates creating a point mesh", 34,
     . "/no/useful/path", 15,
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
      include "visitfortransiminterface.inc"
ccc   local variables
      integer visitstate, result, blocking
ccc   SIMSTATE common block
      integer runflag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime
      save /SIMSTATE/

c     main loop
      runflag = 1
      simcycle = 0
      simtime = 0.
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
ccc   SIMSTATE common block
      integer runFlag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime
ccc   POINTMESH common block (shared with visitgetmesh)
      integer NPTS
      parameter (NPTS = 100)
      real pmx(NPTS), pmy(NPTS), pmz(NPTS), angle
      integer pmndims, pmnnodes
      common /RECTMESH/ pmx, pmy, pmz, pmndims, pmnnodes, angle
ccc   local variables
      real a, t

c Simulate one time step
      pmndims = 3
      pmnnodes = NPTS
      do 10000 i = 0,NPTS-1
          t = float(i) / float(NPTS-1)
          a =  3.14159 * 10. * t
          pmx(i+1) = t * cos(a + (0.5 + 0.5 * t) * angle);
          pmy(i+1) = t * sin(a + (0.5 + 0.5 * t) * angle);
          pmz(i+1) = t
10000 continue
      angle = angle + 0.05
c Advance time
      simcycle = simcycle + 1
      simtime = simtime + 0.0134
      write (6,*) 'Simulating time step: cycle=',simcycle, ' time=', simtime
      call sleep(1)
      end

ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c These functions must be defined to satisfy the visitfortransiminterface lib.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

c---------------------------------------------------------------------------
c visitcommandcallback
c---------------------------------------------------------------------------
      subroutine visitcommandcallback (cmd, lcmd, intdata, 
     .                                 floatdata, stringdata, 
     .                                 lstringdata)
      implicit none
      character*8 cmd, stringdata
      integer     lcmd, lstringdata, intdata
      real        floatdata
      include "visitfortransiminterface.inc"
ccc   SIMSTATE common block
      integer runflag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime
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
c visitslaveprocesscallback
c---------------------------------------------------------------------------
      subroutine visitslaveprocesscallback ()
      implicit none
c     REPLACE WITH MPI COMMUNICATION IF SIMULATION IS PARALLEL
      end

c---------------------------------------------------------------------------
c visitgetmetadata
c---------------------------------------------------------------------------
      integer function visitgetmetadata(handle)
      implicit none
      integer handle
      include "visitfortransiminterface.inc"
ccc   SIMSTATE common block
      integer runflag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime
      integer err, tdim, sdim, mesh, mt, scalar, curve, mat, e

      err = visitmdsetcycletime(handle, simcycle, simtime)
      if(runflag.eq.1) then
          err = visitmdsetrunning(handle, VISIT_SIMMODE_RUNNING)
      else
          err = visitmdsetrunning(handle, VISIT_SIMMODE_STOPPED)
      endif

c     Add a 3D point mesh
      mt = VISIT_MESHTYPE_RECTILINEAR
      tdim = 0
      sdim = 3
      mesh = visitmdmeshcreate(handle, "point3d", 7, mt, tdim, sdim, 1)
      if(mesh.ne.VISIT_INVALID_HANDLE) then
          err = visitmdmeshsetunits(handle, mesh, "cm", 2)
          err = visitmdmeshsetlabels(handle, mesh, "Width", 5,
     .    "Height", 6, "Depth", 5)
          err = visitmdmeshsetblocktitle(handle, mesh, "Domains", 7)
          err = visitmdmeshsetblockpiecename(handle, mesh, "domain", 6)
      endif

c     Add simulation commands
      err = visitmdaddsimcommand(handle, "halt", 4, VISIT_CMDARG_NONE,
     .                           1)
      err = visitmdaddsimcommand(handle, "step", 4, VISIT_CMDARG_NONE,
     .                           1)
      err = visitmdaddsimcommand(handle, "run", 3, VISIT_CMDARG_NONE,
     .                           1)

      visitgetmetadata = VISIT_OKAY
      end

c---------------------------------------------------------------------------
c visitgetmesh
c---------------------------------------------------------------------------
      integer function visitgetmesh(handle, domain, name, lname)
      implicit none
      character*8 name
      integer     handle, domain, lname
      include "visitfortransiminterface.inc" 
ccc   POINTMESH common block (shared with simulate_one_timestep)
      integer NPTS
      parameter (NPTS = 100)
      real pmx(NPTS), pmy(NPTS), pmz(NPTS), angle
      integer pmndims, pmnnodes
      common /RECTMESH/ pmx, pmy, pmz, pmndims, pmnnodes, angle

ccc   local variables
      integer m, baseindex(3), minrealindex(3), maxrealindex(3)
      real rmz

      m = VISIT_ERROR
      if(visitstrcmp(name, lname, "point3d", 7).eq.0) then
c Create a point mesh here
          m = visitmeshpoint(handle, pmndims, pmnnodes, pmx, pmy, pmz)
      endif
      visitgetmesh = m
      end

c---------------------------------------------------------------------------
c visitgetscalar
c---------------------------------------------------------------------------
      integer function visitgetscalar(handle, domain, name, lname)
      implicit none
      character*8 name
      integer     handle, domain, lname
      include "visitfortransiminterface.inc"
      visitgetscalar = VISIT_ERROR
      end


c---------------------------------------------------------------------------
c visitgetcurve
c---------------------------------------------------------------------------
      integer function visitgetcurve(handle, name, lname)
      implicit none
      character*8 name
      integer     handle, lname
      include "visitfortransiminterface.inc"
      visitgetcurve = VISIT_ERROR
      end

c---------------------------------------------------------------------------
c visitgetdomainlist
c---------------------------------------------------------------------------
      integer function visitgetdomainlist(handle)
      implicit none
      integer handle
      include "visitfortransiminterface.inc"
      visitgetdomainlist = VISIT_OKAY
      end

c---------------------------------------------------------------------------
c visitgetmaterial
c---------------------------------------------------------------------------
      integer function visitgetmaterial(handle, domain, name, lname)
      implicit none
      character*8 name
      integer     handle, domain, lname
      include "visitfortransiminterface.inc"
      visitgetmaterial = VISIT_ERROR
      end





























