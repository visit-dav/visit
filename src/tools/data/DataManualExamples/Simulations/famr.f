c Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
c Project developers.  See the top-level LICENSE file for dates and other
c details.  No copyright assignment is required to contribute to VisIt.

c-----------------------------------------------------------------
c Program: main
c
c Programmer: Brad Whitlock
c Date:       Tue Jun  8 12:46:15 PDT 2010
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
      err = visitinitializesim("famr", 4,
     . "Demonstrates domain nesting data access function", 48,
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
      integer runflag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime
      save /SIMSTATE/

c     main loop
      runflag = 1
      simcycle = 0
      simtime = 0.
      do 10
c          if(runflag.eq.1) then
              blocking = 0 
c          else
c              blocking = 1
c          endif

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
ccc   AMRMESH common block
      integer NPATCHES
      parameter (NPATCHES = 5)
      real rmx(2, NPATCHES), rmy(2, NPATCHES)
      integer rmxext(2, NPATCHES), rmyext(2, NPATCHES)
      integer level(NPATCHES), ncpatch(NPATCHES)
      integer cpatch(2, NPATCHES)
      common /AMRMESH/ rmx,rmy,rmxext,rmyext,level,ncpatch,cpatch
      save /AMRMESH/
ccc   AMRMESH data
      data rmx/0., 10., 3., 10., 5., 7.5, 7.5, 10., 6., 8.75/
      data rmy/0., 10., 1., 9., 2., 7., 2., 7., 3., 4.75/
      data rmxext/0,9, 6,19, 20,29, 30,39, 48,69/
      data rmyext/0,9,  2,17,  8,27,  8,27, 24,37/
      data level/0, 1, 2, 2, 3/
      data ncpatch/1,2,1,1,0/
      data cpatch/1,-1,2,3,4,-1,4,-1,-1,-1/
c locals
      integer err

c Simulate one time step
      simcycle = simcycle + 1
      simtime = simtime + 0.0134
      write (6,*) 'Simulating time step: cycle=',simcycle, 
     .' time=', simtime
      err = visittimestepchanged()
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
      character*8 cmd, args
      integer     lcmd, largs
      include "visitfortransimV2interface.inc"
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
ccc   AMRMESH common block
      integer NPATCHES
      parameter (NPATCHES = 5)
      real rmx(2, NPATCHES), rmy(2, NPATCHES)
      integer rmxext(2, NPATCHES), rmyext(2, NPATCHES)
      integer level(NPATCHES), ncpatch(NPATCHES)
      integer cpatch(2, NPATCHES)
      common /AMRMESH/ rmx,rmy,rmxext,rmyext,level,ncpatch,cpatch
ccc   local vars
      integer i, md, mmd, vmd, cmd, emd, err

      if(visitmdsimalloc(md).eq.VISIT_OKAY) then
          err = visitmdsimsetcycletime(md, simcycle, simtime)
          if(runflag.eq.1) then
              err = visitmdsimsetmode(md, VISIT_SIMMODE_RUNNING)
          else
              err = visitmdsimsetmode(md, VISIT_SIMMODE_STOPPED)
          endif

c     Add a 2D AMR mesh
          if(visitmdmeshalloc(mmd).eq.VISIT_OKAY) then
              err = visitmdmeshsetname(mmd, "amr", 3)
              err = visitmdmeshsetmeshtype(mmd, 
     .            VISIT_MESHTYPE_AMR)
              err = visitmdmeshsettopologicaldim(mmd, 2)
              err = visitmdmeshsetspatialdim(mmd, 2)
              err = visitmdmeshsetnumdomains(mmd, NPATCHES)
              err = visitmdmeshsetdomaintitle(mmd, "patches", 7)
              err = visitmdmeshsetdomainpiecename(mmd, "patch", 5)
              err = visitmdmeshsetnumgroups(mmd, 4)
              err = visitmdmeshsetgrouptitle(mmd, "levels", 6)
              err = visitmdmeshsetgrouppiecename(mmd, "level", 5)
              do 1400 i=1,NPATCHES
                  err = visitmdmeshsetgroupids(mmd, level(i))
1400          continue
              err = visitmdmeshsetxunits(mmd, "cm", 2)
              err = visitmdmeshsetyunits(mmd, "cm", 2)
              err = visitmdmeshsetxlabel(mmd, "Width", 5)
              err = visitmdmeshsetylabel(mmd, "Height", 6)
              err = visitmdmeshsetcellorigin(mmd, 1)
              err = visitmdmeshsetnodeorigin(mmd, 1)
              err = visitmdsimaddmesh(md, mmd)
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
      endif
      visitgetmetadata = md
      end

c---------------------------------------------------------------------------
c visitgetmesh
c---------------------------------------------------------------------------
      integer function visitgetmesh(dom, name, lname)
      implicit none
      character*8 name
      integer     dom, lname
      include "visitfortransimV2interface.inc" 
ccc   AMRMESH common block
      integer NPATCHES
      parameter (NPATCHES = 5)
      real rmx(2, NPATCHES), rmy(2, NPATCHES)
      integer rmxext(2, NPATCHES), rmyext(2, NPATCHES)
      integer level(NPATCHES), ncpatch(NPATCHES)
      integer cpatch(2, NPATCHES)
      common /AMRMESH/ rmx,rmy,rmxext,rmyext,level,ncpatch,cpatch
ccc   local variables
      real    cx(100), cy(100), t
      integer h, domain, err, i, nx, ny, x, y

      domain = dom + 1
      h = VISIT_INVALID_HANDLE
      if(visitstrcmp(name, lname, "amr", 3).eq.0) then
          if(visitrectmeshalloc(h).eq.VISIT_OKAY) then
ccc   Create the X coordinates
              nx = rmxext(2,domain)-rmxext(1,domain)+1+1
              write (6,*) "nx=", nx
              do 1500 i = 1,nx
                  t = float(i-1) / float(nx-1)
                  cx(i) = (1.-t)*rmx(1,domain) + t*rmx(2,domain)
1500          continue
ccc   Create the Y coordinates
              ny = rmyext(2,domain)-rmyext(1,domain)+1+1
              write (6,*) "ny=", ny
              do 1600 i = 1,ny
                  t = float(i-1) / float(ny-1)
                  cy(i) = (1.-t)*rmy(1,domain) + t*rmy(2,domain)
1600          continue
              err = visitvardataalloc(x)
              err = visitvardataalloc(y)
              err = visitvardatasetf(x,VISIT_OWNER_COPY,1,nx,cx)
              err = visitvardatasetf(y,VISIT_OWNER_COPY,1,ny,cy)

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
      integer     handle, lname
      include "visitfortransimV2interface.inc"
      visitgetcurve = VISIT_INVALID_HANDLE
      end

c---------------------------------------------------------------------------
c visitgetdomainlist
c---------------------------------------------------------------------------
      integer function visitgetdomainlist()
      implicit none
      integer handle
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
ccc   AMRMESH common block
      integer NPATCHES
      parameter (NPATCHES = 5)
      real rmx(2, NPATCHES), rmy(2, NPATCHES)
      integer rmxext(2, NPATCHES), rmyext(2, NPATCHES)
      integer level(NPATCHES), ncpatch(NPATCHES)
      integer cpatch(2, NPATCHES)
      common /AMRMESH/ rmx,rmy,rmxext,rmyext,level,ncpatch,cpatch
ccc local vars
      integer XMIN,YMIN,ZMIN,XMAX,YMAX,ZMAX
      parameter (XMIN = 1)
      parameter (YMIN = 2)
      parameter (ZMIN = 3)
      parameter (XMAX = 4)
      parameter (YMAX = 5)
      parameter (ZMAX = 6)
      integer h, i, err, dom, ratios(3), ext(6), patch(2)
      data ratios/2,2,1/
      data ext/0,0,0,0,0,0/
      data patch/0,0/

      if(visitdnestingalloc(h).eq.VISIT_OKAY) then
          err = visitdnestingsetdimensions(h, NPATCHES, 4, 2)

          err = visitdnestingsetlevelrefinement(h, 0, ratios)
          err = visitdnestingsetlevelrefinement(h, 1, ratios)
          err = visitdnestingsetlevelrefinement(h, 2, ratios)
          err = visitdnestingsetlevelrefinement(h, 3, ratios)

          do 1800 dom = 1,4
              ext(XMIN) = rmxext(1, dom)
              ext(YMIN) = rmyext(1, dom)
              ext(ZMIN) = 0
              ext(XMAX) = rmxext(2, dom)
              ext(YMAX) = rmyext(2, dom)
              ext(ZMAX) = 0
              do 1700 i=1,ncpatch(dom)
                  patch(i) = cpatch(i, dom)
1700          continue
              err = visitdnestingsetnestingforpatch(h,dom-1,level(dom),
     .              patch, ncpatch(dom), ext)
1800      continue
      endif
      visitgetdomainnesting = h
      end

c---------------------------------------------------------------------------
c visitgetmaterial
c---------------------------------------------------------------------------
      integer function visitgetmaterial(domain, name, lname)
      implicit none
      character*8 name
      integer     handle, domain, lname
      include "visitfortransimV2interface.inc"
      visitgetmaterial = VISIT_ERROR
      end
















