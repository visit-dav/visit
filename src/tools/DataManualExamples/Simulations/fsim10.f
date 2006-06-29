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
      err = visitinitializesim("fsim10", 6,
     . "Demonstrates creating material metadata", 39,
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
c Simulate one time step
ccc   SIMSTATE common block
      integer runFlag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime
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
      integer err, tdim, sdim, mesh, mt, scalar, curve, mat

      err = visitmdsetcycletime(handle, simcycle, simtime)
      if(runflag.eq.1) then
          err = visitmdsetrunning(handle, VISIT_SIMMODE_RUNNING)
      else
          err = visitmdsetrunning(handle, VISIT_SIMMODE_STOPPED)
      endif

c     Add a 2D rectilinear mesh
      mt = VISIT_MESHTYPE_RECTILINEAR
      tdim = 2
      sdim = 2
      mesh = visitmdmeshcreate(handle, "mesh2d", 6, mt, tdim, sdim, 1)
      if(mesh.ne.VISIT_INVALID_HANDLE) then
          err = visitmdmeshsetunits(handle, mesh, "cm", 2)
          err = visitmdmeshsetlabels(handle, mesh, "Width", 5,
     .    "Height", 6, "Depth", 5)
          err = visitmdmeshsetblocktitle(handle, mesh, "Domains", 7)
          err = visitmdmeshsetblockpiecename(handle, mesh, "domain", 6)
      endif

c     Add a 3D curvilinear mesh
      tdim = 3
      sdim = 3
      mt = VISIT_MESHTYPE_CURVILINEAR
      mesh = visitmdmeshcreate(handle, "mesh3d", 6, mt, tdim, 
     .                         sdim, 1)
      if(mesh.ne.VISIT_INVALID_HANDLE) then
          err = visitmdmeshsetunits(handle, mesh, "Miles", 5)
          err = visitmdmeshsetlabels(handle, mesh, "Width", 5,
     .    "Height", 6, "Depth", 5)
          err = visitmdmeshsetblocktitle(handle, mesh, "Domains", 7)
          err = visitmdmeshsetblockpiecename(handle, mesh, "domain", 6)
      endif

c     Add a zonal variable on mesh2d.
      scalar = visitmdscalarcreate(handle, "zonal", 5, "mesh2d", 6,
     . VISIT_VARCENTERING_ZONE)
c     Add a nodal variable on mesh3d.
      scalar = visitmdscalarcreate(handle, "nodal", 5, "mesh3d", 6,
     . VISIT_VARCENTERING_NODE)

c     Add a curve variable
      curve = visitmdcurvecreate(handle, "sine", 4)
      if(curve.ne.VISIT_INVALID_HANDLE) then
          err = visitmdcurvesetlabels(handle, curve, "angle", 5,
     .                                "amplitude", 9)
          err = visitmdcurvesetunits(handle, curve, "radians", 7,
     .                               VISIT_F77NULLSTRING,
     .                               VISIT_F77NULLSTRINGLEN)
      endif

c     Add a material
      mat = visitmdmaterialcreate(handle, "mat", 3, "mesh2d", 6)
      if(mat.ne.VISIT_INVALID_HANDLE) then
          err = visitmdmaterialadd(handle, mat, "Iron", 4)
          err = visitmdmaterialadd(handle, mat, "Copper", 6)
          err = visitmdmaterialadd(handle, mat, "Nickel", 6)
      endif

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
      visitgetmesh = VISIT_ERROR
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





























