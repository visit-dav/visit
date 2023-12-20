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
ccc   SIMSTATE common block
      integer runflag, simcycle, simSave, simUpdate, simExport
      double precision simtime
      common /SIMSTATE/ simtime,runflag,simcycle,simSave,
     . simUpdate,simExport
      save /SIMSTATE/
ccc   local variables
      integer ierr, batch
      integer i, N, len
      character (len=80) str

      batch = 0
      simSave = 0
      simExport = 0

      ierr = visitopentracefile("trace.txt", 9)

ccc   Handle command line arguments
      N = iargc()
      i = 1
      len = 80
5     if (i.le.N) then
          call getarg(i, str)
          if(str.eq."-dir") then
              call getarg(i+1, str)
              ierr = visitsetdirectory(str, len)
              i = i + 1
          elseif(str.eq."-options") then
              call getarg(i+1, str)
              ierr = visitsetoptions(str, len)
              i = i + 1
          elseif(str.eq."-save") then
              simSave = 1
          elseif(str.eq."-export") then
              simExport = 1
          elseif(str.eq."-batch") then
              batch = 1
          endif
          i = i + 1
          goto 5
      endif

      ierr = visitsetupenv()

      if(batch.eq.1) then
          call mainloop_batch()
      else
          ierr = visitinitializesim("fupdateplots", 12,
     .        "Demonstrates visitupdateplots function", 38,
     .        "/no/useful/path", 15,
     .        VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN,
     .        VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN,
     .        VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN)
          call mainloop(simSave, simExport)
      endif
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
      integer runflag, simcycle, simSave, simUpdate, simExport
      double precision simtime
      common /SIMSTATE/ simtime,runflag,simcycle,simSave,
     . simUpdate,simExport

c     main loop
      runflag = 1
      simcycle = 0
      simtime = 0.
      simUpdate = 0
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

      subroutine mainloop_batch()
      implicit none
      include "visitfortransimV2interface.inc"
ccc   Local vars
      integer ierr
ccc   SIMSTATE common block
      integer runflag, simcycle, simSave, simUpdate, simExport
      double precision simtime
      common /SIMSTATE/ simtime,runflag,simcycle,simSave,
     . simUpdate,simExport

      simUpdate = 1

      ierr = visitinitializeruntime()
      ierr = visittimestepchanged()
      ierr = visitaddplot("Mesh", 4, "mesh2d", 6)
      ierr = visitaddplot("Contour", 7, "zonal", 5)
      ierr = visitaddplot("Pseudocolor", 11, "zonal", 5)
      ierr = visitsetplotoptionss("colorTableName",14,"calewhite",9)
      ierr = visitaddoperator("Elevate", 7, 1)
      ierr = visitsetoperatoroptionss("variable", 8, "zonal",5)
      ierr = visitdrawplots()

      do 20
          call simulate_one_timestep()
20    continue
      end

      subroutine simulate_one_timestep()
      implicit none
      include "visitfortransimV2interface.inc" 
ccc   SIMSTATE common block
      integer runflag, simcycle, simSave, simUpdate, simExport
      double precision simtime
      common /SIMSTATE/ simtime,runflag,simcycle,simSave,
     . simUpdate,simExport
ccc   RECTMESH common block
      integer NX, NY
      parameter (NX = 50)
      parameter (NY = 50)
      real rmx(NX), rmy(NY), zonal(NX-1,NY-1), nodeid(NX,NY)
      integer rmdims(3), rmndims
      common /RECTMESH/ rmdims, rmndims, rmx, rmy, zonal, nodeid
      save /RECTMESH/
c Rectilinear mesh data
      data rmndims /2/
      data rmdims /NX, NY, 1/
ccc   Local vars
      integer err, vars, options
      character (len=80) fn

c Simulate one time step
      simcycle = simcycle + 1
      simtime = simtime + 3.14159 / 10.
      write (6,*) 'Simulating time step: cycle=',simcycle, 
     .            ' time=', simtime

      if(simUpdate.eq.1) then
c         Tell VisIt that the timestep changed
          err = visittimestepchanged()
c         Tell VisIt to update its plots
          err = visitupdateplots()

          if(simSave.eq.1) then
              write (fn, "(A11,I4.4,A4)"),"updateplots",simcycle,".jpg"
              err=visitsavewindow(fn,19,800,800,VISIT_IMAGEFORMAT_JPEG)
              if(err.eq.VISIT_OKAY) then
                  write (6,*) 'Saved ', fn
              endif
          endif

          if(simExport.eq.1) then
              err = visitnamelistalloc(vars)
              err = visitnamelistaddname(vars, "default", 7)
              err = visitnamelistaddname(vars, "mesh2d/nodeid", 13)

              err = visitoptionlistalloc(options)
              err = visitoptionlistsetvalueb(options,
     .        "Strip mesh name prefix", 22, 1);

              write (fn, "(A18,I4.4)"), "updateplots_export",simcycle
              err=visitexportdatabasewithoptions(fn,22,
     .            "FieldViewXDB_1.0",16,vars,options)
              if(err.eq.VISIT_OKAY) then
                  write (6,*) 'Exported ', fn
              endif
              err = visitnamelistfree(vars)
              err = visitoptionlistfree(options)
          endif
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
      integer     lcmd, largs, ierr
      include "visitfortransimV2interface.inc"
