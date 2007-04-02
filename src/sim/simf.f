c---------------------------------------------------------------------------
c Program: main
c
c Purpose: Demonstrate how to use the Fortran bindings to the 
c          "visitfortransiminterface" library to instrument a simulation code
c          so that VisIt can directly obtain data from it (runtime graphics).
c
c Programmer: Brad Whitlock
c Date:       Fri Jan 27 12:24:54 PDT 2006
c
c Modifications:
c
c---------------------------------------------------------------------------
      program main
      implicit none
      include "visitfortransiminterface.inc"
ccc   local variables
      integer err

      err = visitsetupenv()
      err = visitinitializesim("fortran_proto", 13,
     .                         "Fortran prototype simulation", 28,
     .                         "/no/useful/path", 15,
     .                         VISIT_F77NULLSTRING,
     .                         VISIT_F77NULLSTRINGLEN,
     .                         VISIT_F77NULLSTRING,
     .                         VISIT_F77NULLSTRINGLEN)

      call initializevariables()
      call mainloop()

      stop
      end

c---------------------------------------------------------------------------
c mainloop
c---------------------------------------------------------------------------
      subroutine mainloop()
      implicit none
      include "visitfortransiminterface.inc"
ccc   SIMSTATE common block
      integer runflag, simcycle
      real    simtime
      common /SIMSTATE/ runflag, simcycle, simtime
      save /SIMSTATE/
ccc   local variables
      integer visitstate, result, i, blocking
      integer processvisitcommand

c     main loop
      do 10
          if(runflag.eq.1) then
              blocking = 0 
          else
              blocking = 1
          endif

          visitstate = visitdetectinput(blocking, -1)

          if (visitstate.eq.-5) then
              goto 1234
          elseif (visitstate.eq.-4) then
              goto 1234
          elseif (visitstate.eq.-3) then
              goto 1234
          elseif (visitstate.eq.-2) then
              goto 1234
          elseif (visitstate.eq.-1) then
              goto 1234
          elseif (visitstate.eq.0) then
              call runsinglecycle()
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
              if (processvisitcommand().eq.0) then
                  result = visitdisconnect()
                  runflag = 1
              endif
          endif
10    continue
1234  end

c---------------------------------------------------------------------------
c processvisitcommand
c---------------------------------------------------------------------------
      integer function processvisitcommand()
      implicit none
      include "visitfortransiminterface.inc"
      processvisitcommand = visitprocessenginecommand()
      end

c---------------------------------------------------------------------------
c initializevariables
c---------------------------------------------------------------------------
      subroutine initializevariables()
      implicit none
ccc   common block
      real    nodal(4,9), zonal(3,8), x(4), y(9)
      integer nx, ny, nz, ndims
      common /vars/ nodal, zonal, x, y, nx, ny, nz, ndims
      save /vars/
ccc   SIMSTATE common block
      integer runflag, simcycle
      real    simtime
      common /SIMSTATE/ runflag, simcycle, simtime
ccc   local variables
      integer i

c     Initialize some sim control-related variables
      runflag = 1
      simcycle = 0
      simtime = 0.

c     Initialize some data-related variables
      nx = 4
      ny = 9
      nz = 1
      ndims = 2

c     Create the mesh.
      do 30 i = 1, nx
          x(i) = (i - 1) * (1. / 30)
30    continue
      do 40 i = 1, ny
          y(i) = (i - 1) * (1. / 40)
40    continue

c     Run the first cycle to initialize the variables
      call runsinglecycle()
      end


c---------------------------------------------------------------------------
c runsinglecycle
c---------------------------------------------------------------------------
      subroutine runsinglecycle()
      implicit none
      include "visitfortransiminterface.inc"
ccc   common block
      real    nodal(4,9), zonal(3,8), x(4), y(9)
      integer nx, ny, nz, ndims
      common /vars/ nodal, zonal, x, y, nx, ny, nz, ndims
ccc   SIMSTATE common block
      integer runflag, simcycle
      real    simtime
      common /SIMSTATE/ runflag, simcycle, simtime
ccc   CURVEMESH common block
      real cx(100,100), cy(100,100)
      common /CURVEMESH/ cx, cy
      save /CURVEMESH/
ccc   local variables
      integer i, j, index
      real    displ, angle

c     Create the nodal variable
      index = 0
      do 50 j=1,ny
          do 51 i=1,nx
              nodal(i,j) = index
              index = index + 1
51        continue
50    continue

c     Create the zonal variable
      index = 0
      do 60 j=1,ny-1
          do 61 i=1,nx-1
              zonal(i,j) = index
              index = index + 1
