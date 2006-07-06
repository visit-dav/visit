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
      err = visitinitializesim("fcurve", 6,
     . "Demonstrates curve data access function", 40,
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
ccc   RECTMESH common block
      integer NX, NY
      parameter (NX = 4)
      parameter (NY = 5)
      real rmx(NX), rmy(NY), zonal(NX-1,NY-1)
      integer rmdims(3), rmndims
      common /RECTMESH/ rmdims, rmndims, rmx, rmy, zonal
      save /RECTMESH/
ccc   CURVMESH common block
      integer CNX, CNY, CNZ
      parameter (CNX = 4)
      parameter (CNY = 3)
      parameter (CNZ = 2)
      integer cmdims(3), cmndims
      real cmx(CNX,CNY,CNZ), cmy(CNX,CNY,CNZ), cmz(CNX,CNY,CNZ)
      double precision nodal(CNX,CNY,CNZ)
      common /CURVMESH/ cmdims, cmndims, cmx, cmy, cmz, nodal
      save /CURVMESH/
c Rectilinear mesh data
      data rmndims /2/
      data rmdims /4, 5, 1/
      data rmx/0., 1., 2.5, 5./
      data rmy/0., 2., 2.25, 2.55,  5./
      data zonal/1.,2.,3.,4.,5.,6.,7.,8.,9.,10.,11.,12./
c Curvilinear mesh data
      data cmx/0.,1.,2.,3., 0.,1.,2.,3., 0.,1.,2.,3.,
     . 0.,1.,2.,3., 0.,1.,2.,3., 0.,1.,2.,3./
      data cmy/0.5,0.,0.,0.5, 1.,1.,1.,1., 1.5,2.,2.,1.5,
     . 0.5,0.,0.,0.5, 1.,1.,1.,1., 1.5,2.,2.,1.5/
      data cmz/0.,0.,0.,0., 0.,0.,0.,0., 0.,0.,0.,0,
     . 1.,1.,1.,1., 1.,1.,1.,1., 1.,1.,1.,1./
      data cmndims /3/
      data cmdims/CNX,CNY,CNZ/
      data nodal/1.,2.,3.,4.,5.,6.,7.,8.,9.,10.,11.,12.,13.,14.,15.,
     . 16.,17.,18.,19.,20.,21.,22.,23.,24./
c Simulate one time step
      simcycle = simcycle + 1
      simtime = simtime + 0.0134
      write (6,*) 'Simulating time step: cycle=',simcycle, 
     .' time=', simtime
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

c     Add some expressions
      e = visitmdexpressioncreate(handle, "zvec", 4,
     . "{zonal, zonal, zonal}", 21, VISIT_VARTYPE_VECTOR)
      e = visitmdexpressioncreate(handle, "nid", 3,
     . "nodeid(mesh3d)", 14, VISIT_VARTYPE_SCALAR)

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
ccc   RECTMESH common block (shared with simulate_one_timestep)
      integer NX, NY
      parameter (NX = 4)
      parameter (NY = 5)
      real rmx(NX), rmy(NY), zonal(NX-1,NY-1)
      integer rmdims(3), rmndims
      common /RECTMESH/ rmdims, rmndims, rmx, rmy, zonal
ccc   CURVMESH common block
      integer CNX, CNY, CNZ
      parameter (CNX = 4)
      parameter (CNY = 3)
      parameter (CNZ = 2)
      integer cmdims(3), cmndims
      real cmx(CNX,CNY,CNZ), cmy(CNX,CNY,CNZ), cmz(CNX,CNY,CNZ)
      double precision nodal(CNX,CNY,CNZ)
      common /CURVMESH/ cmdims, cmndims, cmx, cmy, cmz, nodal

ccc   local variables
      integer m, baseindex(3), minrealindex(3), maxrealindex(3)
      real rmz

      m = VISIT_ERROR
      if(visitstrcmp(name, lname, "mesh2d", 6).eq.0) then
          baseindex(1) = 1
          baseindex(2) = 1
          baseindex(3) = 1
          minrealindex(1) = 0
          minrealindex(2) = 0
          minrealindex(3) = 0
          maxrealindex(1) = rmdims(1)-1
          maxrealindex(2) = rmdims(2)-1
          maxrealindex(3) = rmdims(3)-1
c Create a rectilinear rmesh here
          m = visitmeshrectilinear(handle, baseindex, minrealindex,
     .        maxrealindex, rmdims, rmndims, rmx, rmy, rmz)
      elseif(visitstrcmp(name, lname, "mesh3d", 6).eq.0) then
          baseindex(1) = 1
          baseindex(2) = 1
          baseindex(3) = 1
          minrealindex(1) = 0
          minrealindex(2) = 0
          minrealindex(3) = 0
          maxrealindex(1) = cmdims(1)-1
          maxrealindex(2) = cmdims(2)-1
          maxrealindex(3) = cmdims(3)-1
c Create a curvilinear mesh here
          m = visitmeshcurvilinear(handle, baseindex, minrealindex,
     .        maxrealindex, cmdims, cmndims, cmx, cmy, cmz)
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
ccc   RECTMESH common block
      integer NX, NY
      parameter (NX = 4)
      parameter (NY = 5)
      real rmx(NX), rmy(NY), zonal(NX-1,NY-1)
      integer rmdims(3), rmndims
      common /RECTMESH/ rmdims, rmndims, rmx, rmy, zonal
ccc   CURVMESH common block
      integer CNX, CNY, CNZ
      parameter (CNX = 4)
      parameter (CNY = 3)
      parameter (CNZ = 2)
      integer cmdims(3), cmndims
      real cmx(CNX,CNY,CNZ), cmy(CNX,CNY,CNZ), cmz(CNX,CNY,CNZ)
      double precision nodal(CNX,CNY,CNZ)
      common /CURVMESH/ cmdims, cmndims, cmx, cmy, cmz, nodal
ccc   local vars
      integer m, sdims(3)
      m = VISIT_ERROR
      if(visitstrcmp(name, lname, "zonal", 5).eq.0) then
c A zonal variable has 1 less value in each dimension as there
c are nodes. Send back REAL data.
          sdims(1) = rmdims(1)-1
          sdims(2) = rmdims(2)-1
          sdims(3) = rmdims(3)-1
          m = visitscalarsetdataf(handle, zonal, sdims, rmndims)
      elseif(visitstrcmp(name, lname, "nodal", 5).eq.0) then
c A nodal variable has the same number values in each dimension
c as there are nodes. Send back DOUBLE PRECISION data.
          m = visitscalarsetdatad(handle, nodal, cmdims, cmndims)
      endif

      visitgetscalar = m
      end


c---------------------------------------------------------------------------
c visitgetcurve
c---------------------------------------------------------------------------
      integer function visitgetcurve(handle, name, lname)
      implicit none
      character*8 name
      integer     handle, lname, m, NPTS, i
      parameter   (NPTS = 200)
      real        x(NPTS), y(NPTS), t
      include "visitfortransiminterface.inc"
      m = VISIT_ERROR
      if(visitstrcmp(name, lname, "sine", 4).eq.0) then
          do 10000 i=1,NPTS
              t = float(i-1) / float(NPTS-1)
              x(i) = t * 4. * 3.14159
              y(i) = sin(x(i))
10000     continue          
          m = visitcurvesetdataf(handle, x, y, NPTS)
      endif
      visitgetcurve = m
      end

c---------------------------------------------------------------------------
c visitgetdomainlist
c---------------------------------------------------------------------------
      integer function visitgetdomainlist(handle)
      implicit none
      integer handle
      include "visitfortransiminterface.inc"
ccc   local variables
      integer totaldomains, domainids(1), ndomids

      totaldomains = 1
      domainids(1) = 0
      ndomids = 1
      visitgetdomainlist = visitsetdomainlist(handle, totaldomains,
     .                                        domainids, ndomids)
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





