ccc   SIMSTATE common block
      integer runflag, simcycle, simSave, simUpdate, simExport
      double precision simtime
      common /SIMSTATE/ simtime,runflag,simcycle,simSave,
     . simUpdate,simExport

c     Handle the commands that we define in visitgetmetadata.
      if(visitstrcmp(cmd, lcmd, "halt", 4).eq.0) then
          runflag = 0
      elseif(visitstrcmp(cmd, lcmd, "step", 4).eq.0) then
          call simulate_one_timestep()
      elseif(visitstrcmp(cmd, lcmd, "run", 3).eq.0) then
          runflag = 1
      elseif(visitstrcmp(cmd, lcmd, "addplot", 7).eq.0) then
          ierr =visitexecutecommand(
     .    "AddPlot('Pseudocolor', 'zonal')\n", 32);
          ierr = visitexecutecommand("DrawPlots()\n", 12);
      elseif(visitstrcmp(cmd, lcmd, "export", 6).eq.0) then
          simExport = 1
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
      integer runflag, simcycle, simSave, simUpdate, simExport
      double precision simtime
      common /SIMSTATE/ simtime,runflag,simcycle,simSave,
     . simUpdate,simExport
      integer md, mmd, vmd, cmd, err

      if(visitmdsimalloc(md).eq.VISIT_OKAY) then
          err = visitmdsimsetcycletime(md, simcycle, simtime)
          if(runflag.eq.1) then
              err = visitmdsimsetmode(md, VISIT_SIMMODE_RUNNING)
          else
              err = visitmdsimsetmode(md, VISIT_SIMMODE_STOPPED)
          endif

c     Add a 2D rectilinear mesh
          err = visitmdmeshalloc(mmd);
          if(err.eq.VISIT_OKAY) then
              err = visitmdmeshsetname(mmd, "mesh2d", 6)
              err = visitmdmeshsetmeshtype(mmd, 
     .            VISIT_MESHTYPE_RECTILINEAR)
              err = visitmdmeshsettopologicaldim(mmd, 2)
              err = visitmdmeshsetspatialdim(mmd, 2)
              err = visitmdmeshsetnumdomains(mmd, 1)
              err = visitmdmeshsetdomaintitle(mmd, "Domains", 7)
              err = visitmdmeshsetdomainpiecename(mmd, "domain", 6)
              err = visitmdmeshsetxunits(mmd, "cm", 2)
              err = visitmdmeshsetyunits(mmd, "cm", 2)
              err = visitmdmeshsetxlabel(mmd, "Width", 5)
              err = visitmdmeshsetylabel(mmd, "Height", 6)
              err = visitmdmeshsetcellorigin(mmd, 1)
              err = visitmdmeshsetnodeorigin(mmd, 1)
              err = visitmdsimaddmesh(md, mmd)
          endif

c     Add a zonal variable on mesh2d
      if(visitmdvaralloc(vmd).eq.VISIT_OKAY) then
          err = visitmdvarsetname(vmd, "zonal", 5)
          err = visitmdvarsetmeshname(vmd, "mesh2d", 6)
          err = visitmdvarsetcentering(vmd, VISIT_VARCENTERING_ZONE)
          err = visitmdvarsettype(vmd, VISIT_VARTYPE_SCALAR)
          err = visitmdsimaddvariable(md, vmd)
      endif

c     Add a nodal variable on mesh2d
      if(visitmdvaralloc(vmd).eq.VISIT_OKAY) then
          err = visitmdvarsetname(vmd, "mesh2d/nodeid", 13)
          err = visitmdvarsetmeshname(vmd, "mesh2d", 6)
          err = visitmdvarsetcentering(vmd, VISIT_VARCENTERING_ZONE)
          err = visitmdvarsettype(vmd, VISIT_VARTYPE_SCALAR)
          err = visitmdsimaddvariable(md, vmd)
      endif

c     Add a curve variable
      if(visitmdcurvealloc(cmd).eq.VISIT_OKAY) then
          err = visitmdcurvesetname(cmd, "sine", 4)
          err = visitmdcurvesetxlabel(cmd, "angle", 5)
          err = visitmdcurvesetxunits(cmd, "radians", 7)
          err = visitmdcurvesetylabel(cmd, "amplitude", 9)
          err = visitmdsimaddcurve(md, cmd)
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
              err = visitmdcmdsetname(cmd, "addplot", 7)
              err = visitmdsimaddgenericcommand(md, cmd)
          endif
          err = visitmdcmdalloc(cmd)
          if(err.eq.VISIT_OKAY) then
              err = visitmdcmdsetname(cmd, "export", 6)
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
ccc   RECTMESH common block (shared with simulate_one_timestep)
      integer NX, NY
      parameter (NX = 50)
      parameter (NY = 50)
      real rmx(NX), rmy(NY), zonal(NX-1,NY-1), nodeid(NX,NY)
      integer rmdims(3), rmndims
      common /RECTMESH/ rmdims, rmndims, rmx, rmy, zonal, nodeid