61        continue
60    continue

c     Create a moving curvilinear mesh by filling cx, cy
      displ = (mod(simcycle/10, 100) / 99.) * 3.14159
      do 70 j=1,100
          do 71 i=1,100
              angle = ((i - 1.) / 99.) * 2. * 3.14159
              cx(i,j) = angle
              cy(i,j) = ((j - 1) / 99.) * 5. + sin(angle + displ)
71        continue
70    continue

c     Delay a little so it's like it took more to process the cycle
      call sleep(1)
 
      write (6,*) 'Finished cycle', simcycle, ' time', simtime

c     Advance the time and cycle
      simcycle = simcycle + 10
      simtime = simtime + 0.01
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
      real    simtime
      common /SIMSTATE/ runflag, simcycle, simtime

c     Handle the commands that we define in visitgetmetadata.
      if(visitstrcmp(cmd, lcmd, "halt", 4).eq.0) then
          runflag = 0
      elseif(visitstrcmp(cmd, lcmd, "step", 4).eq.0) then
          call runsinglecycle()
      elseif(visitstrcmp(cmd, lcmd, "run", 3).eq.0) then
          runflag = 1
      elseif(visitstrcmp(cmd, lcmd, "testcommand", 11).eq.0) then
          write (6,*) 'Received testcommand'
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
      real    simtime
      common /SIMSTATE/ runflag, simcycle, simtime
ccc   local variables
      integer md, mesh, scalar, mat, curve, mt, tdim, sdim, err

      err = visitmdsetcycletime(handle, simcycle, simtime)
      err = visitmdsetrunning(handle, VISIT_SIMMODE_STOPPED)

c     Add a mesh
      mt = VISIT_MESHTYPE_RECTILINEAR
      tdim = 2
      sdim = 2
      mesh = visitmdmeshcreate(handle, "mesh", 4, mt, tdim, sdim, 1)
      if(mesh.ne.VISIT_INVALID_HANDLE) then
          err = visitmdmeshsetunits(handle, mesh, "Km", 2)
      endif
c     Add a curvilinear mesh
      mt = VISIT_MESHTYPE_CURVILINEAR
      mesh = visitmdmeshcreate(handle, "curvemesh", 9, mt, tdim, 
     .                         sdim, 1)
      if(mesh.ne.VISIT_INVALID_HANDLE) then
          err = visitmdmeshsetunits(handle, mesh, "Miles", 5)
      endif

c     Add scalar variables.
      scalar = visitmdscalarcreate(handle, "nodal", 5, "mesh", 4,
     .                             VISIT_VARCENTERING_NODE)
      if(scalar.ne.VISIT_INVALID_HANDLE) then
          err = visitmdscalarsetunits(handle, scalar, "g/mL", 4)
      endif
      scalar = visitmdscalarcreate(handle, "zonal", 5, "mesh", 4,
     .                             VISIT_VARCENTERING_ZONE)

c     Add a curve
      curve = visitmdcurvecreate(handle, "sine", 4)
      if(curve.ne.VISIT_INVALID_HANDLE) then
          err = visitmdcurvesetlabels(handle, curve, "angle", 5,
     .                                "amplitude", 9)
          err = visitmdcurvesetunits(handle, curve, "radians", 7,
     .                               VISIT_F77NULLSTRING,
     .                               VISIT_F77NULLSTRINGLEN)
      endif

c     Add a material
      mat = visitmdmaterialcreate(handle, "mat", 3, "mesh", 4)
      if(mat.ne.VISIT_INVALID_HANDLE) then
          err = visitmdmaterialadd(handle, mat, "mat1", 4)
          err = visitmdmaterialadd(handle, mat, "mat2", 4)
          err = visitmdmaterialadd(handle, mat, "mat3", 4)
      endif

c     Add simulation commands
      err = visitmdaddsimcommand(handle, "halt", 4, VISIT_CMDARG_NONE,
     .                           1)
      err = visitmdaddsimcommand(handle, "step", 4, VISIT_CMDARG_NONE,
     .                           1)
      err = visitmdaddsimcommand(handle, "run", 3, VISIT_CMDARG_NONE,
     .                           1)
      err = visitmdaddsimcommand(handle, "testcommand", 11,
     .                           VISIT_CMDARG_NONE, 1)

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
ccc   common block
      real    nodal(4,9), zonal(3,8), x(4), y(9)
      integer nx, ny, nz, ndims
      common /vars/ nodal, zonal, x, y, nx, ny, nz, ndims
ccc   CURVEMESH common block
      real cx(100,100), cy(100,100)
      common /CURVEMESH/ cx, cy
