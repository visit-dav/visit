c Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
c Project developers.  See the top-level LICENSE file for dates and other
c details.  No copyright assignment is required to contribute to VisIt.

c-----------------------------------------------------------------
c Program: main
c
c Programmer: Brad Whitlock
c Date:       Fri Jan 12 14:12:55 PST 2007
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
      err = visitinitializesim("fmesh", 6,
     . "Demonstrates mesh data access function", 38,
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
ccc   SIMSTATE common block
      integer runflag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime
      save /SIMSTATE/
ccc   local variables
      integer visitstate, result, blocking

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
      real rmx(NX), rmy(NY)
      integer rmdims(3), rmndims
      common /RECTMESH/ rmdims, rmndims, rmx, rmy
      save /RECTMESH/
ccc   CURVMESH common block
      integer CNX, CNY, CNZ
      parameter (CNX = 4)
      parameter (CNY = 3)
      parameter (CNZ = 2)
      integer cmdims(3), cmndims
      real cmx(CNX,CNY,CNZ), cmy(CNX,CNY,CNZ), cmz(CNX,CNY,CNZ)
      common /CURVMESH/ cmdims, cmndims, cmx, cmy, cmz
      save /CURVMESH/
c Rectilinear mesh data
      data rmndims /2/
      data rmdims /4, 5, 1/
      data rmx/0., 1., 2.5, 5./
      data rmy/0., 2., 2.25, 2.55,  5./
c Curvilinear mesh data
      data cmx/0.,1.,2.,3., 0.,1.,2.,3., 0.,1.,2.,3.,
     . 0.,1.,2.,3., 0.,1.,2.,3., 0.,1.,2.,3./
      data cmy/0.5,0.,0.,0.5, 1.,1.,1.,1., 1.5,2.,2.,1.5,
     . 0.5,0.,0.,0.5, 1.,1.,1.,1., 1.5,2.,2.,1.5/
      data cmz/0.,0.,0.,0., 0.,0.,0.,0., 0.,0.,0.,0,
     . 1.,1.,1.,1., 1.,1.,1.,1., 1.,1.,1.,1./
      data cmndims /3/
      data cmdims/CNX,CNY,CNZ/

c Simulate one time step
      simcycle = simcycle + 1
      simtime = simtime + 0.0134
      write (6,*) 'Simulating time step: cycle=',simcycle, ' time=', simtime
      call sleep(1)
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
      include "visitfortransimV2interface.inc"
      character*8 cmd, args
      integer     lcmd, largs
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
      integer runflag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime
ccc   local variables
      integer md, m1, m2, cmd, err

      if(visitmdsimalloc(md).eq.VISIT_OKAY) then
          err = visitmdsimsetcycletime(md, simcycle, simtime)
          if(runflag.eq.1) then
              err = visitmdsimsetmode(md, VISIT_SIMMODE_RUNNING)
          else
              err = visitmdsimsetmode(md, VISIT_SIMMODE_STOPPED)
          endif

c Set the first mesh's properties
          if(visitmdmeshalloc(m1).eq.VISIT_OKAY) then
              err = visitmdmeshsetname(m1, "mesh2d", 6)
              err = visitmdmeshsetmeshtype(m1, 
     .            VISIT_MESHTYPE_RECTILINEAR)
              err = visitmdmeshsettopologicaldim(m1, 2)
              err = visitmdmeshsetspatialdim(m1, 2)
              err = visitmdmeshsetxunits(m1, "cm", 2)
              err = visitmdmeshsetyunits(m1, "cm", 2)
              err = visitmdmeshsetxlabel(m1, "Width", 5)
              err = visitmdmeshsetylabel(m1, "Height", 6)
              err = visitmdmeshsetcellorigin(m1, 1)
              err = visitmdmeshsetnodeorigin(m1, 1)

              err = visitmdsimaddmesh(md, m1)
          endif

c Set the second mesh's properties
          if(visitmdmeshalloc(m2).eq.VISIT_OKAY) then
              err = visitmdmeshsetname(m2, "mesh3d", 6)
              err = visitmdmeshsetmeshtype(m2, 
     .            VISIT_MESHTYPE_CURVILINEAR)
              err = visitmdmeshsettopologicaldim(m2, 3)
              err = visitmdmeshsetspatialdim(m2, 3)
              err = visitmdmeshsetxunits(m2, "cm", 2)
              err = visitmdmeshsetyunits(m2, "cm", 2)
              err = visitmdmeshsetzunits(m2, "cm", 2)
              err = visitmdmeshsetxlabel(m2, "Width", 5)
              err = visitmdmeshsetylabel(m2, "Height", 6)
              err = visitmdmeshsetzlabel(m2, "Depth", 5)
              err = visitmdmeshsetcellorigin(m2, 1)
              err = visitmdmeshsetnodeorigin(m2, 1)

              err = visitmdsimaddmesh(md, m2)
          endif

c     Add simulation commands
          if(visitmdcmdalloc(cmd).eq.VISIT_OKAY) then
              err = visitmdcmdsetname(cmd, "halt", 4)
              err = visitmdsimaddgenericcommand(md, cmd)
          endif
          if(visitmdcmdalloc(cmd).eq.VISIT_OKAY) then
              err = visitmdcmdsetname(cmd, "step", 4)
              err = visitmdsimaddgenericcommand(md, cmd)
          endif
          if(visitmdcmdalloc(cmd).eq.VISIT_OKAY) then
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
ccc   RECTMESH common block
      integer NX, NY
      parameter (NX = 4)
      parameter (NY = 5)
      real rmx(NX), rmy(NY)
      integer rmdims(3), rmndims
      common /RECTMESH/ rmdims, rmndims, rmx, rmy
ccc   CURVMESH common block
      integer CNX, CNY, CNZ
      parameter (CNX = 4)
      parameter (CNY = 3)
      parameter (CNZ = 2)
      integer cmdims(3), cmndims
      real cmx(CNX,CNY,CNZ), cmy(CNX,CNY,CNZ), cmz(CNX,CNY,CNZ)
      common /CURVMESH/ cmdims, cmndims, cmx, cmy, cmz
ccc   local variables
      integer h, x, y, z, nnodes, err

      h = VISIT_INVALID_HANDLE
      if(visitstrcmp(name, lname, "mesh2d", 6).eq.0) then
          if(visitrectmeshalloc(h).eq.VISIT_OKAY) then
              err = visitvardataalloc(x)
              err = visitvardataalloc(y)
              err = visitvardatasetf(x,VISIT_OWNER_SIM,1,NX,rmx)
              err = visitvardatasetf(y,VISIT_OWNER_SIM,1,NY,rmy)

              err = visitrectmeshsetcoordsxy(h, x, y)
          endif
      elseif(visitstrcmp(name, lname, "mesh3d", 6).eq.0) then
          if(visitcurvmeshalloc(h).eq.VISIT_OKAY) then
              err = visitvardataalloc(x)
              err = visitvardataalloc(y)
              err = visitvardataalloc(z)
              nnodes = CNX * CNY * CNZ
              err = visitvardatasetf(x,VISIT_OWNER_SIM,1,nnodes,cmx)
              err = visitvardatasetf(y,VISIT_OWNER_SIM,1,nnodes,cmy)
              err = visitvardatasetf(z,VISIT_OWNER_SIM,1,nnodes,cmz)

              err = visitcurvmeshsetcoordsxyz(h, cmdims, x, y, z)
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