ccc   local variables
      integer I, J, h, x, y, err

      h = VISIT_INVALID_HANDLE
      if(visitstrcmp(name, lname, "mesh2d", 6).eq.0) then          
c Create a rectilinear mesh here
          if(visitrectmeshalloc(h).eq.VISIT_OKAY) then
c Create mesh coordinates
              do 300 I=1,NX
                  rmx(I) = (float(I-1)/float(NX-1)) * 5. - 2.5
300           continue
              do 310 I=1,NY
                  rmy(I) = (float(I-1)/float(NY-1)) * 5. - 2.5
310           continue

              err = visitvardataalloc(x)
              err = visitvardataalloc(y)
              err = visitvardatasetf(x,VISIT_OWNER_SIM,1,NX,rmx)
              err = visitvardatasetf(y,VISIT_OWNER_SIM,1,NY,rmy)

              err = visitrectmeshsetcoordsxy(h, x, y)
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
ccc   SIMSTATE common block
      integer runflag, simcycle, simSave, simUpdate, simExport
      double precision simtime
      common /SIMSTATE/ simtime,runflag,simcycle,simSave,
     . simUpdate,simExport
ccc   RECTMESH common block
      integer NX, NY
      parameter (NX = 50)
      parameter (NY = 50)
      real rmx(NX), rmy(NY), zonal(NX-1,NY-1), nodeid(NX,NY)
      integer rmdims(3), rmndims
      common /RECTMESH/ rmdims, rmndims, rmx, rmy, zonal, nodeid
ccc   local vars
      integer h, I, J, index, err
      real angle, xpos, ypos, cellX, cellY, dX, dY, tx, ty

      h = VISIT_INVALID_HANDLE
      if(visitstrcmp(name, lname, "zonal", 5).eq.0) then
c         Calculate a zonal variable that depends on the simulation time.
          angle = simtime
          xpos = 2.5 * cos(angle)
          ypos = 2.5 * sin(angle)
          do 5010 J=1,NY-1
              ty = float(J-1) / float(NY-2)
              cellY = (1.-ty)*(-2.5) + 2.5*ty
              dY = cellY - ypos
              do 5000 I=1,NX-1
                  tx = float(I-1) / float(NX-2)
                  cellX = (1.-tx)*(-2.5) + 2.5*tx
                  dX = cellX - xpos
                  zonal(I,J) = sqrt(dX * dX + dY * dY)
5000          continue
5010      continue

          if(visitvardataalloc(h).eq.VISIT_OKAY) then
              err = visitvardatasetf(h,VISIT_OWNER_SIM,1,
     .            (NX-1)*(NY-1),zonal)
          endif
      elseif(visitstrcmp(name, lname, "mesh2d/nodeid", 13).eq.0) then
          index = 0
          do 5110 J=1,NY
              do 5100 I=1,NX
                  nodeid(I,J) = index
                  index = index + 1
5100          continue
5110      continue
          if(visitvardataalloc(h).eq.VISIT_OKAY) then
              err = visitvardatasetf(h,VISIT_OWNER_SIM,1,
     .              NX*NY,nodeid)
          endif
      endif

      visitgetvariable = h
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
ccc   SIMSTATE common block
      integer runflag, simcycle, simSave, simUpdate, simExport
      double precision simtime
      common /SIMSTATE/ simtime,runflag,simcycle,simSave,
     . simUpdate,simExport
ccc   local vars
      integer i, h, hx, hy, NPTS, err
      parameter   (NPTS = 200)
      real        x(NPTS), y(NPTS), t, angle

      h = VISIT_INVALID_HANDLE
      if(visitstrcmp(name, lname, "sine", 4).eq.0) then
          do 10000 i=1,NPTS
              t = float(i-1) / float(NPTS-1)
              angle = simtime + t * 4. * 3.14159
              x(i) = angle
              y(i) = sin(x(i))
10000     continue

          if(visitcurvedataalloc(h).eq.VISIT_OKAY) then
              err = visitvardataalloc(hx)
              err = visitvardataalloc(hy)
              err = visitvardatasetf(hx, VISIT_OWNER_COPY, 1, NPTS, x)
              err = visitvardatasetf(hy, VISIT_OWNER_COPY, 1, NPTS, y)
              err = visitcurvedatasetcoordsxy(h, hx, hy)
          endif
      endif
      visitgetcurve = h
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





























