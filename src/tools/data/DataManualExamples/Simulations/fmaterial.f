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
      err = visitinitializesim("fmaterial", 9,
     . "Demonstrates material data access function", 42,
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
      double precision simtime
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
      double precision simtime
      common /SIMSTATE/ runflag, simcycle, simtime
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
      character*8 cmd, args
      integer     lcmd, largs
      include "visitfortransimV2interface.inc"
ccc   SIMSTATE common block
      integer runflag, simcycle
      double precision simtime
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
      integer runflag, simcycle
      double precision simtime
      common /SIMSTATE/ runflag, simcycle, simtime
      integer md, mmd, mat, cmd, err

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

c     Add a material
          err = visitmdmatalloc(mat)
          if(err.eq.VISIT_OKAY) then
              err = visitmdmatsetname(mat, "mat", 3)
              err = visitmdmatsetmeshname(mat, "mesh2d", 6)
              err = visitmdmataddmaterialname(mat, "Water", 5)
              err = visitmdmataddmaterialname(mat,
     .           "Membrane", 8)
              err = visitmdmataddmaterialname(mat, 
     .           "Air", 3)
              err = visitmdsimaddmaterial(md, mat)
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
      integer function visitgetmesh(domain, name, lname)
      implicit none
      character*8 name
      integer     domain, lname
      include "visitfortransimV2interface.inc" 
ccc   RECTMESH
      integer NX, NY
      parameter (NX = 5)
      parameter (NY = 4)

      real rmx(NX), rmy(NY)
      data rmx/0., 1., 2., 3., 4./
      data rmy/0., 1., 2., 3./
ccc   local variables
      integer h, x, y, err

      h = VISIT_INVALID_HANDLE
      if(visitstrcmp(name, lname, "mesh2d", 6).eq.0) then          
c Create a rectilinear mesh here
          if(visitrectmeshalloc(h).eq.VISIT_OKAY) then
              err = visitvardataalloc(x)
              err = visitvardataalloc(y)
              err = visitvardatasetf(x,VISIT_OWNER_COPY,1,NX,rmx)
              err = visitvardatasetf(y,VISIT_OWNER_COPY,1,NY,rmy)
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
ccc   RECTMESH
      integer NX, NY
      parameter (NX = 5)
      parameter (NY = 4)

c The matlist table indicates the material numbers that are found in
c each cell. Every 3 numbers indicates the material numbers in a cell.
c A material number of 0 means that the material entry is not used.
      integer matlist(3, NX-1, NY-1)
      data matlist/3,0,0,2,3,0,1,2,0,1,0,0,
     .             3,0,0,2,3,0,1,2,0,1,0,0,
     .             3,0,0,2,3,0,1,2,3,1,2,0/
c The mat_vf table indicates the material volume fractions that are
c found in a cell.
      real mat_vf(3, NX-1, NY-1)
      data mat_vf/1.,0.,0.,0.75,0.25,0.,0.8125,0.1875, 0.,1.,0.,0.,
     . 1.,0.,0.,0.625,0.375,0.,0.5625,0.4375,0.,1.,0.,0.,
     . 1.,0.,0.,0.3,0.7,0.,0.2,0.4,0.4,0.55,0.45,0./

c Size the material object so it has the right dimensions (equal to
c the number of cells in the mesh. Unstructured grids would have
c /ncells,1,1/ size for the material. That means that the cellid array
c would have one linear index in its first element.
      integer err, I, J, m, cellid, nmats, h
      integer matno(3), cellmat(3)
      real cellmatvf(3)

      h = VISIT_INVALID_HANDLE
      err = visitmatdataalloc(h)
      err = visitmatdataappendcells(h, (NX-1) * (NY-1))
      err = visitmatdataaddmat(h, "Water", 5, matno(1))
      err = visitmatdataaddmat(h, "Membrane", 8, matno(2))
      err = visitmatdataaddmat(h, "Air", 3, matno(3))

      cellid = 0
      do 2020 J=1,NY-1
          do 2010 I=1,NX-1
              nmats = 0
              do 2000 m=1,3
                  if(matlist(m,I,J).gt.0) then
                      nmats = nmats + 1
                      cellmat(nmats) = matno(matlist(m,I,J))
                      cellmatvf(nmats) = mat_vf(m,I,J)
                  endif
2000          continue
              if(nmats.gt.1) then
                  err = visitmatdataaddmixedcell(h,
     . cellid, cellmat, cellmatvf, nmats)
              else
                  err = visitmatdataaddcleancell(h, 
     . cellid, cellmat)
              endif
              cellid = cellid + 1
2010      continue
2020  continue

      visitgetmaterial = h
      end





























