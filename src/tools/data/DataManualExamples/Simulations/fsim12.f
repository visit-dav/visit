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
      err = visitinitializesim("fsim12", 6,
     . "Demonstrates creating custom commands", 37,
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
      integer md, m1, m2, vmd, cmd, mat, emd, err

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

c Add a zonal scalar variable on mesh2d. 
          if(visitmdvaralloc(vmd).eq.VISIT_OKAY) then
              err = visitmdvarsetname(vmd, "zonal", 5)
              err = visitmdvarsetmeshname(vmd, "mesh2d", 6)
              err = visitmdvarsettype(vmd, VISIT_VARTYPE_SCALAR)
              err = visitmdvarsetcentering(vmd, VISIT_VARCENTERING_ZONE)

              err = visitmdsimaddvariable(md, vmd)
          endif

c Add a nodal scalar variable on mesh3d. 
          if(visitmdvaralloc(vmd).eq.VISIT_OKAY) then
              err = visitmdvarsetname(vmd, "nodal", 5)
              err = visitmdvarsetmeshname(vmd, "mesh3d", 6)
              err = visitmdvarsettype(vmd, VISIT_VARTYPE_SCALAR)
              err = visitmdvarsetcentering(vmd, VISIT_VARCENTERING_NODE)

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

c     Add a material
          if(visitmdmatalloc(mat).eq.VISIT_OKAY) then
              err = visitmdmatsetname(mat, "mat", 3)
              err = visitmdmatsetmeshname(mat, "mesh2d", 6)
              err = visitmdmataddmaterialname(mat, "Iron", 4)
              err = visitmdmataddmaterialname(mat, "Copper", 6)
              err = visitmdmataddmaterialname(mat, "Nickel", 6)

              err = visitmdsimaddmaterial(md, mat)
          endif

c     Add some expressions
          if(visitmdexpralloc(emd).eq.VISIT_OKAY) then
              err = visitmdexprsetname(emd, "zvec", 4)
              err = visitmdexprsetdefinition(emd, "{zonal, zonal}", 14)
              err = visitmdexprsettype(emd, VISIT_VARTYPE_VECTOR)

              err = visitmdsimaddexpression(md, emd)
          endif
          if(visitmdexpralloc(emd).eq.VISIT_OKAY) then
              err = visitmdexprsetname(emd, "nid", 3)
              err = visitmdexprsetdefinition(emd, "nodeid(mesh3d)", 14)
              err = visitmdexprsettype(emd, VISIT_VARTYPE_SCALAR)

              err = visitmdsimaddexpression(md, emd)
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
      visitgetmesh = VISIT_INVALID_HANDLE
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