ccc   local vars
      integer m, baseindex(3), minrealindex(3), maxrealindex(3)
      integer dims(3), z

      m = VISIT_ERROR
      if(visitstrcmp(name, lname, "mesh", 4).eq.0) then
          dims(1) = nx
          dims(2) = ny
          dims(3) = nz
          baseindex(1) = 1
          baseindex(2) = 1
          baseindex(3) = 1
          minrealindex(1) = 0
          minrealindex(2) = 0
          minrealindex(3) = 0
          maxrealindex(1) = nx-1
          maxrealindex(2) = ny-1
          maxrealindex(3) = nz-1
          m = visitmeshrectilinear(handle, baseindex, minrealindex,
     .                             maxrealindex, dims, ndims, x, y, z)
      elseif(visitstrcmp(name, lname, "curvemesh", 9).eq.0) then
          dims(1) = 100
          dims(2) = 100
          dims(3) = 1
          baseindex(1) = 1
          baseindex(2) = 1
          baseindex(3) = 1
          minrealindex(1) = 0
          minrealindex(2) = 0
          minrealindex(3) = 0
          maxrealindex(1) = 99
          maxrealindex(2) = 99
          maxrealindex(3) = 0
          m = visitmeshcurvilinear(handle, baseindex, 
     .        minrealindex, maxrealindex, dims, ndims, cx, cy, z)
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
ccc   common block
      real    nodal(4,9), zonal(3,8), x(4), y(9)
      integer nx, ny, nz, ndims
      common /vars/ nodal, zonal, x, y, nx, ny, nz, ndims
ccc   local vars
      integer s, dims(3)

      s = VISIT_ERROR
      if(visitstrcmp(name, lname, "nodal", 5).eq.0) then
          dims(1) = nx
          dims(2) = ny
          dims(3) = nz
          s = visitscalarsetdataf(handle, nodal, dims, ndims)
      elseif(visitstrcmp(name, lname, "zonal", 5).eq.0) then
          dims(1) = nx-1
          dims(2) = ny-1
          dims(3) = nz-1
          s = visitscalarsetdataf(handle, zonal, dims, ndims)
      endif

      visitgetscalar = s
      end

c---------------------------------------------------------------------------
c visitgetcurve
c---------------------------------------------------------------------------
      integer function visitgetcurve(handle, name, lname)
      implicit none
      character*8 name
      integer     handle, lname
      include "visitfortransiminterface.inc"
      integer c, i
      real cvx(100), cvy(100)

      c = VISIT_ERROR
      if(visitstrcmp(name, lname, "sine", 4).eq.0) then
c         Calculate the curve coordinates.
          do 100 i=0,99
              cvx(i+1) = (i / 99.) * 3.14159 * 2.
              cvy(i+1) = sin(cvx(i+1))
100       continue
          c = visitcurvesetdataf(handle, cvx, cvy, 100)
      endif
      visitgetcurve = c
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
      domainids(1) = 1
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
ccc   common block
      real    nodal(4,9), zonal(3,8), x(4), y(9)
      integer nx, ny, nz, ndims
      common /vars/ nodal, zonal, x, y, nx, ny, nz, ndims
ccc   local variables
      integer i,j, m, dims(3), cell(3), err
      integer cellmats(2), mat1, mat2, mat3
      real    cellvf(2)

      m = VISIT_ERROR
      if(visitstrcmp(name, lname, "mat", 3).eq.0) then
          dims(1) = nx-1
          dims(2) = ny-1
          dims(3) = nz-1
          err = visitmaterialsetdims(handle, dims, ndims)
          mat1 = visitmaterialadd(handle, "mat1", 4)
          mat2 = visitmaterialadd(handle, "mat2", 4)
          mat3 = visitmaterialadd(handle, "mat3", 4)
c         Iterate over the cells and set each cell's material(s)
          do 200 j=1,ny-1
              do 210 i=1,nx-1
                  cell(1) = i
                  cell(2) = j
                  if(j.gt.6) then
                      err = visitmaterialaddclean(handle, cell, mat3)
                  elseif(i.eq.2) then
                      cellmats(1) = mat1
                      cellmats(2) = mat2
                      cellvf(1) = 0.5
                      cellvf(2) = 0.5
                      err = visitmaterialaddmixed(handle, cell, 
     .                                            cellmats, cellvf, 2)
                  elseif(i.eq.1) then
                      err = visitmaterialaddclean(handle, cell, mat1)
                  else
                      err = visitmaterialaddclean(handle, cell, mat2)
                  endif
210           continue
200       continue
          m = VISIT_OKAY
      endif
      visitgetmaterial = m
      end
